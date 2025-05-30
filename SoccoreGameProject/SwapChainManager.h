#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

class SwapChainManager {
public:
    bool Initialize(HWND hWnd, ID3D12Device* device, ID3D12CommandQueue* cmdQueue, UINT width, UINT height);
    void Cleanup();

    IDXGISwapChain4* GetSwapChain() const { return m_swapChain.Get(); }
    ID3D12DescriptorHeap* GetRTVHeap() const { return m_rtvHeap.Get(); }
    ID3D12Resource* GetBackBuffer(int idx) const { return m_backBuffers[idx].Get(); }
    UINT GetBufferCount() const { return m_bufferCount; }
    UINT GetRTVHeapSize() const { return m_rtvHeapSize; }
    UINT GetWidth()  const { return m_width; }
    UINT GetHeight() const { return m_height; }

private:
    Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_backBuffers[2]; // 2–‡‚ÅOK
    UINT m_bufferCount = 2;
    UINT m_rtvHeapSize = 0;
    UINT m_width = 0;
    UINT m_height = 0;
};
