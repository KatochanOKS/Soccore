#include "EngineManager.h"
#include "GameScene.h"
#include "MeshLibrary.h"
#include"StartScene.h"
#include <memory>

// ★ ImGui 関連
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}


void EngineManager::Initialize() {
    m_deviceManager.Initialize();
    auto* device = m_deviceManager.GetDevice();
    auto* cmdQueue = m_deviceManager.GetCommandQueue();
    m_swapChainManager.Initialize(m_hWnd, device, cmdQueue, 1280, 720);
    m_depthBufferManager.Initialize(device, 1280, 720);
    m_pipelineManager.Initialize(
        device,
        L"assets/VertexShader.cso", L"assets/PixelShader.cso",
        L"assets/SkinningVS.cso", L"assets/SkinningPS.cso",
        L"assets/UIVertexShader.cso", L"assets/UIPixelShader.cso"
    );

    m_textureManager.Initialize(device);

    constexpr size_t CBV_SIZE = 256;
    m_bufferManager.CreateConstantBuffer(device, CBV_SIZE * 100);

    std::vector<Vertex> quadVertices;
    std::vector<uint16_t> quadIndices;
    MeshLibrary::GetQuadMesh2D(quadVertices, quadIndices);
    m_quadBufferManager.CreateVertexBuffer(device, quadVertices);
    m_quadBufferManager.CreateIndexBuffer(device, quadIndices);
    m_quadBufferManager.CreateConstantBuffer(device, CBV_SIZE * 100);

    std::vector<Vertex> sphereVertices;
    std::vector<uint16_t> sphereIndices;
    MeshLibrary::GetSphereMesh(sphereVertices, sphereIndices, 1.0f, 32, 32);
    m_sphereBufferManager.CreateVertexBuffer(device, sphereVertices);
    m_sphereBufferManager.CreateIndexBuffer(device, sphereIndices);
    m_sphereBufferManager.CreateConstantBuffer(device, CBV_SIZE * 100);

    std::vector<Vertex> cubeVertices;
    std::vector<uint16_t> cubeIndices;
    MeshLibrary::GetCubeMesh(cubeVertices, cubeIndices);
    m_cubeBufferManager.CreateVertexBuffer(device, cubeVertices);
    m_cubeBufferManager.CreateIndexBuffer(device, cubeIndices);
    m_cubeBufferManager.CreateConstantBuffer(device, CBV_SIZE * 100);


    m_renderer.Initialize(
        &m_deviceManager,
        &m_swapChainManager,
        &m_depthBufferManager,
        &m_pipelineManager,
        &m_textureManager,
        &m_bufferManager,
        &m_modelBufferManager,
        &m_quadBufferManager,
        &m_skyBufferManager,
        &m_sphereBufferManager,
        GetModelVertexInfo()
    );

    // ==== ★★ ImGui 初期化ここから ★★ ====
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;




    // キーボード操作ができるように
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	
    // 見た目
    ImGui::StyleColorsDark();

    // ImGui 用の SRV ヒープを作成（1つだけでOK）
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.NumDescriptors = 1;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    heapDesc.NodeMask = 0;
    device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_imguiSrvHeap));

    // Win32 backend 初期化
    ImGui_ImplWin32_Init(m_hWnd);

    // ★★ ここを「新しい InitInfo 版」に書き換える ★★
    ImGui_ImplDX12_InitInfo init_info{};
    init_info.Device = device;
    init_info.CommandQueue = cmdQueue; // ← すでに上で取っているコマンドキュー
    init_info.NumFramesInFlight = m_swapChainManager.GetBufferCount();
    init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    // DSVFormat は実際のデプスのフォーマットに合わせる
    // （DepthBufferManager が D32_FLOAT ならそれに合わせる）
    init_info.DSVFormat = DXGI_FORMAT_D32_FLOAT;

    // ImGui の SRV を発行するヒープ
    init_info.SrvDescriptorHeap = m_imguiSrvHeap.Get();

    // レガシー互換用の“1個だけ”フォント用 SRV ハンドル
    init_info.LegacySingleSrvCpuDescriptor = m_imguiSrvHeap->GetCPUDescriptorHandleForHeapStart();
    init_info.LegacySingleSrvGpuDescriptor = m_imguiSrvHeap->GetGPUDescriptorHandleForHeapStart();

    // 新シグネチャ版 Init
    ImGui_ImplDX12_Init(&init_info);

    // ★ これを追加！
    m_renderer.SetImGuiSrvHeap(m_imguiSrvHeap.Get());

    // ★★ ここでは ImGui_ImplDX12_CreateDeviceObjects() を呼ばない！ ★★
    // 必要なときに ImGui_ImplDX12_NewFrame() 側で勝手に作ってくれる
    // ===============================


    m_activeScene = std::make_unique<StartScene>(this);
    m_activeScene->Start();
}

void EngineManager::Start() {}

void EngineManager::Update()
{
    // ① ImGui のフレーム開始（順番は Win32 → DX12）
    ImGui_ImplWin32_NewFrame();
    ImGui_ImplDX12_NewFrame();

    // ② ウィンドウのクライアントサイズを取得
    RECT rc{};
    GetClientRect(m_hWnd, &rc);
    float clientW = static_cast<float>(rc.right - rc.left);
    float clientH = static_cast<float>(rc.bottom - rc.top);

    // ③ バックバッファ（スワップチェイン）のサイズを取得
    float fbW = static_cast<float>(m_swapChainManager.GetWidth());   // 1280
    float fbH = static_cast<float>(m_swapChainManager.GetHeight());  // 720

    // ④ ImGui に「論理サイズ = バックバッファ」を教える
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(fbW, fbH);

    // ⑤ ウィンドウ座標 → バックバッファ座標へのスケール
    //    （マウス・クリッピング用。ここがミソ）
    io.DisplayFramebufferScale = ImVec2(
        fbW / clientW,   // 1280 / 1262 ≒ 1.01
        fbH / clientH    // 720  / 673  ≒ 1.07
    );

    // ⑥ ImGui フレーム開始
    ImGui::NewFrame();

    // 以降はいつも通り
    if (m_activeScene) {
        m_activeScene->Update();
    }
}





void EngineManager::Draw() {
    if (m_activeScene) m_activeScene->Draw();
}


void EngineManager::Shutdown() {
    // ImGui 終了
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    m_deviceManager.Cleanup();
    m_swapChainManager.Cleanup();
}



void EngineManager::ChangeScene(std::unique_ptr<Scene> nextScene) {
    // 旧シーンは unique_ptr のムーブで自動破棄
    m_activeScene = std::move(nextScene);
    if (m_activeScene) m_activeScene->Start();
}