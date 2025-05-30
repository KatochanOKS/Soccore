#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <unordered_map>
#include <vector>

class TextureManager {
public:
    void Initialize(ID3D12Device* device, UINT maxTextureNum = 64);
    int LoadTexture(const std::wstring& filename, ID3D12GraphicsCommandList* cmdList);
    ID3D12DescriptorHeap* GetSRVHeap();
    D3D12_GPU_DESCRIPTOR_HANDLE GetSRV(int index);

private:
    Microsoft::WRL::ComPtr<ID3D12Device> m_device;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvHeap;
    std::unordered_map<std::wstring, int> m_textureIndices;
    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_textures;
    // ÉwÉbÉ_Å[Ç…
    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_uploadBuffers;
    UINT m_descriptorSize = 0;
    UINT m_nextIndex = 0;
};
