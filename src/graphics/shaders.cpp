#include "shaders.h"

/*
This class handles all shader hooks and shader related functionality.
*/

namespace Shaders
{

    static std::string UPDBNameToHostShader(std::string filePath, XenosShaderType type)
    {
        // parse as file path
        std::filesystem::path path(filePath);
        std::string newFileName = path.filename().string();

        // remove the last 4 characters from the path
        newFileName = newFileName.erase(newFileName.size() - 5) + (type == XPIXEL_SHADER ? ".vpo" : ".fpo");

        return "shaders//" + newFileName;
    }

    bool CompileShaderFromFile(LPCWSTR szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob **ppBlobOut)
    {
        ID3DBlob *pErrorBlob = nullptr;
        HRESULT hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 0, 0, ppBlobOut, &pErrorBlob);

        if (FAILED(hr))
        {
            if (pErrorBlob)
            {
                Log::Error("ShaderComp", "Error compiling shader: ", (char *)pErrorBlob->GetBufferPointer());
                pErrorBlob->Release();
            }

            return false;
        }
        else
        {
            Log::Info("ShaderComp", "Shader compiled successfully.");

            // get the file name without extension
            std::string fileName = std::filesystem::path(szFileName).filename().string();
            fileName = fileName.substr(0, fileName.find_last_of('.'));

            // merge filename with entry point to create a unique name for the shader
            std::string shaderName = "shaders//" + fileName + "_" + szEntryPoint;

            // .vpo if its a vertex shader, .fpo if its a pixel shader
            if (strcmp(szShaderModel, "vs_3_0") == 0)
                shaderName += ".vpo";
            else if (strcmp(szShaderModel, "ps_3_0") == 0)
                shaderName += ".fpo";

            std::ofstream shaderFile(shaderName, std::ios::binary);
            if (shaderFile.is_open())
            {
                shaderFile.write((const char *)(*ppBlobOut)->GetBufferPointer(), (*ppBlobOut)->GetBufferSize());
                shaderFile.close();
                Log::Info("ShaderComp", "Shader saved to shader.bin.");
            }
            else
            {
                Log::Error("ShaderComp", "Failed to open shader file for writing: ", shaderName);
            }
        }

        if (pErrorBlob)
            pErrorBlob->Release();

        return true;
    }

    void PrecompileShaders()
    {
        HRESULT hr = S_OK;

        ID3DBlob *pVSBlob = nullptr;
        if (CompileShaderFromFile(L"shaders//vp6.fx", "defaultVS", "vs_3_0", &pVSBlob))
        {
            g_shaderErrorCount += g_video->m_d3dDevice->CreateVertexShader((const DWORD *)pVSBlob->GetBufferPointer(), &g_pVertexShader);
            pVSBlob->Release();
        }

        ID3DBlob *pPSBlob = nullptr;
        if (CompileShaderFromFile(L"shaders//vp6.fx", "defaultPS", "ps_3_0", &pPSBlob))
        {
            g_shaderErrorCount += g_video->m_d3dDevice->CreatePixelShader((const DWORD *)pPSBlob->GetBufferPointer(), &g_pPixelShader);
            pPSBlob->Release();
        }

        g_isShaderLoaded = (g_shaderErrorCount == 0);

        // This is dangerous under any circumstances that SetPixelShader somehow gets called before this, just for debug atm.
        StateShadow::curPixelProgramBuffer = Memory::Translate<be<uint32_t> *>(0x82DF33F8);
    }

    void D3DDevice_SetVertexShader(void *pDevice, void *pShader)
    {
    }

    void StateShadow_SetPixelProgram(renderengine::ProgramBuffer *pProg)
    {
        if (Memory::MapVirtual(pProg) != StateShadow::curPixelProgramBuffer->get())
        {
            // Log::Info("StateShadow_SetPixelProgram", "Setting pixel shader: ", pProg->m_shaderName);

            if (pProg)
            {
                g_video->m_d3dDevice->SetPixelShader(pProg->m_pixelShader);
            }

            StateShadow::curPixelProgramBuffer->set(Memory::MapVirtual(pProg));
        }
    }

    PPC_FUNC_IMPL(__imp__sub_829FE9A0);
    renderengine::ProgramBuffer *ProgramBuffer_Initialize(void *resource, const renderengine::ProgBuffer_Parameters *params)
    {
        // we can figure out what shader it is by looking at the updb file name entry in the bytecode
        // and processing the filename
        // shaders that are loaded via files, the params->size == 4, otherwise its embedded bytecode and will have an actual size.

        __imp__sub_829FE9A0(*PPCLocal::g_ppcContext, Memory::g_base);

        if (params->m_size != 4 || params->m_type != renderengine::Type::TYPE_PIXEL)
            return (renderengine::ProgramBuffer *)PPCLocal::g_ppcContext->r3.u32;

        // only support pixel shaders right now.
        XenosShaderHeader *shaderHeader = Memory::Translate<XenosShaderHeader *>(params->m_buffer);
        std::string hostPath = UPDBNameToHostShader(shaderHeader->m_updbPath, shaderHeader->m_type);

        // alloc new prog buffer on guest heap
        auto progBuffer = (renderengine::ProgramBuffer *)g_heap->AllocPhysical(sizeof(renderengine::ProgramBuffer), 128);
        progBuffer->m_type = renderengine::Type::TYPE_PIXEL;
        progBuffer->m_pixelShader = nullptr;

        progBuffer->m_shaderName = new char[hostPath.size() + 1];
        strcpy(progBuffer->m_shaderName, hostPath.c_str());

        std::ifstream shaderFile(hostPath, std::ios::binary);
        if (shaderFile.is_open())
        {
            // read whole file into byte vector
            shaderFile.seekg(0, std::ios::end);
            size_t fileSize = shaderFile.tellg();
            shaderFile.seekg(0, std::ios::beg);

            std::vector<char> shaderData(fileSize);
            shaderFile.read(shaderData.data(), fileSize);
            shaderFile.close();

            ID3DBlob *pVSBlob = nullptr;
            HRESULT hr = g_video->m_d3dDevice->CreatePixelShader((const DWORD *)shaderData.data(), &progBuffer->m_pixelShader);

            if (FAILED(hr))
            {
                Log::Error("ProgramBuffer_Initialize", "Failed to create pixel shader: ", hostPath, " | Result -> ", hr);
                DebugBreak();
            }

            Log::Info("ProgramBuffer_Initialize", "Created pixel shader: ", hostPath, " | Address -> ", progBuffer->m_pixelShader, " Heap Address -> ", progBuffer);
        }
        else
        {
            Log::Error("ProgramBuffer_Initialize", "Trying to load shader that doesn't exist -> ", hostPath);
        }

        return progBuffer;
    }

    int ProgramBuffer_GetVariableHandleByName(void *, void *, renderengine::ProgramVariableHandle *stateHandle)
    {
        // handle is stack allocated

        stateHandle->m_dataType = 0;
        stateHandle->m_programType = 0;
        stateHandle->m_numConstants = 0;
        stateHandle->m_index = 0;

        return 0;
    }

    void Technique_SetParamsFromShader()
    {
        // no-op
        return;
    }
}

// D3D Hooks
GUEST_FUNCTION_HOOK(sub_82364E98, Shaders::D3DDevice_SetVertexShader)
GUEST_FUNCTION_HOOK(sub_82365A88, Shaders::StateShadow_SetPixelProgram)

// Game Hooks
GUEST_FUNCTION_HOOK(sub_829FEB90, Shaders::ProgramBuffer_GetVariableHandleByName)
GUEST_FUNCTION_HOOK(sub_829FE9A0, Shaders::ProgramBuffer_Initialize)
GUEST_FUNCTION_HOOK(sub_82BCE6B0, Shaders::Technique_SetParamsFromShader)
