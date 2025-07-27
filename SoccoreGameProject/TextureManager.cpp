#include "TextureManager.h"
#include <DirectXTex.h>
#include <cassert>
#include"d3dx12.h"
void TextureManager::Initialize(ID3D12Device* device, UINT maxTextureNum) {
    m_device = device;
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = maxTextureNum;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_srvHeap));
    m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_nextIndex = 0;
}

int TextureManager::LoadTexture(const std::wstring& filename, ID3D12GraphicsCommandList* cmdList) {
    // すでに読み込み済みならそれを返す
    auto it = m_textureIndices.find(filename);
    if (it != m_textureIndices.end()) return it->second;

    DirectX::TexMetadata metadata = {};
    DirectX::ScratchImage image = {};
    HRESULT hr = DirectX::LoadFromWICFile(filename.c_str(), DirectX::WIC_FLAGS_NONE, &metadata, image);
    assert(SUCCEEDED(hr));

    // GPU用のリソース作成
    Microsoft::WRL::ComPtr<ID3D12Resource> texture;
    hr = DirectX::CreateTexture(m_device.Get(), metadata, &texture);
    assert(SUCCEEDED(hr));

    // アップロード用リソース作成
    size_t uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, (UINT)image.GetImageCount());
    Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
    m_device->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr, IID_PPV_ARGS(&uploadBuffer));

    // テクスチャ転送
    std::vector<D3D12_SUBRESOURCE_DATA> subresources;
    auto* img = image.GetImages();
    for (size_t i = 0; i < image.GetImageCount(); ++i) {
        D3D12_SUBRESOURCE_DATA sub = {};
        sub.pData = img[i].pixels;
        sub.RowPitch = img[i].rowPitch;
        sub.SlicePitch = img[i].slicePitch;
        subresources.push_back(sub);
    }
    UpdateSubresources(cmdList, texture.Get(), uploadBuffer.Get(), 0, 0, (UINT)subresources.size(), subresources.data());
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    cmdList->ResourceBarrier(1, &barrier);

    // SRV作成
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = metadata.format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = (UINT)metadata.mipLevels;

    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += m_nextIndex * m_descriptorSize;
    m_device->CreateShaderResourceView(texture.Get(), &srvDesc, handle);

    m_textures.push_back(texture);
    int texIndex = (int)m_nextIndex;
    m_textureIndices[filename] = texIndex;
    m_nextIndex++;
    m_uploadBuffers.push_back(uploadBuffer);
    if (FAILED(hr)) {
        OutputDebugStringA("★テクスチャ読み込み失敗！\n");
    }

    return texIndex;

}

ID3D12DescriptorHeap* TextureManager::GetSRVHeap() {
    return m_srvHeap.Get();
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSRV(int index) {
    D3D12_GPU_DESCRIPTOR_HANDLE handle = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
    handle.ptr += index * m_descriptorSize;
    return handle;
}
