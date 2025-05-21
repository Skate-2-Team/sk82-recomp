#pragma once
#include "../heap.h"
#include <unordered_map>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/error.h>
#include <libavutil/samplefmt.h>
#include <libavutil/intreadwrite.h> // For AV_WL32 etc.
}

namespace XMA
{
    #pragma pack(push, 1)
    struct CONTEXT_DATA {
        // DWORD 0
        uint32_t input_buffer_0_packet_count : 12;  // XMASetInputBuffer0, number of
                                                  // 2KB packets. Max 4095 packets.
                                                  // These packets form a block.
        uint32_t loop_count : 8;                    // +12bit, XMASetLoopData NumLoops
        uint32_t input_buffer_0_valid : 1;          // +20bit, XMAIsInputBuffer0Valid
        uint32_t input_buffer_1_valid : 1;          // +21bit, XMAIsInputBuffer1Valid
        uint32_t output_buffer_block_count : 5;     // +22bit SizeWrite 256byte blocks
        uint32_t output_buffer_write_offset : 5;    // +27bit
                                                  // XMAGetOutputBufferWriteOffset
                                                  // AKA OffsetWrite

        // DWORD 1
        uint32_t input_buffer_1_packet_count : 12;  // XMASetInputBuffer1, number of
                                                  // 2KB packets. Max 4095 packets.
                                                  // These packets form a block.
        uint32_t loop_subframe_start : 2;           // +12bit, XMASetLoopData
        uint32_t loop_subframe_end : 3;             // +14bit, XMASetLoopData
        uint32_t loop_subframe_skip : 3;            // +17bit, XMASetLoopData might be
                                                  // subframe_decode_count
        uint32_t subframe_decode_count : 4;         // +20bit
        uint32_t subframe_skip_count : 3;           // +24bit
        uint32_t sample_rate : 2;                   // +27bit enum of sample rates
        uint32_t is_stereo : 1;                     // +29bit
        uint32_t unk_dword_1_c : 1;                 // +30bit
        uint32_t output_buffer_valid : 1;           // +31bit, XMAIsOutputBufferValid

        // DWORD 2
        uint32_t input_buffer_read_offset : 26;  // XMAGetInputBufferReadOffset
        uint32_t unk_dword_2 : 6;                // ErrorStatus/ErrorSet (?)

        // DWORD 3
        uint32_t loop_start : 26;  // XMASetLoopData LoopStartOffset
                                 // frame offset in bits
        uint32_t unk_dword_3 : 6;  // ? ParserErrorStatus/ParserErrorSet(?)

        // DWORD 4
        uint32_t loop_end : 26;        // XMASetLoopData LoopEndOffset
                                     // frame offset in bits
        uint32_t packet_metadata : 5;  // XMAGetPacketMetadata
        uint32_t current_buffer : 1;   // ?

        // DWORD 5
        uint32_t input_buffer_0_ptr;  // physical address
        // DWORD 6
        uint32_t input_buffer_1_ptr;  // physical address
        // DWORD 7
        uint32_t output_buffer_ptr;  // physical address
        // DWORD 8
        uint32_t work_buffer_ptr;  // PtrOverlapAdd(?)

        // DWORD 9
        // +0bit, XMAGetOutputBufferReadOffset AKA WriteBufferOffsetRead
        uint32_t output_buffer_read_offset : 5;
        uint32_t : 25;
        uint32_t stop_when_done : 1;       // +30bit
        uint32_t interrupt_when_done : 1;  // +31bit

        // DWORD 10-15
        uint32_t unk_dwords_10_15[6];  // reserved?

        AVCodecContext* codec_ctx = nullptr;
        AVFrame* frame = nullptr;
        AVPacket* packet = nullptr;
        bool using_fallback;
        bool enabled = false;
    };

    struct LOOP_DATA {
        uint32_t loop_start;
        uint32_t loop_end;
        uint8_t loop_count;
        uint8_t loop_subframe_end;
        uint8_t loop_subframe_skip;
    };

    struct CONTEXT_INIT {
        uint32_t input_buffer_0_ptr;
        uint32_t input_buffer_0_packet_count;
        uint32_t input_buffer_1_ptr;
        uint32_t input_buffer_1_packet_count;
        uint32_t input_buffer_read_offset;
        uint32_t output_buffer_ptr;
        uint32_t output_buffer_block_count;
        uint32_t work_buffer;
        uint32_t subframe_decode_count;
        uint32_t channel_count;
        uint32_t sample_rate;
        LOOP_DATA loop_data;
    };
    #pragma pack(pop)

    // Replace the unordered_map with a simple vector or array
    inline std::vector<CONTEXT_DATA*> g_contexts;
    inline std::mutex g_mutex;

    inline uint32_t AllocateCtxHandle() {
        std::lock_guard lock(g_mutex);
        CONTEXT_DATA* ptr = reinterpret_cast<CONTEXT_DATA*>(g_heap->Alloc(sizeof(CONTEXT_DATA)));
        memset(ptr, 0, sizeof(CONTEXT_DATA));

        g_contexts.push_back(ptr);
        uint32_t handle = static_cast<uint32_t>(g_contexts.size());
        Log::Info("AllocateCtxHandle", "Allocated handle:", handle, ", ctx=" ,ptr);
        return handle;
    }

    inline CONTEXT_DATA* GetCtxPtr(uint32_t handle) {
        uint32_t idx = std::byteswap(handle);
        if (idx == 0 || idx > g_contexts.size()) {
            return nullptr;
        }
        std::lock_guard lock(g_mutex);
        CONTEXT_DATA* ctx = g_contexts[idx-1];
        return ctx;
    }

}