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

/// <summary>
/// 描画全般を管理するクラス。各種バッファ・テクスチャ・パイプラインをまとめて制御し、
/// ゲームオブジェクトやUIの描画を担当する。
/// </summary>
class Renderer {
public:
    /// <summary>
    /// 各種マネージャ・バッファの初期化
    /// </summary>
    void Initialize(
        DeviceManager* deviceMgr,
        SwapChainManager* swapMgr,
        DepthBufferManager* depthMgr,
        PipelineManager* pipeMgr,
        TextureManager* texMgr,
        BufferManager* cubeBufMgr,
        BufferManager* modelBufMgr,
        BufferManager* quadBufMgr,
        BufferManager* skyBufMgr,
        BufferManager* sphereBufMgr,
        FbxModelLoader::VertexInfo* modelVertexInfo
    );

    /// <summary>
    /// フレーム開始処理
    /// </summary>
    void BeginFrame();

    /// <summary>
    /// ゲームオブジェクトの描画
    /// </summary>
    void DrawObject(GameObject* obj, size_t idx, const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj);

    /// <summary>
    /// UI画像の描画
    /// </summary>
    void DrawUIImage(class UIImage* image, size_t idx);

    /// <summary>
    /// スカイドームの描画
    /// </summary>
    void DrawSkySphere(GameObject* obj, size_t idx, const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj);

    /// <summary>
    /// サッカーボールの描画
    /// </summary>
    void DrawSoccerBall(GameObject* obj, size_t idx, const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj);

    /// <summary>
    /// フレーム終了処理
    /// </summary>
    void EndFrame();

	void SetImGuiSrvHeap(ID3D12DescriptorHeap* heap) { m_ImGuiSrvHeap = heap; } ///< ImGui用SRVヒープ設定

private:
    DeviceManager* m_DeviceMgr = nullptr;                ///< デバイス管理
    SwapChainManager* m_SwapMgr = nullptr;               ///< スワップチェーン管理
    DepthBufferManager* m_DepthMgr = nullptr;            ///< 深度バッファ管理
    PipelineManager* m_PipeMgr = nullptr;                ///< パイプライン管理
    TextureManager* m_TexMgr = nullptr;                  ///< テクスチャ管理
    BufferManager* m_CubeBufMgr = nullptr;               ///< キューブ用バッファ
    BufferManager* m_ModelBufMgr = nullptr;              ///< モデル用バッファ
    FbxModelLoader::VertexInfo* m_ModelVertexInfo = nullptr; ///< モデル頂点情報
    BufferManager* m_CbvBufferMgr = nullptr;             ///< 定数バッファ用バッファ
    BufferManager* m_QuadBufferMgr = nullptr;            ///< クワッド用バッファ
    BufferManager* m_SkyBufferMgr = nullptr;             ///< スカイドーム用バッファ
    BufferManager* m_SphereBufferMgr = nullptr;          ///< サッカーボール用バッファ
    Microsoft::WRL::ComPtr<ID3D12Resource> m_SkinningConstantBuffer; ///< スキニング用CBV
    D3D12_GPU_VIRTUAL_ADDRESS m_SkinCBGpuAddr = 0;       ///< スキニングCBVのGPUアドレス
    size_t m_SkinCBSize = 0;                             ///< スキニングCBVのサイズ

    UINT m_BackBufferIndex = 0;                          ///< バックバッファインデックス
    ID3D12GraphicsCommandList* m_CmdList = nullptr;      ///< コマンドリスト
    UINT m_Width = 1280;                             ///< 画面幅
    UINT m_Height = 720;                             ///< 画面高さ

    
	ID3D12DescriptorHeap* m_ImGuiSrvHeap = nullptr; 	  ///< ImGui用SRVヒープ

};