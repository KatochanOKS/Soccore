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
#include "Animator.h"
#include "Scene.h"
#include <memory>

struct ObjectCB {
    DirectX::XMMATRIX WorldViewProj;
    DirectX::XMFLOAT4 Color;
    int UseTexture;
    float padding[3]; // 16�o�C�g�A���C��
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
	BufferManager* GetQuadBufferManager() { return &m_quadBufferManager; } // ���ǉ��I
	BufferManager* GetSkyBufferManager() { return &m_skyBufferManager; } // �X�J�C�h�[����p�o�b�t�@
    Renderer* GetRenderer() { return &m_renderer; }  // ���ǉ�
    std::vector<GameObject*> m_gameObjects;
    int m_texIdx = -1;
    int m_cubeTexIdx = -1;
private:
    HWND m_hWnd = nullptr;
    DeviceManager m_deviceManager;
    SwapChainManager m_swapChainManager;
    DepthBufferManager m_depthBufferManager;
    PipelineManager m_pipelineManager;
    BufferManager m_bufferManager;      // ���ʃo�b�t�@�iCube, Ground���j
    BufferManager m_modelBufferManager; // FBX���f����p�o�b�t�@
    BufferManager m_quadBufferManager;  // ���ǉ��I
    BufferManager m_skyBufferManager; // �X�J�C�h�[����p�o�b�t�@

    TextureManager m_textureManager;
    FbxModelLoader::VertexInfo m_modelVertexInfo;
    Renderer m_renderer; // ���`��Ǘ��N���X�I

    std::unique_ptr<Animator> m_animator;
    std::unique_ptr<Scene> m_activeScene; // �A�N�e�B�u�ȃV�[��
	bool isMoving = false; // 0:��~, 1:�ړ���
};
