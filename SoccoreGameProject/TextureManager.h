#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <unordered_map>
#include <vector>

// �����F�A�v���S�̂Ŏg���e�N�X�`�����ꊇ�Ǘ����ASRV�f�B�X�N���v�^�q�[�v�ɕ��ׂĂ����B
// - Initialize() �� SRV�p�f�B�X�N���v�^�q�[�v���쐬
// - LoadTexture() �ŉ摜�t�@�C����ǂݍ��� �� GPU�e�N�X�`��������ē]�� �� SRV�����蓖��
// - GetSRVHeap()/GetSRV() �ŕ`�摤��SRV�n���h����n��
class TextureManager {
public:
    // device: D3D12�f�o�C�X
    // maxTextureNum: SRV�q�[�v�Ɋm�ۂ���ő�e�N�X�`�����i�f�B�X�N���v�^���j
    void Initialize(ID3D12Device* device, UINT maxTextureNum = 64);

    // filename: �ǂݍ��މ摜�iWIC�Ή��Fpng/jpg/bmp�Ȃǁj
    // cmdList : �]���R�}���h�L�^�p�iUpload��Default �ւ̃R�s�[�Ŏg�p�j
    // �߂�l : SRV�q�[�v��̃C���f�b�N�X�i�����[�g�p�����[�^�ɓn���Ƃ��̃I�t�Z�b�g�Ɏg���j
    int LoadTexture(const std::wstring& filename, ID3D12GraphicsCommandList* cmdList);

    // �`��O�� SetDescriptorHeaps �œn�����߂�SRV�q�[�v���|�C���^
    ID3D12DescriptorHeap* GetSRVHeap();

    // SRV�e�[�u���� GPU �n���h�����擾�iindex �� LoadTexture �̖߂�l�j
    D3D12_GPU_DESCRIPTOR_HANDLE GetSRV(int index);

private:
    Microsoft::WRL::ComPtr<ID3D12Device> m_device;                 // �f�o�C�X�iCreate* �p�j
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvHeap;        // SRV�̒u����i�V�F�[�_���j
    std::unordered_map<std::wstring, int> m_textureIndices;        // �����t�@�C���̏d���ǂݍ��݂�h��
    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_textures; // ���́iDefault Heap�j
    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_uploadBuffers; // Upload�ێ��iGPU�]�������S�ɏI���܂ŉ�����Ȃ��j
    UINT m_descriptorSize = 0;                                     // SRV�f�B�X�N���v�^�T�C�Y�i�C���N�������g���j
    UINT m_nextIndex = 0;                                          // ���Ɋ��蓖�Ă�SRV�X���b�g
};
