#include "EngineManager.h"
#include "GameScene.h" // ← シーン分離型
#include "MeshLibrary.h"
#include <memory>

void EngineManager::Initialize() {
    m_deviceManager.Initialize();
    auto* device = m_deviceManager.GetDevice();
    auto* cmdQueue = m_deviceManager.GetCommandQueue();
    m_swapChainManager.Initialize(m_hWnd, device, cmdQueue, 1280, 720);
    m_depthBufferManager.Initialize(device, 1280, 720);
    m_pipelineManager.Initialize(
        device,
        L"assets/VertexShader.cso", L"assets/PixelShader.cso",   // 通常
        L"assets/SkinningVS.cso", L"assets/SkinningPS.cso",     // スキン
        L"assets/UIVertexShader.cso", L"assets/UIPixelShader.cso" // ★UI用
    );

    m_textureManager.Initialize(device);

    // CBVは十分大きく確保（仮に100オブジェクト分）
    constexpr size_t CBV_SIZE = 256;
    m_bufferManager.CreateConstantBuffer(device, CBV_SIZE * 100);

    std::vector<Vertex> quadVertices;
    std::vector<uint16_t> quadIndices;
    MeshLibrary::GetQuadMesh2D(quadVertices, quadIndices);
    m_quadBufferManager.CreateVertexBuffer(device, quadVertices);
    m_quadBufferManager.CreateIndexBuffer(device, quadIndices);
    // ★ここを追加！！
    m_quadBufferManager.CreateConstantBuffer(device, CBV_SIZE * 100);
    // Renderer初期化
    m_renderer.Initialize(
        &m_deviceManager,
        &m_swapChainManager,
        &m_depthBufferManager,
        &m_pipelineManager,
        &m_textureManager,
        &m_bufferManager,          // Cube, 地面用バッファ
        &m_modelBufferManager,     // FBXモデル用バッファ
        &m_quadBufferManager, // ★Quad用バッファを渡す
        GetModelVertexInfo()
    );

    // ★GameSceneで全てのオブジェクトを生成・管理する
    m_activeScene = std::make_unique<GameScene>(this);
    m_activeScene->Start();

}

void EngineManager::Start() {}

void EngineManager::Update() {

    if (m_activeScene) m_activeScene->Update();
}

void EngineManager::Draw() {
    // ★Scene主導で描画
    if (m_activeScene) m_activeScene->Draw();
}

void EngineManager::Shutdown() {
    // ★GameScene側でオブジェクトの破棄を管理する場合ここは空でOK
    m_deviceManager.Cleanup();
    m_swapChainManager.Cleanup();
}
