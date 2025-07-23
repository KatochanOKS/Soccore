#pragma once
#include <d3d12.h>
#include <wrl.h>

class PipelineManager {
public:
    bool Initialize(ID3D12Device* device, LPCWSTR vsPath, LPCWSTR psPath);
    void Cleanup();

    ID3D12PipelineState* GetPipelineState(bool skinning) const {
        return m_pipelineState.Get();
    }
    ID3D12RootSignature* GetRootSignature(bool skinning) const {
        return  m_rootSignature.Get();
    }

private:
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

};
