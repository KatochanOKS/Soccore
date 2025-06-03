#pragma once
#include <Windows.h>
#include "DeviceManager.h"
#include "SwapChainManager.h" // ’Ç‰Á
#include "DepthBufferManager.h" // ’Ç‰Á
#include "PipelineManager.h" // ’Ç‰Á
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
	void CreateTestCube(); // ’Ç‰Á    
    DeviceManager* GetDeviceManager() { return &m_deviceManager; }
    SwapChainManager* GetSwapChainManager() { return &m_swapChainManager; } // ’Ç‰Á
    DepthBufferManager* GetDepthBufferManager() { return &m_depthBufferManager; } // ’Ç‰Á
    BufferManager* GetBufferManager() { return &m_bufferManager; }
	PipelineManager* GetPipelineManager() { return &m_pipelineManager; } // ’Ç‰Á
	TextureManager* GetTextureManager() { return &m_textureManager; } // ’Ç‰Á
	BufferManager* GetCubeBufferManager() { return &m_cubeBufferManager; } // ’Ç‰Á

    std::vector<GameObject*> m_gameObjects;
    int m_texIdx = -1;
    //============================================================================================================

    void CreateTestQuad(); // ’Ç‰Á
private:
    HWND m_hWnd = nullptr;
    DeviceManager m_deviceManager;
    SwapChainManager m_swapChainManager; // ’Ç‰Á
    DepthBufferManager m_depthBufferManager; // ’Ç‰Á
    PipelineManager m_pipelineManager; // ’Ç‰Á
    BufferManager m_bufferManager; // ’Ç‰Á
	TextureManager m_textureManager; // ’Ç‰Á
	BufferManager m_cubeBufferManager; // ’Ç‰Á
	Transform m_groundTransform; // ’n–Ê—p‚ÌTransform
    // ‘¼‚ÌManager‚à“¯—l‚É’Ç‰Á
};
