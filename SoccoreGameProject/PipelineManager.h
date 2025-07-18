#pragma once
#include <d3d12.h>
#include <wrl.h>

class PipelineManager {
public:
    bool Initialize(ID3D12Device* device, LPCWSTR vsPath, LPCWSTR psPath);
    bool InitializeSkinning(ID3D12Device* device, LPCWSTR vsPath, LPCWSTR psPath);       // ���X�L���A�j���p
    void Cleanup();

    ID3D12PipelineState* GetPipelineState(bool skinning) const {
        return skinning ? m_skinPipelineState.Get() : m_pipelineState.Get();
    }
    ID3D12RootSignature* GetRootSignature(bool skinning) const {
        return skinning ? m_skinRootSignature.Get() : m_rootSignature.Get();
    }

private:
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_skinPipelineState;    // ���ǉ�
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_skinRootSignature;    // ���ǉ�
};
