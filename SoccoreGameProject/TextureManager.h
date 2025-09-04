#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <unordered_map>
#include <vector>

// 役割：アプリ全体で使うテクスチャを一括管理し、SRVディスクリプタヒープに並べていく。
// - Initialize() で SRV用ディスクリプタヒープを作成
// - LoadTexture() で画像ファイルを読み込み → GPUテクスチャを作って転送 → SRVを割り当て
// - GetSRVHeap()/GetSRV() で描画側にSRVハンドルを渡す
class TextureManager {
public:
    // device: D3D12デバイス
    // maxTextureNum: SRVヒープに確保する最大テクスチャ数（ディスクリプタ数）
    void Initialize(ID3D12Device* device, UINT maxTextureNum = 64);

    // filename: 読み込む画像（WIC対応：png/jpg/bmpなど）
    // cmdList : 転送コマンド記録用（Upload→Default へのコピーで使用）
    // 戻り値 : SRVヒープ上のインデックス（＝ルートパラメータに渡すときのオフセットに使う）
    int LoadTexture(const std::wstring& filename, ID3D12GraphicsCommandList* cmdList);

    // 描画前に SetDescriptorHeaps で渡すためのSRVヒープ生ポインタ
    ID3D12DescriptorHeap* GetSRVHeap();

    // SRVテーブルの GPU ハンドルを取得（index は LoadTexture の戻り値）
    D3D12_GPU_DESCRIPTOR_HANDLE GetSRV(int index);

private:
    Microsoft::WRL::ComPtr<ID3D12Device> m_device;                 // デバイス（Create* 用）
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvHeap;        // SRVの置き場（シェーダ可視）
    std::unordered_map<std::wstring, int> m_textureIndices;        // 同じファイルの重複読み込みを防ぐ
    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_textures; // 実体（Default Heap）
    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_uploadBuffers; // Upload保持（GPU転送が完全に終わるまで解放しない）
    UINT m_descriptorSize = 0;                                     // SRVディスクリプタサイズ（インクリメント幅）
    UINT m_nextIndex = 0;                                          // 次に割り当てるSRVスロット
};
