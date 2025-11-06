#include "Renderer.h"
#include "d3dx12.h"
#include "Transform.h"
#include "StaticMeshRenderer.h"
#include "SkinnedMeshRenderer.h"
#include "EngineManager.h"
#include "UIImage.h"
using namespace DirectX;

// 初期化
void Renderer::Initialize(
    DeviceManager* deviceMgr,
    SwapChainManager* swapMgr,
    DepthBufferManager* depthMgr,
    PipelineManager* pipeMgr,
    TextureManager* texMgr,
    BufferManager* cubeBufMgr,
    BufferManager* modelBufMgr,
    BufferManager* quadBufMgr,
	// ★追加：スカイドーム専用バッファ
	BufferManager* skyBufMgr,
	// ★追加：サッカーボール用の球体バッファ
	BufferManager* sphereBufMgr,
    FbxModelLoader::VertexInfo* modelVertexInfo
) {
    m_DeviceMgr = deviceMgr;
    m_SwapMgr = swapMgr;
    m_DepthMgr = depthMgr;
    m_PipeMgr = pipeMgr;
    m_TexMgr = texMgr;
    m_CubeBufMgr = cubeBufMgr;
    m_ModelBufMgr = modelBufMgr;
    m_QuadBufferMgr = quadBufMgr;
	m_SkyBufferMgr = skyBufMgr; // スカイドーム専用バッファ
	m_SphereBufferMgr = sphereBufMgr; // サッカーボール用の球体バッファ
    m_ModelVertexInfo = modelVertexInfo;
    m_Width = static_cast<float>(m_SwapMgr->GetWidth());
    m_Height = static_cast<float>(m_SwapMgr->GetHeight());

    // スキニング用CBVバッファ作成（80ボーン想定）
    m_SkinCBSize = sizeof(DirectX::XMMATRIX) * 80;
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC cbDesc = CD3DX12_RESOURCE_DESC::Buffer((m_SkinCBSize + 255) & ~255);

    m_DeviceMgr->GetDevice()->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &cbDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&m_SkinningConstantBuffer)
    );
    m_SkinCBGpuAddr = m_SkinningConstantBuffer->GetGPUVirtualAddress();
}

// フレーム開始
void Renderer::BeginFrame() {
    m_BackBufferIndex = m_SwapMgr->GetSwapChain()->GetCurrentBackBufferIndex();
    m_CmdList = m_DeviceMgr->GetCommandList();

    // バリア設定
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = m_SwapMgr->GetBackBuffer(m_BackBufferIndex);
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_CmdList->ResourceBarrier(1, &barrier);

    // RTV/DSV設定
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_SwapMgr->GetRTVHeap()->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += m_BackBufferIndex * m_SwapMgr->GetRTVHeapSize();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_DepthMgr->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
    m_CmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // 画面クリア
    const float clearColor[] = { 0.75f, 0.85f, 0.95f, 1.0f }; // 明るい空色

    m_CmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_CmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // ビューポート・シザー設定
    D3D12_VIEWPORT viewport = { 0, 0, m_Width, m_Height, 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, (LONG)m_Width, (LONG)m_Height };
    m_CmdList->RSSetViewports(1, &viewport);
    m_CmdList->RSSetScissorRects(1, &scissorRect);

    // テクスチャヒープをセット（どのPSOでも共通）
    ID3D12DescriptorHeap* heaps[] = { m_TexMgr->GetSRVHeap() };
    m_CmdList->SetDescriptorHeaps(_countof(heaps), heaps);
}

// 描画処理（型ごとに必ずPSOセット！）
// 描画処理（型ごとに必ずPSOセット！）
void Renderer::DrawObject(GameObject* obj, size_t idx, const XMMATRIX& view, const XMMATRIX& proj) {
    constexpr size_t CBV_SIZE = 256;

    // 1. スキニングメッシュ（アニメ付きモデル）
    if (auto* smr = obj->GetComponent<SkinnedMeshRenderer>()) {
        m_CmdList->SetPipelineState(m_PipeMgr->GetPipelineState(true));
        m_CmdList->SetGraphicsRootSignature(m_PipeMgr->GetRootSignature(true));

        if (smr->m_TexIndex >= 0)
            m_CmdList->SetGraphicsRootDescriptorTable(0, m_TexMgr->GetSRV(smr->m_TexIndex));

        XMMATRIX world = obj->GetComponent<Transform>()->GetWorldMatrix();
        XMMATRIX wvp = XMMatrixTranspose(world * view * proj);

        std::vector<XMMATRIX> skinnedMats;
        if (smr->m_Animator && smr->m_SkinVertexInfo) {
            skinnedMats = smr->m_Animator->GetSkinnedPose(smr->m_SkinVertexInfo->bindPoses);
            for (auto& m : skinnedMats)
                m = XMMatrixTranspose(m);
        }

        void* mapped = nullptr;
        if (SUCCEEDED(smr->m_BoneCB->GetConstantBuffer()->Map(0, nullptr, &mapped))) {
            memcpy((char*)mapped, &wvp, sizeof(XMMATRIX));
            if (!skinnedMats.empty())
                memcpy((char*)mapped + 256, skinnedMats.data(), sizeof(XMMATRIX) * skinnedMats.size());
            smr->m_BoneCB->GetConstantBuffer()->Unmap(0, nullptr);

            m_CmdList->SetGraphicsRootConstantBufferView(1, smr->m_BoneCB->GetConstantBufferGPUAddress());
            m_CmdList->SetGraphicsRootConstantBufferView(2, smr->m_BoneCB->GetConstantBufferGPUAddress() + 256);
        }

        if (smr->m_ModelBuffer && smr->m_SkinVertexInfo) {
            auto vbv = smr->m_ModelBuffer->GetVertexBufferView();
            auto ibv = smr->m_ModelBuffer->GetIndexBufferView();
            m_CmdList->IASetVertexBuffers(0, 1, &vbv);
            m_CmdList->IASetIndexBuffer(&ibv);
            m_CmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            m_CmdList->DrawIndexedInstanced((UINT)smr->m_SkinVertexInfo->indices.size(), 1, 0, 0, 0);
        }
        return;
    }

    // 2. 静的メッシュ（Cube / FBX / SkyDome / etc.）
    if (auto* mr = obj->GetComponent<StaticMeshRenderer>()) {
        // 2-1. スカイドーム
        if (mr->IsSkySphere) {
            DrawSkySphere(obj, idx, view, proj);
            return;
        }

        m_CmdList->SetPipelineState(m_PipeMgr->GetPipelineState(false));
        m_CmdList->SetGraphicsRootSignature(m_PipeMgr->GetRootSignature(false));

        // 共通: 定数バッファ
        D3D12_GPU_VIRTUAL_ADDRESS cbvAddr = m_CubeBufMgr->GetConstantBufferGPUAddress() + CBV_SIZE * idx;
        m_CmdList->SetGraphicsRootConstantBufferView(1, cbvAddr);

        if (mr->m_TexIndex >= 0)
            m_CmdList->SetGraphicsRootDescriptorTable(0, m_TexMgr->GetSRV(mr->m_TexIndex));
        m_CmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // 2-2. FBXモデル → 個別バッファ＋vertexInfoあり
        if (mr->m_ModelBuffer && mr->m_VertexInfo) {
            auto vbv = mr->m_ModelBuffer->GetVertexBufferView();
            auto ibv = mr->m_ModelBuffer->GetIndexBufferView();
            m_CmdList->IASetVertexBuffers(0, 1, &vbv);
            m_CmdList->IASetIndexBuffer(&ibv);
            m_CmdList->DrawIndexedInstanced((UINT)mr->m_VertexInfo->indices.size(), 1, 0, 0, 0);
        }
        // 2-3. Cube, Quad, Sphereなど → EngineManagerの共通バッファ参照（vertexInfo==nullptr）
        else if (mr->m_ModelBuffer && mr->m_VertexInfo == nullptr) {
            auto vbv = mr->m_ModelBuffer->GetVertexBufferView();
            auto ibv = mr->m_ModelBuffer->GetIndexBufferView();
            m_CmdList->IASetVertexBuffers(0, 1, &vbv);
            m_CmdList->IASetIndexBuffer(&ibv);

            // Cubeの場合：36, Quadなら6, Sphereなら動的に管理してもOK
            int indexCount = 36; // Cubeの場合
            // Sphere/Quadなら別途管理（ex: obj->nameやフラグで分岐可）

            m_CmdList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
        }
        // 万一バッファがなければ何もしない
        return;
    }

    // 3. UI画像
    if (auto* uiImage = obj->GetComponent<UIImage>()) {
        DrawUIImage(uiImage, idx);
        return;
    }
}




// フレーム終了
void Renderer::EndFrame() {
    // バリア設定
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = m_SwapMgr->GetBackBuffer(m_BackBufferIndex);
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_CmdList->ResourceBarrier(1, &barrier);

    // コマンドリスト実行とPresent
    m_CmdList->Close();
    ID3D12CommandList* commandLists[] = { m_CmdList };
    m_DeviceMgr->GetCommandQueue()->ExecuteCommandLists(1, commandLists);
    m_DeviceMgr->WaitForGpu();
    m_SwapMgr->GetSwapChain()->Present(1, 0);

    // コマンドアロケータ/リストをリセット
    m_DeviceMgr->GetCommandAllocator()->Reset();
    m_CmdList->Reset(m_DeviceMgr->GetCommandAllocator(), nullptr);
}

// UI専用描画
void Renderer::DrawUIImage(UIImage* image, size_t idx) {
    // ★UI専用PSO/RootSignatureに切り替え
    m_CmdList->SetPipelineState(m_PipeMgr->GetPipelineStateUI());
    m_CmdList->SetGraphicsRootSignature(m_PipeMgr->GetRootSignatureUI());
    ID3D12DescriptorHeap* heaps[] = { m_TexMgr->GetSRVHeap() };
    m_CmdList->SetDescriptorHeaps(_countof(heaps), heaps);

    // テクスチャSRV
    if (image->m_TexIndex >= 0)
        m_CmdList->SetGraphicsRootDescriptorTable(0, m_TexMgr->GetSRV(image->m_TexIndex));

    // --- 左上基準・ピクセル指定のNDC変換 ---
    // 画面解像度
    float screenW = m_Width;
    float screenH = m_Height;

    // ピクセル座標（左上）
    float px = image->m_Position.x;
    float py = image->m_Position.y;
    float sx = image->m_Size.x;
    float sy = image->m_Size.y;

    // 左上と右下をNDCに変換
    float ndcL = (px / screenW) * 2.0f - 1.0f;
    float ndcT = 1.0f - (py / screenH) * 2.0f;
    float ndcR = ((px + sx) / screenW) * 2.0f - 1.0f;
    float ndcB = 1.0f - ((py + sy) / screenH) * 2.0f;

    // スケール・平行移動計算
    float ndcW = ndcR - ndcL;
    float ndcH = ndcB - ndcT;

    // 左上(NDC)へ平行移動 → 幅・高さでスケール
    DirectX::XMMATRIX world =
        DirectX::XMMatrixScaling(ndcW, ndcH, 1.0f) *
        DirectX::XMMatrixTranslation(ndcL, ndcT, 0);

    // 定数バッファ
    ObjectCB cbData{};
    cbData.WorldViewProj = DirectX::XMMatrixTranspose(world);
    cbData.Color = image->m_Color;
    cbData.UseTexture = (image->m_TexIndex >= 0) ? 1 : 0;

    constexpr size_t CBV_SIZE = 256;
    void* mapped = nullptr;
    m_QuadBufferMgr->GetConstantBuffer()->Map(0, nullptr, &mapped);
    memcpy((char*)mapped + CBV_SIZE * idx, &cbData, sizeof(cbData));
    m_QuadBufferMgr->GetConstantBuffer()->Unmap(0, nullptr);

    D3D12_GPU_VIRTUAL_ADDRESS cbvAddr = m_QuadBufferMgr->GetConstantBufferGPUAddress() + CBV_SIZE * idx;
    m_CmdList->SetGraphicsRootConstantBufferView(1, cbvAddr);

    // 頂点・インデックスバッファ（左上基準1x1のQuadを使うこと！）
    auto vbv = m_QuadBufferMgr->GetVertexBufferView();
    auto ibv = m_QuadBufferMgr->GetIndexBufferView();
    m_CmdList->IASetVertexBuffers(0, 1, &vbv);
    m_CmdList->IASetIndexBuffer(&ibv);
    m_CmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 描画
    m_CmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}


void Renderer::DrawSkySphere(GameObject* obj, size_t idx, const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj)
{
    constexpr size_t CBV_SIZE = 256;
    // スカイドーム専用PSO/RootSignatureに切り替え（必要に応じて用意）
    // 例: m_pipeMgr->GetPipelineStateSkyDome(), m_pipeMgr->GetRootSignatureSkyDome()
    m_CmdList->SetPipelineState(m_PipeMgr->GetPipelineStateSkyDome());
    m_CmdList->SetGraphicsRootSignature(m_PipeMgr->GetRootSignatureSkyDome());

    // 定数バッファアドレス
    D3D12_GPU_VIRTUAL_ADDRESS cbvAddr = m_CubeBufMgr->GetConstantBufferGPUAddress() + CBV_SIZE * idx;
    m_CmdList->SetGraphicsRootConstantBufferView(1, cbvAddr);

    // テクスチャ
    auto* mr = obj->GetComponent<StaticMeshRenderer>();
    if (mr && mr->m_TexIndex >= 0)
        m_CmdList->SetGraphicsRootDescriptorTable(0, m_TexMgr->GetSRV(mr->m_TexIndex));

    m_CmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 頂点・インデックスバッファ（球体バッファを使う）
    auto vbv = m_SkyBufferMgr->GetVertexBufferView();
    auto ibv = m_SkyBufferMgr->GetIndexBufferView();
    m_CmdList->IASetVertexBuffers(0, 1, &vbv);
    m_CmdList->IASetIndexBuffer(&ibv);

    // 球体のインデックス数（MeshLibraryで使ってる値と合わせる）
    m_CmdList->DrawIndexedInstanced(32 * 64 * 6, 1, 0, 0, 0);
}

void Renderer::DrawSoccerBall(GameObject* obj, size_t idx, const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj)
{
    constexpr size_t CBV_SIZE = 256;

    // パイプライン設定（通常のPSO/RootSigを使用）
    m_CmdList->SetPipelineState(m_PipeMgr->GetPipelineState(false));
    m_CmdList->SetGraphicsRootSignature(m_PipeMgr->GetRootSignature(false));

    // 定数バッファアドレス（通常のCubeと共用でOK）
    D3D12_GPU_VIRTUAL_ADDRESS cbvAddr = m_CubeBufMgr->GetConstantBufferGPUAddress() + CBV_SIZE * idx;
    m_CmdList->SetGraphicsRootConstantBufferView(1, cbvAddr);

    // テクスチャがあればSRVを設定
    auto* mr = obj->GetComponent<StaticMeshRenderer>();
    if (mr && mr->m_TexIndex >= 0)
        m_CmdList->SetGraphicsRootDescriptorTable(0, m_TexMgr->GetSRV(mr->m_TexIndex));

    m_CmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 球体のVB/IB（m_sphereBufferMgrを使う）
    auto vbv = m_SphereBufferMgr->GetVertexBufferView();
    auto ibv = m_SphereBufferMgr->GetIndexBufferView();
    m_CmdList->IASetVertexBuffers(0, 1, &vbv);
    m_CmdList->IASetIndexBuffer(&ibv);

    // インデックス数はスカイドームと同じ（32×64×6）
    m_CmdList->DrawIndexedInstanced(32 * 64 * 6, 1, 0, 0, 0);
}
