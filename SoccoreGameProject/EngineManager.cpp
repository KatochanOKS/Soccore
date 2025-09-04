#include "EngineManager.h"
#include "GameScene.h"
#include "MeshLibrary.h"
#include"StartScene.h"
#include <memory>
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

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

    /*m_activeScene = std::make_unique<GameScene>(this);
    m_activeScene->Start();*/

	m_activeScene = std::make_unique<StartScene>(this);
	m_activeScene->Start();

}

void EngineManager::Start() {}

void EngineManager::Update() {
    if (m_activeScene) m_activeScene->Update();
}

void EngineManager::Draw() {
    if (m_activeScene) m_activeScene->Draw();
}


void EngineManager::Shutdown() {
    m_deviceManager.Cleanup();
    m_swapChainManager.Cleanup();
}


void EngineManager::ChangeScene(std::unique_ptr<Scene> nextScene) {
    // 旧シーンは unique_ptr のムーブで自動破棄
    m_activeScene = std::move(nextScene);
    if (m_activeScene) m_activeScene->Start();
}