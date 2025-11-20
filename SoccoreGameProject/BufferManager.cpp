#include "BufferManager.h"
#include "d3dx12.h"   
#include <DirectXMath.h>
//--------------------------------------------------------------------------------------
// CreateVertexBuffer
// 指定された頂点データからDirectX 12の頂点バッファを作成し、GPUにアップロードする。
// - 頂点数・サイズからバッファを確保
// - アップロード用ヒープを利用しCPUからデータ転送
// - バッファビューをセットし描画時に利用可能にする
//--------------------------------------------------------------------------------------
void BufferManager::CreateVertexBuffer(ID3D12Device* device, const std::vector<Vertex>& vertices)
{
    if (vertices.empty()) return; // 頂点データが空なら何もしない

    const UINT bufferSize = UINT(vertices.size() * sizeof(Vertex)); // バッファサイズ計算

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD); // アップロード用ヒープ
    CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize); // バッファ記述子

    // バッファリソース生成（アップロード用ヒープに確保）
    HRESULT hr = device->CreateCommittedResource(
        &heapProps, // ヒーププロパティ
        D3D12_HEAP_FLAG_NONE, // ヒープフラグ
        &resDesc, // リソース記述子
        D3D12_RESOURCE_STATE_GENERIC_READ, // 初期状態
        nullptr, // クリア値不要
        IID_PPV_ARGS(&m_vertexBuffer) // バッファのポインタ
    );
    if (FAILED(hr)) return; // 失敗時は何もしない

    void* mapped = nullptr;
    hr = m_vertexBuffer->Map(0, nullptr, &mapped); // バッファをCPUメモリ空間にマップ
    if (SUCCEEDED(hr)) {
        memcpy(mapped, vertices.data(), bufferSize); // 頂点データをコピー
        m_vertexBuffer->Unmap(0, nullptr); // マッピング解除
    }

    // 頂点バッファビューをセット
    m_vbv.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress(); // GPU仮想アドレス
    m_vbv.SizeInBytes = bufferSize; // バッファ全体サイズ
    m_vbv.StrideInBytes = sizeof(Vertex); // 1頂点あたりのサイズ
}

void BufferManager::CreateSkinningVertexBuffer(ID3D12Device* device, const std::vector<SkinningVertex>& vertices) {
    const UINT bufferSize = UINT(vertices.size() * sizeof(SkinningVertex));
   
    // アップロード用ヒーププロパティとリソース記述子を作成
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    // バッファリソースを生成
    device->CreateCommittedResource(
        &heapProps, // ヒーププロパティ
        D3D12_HEAP_FLAG_NONE, // ヒープフラグ
        &resDesc, // リソース記述子
        D3D12_RESOURCE_STATE_GENERIC_READ, // リソース初期状態
        nullptr, // クリア値不要
        IID_PPV_ARGS(&m_vertexBuffer) // バッファのポインタ
    );

    // バッファをCPUメモリ空間にマッピング（書き込み可能にする）
    void* mapped = nullptr;
    m_vertexBuffer->Map(0, nullptr, &mapped);

    // バッファに頂点データを書き込む
    memcpy(mapped, vertices.data(), bufferSize);

    // マッピング解除（GPUアクセス可能状態へ戻す）
    m_vertexBuffer->Unmap(0, nullptr);

    // 頂点バッファビュー（GPUに渡すバッファ情報）をセット
    m_vbv.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vbv.SizeInBytes = bufferSize;

    m_vbv.StrideInBytes = sizeof(SkinningVertex);
}

// インデックスバッファを作成する関数
void BufferManager::CreateIndexBuffer(ID3D12Device* device, const std::vector<uint16_t>& indices)
{
    // バッファサイズを計算（インデックス数 × インデックスサイズ）
    const UINT bufferSize = UINT(indices.size() * sizeof(uint16_t));

    // アップロード用ヒーププロパティとリソース記述子を作成
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    // バッファリソースを生成
    device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_indexBuffer)
    );

    // バッファをCPUメモリ空間にマッピング（書き込み可能にする）
    void* mapped = nullptr;
    m_indexBuffer->Map(0, nullptr, &mapped);

    // バッファにインデックスデータを書き込む
    memcpy(mapped, indices.data(), bufferSize);

    // マッピング解除（GPUアクセス可能状態へ戻す）
    m_indexBuffer->Unmap(0, nullptr);

    // インデックスバッファビュー（GPUに渡すバッファ情報）をセット
    m_ibv.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    m_ibv.SizeInBytes = bufferSize;
    m_ibv.Format = DXGI_FORMAT_R16_UINT; // 16ビットインデックス
}

// BufferManager.cpp



// 定数バッファ（CBV: Constant Buffer View）を作成する関数
void BufferManager::CreateConstantBuffer(ID3D12Device* device, size_t size)
{
    // 256バイト境界にアライン（定数バッファは256バイト単位で確保する必要あり）
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer((size + 255) & ~255);

    // バッファリソースを生成
    device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_constantBuffer)
    );

    // GPUバッファの仮想アドレスを保持
    m_cbGpuAddress = m_constantBuffer->GetGPUVirtualAddress();
}

// 定数バッファのGPUアドレスを取得する関数
D3D12_GPU_VIRTUAL_ADDRESS BufferManager::GetConstantBufferGPUAddress() const
{
    return m_cbGpuAddress;
}
