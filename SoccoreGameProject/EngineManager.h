#pragma once
#include <Windows.h>
#include "DeviceManager.h"
#include "SwapChainManager.h" // �ǉ�
#include "DepthBufferManager.h" // �ǉ�
#include "PipelineManager.h" // �ǉ�
#include "BufferManager.h"
#include"TextureManager.h"
#include "Transform.h"
#include "GameObject.h"
class EngineManager {
public:
    void SetHWND(HWND hwnd) { m_hWnd = hwnd; }
    HWND GetHWND() const { return m_hWnd; }

    void Initialize();
    void Start();
    void Update();
    void Draw();
    void Shutdown();
	void CreateTestCube(); // �ǉ�    
    DeviceManager* GetDeviceManager() { return &m_deviceManager; }
    SwapChainManager* GetSwapChainManager() { return &m_swapChainManager; } // �ǉ�
    DepthBufferManager* GetDepthBufferManager() { return &m_depthBufferManager; } // �ǉ�
    BufferManager* GetBufferManager() { return &m_bufferManager; }
	PipelineManager* GetPipelineManager() { return &m_pipelineManager; } // �ǉ�
	TextureManager* GetTextureManager() { return &m_textureManager; } // �ǉ�
	BufferManager* GetCubeBufferManager() { return &m_cubeBufferManager; } // �ǉ�

    std::vector<GameObject*> m_gameObjects;
    int m_texIdx = -1;
    //============================================================================================================

    void CreateTestQuad(); // �ǉ�
private:
    HWND m_hWnd = nullptr;
    DeviceManager m_deviceManager;
    SwapChainManager m_swapChainManager; // �ǉ�
    DepthBufferManager m_depthBufferManager; // �ǉ�
    PipelineManager m_pipelineManager; // �ǉ�
    BufferManager m_bufferManager; // �ǉ�
	TextureManager m_textureManager; // �ǉ�
	BufferManager m_cubeBufferManager; // �ǉ�
	Transform m_groundTransform; // �n�ʗp��Transform
    // ����Manager�����l�ɒǉ�
};
