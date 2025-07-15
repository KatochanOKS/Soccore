#pragma once
#include <vector>
#include <d3d12.h>
#include <wrl.h>

struct Vertex {
    float x, y, z;
    float nx, ny, nz;   // ← 法線
    float u, v;
};

// ★ スキニング対応の頂点構造体
struct SkinningVertex {
    float x, y, z;        // 位置
    float nx, ny, nz;     // 法線
    float u, v;           // UV
    uint32_t boneIndices[4] = { 0, 0, 0, 0 }; // 影響ボーン番号（最大4つ。0埋めでOK）
    float boneWeights[4] = { 0, 0, 0, 0 };   // 各ボーンのウェイト（0埋めでOK）
};

class BufferManager {
public:
    void CreateVertexBuffer(ID3D12Device* device, const std::vector<Vertex>& vertices);
    void CreateIndexBuffer(ID3D12Device* device, const std::vector<uint16_t>& indices);

    // --- 新規追加: スキニング頂点用のバッファ生成 ---
    void CreateSkinningVertexBuffer(ID3D12Device* device, const std::vector<SkinningVertex>& vertices);

    ID3D12Resource* GetConstantBuffer() const { return m_constantBuffer.Get(); }
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
