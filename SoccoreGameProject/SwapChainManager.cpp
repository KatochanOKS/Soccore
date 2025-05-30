#include "SwapChainManager.h"
#include <cassert>

bool SwapChainManager::Initialize(HWND hWnd, ID3D12Device* device, ID3D12CommandQueue* cmdQueue, UINT width, UINT height) {
    m_width = width;    // 追加
    m_height = height;  // 追加
    Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
    HRESULT hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory));
    if (FAILED(hr)) return false;

    DXGI_SWAP_CHAIN_DESC1 scDesc = {};
    scDesc.BufferCount = m_bufferCount;
    scDesc.Width = width;
    scDesc.Height = height;
    scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scDesc.SampleDesc.Count = 1;

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
    hr = dxgiFactory->CreateSwapChainForHwnd(cmdQueue, hWnd, &scDesc, nullptr, nullptr, &swapChain1);
    if (FAILED(hr)) return false;

    hr = swapChain1.As(&m_swapChain);
    if (FAILED(hr)) return false;

    // RTVヒープ作成
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = m_bufferCount;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_rtvHeap));
    if (FAILED(hr)) return false;

    m_rtvHeapSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // バックバッファ取得＆RTV作成
    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < m_bufferCount; ++i) {
        hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i]));
        if (FAILED(hr)) return false;
        device->CreateRenderTargetView(m_backBuffers[i].Get(), nullptr, handle);
        handle.ptr += m_rtvHeapSize;
    }

    return true;
}

void SwapChainManager::Cleanup() {
    for (UINT i = 0; i < m_bufferCount; ++i) {
        m_backBuffers[i].Reset();
    }
    m_rtvHeap.Reset();
    m_swapChain.Reset();
}
