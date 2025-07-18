#pragma once
#include <Windows.h>
#include <vector>
#include <DirectXMath.h>
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
    std::vector<GameObject*> m_gameObjects;
    int m_texIdx = -1;
    int m_cubeTexIdx = -1;
    double m_animTime = 0.0; // アニメーション再生時間（秒）
private:
    HWND m_hWnd = nullptr;
    DeviceManager m_deviceManager;
    SwapChainManager m_swapChainManager;
    DepthBufferManager m_depthBufferManager;
    PipelineManager m_pipelineManager;
    BufferManager m_bufferManager;      // 共通バッファ（Cube, Ground等）
    BufferManager m_modelBufferManager; // FBXモデル専用バッファ
    TextureManager m_textureManager;
    FbxModelLoader::VertexInfo m_modelVertexInfo;

    Renderer m_renderer; // ★描画管理クラス！
};
