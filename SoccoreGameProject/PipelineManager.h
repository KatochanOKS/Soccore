#pragma once
#include <wrl.h>
#include <d3d12.h>

class PipelineManager {
public:
    // Initialize: �ʏ�p/�X�L�j���O�p�̗����p�X��n����
    bool Initialize(
        ID3D12Device* device,
        LPCWSTR vsPath, LPCWSTR psPath,             // ��X�L��
        LPCWSTR skinVSPath = nullptr, LPCWSTR skinPSPath = nullptr // �X�L��
    );
    void Cleanup();

    // �擾�֐�
    ID3D12PipelineState* GetPipelineState(bool skinning) const;
    ID3D12RootSignature* GetRootSignature(bool skinning) const;

private:
    // ��X�L���p
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignatureNormal;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineStateNormal;
    // �X�L���p
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignatureSkin;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineStateSkin;
};
