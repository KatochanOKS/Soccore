#pragma once
#include <vector>
#include <d3d12.h>
#include <wrl.h>

struct Vertex { // Ç∑Ç≈Ç…íËã`ÇµÇΩèÍçáÇÕïsóv
    float x, y, z;
    float u, v;
};

class BufferManager {
public:
    void CreateVertexBuffer(ID3D12Device* device, const std::vector<Vertex>& vertices);
    void CreateIndexBuffer(ID3D12Device* device, const std::vector<uint16_t>& indices);

    D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const { return m_vbv; }
    D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const { return m_ibv; }
    void CreateConstantBuffer(ID3D12Device* device, size_t size);
    D3D12_GPU_VIRTUAL_ADDRESS GetConstantBufferGPUAddress() const;
private:
    Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vbv{};
    D3D12_INDEX_BUFFER_VIEW m_ibv{};
    Microsoft::WRL::ComPtr<ID3D12Resource> m_constantBuffer;
    D3D12_GPU_VIRTUAL_ADDRESS m_cbGpuAddress = 0;
};
