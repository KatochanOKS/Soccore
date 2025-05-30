#include "BufferManager.h"
#include"d3dx12.h"   
void BufferManager::CreateVertexBuffer(ID3D12Device* device, const std::vector<Vertex>& vertices)
{
    const UINT bufferSize = UINT(vertices.size() * sizeof(Vertex));

    // バッファ作成
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_vertexBuffer)
    );

    // バッファにデータ転送
    void* mapped = nullptr;
    m_vertexBuffer->Map(0, nullptr, &mapped);
    memcpy(mapped, vertices.data(), bufferSize);
    m_vertexBuffer->Unmap(0, nullptr);

    // ビュー
    m_vbv.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vbv.SizeInBytes = bufferSize;
    m_vbv.StrideInBytes = sizeof(Vertex);
}

void BufferManager::CreateIndexBuffer(ID3D12Device* device, const std::vector<uint16_t>& indices)
{
    const UINT bufferSize = UINT(indices.size() * sizeof(uint16_t));

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_indexBuffer)
    );

    // バッファにデータ転送
    void* mapped = nullptr;
    m_indexBuffer->Map(0, nullptr, &mapped);
    memcpy(mapped, indices.data(), bufferSize);
    m_indexBuffer->Unmap(0, nullptr);

    // ビュー
    m_ibv.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    m_ibv.SizeInBytes = bufferSize;
    m_ibv.Format = DXGI_FORMAT_R16_UINT;
}

void BufferManager::CreateConstantBuffer(ID3D12Device* device, size_t size)
{
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer((size + 255) & ~255); // 256バイトアライン
    device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_constantBuffer)
    );
    m_cbGpuAddress = m_constantBuffer->GetGPUVirtualAddress();
}

D3D12_GPU_VIRTUAL_ADDRESS BufferManager::GetConstantBufferGPUAddress() const
{
    return m_cbGpuAddress;
}
