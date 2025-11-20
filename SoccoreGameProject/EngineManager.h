#pragma once
#include <Windows.h>
#include <vector>
#include <DirectXMath.h>
#include <memory>
#include "DeviceManager.h"
#include "SwapChainManager.h"
#include "DepthBufferManager.h"
#include "PipelineManager.h"
#include "BufferManager.h"
#include "TextureManager.h"
#include "Transform.h"
#include "GameObject.h"
#include "FbxModelLoader.h"
#include "Renderer.h"
#include "Animator.h"
#include "Scene.h"
#include "Camera.h"
#include <wrl.h>
struct ObjectCB {
    DirectX::XMMATRIX WorldViewProj;
    DirectX::XMFLOAT4 Color;
    int UseTexture;
    float padding[3]; // 16バイトアライン
};

class EngineManager {
public:
    void SetHWND(HWND hwnd) { m_hWnd = hwnd; }
    HWND GetHWND() const { return m_hWnd; }

    void Initialize();
    void Start();
    void Update();
    void Draw();
    void Shutdown();

    DeviceManager* GetDeviceManager() { return &m_deviceManager; }
    SwapChainManager* GetSwapChainManager() { return &m_swapChainManager; }
    DepthBufferManager* GetDepthBufferManager() { return &m_depthBufferManager; }
    BufferManager* GetBufferManager() { return &m_bufferManager; }
    PipelineManager* GetPipelineManager() { return &m_pipelineManager; }
    TextureManager* GetTextureManager() { return &m_textureManager; }
    FbxModelLoader::VertexInfo* GetModelVertexInfo() { return &m_modelVertexInfo; }
    BufferManager* GetModelBufferManager() { return &m_modelBufferManager; }
    BufferManager* GetQuadBufferManager() { return &m_quadBufferManager; }
    BufferManager* GetCubeBufferManager() { return &m_cubeBufferManager; }
    BufferManager* GetSkyBufferManager() { return &m_skyBufferManager; }
    BufferManager* GetSphereBufferManager() { return &m_sphereBufferManager; }
    Renderer* GetRenderer() { return &m_renderer; }

    Camera* GetCamera() { return &m_camera; }
    const DirectX::XMFLOAT3& GetCameraPosition() const { return m_camera.GetPosition(); }

    // 次のシーンに切り替えて Start() まで呼ぶ高水準API
    void ChangeScene(std::unique_ptr<Scene> nextScene);

    std::vector<GameObject*> m_gameObjects;
    int m_texIdx = -1;
    int m_cubeTexIdx = -1;
private:
    HWND m_hWnd = nullptr;
    DeviceManager m_deviceManager;
    SwapChainManager m_swapChainManager;
    DepthBufferManager m_depthBufferManager;
    PipelineManager m_pipelineManager;
    BufferManager m_bufferManager;
    BufferManager m_modelBufferManager;
    BufferManager m_quadBufferManager;
    BufferManager m_skyBufferManager;
    BufferManager m_sphereBufferManager;
    BufferManager m_cubeBufferManager;
    TextureManager m_textureManager;
    FbxModelLoader::VertexInfo m_modelVertexInfo;
    Renderer m_renderer;
    Camera m_camera;

    std::unique_ptr<Animator> m_animator;
    std::unique_ptr<Scene> m_activeScene;
    bool isMoving = false;


    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_imguiSrvHeap; // ImGui 用ヒープ
};
