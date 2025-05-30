#pragma once
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#include<dxgi1_6.h>
#include<cmath>
#include<d3d12.h>
#include<cassert>
#include<Windows.h>
#include<DirectXMath.h>
using namespace DirectX;

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <wrl/client.h>
using Microsoft::WRL::ComPtr;


void WaitDrawDone();
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

//ウィンドウ------------------------------------------------------------------------
LPCWSTR	WindowTitle = L"loveDX12";
const int ClientWidth = 1280;
const int ClientHeight = 720;
const int ClientPosX = (GetSystemMetrics(SM_CXSCREEN) - ClientWidth) / 2;//中央表示
const int ClientPosY = (GetSystemMetrics(SM_CYSCREEN) - ClientHeight) / 2;//中央表示
const float Aspect = static_cast<float>(ClientWidth) / ClientHeight;
#if 1 
DWORD WindowStyle = WS_OVERLAPPEDWINDOW;
#else 
DWORD WindowStyle = WS_POPUP;//Alt + F4で閉じる
#endif
HWND HWnd;

//デバイス--------------------------------------------------------------------------
ComPtr<ID3D12Device>  Device;
//コマンド
ID3D12CommandAllocator* CommandAllocator;
ID3D12GraphicsCommandList* CommandList;
ID3D12CommandQueue* CommandQueue;
//フェンス
ID3D12Fence* Fence;
HANDLE FenceEvent;
UINT64 FenceValue;
//デバッグ
HRESULT Hr;
//リソース----------------------------------------------------------------------
//バックバッファ
IDXGISwapChain4* SwapChain;
ID3D12Resource* BackBufs[2];
UINT BackBufIdx;
ID3D12DescriptorHeap* BbvHeap;//"Bbv"は"BackBufView"の略
UINT BbvHeapSize;
//デプスステンシルバッファ
ID3D12Resource* DepthStencilBuf;
ID3D12DescriptorHeap* DsvHeap;//"Dsv"は"DepthStencilBufView"の略
//頂点位置バッファ
ID3D12Resource* PositionBuf;
D3D12_VERTEX_BUFFER_VIEW PositionBufView;

//テクスチャ位置バッファ
ID3D12Resource* TexcoordBuf;
D3D12_VERTEX_BUFFER_VIEW TexcoordBufView;

//頂点インデックスバッファ
ID3D12Resource* IndexBuf;
D3D12_INDEX_BUFFER_VIEW	IndexBufView;
//コンスタントバッファ０
struct CONST_BUF0 {
    XMMATRIX mat;
};
//コンスタントバッファ１
struct CONST_BUF1 {
    XMFLOAT4 diffuse;
};
//メッシュ_a------------------------
CONST_BUF0* MappedConstBuf0_a;
CONST_BUF1* MappedConstBuf1_a;
ID3D12Resource* ConstBuf0_a;
ID3D12Resource* ConstBuf1_a;
ID3D12Resource* TextureBuf_a;
//ID3D12DescriptorHeap* CbvTbvHeap_a;

//メッシュ_b------------------------
CONST_BUF0* MappedConstBuf0_b;
CONST_BUF1* MappedConstBuf1_b;
ID3D12Resource* ConstBuf0_b;
ID3D12Resource* ConstBuf1_b;
ID3D12Resource* TextureBuf_b;
//ID3D12DescriptorHeap* CbvTbvHeap_b;
ID3D12DescriptorHeap* CbvTbvHeap;


//パイプライン--------------------------------------------------------------------
ID3D12RootSignature* RootSignature;
ID3D12PipelineState* PipelineState;
D3D12_VIEWPORT Viewport;
D3D12_RECT ScissorRect;


