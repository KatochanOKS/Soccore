#pragma once
#include <wrl.h>
#include <d3d12.h>

class PipelineManager {
public:
    // Initialize: 通常用/スキニング用の両方パスを渡せる
    bool Initialize(
        ID3D12Device* device,
        LPCWSTR vsPath, LPCWSTR psPath,             // 非スキン
        LPCWSTR skinVSPath = nullptr, LPCWSTR skinPSPath = nullptr // スキン
    );
    void Cleanup();

    // 取得関数
    ID3D12PipelineState* GetPipelineState(bool skinning) const;
    ID3D12RootSignature* GetRootSignature(bool skinning) const;

private:
    // 非スキン用
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignatureNormal;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineStateNormal;
    // スキン用
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignatureSkin;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineStateSkin;
};
