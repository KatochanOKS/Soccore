#include "pch.h"
#include "DeviceManager.h"
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
bool DeviceManager::Initialize() {
#ifdef _DEBUG
    Microsoft::WRL::ComPtr<ID3D12Debug> debug;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
        debug->EnableDebugLayer();
#endif

    HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device));
    if (FAILED(hr)) return false;

    hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));
    if (FAILED(hr)) return false;

    hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList));
    if (FAILED(hr)) return false;

    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    hr = m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_commandQueue));
    if (FAILED(hr)) return false;

    hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
    if (FAILED(hr)) return false;

    m_fenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
    if (!m_fenceEvent) return false;

    m_fenceValue = 1;
    return true;
}

void DeviceManager::Cleanup() {
    if (m_fenceEvent) CloseHandle(m_fenceEvent);
    // ComPtrは自動で解放
}

void DeviceManager::WaitForGpu() {
    // m_fence, m_fenceValue, m_commandQueue, m_fenceEvent を事前に作っておくこと！
    const UINT64 fenceToWaitFor = ++m_fenceValue;
    m_commandQueue->Signal(m_fence.Get(), fenceToWaitFor);
    if (m_fence->GetCompletedValue() < fenceToWaitFor) {
        m_fence->SetEventOnCompletion(fenceToWaitFor, m_fenceEvent);
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
}