#include "Renderer.h"
#include "d3dx12.h"
#include "Transform.h"
#include "StaticMeshRenderer.h"   // ← 追加！
#include "SkinnedMeshRenderer.h"  // ← 追加！
#include"EngineManager.h"
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
	BufferManager* quadBufMgr, // Quad用バッファ
    FbxModelLoader::VertexInfo* modelVertexInfo
) {
    m_deviceMgr = deviceMgr;
    m_swapMgr = swapMgr;
    m_depthMgr = depthMgr;
    m_pipeMgr = pipeMgr;
    m_texMgr = texMgr;
    m_cubeBufMgr = cubeBufMgr;
    m_modelBufMgr = modelBufMgr;
	m_quadBufferMgr = quadBufMgr; // Quad用バッファを保持
    m_modelVertexInfo = modelVertexInfo;
    m_width = static_cast<float>(m_swapMgr->GetWidth());
    m_height = static_cast<float>(m_swapMgr->GetHeight());

    // --- スキニング用CBVバッファ作成（80ボーン想定）
    m_skinCBSize = sizeof(DirectX::XMMATRIX) * 80; // 必要に応じて調整
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC cbDesc = CD3DX12_RESOURCE_DESC::Buffer((m_skinCBSize + 255) & ~255); // 256アライン

    m_deviceMgr->GetDevice()->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &cbDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&m_skinningConstantBuffer)
    );
    m_skinCBGpuAddr = m_skinningConstantBuffer->GetGPUVirtualAddress();
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

// 描画処理（型ごとに分岐！）
void Renderer::DrawObject(GameObject* obj, size_t idx, const XMMATRIX& view, const XMMATRIX& proj) {
    constexpr size_t CBV_SIZE = 256;

    // 1. スキニングメッシュ（アニメ付きモデル）
    if (auto* smr = obj->GetComponent<SkinnedMeshRenderer>()) {
        m_cmdList->SetPipelineState(m_pipeMgr->GetPipelineState(true));
        m_cmdList->SetGraphicsRootSignature(m_pipeMgr->GetRootSignature(true));

        if (smr->texIndex >= 0)
            m_cmdList->SetGraphicsRootDescriptorTable(0, m_texMgr->GetSRV(smr->texIndex));

        XMMATRIX world = obj->GetComponent<Transform>()->GetWorldMatrix();
        XMMATRIX wvp = XMMatrixTranspose(world * view * proj);

        // スキニング行列取得（アニメーター必須！）
        std::vector<XMMATRIX> skinnedMats;
        if (smr->animator && smr->skinVertexInfo) {
            skinnedMats = smr->animator->GetSkinnedPose(smr->skinVertexInfo->bindPoses);
            for (auto& m : skinnedMats)
                m = XMMatrixTranspose(m);
        }

        // ボーン行列＋WVPを定数バッファに書き込む
        void* mapped = nullptr;
        if (SUCCEEDED(m_skinningConstantBuffer->Map(0, nullptr, &mapped))) {
            memcpy((char*)mapped, &wvp, sizeof(XMMATRIX)); // b0: WVP
            if (!skinnedMats.empty())
                memcpy((char*)mapped + 256, skinnedMats.data(), sizeof(XMMATRIX) * skinnedMats.size()); // b1: Bone array
            m_skinningConstantBuffer->Unmap(0, nullptr);

            m_cmdList->SetGraphicsRootConstantBufferView(1, m_skinCBGpuAddr);           // b0
            m_cmdList->SetGraphicsRootConstantBufferView(2, m_skinCBGpuAddr + 256);     // b1
        }

        if (smr->modelBuffer && smr->skinVertexInfo) {
            auto vbv = smr->modelBuffer->GetVertexBufferView();
            auto ibv = smr->modelBuffer->GetIndexBufferView();
            m_cmdList->IASetVertexBuffers(0, 1, &vbv);
            m_cmdList->IASetIndexBuffer(&ibv);
            m_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            m_cmdList->DrawIndexedInstanced((UINT)smr->skinVertexInfo->indices.size(), 1, 0, 0, 0);
        }
        return;
    }

    // 2. 静的メッシュ（Cubeや通常FBX）
    if (auto* mr = obj->GetComponent<StaticMeshRenderer>()) {
        D3D12_GPU_VIRTUAL_ADDRESS cbvAddr = m_cubeBufMgr->GetConstantBufferGPUAddress() + CBV_SIZE * idx;
        m_cmdList->SetGraphicsRootConstantBufferView(1, cbvAddr);

        if (mr->texIndex >= 0)
            m_cmdList->SetGraphicsRootDescriptorTable(0, m_texMgr->GetSRV(mr->texIndex));
        m_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // FBXモデルの場合
        if (mr->modelBuffer && mr->vertexInfo) {
            auto vbv = mr->modelBuffer->GetVertexBufferView();
            auto ibv = mr->modelBuffer->GetIndexBufferView();
            m_cmdList->IASetVertexBuffers(0, 1, &vbv);
            m_cmdList->IASetIndexBuffer(&ibv);
            m_cmdList->DrawIndexedInstanced((UINT)mr->vertexInfo->indices.size(), 1, 0, 0, 0);
        }
        // Cubeなどの共通バッファの場合
        else {
            auto vbv = m_cubeBufMgr->GetVertexBufferView();
            auto ibv = m_cubeBufMgr->GetIndexBufferView();
            m_cmdList->IASetVertexBuffers(0, 1, &vbv);
            m_cmdList->IASetIndexBuffer(&ibv);
            m_cmdList->DrawIndexedInstanced(36, 1, 0, 0, 0);
        }
        return;
    }

    if (auto* uiImage = obj->GetComponent<UIImage>()) {
        DrawUIImage(uiImage, idx);
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


void Renderer::DrawUIImage(UIImage* image, size_t idx) {
    // ----- 1. パイプライン（通常）を使う -----
    m_cmdList->SetPipelineState(m_pipeMgr->GetPipelineState(false));
    m_cmdList->SetGraphicsRootSignature(m_pipeMgr->GetRootSignature(false));
    ID3D12DescriptorHeap* heaps[] = { m_texMgr->GetSRVHeap() };
    m_cmdList->SetDescriptorHeaps(_countof(heaps), heaps);

    // ----- 2. テクスチャSRV -----
    if (image->texIndex >= 0)
        m_cmdList->SetGraphicsRootDescriptorTable(0, m_texMgr->GetSRV(image->texIndex));

    // ----- 3. NDC変換（ピクセル→-1〜1正規化） -----
    float ndcX = (image->position.x / m_width) * 2.0f - 1.0f;
    float ndcY = 1.0f - (image->position.y / m_height) * 2.0f;
    float ndcW = image->size.x / m_width;
    float ndcH = image->size.y / m_height;

    DirectX::XMMATRIX world =
        DirectX::XMMatrixScaling(ndcW, ndcH, 1.0f) *
        DirectX::XMMatrixTranslation(ndcX, ndcY, 0);

    // ----- 4. 定数バッファ（色・WorldViewProj） -----
    ObjectCB cbData{};
    cbData.WorldViewProj = DirectX::XMMatrixTranspose(world);
    cbData.Color = image->color;
    cbData.UseTexture = (image->texIndex >= 0) ? 1 : 0;

    constexpr size_t CBV_SIZE = 256;
    void* mapped = nullptr;
    m_quadBufferMgr->GetConstantBuffer()->Map(0, nullptr, &mapped);
    memcpy((char*)mapped + CBV_SIZE * idx, &cbData, sizeof(cbData));
    m_quadBufferMgr->GetConstantBuffer()->Unmap(0, nullptr);

    D3D12_GPU_VIRTUAL_ADDRESS cbvAddr = m_quadBufferMgr->GetConstantBufferGPUAddress() + CBV_SIZE * idx;
    m_cmdList->SetGraphicsRootConstantBufferView(1, cbvAddr);

    // ----- 5. 頂点・インデックスバッファ -----
    auto vbv = m_quadBufferMgr->GetVertexBufferView();
    auto ibv = m_quadBufferMgr->GetIndexBufferView();
    m_cmdList->IASetVertexBuffers(0, 1, &vbv);
    m_cmdList->IASetIndexBuffer(&ibv);
    m_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // ----- 6. 描画 -----
    m_cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}
