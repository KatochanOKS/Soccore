#pragma once
#include <wrl.h>
#include <d3d12.h>

class PipelineManager {
public:
    // Initialize: 通常用/スキニング用の両方パスを渡せる
    bool Initialize(
        ID3D12Device* device,
        LPCWSTR vsPath, LPCWSTR psPath,
        LPCWSTR skinVSPath = nullptr, LPCWSTR skinPSPath = nullptr,
        LPCWSTR uiVSPath = nullptr, LPCWSTR uiPSPath = nullptr // ←追加部分に「= nullptr」
    );

    void Cleanup();

    // 取得関数
    ID3D12PipelineState* GetPipelineState(bool skinning) const;
    ID3D12RootSignature* GetRootSignature(bool skinning) const;

    ID3D12PipelineState* GetPipelineStateUI() const { return m_pipelineStateUI.Get(); }
    ID3D12RootSignature* GetRootSignatureUI() const { return m_rootSignatureUI.Get(); }

    ID3D12PipelineState* GetPipelineStateSkyDome() const { return m_pipelineStateSkyDome.Get(); }
    ID3D12RootSignature* GetRootSignatureSkyDome() const { return m_rootSignatureSkyDome.Get(); }

private:
    // 非スキン用
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignatureNormal;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineStateNormal;
    // スキン用
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignatureSkin;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineStateSkin;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignatureUI;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineStateUI;

    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineStateSkyDome;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignatureSkyDome;
};
