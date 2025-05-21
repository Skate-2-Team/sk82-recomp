#include "hooks.h"
#include "xma.h"

namespace Hooks
{
    HRESULT Import_XMACreateContext(uint32_t* ctxHandle) {
        uint32_t handle = XMA::AllocateCtxHandle();
        *ctxHandle = handle;
        return STATUS_SUCCESS;
    }

    void Import_XMAReleaseContext(uint32_t handle) {
        XMA::CONTEXT_DATA* ctx = XMA::GetCtxPtr(handle);
        if (!ctx) return;
        if (ctx->codec_ctx) avcodec_free_context(&ctx->codec_ctx);
        if (ctx->frame) av_frame_free(&ctx->frame);
        if (ctx->packet) av_packet_free(&ctx->packet);

        g_heap->Free(ctx);
    }

    void Import_XMAEnableContext(uint32_t handle) {
        XMA::CONTEXT_DATA* ctx = XMA::GetCtxPtr(handle);
        if (ctx) ctx->enabled = true;
    }

    HRESULT Import_XMADisableContext(uint32_t handle, bool blockIfBusy) {
        XMA::CONTEXT_DATA* ctx = XMA::GetCtxPtr(handle);
        if (!ctx) return STATUS_INVALID_PARAMETER;
        ctx->enabled = false;
        return STATUS_SUCCESS;
    }

    void Import_XMAInitializeContext(uint32_t handle, XMA::CONTEXT_INIT* initcontext) {
        XMA::CONTEXT_DATA* ctx = XMA::GetCtxPtr(handle);
        if (!ctx) return;
        ctx->input_buffer_0_ptr = initcontext->input_buffer_0_ptr;
        ctx->input_buffer_0_packet_count = initcontext->input_buffer_0_packet_count;
        ctx->input_buffer_1_ptr = initcontext->input_buffer_1_ptr;
        ctx->input_buffer_1_packet_count = initcontext->input_buffer_1_packet_count;
        ctx->input_buffer_read_offset = initcontext->input_buffer_read_offset;
        ctx->output_buffer_ptr = initcontext->output_buffer_ptr;
        ctx->output_buffer_block_count = initcontext->output_buffer_block_count;
        ctx->work_buffer_ptr = initcontext->work_buffer;
        ctx->subframe_decode_count = initcontext->subframe_decode_count;
        ctx->sample_rate = initcontext->sample_rate;
        ctx->loop_start = initcontext->loop_data.loop_start;
        ctx->loop_end = initcontext->loop_data.loop_end;
        ctx->loop_count = initcontext->loop_data.loop_count;
        ctx->loop_subframe_end = initcontext->loop_data.loop_subframe_end;
        ctx->loop_subframe_skip = initcontext->loop_data.loop_subframe_skip;
        ctx->codec_ctx = nullptr;
        ctx->frame = nullptr;
        ctx->packet = nullptr;
        ctx->enabled = false;
    }

    // All other API functions: use handle directly, no byteswapping.
    void Import_XMASetInputBuffer0(uint32_t handle, void* bufPtr, uint32_t len) {
        XMA::CONTEXT_DATA* ctx = XMA::GetCtxPtr(handle);
        if (ctx) {
            ctx->input_buffer_0_ptr = reinterpret_cast<uint32_t>(bufPtr);
            ctx->input_buffer_0_packet_count = len;
        }
    }
    void Import_XMASetInputBuffer0Valid(uint32_t handle) {
        XMA::CONTEXT_DATA* ctx = XMA::GetCtxPtr(handle);
        if (ctx) ctx->input_buffer_0_valid = true;
    }
    bool Import_XMAIsInputBuffer0Valid(uint32_t handle) {
        XMA::CONTEXT_DATA* ctx = XMA::GetCtxPtr(handle);
        return ctx && ctx->input_buffer_0_valid;
    }
    void Import_XMASetInputBufferReadOffset(uint32_t handle, uint32_t offset) {
        XMA::CONTEXT_DATA* ctx = XMA::GetCtxPtr(handle);
        if (ctx) ctx->input_buffer_read_offset = offset;
    }
    // ...Repeat pattern for all other buffer/context functions...
    void Import_XMASetInputBuffer1(uint32_t handle, void* bufPtr, uint32_t len) {
        XMA::CONTEXT_DATA* ctx = XMA::GetCtxPtr(handle);
        if (ctx) {
            ctx->input_buffer_1_ptr = reinterpret_cast<uint32_t>(bufPtr);
            ctx->input_buffer_1_packet_count = len;
        }
    }
    void Import_XMASetInputBuffer1Valid(uint32_t handle) {
        XMA::CONTEXT_DATA* ctx = XMA::GetCtxPtr(handle);
        if (ctx) ctx->input_buffer_1_valid = true;
    }
    bool Import_XMAIsInputBuffer1Valid(uint32_t handle) {
        XMA::CONTEXT_DATA* ctx = XMA::GetCtxPtr(handle);
        return ctx && ctx->input_buffer_1_valid;
    }
    void Import_XMASetOutputBufferValid(uint32_t handle) {
        XMA::CONTEXT_DATA* ctx = XMA::GetCtxPtr(handle);
        if (ctx) ctx->output_buffer_valid = true;
    }
    bool Import_XMAIsOutputBufferValid(uint32_t handle) {
        XMA::CONTEXT_DATA* ctx = XMA::GetCtxPtr(handle);
        return ctx && ctx->output_buffer_valid;
    }
    void Import_XMASetOutputBufferReadOffset(uint32_t handle, uint32_t offset) {
        XMA::CONTEXT_DATA* ctx = XMA::GetCtxPtr(handle);
        if (ctx) ctx->output_buffer_read_offset = offset;
    }
    uint32_t Import_XMAGetOutputBufferReadOffset(uint32_t handle) {
        XMA::CONTEXT_DATA* ctx = XMA::GetCtxPtr(handle);
        return ctx ? ctx->output_buffer_read_offset : 0;
    }
    uint32_t Import_XMAGetOutputBufferWriteOffset(uint32_t handle) {
        XMA::CONTEXT_DATA* ctx = XMA::GetCtxPtr(handle);
        return ctx ? ctx->output_buffer_write_offset : 0;
    }
};

// XMA Functions
GUEST_FUNCTION_HOOK(__imp__XMAEnableContext, Hooks::Import_XMAEnableContext)
GUEST_FUNCTION_HOOK(__imp__XMAGetOutputBufferReadOffset, Hooks::Import_XMAGetOutputBufferReadOffset)
GUEST_FUNCTION_HOOK(__imp__XMAIsInputBuffer1Valid, Hooks::Import_XMAIsInputBuffer1Valid)
GUEST_FUNCTION_HOOK(__imp__XMAIsOutputBufferValid, Hooks::Import_XMAIsOutputBufferValid)
GUEST_FUNCTION_HOOK(__imp__XMASetInputBuffer0, Hooks::Import_XMASetInputBuffer0)
GUEST_FUNCTION_HOOK(__imp__XMASetInputBuffer0Valid, Hooks::Import_XMASetInputBuffer0Valid)
GUEST_FUNCTION_HOOK(__imp__XMASetInputBuffer1Valid, Hooks::Import_XMASetInputBuffer1Valid)
GUEST_FUNCTION_HOOK(__imp__XMASetInputBufferReadOffset, Hooks::Import_XMASetInputBufferReadOffset)
GUEST_FUNCTION_HOOK(__imp__XMASetOutputBufferReadOffset, Hooks::Import_XMASetOutputBufferReadOffset)
GUEST_FUNCTION_HOOK(__imp__XMASetOutputBufferValid, Hooks::Import_XMASetOutputBufferValid)
GUEST_FUNCTION_HOOK(__imp__XMADisableContext, Hooks::Import_XMADisableContext)
GUEST_FUNCTION_HOOK(__imp__XMAGetOutputBufferWriteOffset, Hooks::Import_XMAGetOutputBufferWriteOffset)
GUEST_FUNCTION_HOOK(__imp__XMAInitializeContext, Hooks::Import_XMAInitializeContext)
GUEST_FUNCTION_HOOK(__imp__XMAIsInputBuffer0Valid, Hooks::Import_XMAIsInputBuffer0Valid)
GUEST_FUNCTION_HOOK(__imp__XMASetInputBuffer1, Hooks::Import_XMASetInputBuffer1)
GUEST_FUNCTION_HOOK(__imp__XMAReleaseContext, Hooks::Import_XMAReleaseContext)
GUEST_FUNCTION_HOOK(__imp__XMACreateContext, Hooks::Import_XMACreateContext)