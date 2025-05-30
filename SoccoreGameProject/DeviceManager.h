#pragma once
#include <d3d12.h>
#include <wrl.h>

class DeviceManager {
public:
    bool Initialize();
    void Cleanup();

    ID3D12Device* GetDevice() const { return m_device.Get(); }
    ID3D12CommandQueue* GetCommandQueue() const { return m_commandQueue.Get(); }
    ID3D12CommandAllocator* GetCommandAllocator() const { return m_commandAllocator.Get(); }
    ID3D12GraphicsCommandList* GetCommandList() const { return m_commandList.Get(); }
    ID3D12Fence* GetFence() const { return m_fence.Get(); }
    HANDLE GetFenceEvent() const { return m_fenceEvent; }
    UINT64& GetFenceValue() { return m_fenceValue; }
    void WaitForGpu();
private:
    Microsoft::WRL::ComPtr<ID3D12Device> m_device;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
    Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
    HANDLE m_fenceEvent = nullptr;
    UINT64 m_fenceValue = 1;
};
