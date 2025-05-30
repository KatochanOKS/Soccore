#include "EngineManager.h"
#include "BufferManager.h" // Vertexを使うため

void EngineManager::Initialize() {
    // DeviceManager初期化
    m_deviceManager.Initialize();

    // SwapChainManager初期化
    auto* device = m_deviceManager.GetDevice();
    auto* cmdQueue = m_deviceManager.GetCommandQueue();
    m_swapChainManager.Initialize(m_hWnd, device, cmdQueue, 1280, 720);
    m_depthBufferManager.Initialize(device, 1280, 720);
    m_pipelineManager.Initialize(device, L"assets/VertexShader.cso", L"assets/PixelShader.cso");

    // ここでcmdListを取得
    auto* cmdList = m_deviceManager.GetCommandList();

    // テクスチャマネージャ初期化＆テクスチャロード
    m_textureManager.Initialize(device);
    m_texIdx = m_textureManager.LoadTexture(L"assets/penguin1.png", m_deviceManager.GetCommandList());

    // 必要なバッファ・リソースはここでまとめて呼ぶ
    CreateTestQuad();
    // LoadPenguinModel();  // など今後どんどん追加でOK
}


void EngineManager::Start() {
    // ゲーム開始時の処理（必要なければ空でOK）
}

void EngineManager::Update() {
    // 毎フレーム更新処理
}

void EngineManager::Draw() {
    // 1. バックバッファ取得
    UINT backBufferIndex = m_swapChainManager.GetSwapChain()->GetCurrentBackBufferIndex();
    auto* cmdList = m_deviceManager.GetCommandList();

    // 2. バリアで描画用に切り替え
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = m_swapChainManager.GetBackBuffer(backBufferIndex);
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    cmdList->ResourceBarrier(1, &barrier);

    // 3. RTV/DSVハンドル取得
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_swapChainManager.GetRTVHeap()->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += backBufferIndex * m_swapChainManager.GetRTVHeapSize();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_depthBufferManager.GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();

    // 4. レンダーターゲット設定
    cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // 5. クリア
    const float clearColor[] = { 0.1f, 0.3f, 0.6f, 1.0f };
    cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // 6. ビューポート・シザー矩形の設定
    float width = static_cast<float>(m_swapChainManager.GetWidth());
    float height = static_cast<float>(m_swapChainManager.GetHeight());
    D3D12_VIEWPORT viewport = { 0, 0, width, height, 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, (LONG)width, (LONG)height };
    cmdList->RSSetViewports(1, &viewport);
    cmdList->RSSetScissorRects(1, &scissorRect);

    // 7. パイプライン・ルートシグネチャ設定
    cmdList->SetPipelineState(m_pipelineManager.GetPipelineState());
    cmdList->SetGraphicsRootSignature(m_pipelineManager.GetRootSignature());

    // 8. SRVヒープセット & ルートパラメータ0番にSRVをバインド
    ID3D12DescriptorHeap* heaps[] = { m_textureManager.GetSRVHeap() };
    cmdList->SetDescriptorHeaps(_countof(heaps), heaps);
    cmdList->SetGraphicsRootDescriptorTable(0, m_textureManager.GetSRV(m_texIdx));

    // 9. 四角形描画コマンド
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    D3D12_VERTEX_BUFFER_VIEW vbv = m_bufferManager.GetVertexBufferView();
    D3D12_INDEX_BUFFER_VIEW ibv = m_bufferManager.GetIndexBufferView();
    cmdList->IASetVertexBuffers(0, 1, &vbv);
    cmdList->IASetIndexBuffer(&ibv);
    cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);

    // 10. バリアでPRESENTに戻す
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    cmdList->ResourceBarrier(1, &barrier);

    // 11. コマンドリスト実行
    cmdList->Close();
    ID3D12CommandList* commandLists[] = { cmdList };
    m_deviceManager.GetCommandQueue()->ExecuteCommandLists(1, commandLists);
    m_deviceManager.WaitForGpu(); // GPU完了待ち

    // 12. Present
    m_swapChainManager.GetSwapChain()->Present(1, 0);

    // 13. コマンドアロケータ/リストリセット
    m_deviceManager.GetCommandAllocator()->Reset();
    cmdList->Reset(m_deviceManager.GetCommandAllocator(), nullptr);
}




void EngineManager::Shutdown() {
    // 終了処理
    m_deviceManager.Cleanup();
	m_swapChainManager.Cleanup();
}


//================================================================================================================================

void EngineManager::CreateTestQuad() {
    // アスペクト比補正
    float width = static_cast<float>(m_swapChainManager.GetWidth());
    float height = static_cast<float>(m_swapChainManager.GetHeight());
    float aspect = width / height;

    // アスペクト補正済みの頂点座標
    std::vector<Vertex> vertices = {
        { -0.5f / aspect, -0.5f, 0.0f, 0.0f, 1.0f }, // 左下
        { -0.5f / aspect,  0.5f, 0.0f, 0.0f, 0.0f }, // 左上
        {  0.5f / aspect, -0.5f, 0.0f, 1.0f, 1.0f }, // 右下
        {  0.5f / aspect,  0.5f, 0.0f, 1.0f, 0.0f }, // 右上
    };
    std::vector<uint16_t> indices = { 0, 1, 2, 2, 1, 3 };
    m_bufferManager.CreateVertexBuffer(m_deviceManager.GetDevice(), vertices);
    m_bufferManager.CreateIndexBuffer(m_deviceManager.GetDevice(), indices);
}

