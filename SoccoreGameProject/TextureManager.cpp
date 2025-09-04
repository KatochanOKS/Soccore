#include "TextureManager.h"
#include <DirectXTex.h>   // WICローダ / CreateTexture / UpdateSubresources に必要
#include <cassert>
#include "d3dx12.h"

void TextureManager::Initialize(ID3D12Device* device, UINT maxTextureNum) {
    // 1) SRV用のディスクリプタヒープを作成（シェーダから見えるように SHADER_VISIBLE）
    m_device = device;
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = maxTextureNum;                               // 予約数
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;               // SRV/CBV/UAV用
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;           // これ大事：シェーダから使う

    // CreateDescriptorHeap に失敗すると致命的。hrチェックを厳密にするなら assert の代わりに if(FAILED(hr)) でログ＆return。
    HRESULT hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_srvHeap));
    assert(SUCCEEDED(hr));

    // 2) SRVディスクリプタの「1個あたりのサイズ」を問い合わせておく（オフセット計算に使う）
    m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // 3) 次に使う空きスロット番号を0にリセット
    m_nextIndex = 0;
}

int TextureManager::LoadTexture(const std::wstring& filename, ID3D12GraphicsCommandList* cmdList) {
    // 0) 二重読み込み対策：同じパスは同じインデックスを返す
    auto it = m_textureIndices.find(filename);
    if (it != m_textureIndices.end()) return it->second;

    // 1) CPU側へ画像ロード（WIC：png/jpg/bmp等）
    DirectX::TexMetadata metadata = {};
    DirectX::ScratchImage image = {};
    // メモ：sRGB画像なら WIC_FLAGS_FORCE_SRGB を使うと発色が正しくなる場合がある
    HRESULT hr = DirectX::LoadFromWICFile(
        filename.c_str(),
        DirectX::WIC_FLAGS_NONE, // ここを WIC_FLAGS_FORCE_SRGB や WIC_FLAGS_IGNORE_SRGB に変えると色空間を制御できる
        &metadata,
        image
    );
    assert(SUCCEEDED(hr)); // 失敗ならファイルパス/拡張子/相対パスを確認

    // 2) GPU(Default heap)側のテクスチャリソースを作成（まだ中身は空）
    Microsoft::WRL::ComPtr<ID3D12Resource> texture;
    hr = DirectX::CreateTexture(m_device.Get(), metadata, &texture);
    assert(SUCCEEDED(hr));

    // 3) Upload heap の一時バッファを用意（コピー元）
    //    画像の全サブリソース（ミップ/配列/面）を転送するのに必要なサイズを見積もる
    const UINT subresourceCount = static_cast<UINT>(image.GetImageCount());
    size_t uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, subresourceCount);

    Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);           // CPU書き込み可
    CD3DX12_RESOURCE_DESC   resDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
    hr = m_device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, // Uploadは常にこのステート
        nullptr,
        IID_PPV_ARGS(&uploadBuffer)
    );
    assert(SUCCEEDED(hr));

    // 4) UpdateSubresources で Upload → Default へコピーするための構造体配列を作成
    std::vector<D3D12_SUBRESOURCE_DATA> subresources;
    subresources.reserve(subresourceCount);
    const DirectX::Image* img = image.GetImages();
    for (size_t i = 0; i < image.GetImageCount(); ++i) {
        D3D12_SUBRESOURCE_DATA sub = {};
        sub.pData = img[i].pixels;     // ピクセル先頭
        sub.RowPitch = img[i].rowPitch;   // 1行あたりのバイト数
        sub.SlicePitch = img[i].slicePitch; // 1面（ミップ/アレイ要素）のバイト数
        subresources.push_back(sub);
    }

    // 5) コピー（CopyDest → PixelShaderResource へ遷移）
    //    UpdateSubresources は内部で CopyBufferRegion / CopyTextureRegion を記録してくれる
    UpdateSubresources(cmdList, texture.Get(), uploadBuffer.Get(), 0, 0, subresourceCount, subresources.data());

    // 6) シェーダから参照できるようにステート遷移（COPY_DEST → PIXEL_SHADER_RESOURCE）
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        texture.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );
    cmdList->ResourceBarrier(1, &barrier);

    // 7) SRV（Shader Resource View）を、ヒープの m_nextIndex 番に作成
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; // 通常は既定マッピング
    srvDesc.Format = metadata.format;                      // 画像フォーマット（sRGBにしたいなら変換を検討）
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 今回は2D限定の想定
    srvDesc.Texture2D.MipLevels = static_cast<UINT>(metadata.mipLevels); // WIC読み込みは通常ミップ1枚。必要なら自前でミップ生成

    // 7-1) ヒープ先頭CPUハンドル + オフセット（インクリメント幅×インデックス）
    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<SIZE_T>(m_nextIndex) * m_descriptorSize;

    // 7-2) SRVを作る（この時点でテクスチャは PS から参照可能）
    m_device->CreateShaderResourceView(texture.Get(), &srvDesc, handle);

    // 8) 管理テーブルに登録
    m_textures.push_back(texture);                // 実体（Default）保持
    const int texIndex = static_cast<int>(m_nextIndex);
    m_textureIndices[filename] = texIndex;        // パス→インデックスの辞書
    m_nextIndex++;                                // 次の空きスロットへ
    m_uploadBuffers.push_back(uploadBuffer);      // ★重要：GPU転送完了まで解放しないため保持

    // 9) エラーログ（hr は直前のCreateSRVの戻りではないので注意。必要なら個別チェックにする）
    if (FAILED(hr)) {
        OutputDebugStringA("★テクスチャ読み込み失敗！\n");
    }

    // 10) SRVスロット番号を返す（描画側は GetSRV(texIndex) で GPUハンドルを得る）
    return texIndex;
}

ID3D12DescriptorHeap* TextureManager::GetSRVHeap() {
    // 描画前に CommandList::SetDescriptorHeaps でこのヒープをセットするために返す
    return m_srvHeap.Get();
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSRV(int index) {
    // SRVテーブルの「GPU可視」先頭から index 個分だけポインタを進めたハンドルを返す
    D3D12_GPU_DESCRIPTOR_HANDLE handle = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<UINT64>(index) * m_descriptorSize;
    return handle;
}
