#include "PipelineManager.h"
#include <d3dcompiler.h>
#include <vector>
#include <fstream>
#include <cassert>
#include "d3dx12.h"

// �t�@�C���ǂݍ��݃��[�e�B���e�B
static std::vector<char> LoadShaderFile(LPCWSTR filename) {
    std::ifstream ifs(filename, std::ios::binary);
    return std::vector<char>((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

// Initialize
bool PipelineManager::Initialize(
    ID3D12Device* device,
    LPCWSTR vsPath, LPCWSTR psPath,
    LPCWSTR skinVSPath, LPCWSTR skinPSPath,
    LPCWSTR uiVSPath, LPCWSTR uiPSPath
) {
    // --- ��X�L���i�ʏ�j ---
    {
        // ���[�g�p�����[�^ SRV(t0), CBV(b0)
        CD3DX12_ROOT_PARAMETER rootParam[2] = {};
        CD3DX12_DESCRIPTOR_RANGE descRange;
        descRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
        rootParam[0].InitAsDescriptorTable(1, &descRange, D3D12_SHADER_VISIBILITY_PIXEL);
        rootParam[1].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);

        CD3DX12_STATIC_SAMPLER_DESC staticSampler(
            0, D3D12_FILTER_MIN_MAG_MIP_LINEAR,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP
        );
        CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
            2, rootParam,
            1, &staticSampler,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
        );
        Microsoft::WRL::ComPtr<ID3DBlob> signature, error;
        HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
        if (FAILED(hr)) {
            if (error) OutputDebugStringA((char*)error->GetBufferPointer());
            Cleanup();
            return false;
        }
        hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignatureNormal));
        if (FAILED(hr)) { Cleanup(); return false; }

        auto vsCode = LoadShaderFile(vsPath);
        auto psCode = LoadShaderFile(psPath);
        // ���_���C�A�E�g
        D3D12_INPUT_ELEMENT_DESC layout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.pRootSignature = m_rootSignatureNormal.Get();
        psoDesc.VS = { vsCode.data(), vsCode.size() };
        psoDesc.PS = { psCode.data(), psCode.size() };
        psoDesc.InputLayout = { layout, _countof(layout) };
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        // --- �����ʃJ�����OOFF�ɐݒ� ---
        psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;  // �� ����1�s�ŗ��ʂ��`��I
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        psoDesc.SampleDesc.Count = 1;

        hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineStateNormal));
        if (FAILED(hr)) { Cleanup(); return false; }
    }

    // --- �X�L���i�X�L�j���O�j�p ---
    if (skinVSPath && skinPSPath) {
        // ���[�g�p�����[�^ SRV(t0), CBV(b0), CBV(b1) (b1=�{�[���z��)
        CD3DX12_ROOT_PARAMETER rootParam[3] = {};
        CD3DX12_DESCRIPTOR_RANGE descRange;
        descRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
        rootParam[0].InitAsDescriptorTable(1, &descRange, D3D12_SHADER_VISIBILITY_PIXEL);
        rootParam[1].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL); // ���[���h�s��
        rootParam[2].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL); // �{�[���z��

        CD3DX12_STATIC_SAMPLER_DESC staticSampler(
            0, D3D12_FILTER_MIN_MAG_MIP_LINEAR,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP
        );
        CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
            3, rootParam,
            1, &staticSampler,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
        );
        Microsoft::WRL::ComPtr<ID3DBlob> signature, error;
        HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
        if (FAILED(hr)) {
            if (error) OutputDebugStringA((char*)error->GetBufferPointer());
            Cleanup();
            return false;
        }
        hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignatureSkin));
        if (FAILED(hr)) { Cleanup(); return false; }

        auto vsCode = LoadShaderFile(skinVSPath);
        auto psCode = LoadShaderFile(skinPSPath);
        // �X�L���p���C�A�E�g�i�{�[�����܂ށj
        D3D12_INPUT_ELEMENT_DESC layout[] = {
            { "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "BLENDWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.pRootSignature = m_rootSignatureSkin.Get();
        psoDesc.VS = { vsCode.data(), vsCode.size() };
        psoDesc.PS = { psCode.data(), psCode.size() };
        psoDesc.InputLayout = { layout, _countof(layout) };
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        // --- �����ʃJ�����OOFF�ɐݒ� ---
        psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        psoDesc.SampleDesc.Count = 1;

        hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineStateSkin));
        if (FAILED(hr)) { Cleanup(); return false; }
    }

    // === UI��p�p�C�v���C�� ===
    {
        // SRV(t0), CBV(b0)
        CD3DX12_ROOT_PARAMETER rootParam[2] = {};
        CD3DX12_DESCRIPTOR_RANGE descRange;
        descRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
        rootParam[0].InitAsDescriptorTable(1, &descRange, D3D12_SHADER_VISIBILITY_PIXEL);
        rootParam[1].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);

        CD3DX12_STATIC_SAMPLER_DESC staticSampler(
            0, D3D12_FILTER_MIN_MAG_MIP_LINEAR,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP
        );
        CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
            2, rootParam,
            1, &staticSampler,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
        );
        Microsoft::WRL::ComPtr<ID3DBlob> signature, error;
        HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
        if (FAILED(hr)) return false;
        hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignatureUI));
        if (FAILED(hr)) return false;

        // �V�F�[�_�ǂݍ���
        auto vsCode = LoadShaderFile(uiVSPath);
        auto psCode = LoadShaderFile(uiPSPath);
        D3D12_INPUT_ELEMENT_DESC layout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.pRootSignature = m_rootSignatureUI.Get();
        psoDesc.VS = { vsCode.data(), vsCode.size() };
        psoDesc.PS = { psCode.data(), psCode.size() };
        psoDesc.InputLayout = { layout, _countof(layout) };
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        // --- �����ʃJ�����OOFF�ɐݒ� ---
        psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        // --- ���A���t�@�u�����h�L�� ---
        D3D12_BLEND_DESC blendDesc = {};
        blendDesc.AlphaToCoverageEnable = FALSE;
        blendDesc.IndependentBlendEnable = FALSE;
        auto& rtBlend = blendDesc.RenderTarget[0];
        rtBlend.BlendEnable = TRUE;
        rtBlend.LogicOpEnable = FALSE;
        rtBlend.SrcBlend = D3D12_BLEND_SRC_ALPHA;
        rtBlend.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        rtBlend.BlendOp = D3D12_BLEND_OP_ADD;
        rtBlend.SrcBlendAlpha = D3D12_BLEND_ONE;
        rtBlend.DestBlendAlpha = D3D12_BLEND_ZERO;
        rtBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
        rtBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        psoDesc.BlendState = blendDesc;
        // ----------------------------

        psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        psoDesc.SampleDesc.Count = 1;
        hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineStateUI));
        if (FAILED(hr)) return false;
    }

    return true;
}

void PipelineManager::Cleanup() {
    m_rootSignatureNormal.Reset();
    m_pipelineStateNormal.Reset();
    m_rootSignatureSkin.Reset();
    m_pipelineStateSkin.Reset();
}

ID3D12PipelineState* PipelineManager::GetPipelineState(bool skinning) const {
    return skinning ? m_pipelineStateSkin.Get() : m_pipelineStateNormal.Get();
}
ID3D12RootSignature* PipelineManager::GetRootSignature(bool skinning) const {
    return skinning ? m_rootSignatureSkin.Get() : m_rootSignatureNormal.Get();
}
