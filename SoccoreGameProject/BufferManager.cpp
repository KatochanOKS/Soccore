#include "BufferManager.h"
#include "d3dx12.h"   
#include <DirectXMath.h>
// ���_�o�b�t�@���쐬����֐�
void BufferManager::CreateVertexBuffer(ID3D12Device* device, const std::vector<Vertex>& vertices)
{
    // �o�b�t�@�T�C�Y���v�Z�i���_�� �~ ���_�T�C�Y�j
    const UINT bufferSize = UINT(vertices.size() * sizeof(Vertex));

    // �A�b�v���[�h�p�q�[�v�v���p�e�B�ƃ��\�[�X�L�q�q���쐬
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    // �o�b�t�@���\�[�X�𐶐�
    device->CreateCommittedResource(
        &heapProps, // �q�[�v�v���p�e�B
        D3D12_HEAP_FLAG_NONE, // �q�[�v�t���O
        &resDesc, // ���\�[�X�L�q�q
        D3D12_RESOURCE_STATE_GENERIC_READ, // ���\�[�X�������
        nullptr, // �N���A�l�s�v
        IID_PPV_ARGS(&m_vertexBuffer) // �o�b�t�@�̃|�C���^
    );

    // �o�b�t�@��CPU��������ԂɃ}�b�s���O�i�������݉\�ɂ���j
    void* mapped = nullptr;
    m_vertexBuffer->Map(0, nullptr, &mapped);

    // �o�b�t�@�ɒ��_�f�[�^����������
    memcpy(mapped, vertices.data(), bufferSize);

    // �}�b�s���O�����iGPU�A�N�Z�X�\��Ԃ֖߂��j
    m_vertexBuffer->Unmap(0, nullptr);

    // ���_�o�b�t�@�r���[�iGPU�ɓn���o�b�t�@���j���Z�b�g
    m_vbv.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vbv.SizeInBytes = bufferSize;
    m_vbv.StrideInBytes = sizeof(Vertex);
}

// �C���f�b�N�X�o�b�t�@���쐬����֐�
void BufferManager::CreateIndexBuffer(ID3D12Device* device, const std::vector<uint16_t>& indices)
{
    // �o�b�t�@�T�C�Y���v�Z�i�C���f�b�N�X�� �~ �C���f�b�N�X�T�C�Y�j
    const UINT bufferSize = UINT(indices.size() * sizeof(uint16_t));

    // �A�b�v���[�h�p�q�[�v�v���p�e�B�ƃ��\�[�X�L�q�q���쐬
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    // �o�b�t�@���\�[�X�𐶐�
    device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_indexBuffer)
    );

    // �o�b�t�@��CPU��������ԂɃ}�b�s���O�i�������݉\�ɂ���j
    void* mapped = nullptr;
    m_indexBuffer->Map(0, nullptr, &mapped);

    // �o�b�t�@�ɃC���f�b�N�X�f�[�^����������
    memcpy(mapped, indices.data(), bufferSize);

    // �}�b�s���O�����iGPU�A�N�Z�X�\��Ԃ֖߂��j
    m_indexBuffer->Unmap(0, nullptr);

    // �C���f�b�N�X�o�b�t�@�r���[�iGPU�ɓn���o�b�t�@���j���Z�b�g
    m_ibv.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    m_ibv.SizeInBytes = bufferSize;
    m_ibv.Format = DXGI_FORMAT_R16_UINT; // 16�r�b�g�C���f�b�N�X
}

// BufferManager.cpp

// �� �X�L�j���O���_�p�o�b�t�@����
void BufferManager::CreateSkinningVertexBuffer(ID3D12Device* device, const std::vector<SkinningVertex>& vertices)
{
    const UINT bufferSize = UINT(vertices.size() * sizeof(SkinningVertex));

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    // �o�b�t�@���\�[�X����
    device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_vertexBuffer)
    );

    // CPU�A�N�Z�X�Ń}�b�v
    void* mapped = nullptr;
    m_vertexBuffer->Map(0, nullptr, &mapped);
    memcpy(mapped, vertices.data(), bufferSize);
    m_vertexBuffer->Unmap(0, nullptr);

    // ���_�o�b�t�@�r���[�̃Z�b�g�iStride���d�v�I�j
    m_vbv.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vbv.SizeInBytes = bufferSize;
    m_vbv.StrideInBytes = sizeof(SkinningVertex);
}


// �萔�o�b�t�@�iCBV: Constant Buffer View�j���쐬����֐�
void BufferManager::CreateConstantBuffer(ID3D12Device* device, size_t size)
{
    // 256�o�C�g���E�ɃA���C���i�萔�o�b�t�@��256�o�C�g�P�ʂŊm�ۂ���K�v����j
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer((size + 255) & ~255);

    // �o�b�t�@���\�[�X�𐶐�
    device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_constantBuffer)
    );

    // GPU�o�b�t�@�̉��z�A�h���X��ێ�
    m_cbGpuAddress = m_constantBuffer->GetGPUVirtualAddress();
}

void BufferManager::CreateBoneConstantBuffer(ID3D12Device* device, size_t boneCount) {
    if (boneCount == 0) {
        m_boneConstantBuffer.Reset();    // ���S��null�N���A
        m_boneCbGpuAddress = 0;
        return;
    }
    size_t size = sizeof(DirectX::XMMATRIX) * boneCount;
    // 256�o�C�g�A���C��
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer((size + 255) & ~255);

    device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_boneConstantBuffer)
    );
    m_boneCbGpuAddress = m_boneConstantBuffer->GetGPUVirtualAddress();
}


D3D12_GPU_VIRTUAL_ADDRESS BufferManager::GetBoneConstantBufferGPUAddress() const {
    return m_boneCbGpuAddress;
}

// �萔�o�b�t�@��GPU�A�h���X���擾����֐�
D3D12_GPU_VIRTUAL_ADDRESS BufferManager::GetConstantBufferGPUAddress() const
{
    return m_cbGpuAddress;
}
