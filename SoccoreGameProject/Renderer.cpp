#include "Renderer.h"
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
    FbxModelLoader::VertexInfo* modelVertexInfo
) {
    m_deviceMgr = deviceMgr;
    m_swapMgr = swapMgr;
    m_depthMgr = depthMgr;
    m_pipeMgr = pipeMgr;
    m_texMgr = texMgr;
    m_cubeBufMgr = cubeBufMgr;
    m_modelBufMgr = modelBufMgr;
    m_modelVertexInfo = modelVertexInfo;
    m_width = static_cast<float>(m_swapMgr->GetWidth());
    m_height = static_cast<float>(m_swapMgr->GetHeight());
}

// フレーム開始（バッファクリア・ターゲット設定）
void Renderer::BeginFrame() {
    m_backBufferIndex = m_swapMgr->GetSwapChain()->GetCurrentBackBufferIndex();
    m_cmdList = m_deviceMgr->GetCommandList();

    // バリア設定（Present→RenderTarget）
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = m_swapMgr->GetBackBuffer(m_backBufferIndex);
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_cmdList->ResourceBarrier(1, &barrier);

    // RTV/DSV設定
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_swapMgr->GetRTVHeap()->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += m_backBufferIndex * m_swapMgr->GetRTVHeapSize();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_depthMgr->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
    m_cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // 画面クリア
    const float clearColor[] = { 0.1f, 0.3f, 0.6f, 1.0f };
    m_cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // ビューポート・シザー設定
    D3D12_VIEWPORT viewport = { 0, 0, m_width, m_height, 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, (LONG)m_width, (LONG)m_height };
    m_cmdList->RSSetViewports(1, &viewport);
    m_cmdList->RSSetScissorRects(1, &scissorRect);

    // デフォルトは非スキニングパイプライン
    m_cmdList->SetPipelineState(m_pipeMgr->GetPipelineState(false));
    m_cmdList->SetGraphicsRootSignature(m_pipeMgr->GetRootSignature(false));
    ID3D12DescriptorHeap* heaps[] = { m_texMgr->GetSRVHeap() };
    m_cmdList->SetDescriptorHeaps(_countof(heaps), heaps);
}

// 描画処理
void Renderer::DrawObject(GameObject* obj, size_t idx, const XMMATRIX& view, const XMMATRIX& proj) {
    auto* mr = obj->GetComponent<MeshRenderer>();
    if (!mr) return;

    // === スキニングアニメ用メッシュ判定 ===
// meshType==2 などで「スキンメッシュ」判定（ここは設計に合わせて修正OK！）
    if (mr->meshType == 2 && mr->skinVertexInfo && mr->animator) {
        // ここに「アニメーションモデルの描画処理」を追記します！

        // 1. AnimatorからboneMatricesを取得
        const auto& boneMats = mr->animator->GetCurrentPose();

        // --- デバッグ：boneMatsのサイズ・一部の値をprint ---
        char msg[128];
        sprintf_s(msg, "[Debug] boneCount=%zu first[3]=%.2f\n", boneMats.size(), boneMats[0].r[3].m128_f32[0]);
        OutputDebugStringA(msg);

        // 2. 専用のCBVに書き込み（BufferManagerまたは専用リソース、設計により違う）
        // 例：m_skinCB->Map(..., &mapped); memcpy(mapped, boneMats.data(), sizeof(XMMATRIX)*boneMats.size()); Unmap...

        // 3. b1（ボーン行列用スロット）にCBVバインド
        // m_cmdList->SetGraphicsRootConstantBufferView(1, m_skinCB->GetGPUVirtualAddress());

        // 4. 通常通り、SRVや頂点バッファ・インデックスバッファもskinning用をセット
        // 頂点バッファ: mr->skinVertexBufferView
        // インデックスバッファ: mr->skinIndexBufferView

        // m_cmdList->IASetVertexBuffers...
        // m_cmdList->IASetIndexBuffer...
        // m_cmdList->DrawIndexedInstanced(...);
        return;
    }

    constexpr size_t CBV_SIZE = 256;
    void* mapped = nullptr;

    // ----------------------
    // 通常キューブ・モデル描画
    // ----------------------
    if (mr->meshType == 0 || (mr->meshType == 1 && mr->modelBuffer && mr->vertexInfo)) {
        // 定数バッファ設定
        m_cubeBufMgr->GetConstantBuffer()->Map(0, nullptr, &mapped);
        D3D12_GPU_VIRTUAL_ADDRESS cbvAddr = m_cubeBufMgr->GetConstantBufferGPUAddress() + CBV_SIZE * idx;
        m_cmdList->SetGraphicsRootConstantBufferView(1, cbvAddr);

        // SRV（テクスチャ）
        if (mr->texIndex >= 0)
            m_cmdList->SetGraphicsRootDescriptorTable(0, m_texMgr->GetSRV(mr->texIndex));
        m_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // モデル or キューブ描画
        if (mr->meshType == 1) {
            D3D12_VERTEX_BUFFER_VIEW vbv = mr->modelBuffer->GetVertexBufferView();
            D3D12_INDEX_BUFFER_VIEW ibv = mr->modelBuffer->GetIndexBufferView();
            m_cmdList->IASetVertexBuffers(0, 1, &vbv);
            m_cmdList->IASetIndexBuffer(&ibv);
            m_cmdList->DrawIndexedInstanced((UINT)mr->vertexInfo->indices.size(), 1, 0, 0, 0);
        }
        else {
            D3D12_VERTEX_BUFFER_VIEW vbv = m_cubeBufMgr->GetVertexBufferView();
            D3D12_INDEX_BUFFER_VIEW ibv = m_cubeBufMgr->GetIndexBufferView();
            m_cmdList->IASetVertexBuffers(0, 1, &vbv);
            m_cmdList->IASetIndexBuffer(&ibv);
            m_cmdList->DrawIndexedInstanced(36, 1, 0, 0, 0);
        }
        m_cubeBufMgr->GetConstantBuffer()->Unmap(0, nullptr);
        return;
    }
}

// フレーム終了（Present & コマンドリストリセット）
void Renderer::EndFrame() {
    // バリア設定（RenderTarget→Present）
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = m_swapMgr->GetBackBuffer(m_backBufferIndex);
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_cmdList->ResourceBarrier(1, &barrier);

    // コマンドリスト実行とPresent
    m_cmdList->Close();
    ID3D12CommandList* commandLists[] = { m_cmdList };
    m_deviceMgr->GetCommandQueue()->ExecuteCommandLists(1, commandLists);
    m_deviceMgr->WaitForGpu();
    m_swapMgr->GetSwapChain()->Present(1, 0);

    // コマンドアロケータ/リストをリセット
    m_deviceMgr->GetCommandAllocator()->Reset();
    m_cmdList->Reset(m_deviceMgr->GetCommandAllocator(), nullptr);
}
