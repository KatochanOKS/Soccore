#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <unordered_map>
#include <vector>

/// <summary>
/// テクスチャを一括管理し、SRVディスクリプタヒープ経由で描画に利用できるようにするクラス。
/// </summary>
class TextureManager {
public:
    /// <summary>
    /// SRVディスクリプタヒープの初期化
    /// </summary>
    void Initialize(ID3D12Device* device, UINT maxTextureNum = 64);

    /// <summary>
    /// テクスチャの読み込みとSRV割り当て
    /// </summary>
    int LoadTexture(const std::wstring& filename, ID3D12GraphicsCommandList* cmdList);

    /// <summary>
    /// SRVヒープの取得
    /// </summary>
    ID3D12DescriptorHeap* GetSRVHeap();

    /// <summary>
    /// SRVハンドルの取得
    /// </summary>
    D3D12_GPU_DESCRIPTOR_HANDLE GetSRV(int index);

private:
    Microsoft::WRL::ComPtr<ID3D12Device> m_Device;                 ///< デバイス
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_SrvHeap;        ///< SRVヒープ
    std::unordered_map<std::wstring, int> m_TextureIndices;        ///< 重複読み込み防止
    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_Textures; ///< テクスチャリソース
    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_UploadBuffers; ///< Uploadバッファ
    UINT m_DescriptorSize = 0;                                     ///< SRVディスクリプタサイズ
    UINT m_NextIndex = 0;                                          ///< 次のSRVスロット
};