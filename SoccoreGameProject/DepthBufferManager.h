#pragma once
#include <d3d12.h>
#include <wrl.h>

class DepthBufferManager {
public:
    bool Initialize(ID3D12Device* device, UINT width, UINT height);
    void Cleanup();

    ID3D12Resource* GetDepthStencilBuffer() const { return m_depthStencilBuffer.Get(); }
    ID3D12DescriptorHeap* GetDSVHeap() const { return m_dsvHeap.Get(); }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencilBuffer;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
};
