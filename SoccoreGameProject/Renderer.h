#pragma once
#include <vector>
#include <DirectXMath.h>
#include "DeviceManager.h"
#include "SwapChainManager.h"
#include "DepthBufferManager.h"
#include "PipelineManager.h"
#include "TextureManager.h"
#include "BufferManager.h"
#include "GameObject.h"
#include "FbxModelLoader.h"


class Renderer {
public:
    void Initialize(
        DeviceManager* deviceMgr,
        SwapChainManager* swapMgr,
        DepthBufferManager* depthMgr,
        PipelineManager* pipeMgr,
        TextureManager* texMgr,
        BufferManager* cubeBufMgr,
        BufferManager* modelBufMgr,
        BufferManager* quadBufMgr, // Quad�p�o�b�t�@
		BufferManager* skyBufMgr, // �X�J�C�h�[����p�o�b�t�@
		BufferManager* sphereBufMgr, // �T�b�J�[�{�[���p�̋��̃o�b�t�@
        FbxModelLoader::VertexInfo* modelVertexInfo
    );

    void BeginFrame();
    void DrawObject(GameObject* obj, size_t idx, const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj);
    // Renderer.h
    void DrawUIImage(class UIImage* image, size_t idx);
    // SkySphere �`��p�֐��i�ǉ��j
    void DrawSkySphere(GameObject* obj, size_t idx, const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj);
	// �T�b�J�[�{�[���`��p�֐��i�ǉ��j
	void DrawSoccerBall(GameObject* obj, size_t idx, const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj);
    void EndFrame();

private:
    DeviceManager* m_deviceMgr = nullptr;
    SwapChainManager* m_swapMgr = nullptr;
    DepthBufferManager* m_depthMgr = nullptr;
    PipelineManager* m_pipeMgr = nullptr;
    TextureManager* m_texMgr = nullptr;
    BufferManager* m_cubeBufMgr = nullptr;
    BufferManager* m_modelBufMgr = nullptr;
    FbxModelLoader::VertexInfo* m_modelVertexInfo = nullptr;
    BufferManager* m_cbvBufferMgr = nullptr;  // �萔�o�b�t�@�p�����Ɏg���o�b�t�@�}�l�[�W��
    BufferManager* m_quadBufferMgr = nullptr;  // �g���́h����Ȃ��g�|�C���^�h
	BufferManager* m_skyBufferMgr = nullptr; // �X�J�C�h�[����p�o�b�t�@
	BufferManager* m_sphereBufferMgr = nullptr; // �T�b�J�[�{�[���p�̋��̃o�b�t�@
    Microsoft::WRL::ComPtr<ID3D12Resource> m_skinningConstantBuffer; // �X�L�j���O�pCBV
    D3D12_GPU_VIRTUAL_ADDRESS m_skinCBGpuAddr = 0; // �A�h���X�ێ��p
    size_t m_skinCBSize = 0; // �o�b�t�@�T�C�Y

    UINT m_backBufferIndex = 0;
    ID3D12GraphicsCommandList* m_cmdList = nullptr;
    float m_width = 1280.0f;
    float m_height = 720.0f;
};