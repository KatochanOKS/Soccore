#include "PipelineManager.h"
#include <d3dcompiler.h>
#include <vector>
#include <fstream>
#include <cassert>
#include "d3dx12.h"

// --- �V�F�[�_�t�@�C���̃o�C�i���ǂݍ��� ---
static std::vector<char> LoadShaderFile(LPCWSTR filename) {
    std::ifstream ifs(filename, std::ios::binary);
    return std::vector<char>((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

// --- �ʏ탂�f���p�i��X�L���j ---
bool PipelineManager::Initialize(ID3D12Device* device, LPCWSTR vsPath, LPCWSTR psPath) {
    // ���[�g�p�����[�^�F0=SRV(t0), 1=CBV(b0)
    CD3DX12_ROOT_PARAMETER rootParam[2] = {};

    // SRV (t0) : �s�N�Z���V�F�[�_�p
    CD3DX12_DESCRIPTOR_RANGE descRange;
    descRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0
    rootParam[0].InitAsDescriptorTable(1, &descRange, D3D12_SHADER_VISIBILITY_PIXEL);

    // CBV (b0) : ���_�V�F�[�_�p
    rootParam[1].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
    //                               ��������������������������������������������������

    // �T���v���[
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
    hr = device->CreateRootSignature(
        0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)
    );
    if (FAILED(hr)) { Cleanup(); return false; }

    // �V�F�[�_�ǂݍ���
    auto vsCode = LoadShaderFile(vsPath);
    auto psCode = LoadShaderFile(psPath);

    // ���_���C�A�E�g�iPOSITION, NORMAL, TEXCOORD�j
    D3D12_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = { vsCode.data(), vsCode.size() };
    psoDesc.PS = { psCode.data(), psCode.size() };
    psoDesc.InputLayout = { layout, _countof(layout) };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleDesc.Count = 1;

    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
    if (FAILED(hr)) { Cleanup(); return false; }
    return true;
}

// --- �X�L���A�j�����f���p ---
bool PipelineManager::InitializeSkinning(ID3D12Device* device, LPCWSTR vsPath, LPCWSTR psPath) {
    // ���[�g�p�����[�^: 0=CBV(b0/WorldViewProj), 1=CBV(b1/Bone), 2=SRV(t0/�e�N�X�`��)
    CD3DX12_ROOT_PARAMETER1 rootParams[3];
    rootParams[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);
    rootParams[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);
    CD3DX12_DESCRIPTOR_RANGE1 texRange;
    texRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
    rootParams[2].InitAsDescriptorTable(1, &texRange, D3D12_SHADER_VISIBILITY_PIXEL);

    // �T���v���[
    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc;
    rootSigDesc.Init_1_1(_countof(rootParams), rootParams, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    Microsoft::WRL::ComPtr<ID3DBlob> sigBlob, errBlob;
    HRESULT hr = D3DX12SerializeVersionedRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &sigBlob, &errBlob);
    if (FAILED(hr)) {
        if (errBlob) OutputDebugStringA((char*)errBlob->GetBufferPointer());
        Cleanup();
        return false;
    }
    hr = device->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&m_skinRootSignature));
    if (FAILED(hr)) { Cleanup(); return false; }

    // �V�F�[�_�ǂݍ���
    auto vsCode = LoadShaderFile(vsPath);
    auto psCode = LoadShaderFile(psPath);

    // �X�L���p���_���C�A�E�g
    D3D12_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,     0, 0,   D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,     0, 12,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,        0, 24,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT,   0, 32,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, 48,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = m_skinRootSignature.Get();
    psoDesc.VS = { vsCode.data(), vsCode.size() };
    psoDesc.PS = { psCode.data(), psCode.size() };
    psoDesc.InputLayout = { layout, _countof(layout) };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleDesc.Count = 1;

    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_skinPipelineState));
    if (FAILED(hr)) { Cleanup(); return false; }
    return true;
}

void PipelineManager::Cleanup() {
    m_rootSignature.Reset();
    m_pipelineState.Reset();
    m_skinRootSignature.Reset();
    m_skinPipelineState.Reset();
}
