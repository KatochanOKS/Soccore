#include "EngineManager.h"
#include "BufferManager.h" // Vertexを使うため
#include <DirectXMath.h>
using namespace DirectX;

// エンジンの初期化処理
void EngineManager::Initialize() {
    // DeviceManagerの初期化（GPUデバイス・コマンド周り）
    m_deviceManager.Initialize();

    // SwapChainManagerの初期化（ウィンドウ・バックバッファ管理）
    auto* device = m_deviceManager.GetDevice();
    auto* cmdQueue = m_deviceManager.GetCommandQueue();
    m_swapChainManager.Initialize(m_hWnd, device, cmdQueue, 1280, 720);

    // デプスバッファ（奥行きテスト用）
    m_depthBufferManager.Initialize(device, 1280, 720);

    // パイプライン・シェーダ初期化
    m_pipelineManager.Initialize(device, L"assets/VertexShader.cso", L"assets/PixelShader.cso");

    // 定数バッファ（CBV）を「4個分」まとめて確保（キューブ4つ分の行列）
    m_bufferManager.CreateConstantBuffer(m_deviceManager.GetDevice(), sizeof(XMMATRIX) * 4);

    // コマンドリスト取得（リソース作成用）
    auto* cmdList = m_deviceManager.GetCommandList();

    // テクスチャマネージャ初期化＆画像ロード（penguin1.pngをSRVとしてセット）
    m_textureManager.Initialize(device);
    m_texIdx = m_textureManager.LoadTexture(L"assets/penguin1.png", m_deviceManager.GetCommandList());

    // --- ここからシーンに配置するキューブを設定 ---
    m_cubeTransforms.clear();

    // キューブ1
    Transform t1;
    t1.position = XMFLOAT3(-2.0f, 0.0f, 0.0f); // 左
    t1.scale = XMFLOAT3(1.0f, 1.0f, 1.0f);     // 標準サイズ
    m_cubeTransforms.push_back(t1);

    // キューブ2
    Transform t2;
    t2.position = XMFLOAT3(0.0f, 5.0f, 0.0f);  // 上にある
    t2.scale = XMFLOAT3(1.0f, 1.0f, 1.0f);     // 標準サイズ
    m_cubeTransforms.push_back(t2);

    // キューブ3
    Transform t3;
    t3.position = XMFLOAT3(2.0f, 2.0f, 2.0f);  // 右奥
    t3.scale = XMFLOAT3(1.0f, 1.0f, 1.0f);     // 標準サイズ
    m_cubeTransforms.push_back(t3);

    // キューブ4
    Transform t4;
    t4.position = XMFLOAT3(-5.0f, 6.0f, 0.0f); // 左上
    t4.scale = XMFLOAT3(2.0f, 2.0f, 2.0f);     // 大きめ
    m_cubeTransforms.push_back(t4);

    // 頂点・インデックスバッファ生成（立方体1個分のみ作ればOK）
    CreateTestCube();
}

void EngineManager::Start() {
    // ゲーム開始時の処理（使わなければ空でOK）
}

void EngineManager::Update() {
    // 毎フレームの更新処理（使わなければ空でOK）
}

// メイン描画関数
void EngineManager::Draw() {
    // 1. 今描画するバックバッファ番号取得、コマンドリスト取得
    UINT backBufferIndex = m_swapChainManager.GetSwapChain()->GetCurrentBackBufferIndex();
    auto* cmdList = m_deviceManager.GetCommandList();

    // 2. バックバッファの状態を「描画可能」に切り替えるバリア設定
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = m_swapChainManager.GetBackBuffer(backBufferIndex);
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    cmdList->ResourceBarrier(1, &barrier);

    // 3. RTV（カラーバッファ）・DSV（深度バッファ）のハンドル取得
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_swapChainManager.GetRTVHeap()->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += backBufferIndex * m_swapChainManager.GetRTVHeapSize();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_depthBufferManager.GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();

    // 4. レンダーターゲット（RTV/DSV）を設定
    cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // 5. 画面クリア（青色、奥行きもクリア）
    const float clearColor[] = { 0.1f, 0.3f, 0.6f, 1.0f };
    cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // 6. ビューポート・シザー設定（画面全体）
    float width = static_cast<float>(m_swapChainManager.GetWidth());
    float height = static_cast<float>(m_swapChainManager.GetHeight());
    D3D12_VIEWPORT viewport = { 0, 0, width, height, 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, (LONG)width, (LONG)height };
    cmdList->RSSetViewports(1, &viewport);
    cmdList->RSSetScissorRects(1, &scissorRect);

    // 7. パイプラインステート・ルートシグネチャ設定（forループの外で1回だけ！）
    cmdList->SetPipelineState(m_pipelineManager.GetPipelineState());
    cmdList->SetGraphicsRootSignature(m_pipelineManager.GetRootSignature());

    // 8. テクスチャ（SRV）をシェーダにバインド
    ID3D12DescriptorHeap* heaps[] = { m_textureManager.GetSRVHeap() };
    cmdList->SetDescriptorHeaps(_countof(heaps), heaps);
    cmdList->SetGraphicsRootDescriptorTable(0, m_textureManager.GetSRV(m_texIdx));

    // 9. カメラの行列（ビュー・プロジェクション）
    static float angle = 0.0f;
    angle += 0.01f; // 画面全体を回転させるための角度
    XMMATRIX view = XMMatrixLookAtLH(
        XMVectorSet(0, 10, -10, 1),  // カメラ位置
        XMVectorSet(0, 0, 0, 1),    // 注視点
        XMVectorSet(0, 1, 0, 0)     // 上方向
    );
    XMMATRIX proj = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(60.0f),
        width / height,
        0.1f, 100.0f
    );

    // 10. 定数バッファをMap（forの外で一度だけ）
    void* mapped = nullptr;
    HRESULT hr = m_bufferManager.GetConstantBuffer()->Map(0, nullptr, &mapped);
    if (FAILED(hr) || mapped == nullptr) return;

    // 11. すべてのキューブをfor文で描画
    for (int i = 0; i < m_cubeTransforms.size(); ++i) {
        const auto& t = m_cubeTransforms[i];
        // ワールド行列（全体をY軸回転させる）
        XMMATRIX world = XMMatrixRotationY(angle) * t.GetWorldMatrix();
        XMMATRIX wvp = XMMatrixTranspose(world * view * proj);

        // CBVバッファ内の自分の領域へ行列を書き込み
        memcpy((char*)mapped + sizeof(XMMATRIX) * i, &wvp, sizeof(wvp));

        // 自分用の定数バッファアドレスをバインド
        D3D12_GPU_VIRTUAL_ADDRESS cbvAddress = m_bufferManager.GetConstantBufferGPUAddress() + sizeof(XMMATRIX) * i;
        cmdList->SetGraphicsRootConstantBufferView(1, cbvAddress);

        // バッファバインド・描画コマンド発行
        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        D3D12_VERTEX_BUFFER_VIEW vbv = m_bufferManager.GetVertexBufferView();
        D3D12_INDEX_BUFFER_VIEW ibv = m_bufferManager.GetIndexBufferView();
        cmdList->IASetVertexBuffers(0, 1, &vbv);
        cmdList->IASetIndexBuffer(&ibv);
        cmdList->DrawIndexedInstanced(36, 1, 0, 0, 0);
    }
    // 定数バッファアンマップ
    m_bufferManager.GetConstantBuffer()->Unmap(0, nullptr);

    // 12. バリアでPresent用に切り替え
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    cmdList->ResourceBarrier(1, &barrier);

    // 13. コマンドリストをGPUに実行させる
    cmdList->Close();
    ID3D12CommandList* commandLists[] = { cmdList };
    m_deviceManager.GetCommandQueue()->ExecuteCommandLists(1, commandLists);
    m_deviceManager.WaitForGpu();

    // 14. 画面を表示（スワップチェインのPresent）
    m_swapChainManager.GetSwapChain()->Present(1, 0);

    // 15. コマンドアロケータ/リストをリセットして次のフレーム準備
    m_deviceManager.GetCommandAllocator()->Reset();
    cmdList->Reset(m_deviceManager.GetCommandAllocator(), nullptr);
}

// 終了時の後始末
void EngineManager::Shutdown() {
    m_deviceManager.Cleanup();
    m_swapChainManager.Cleanup();
}

// 頂点・インデックスバッファを作成し、立方体モデルデータを格納
void EngineManager::CreateTestCube() {
    std::vector<Vertex> vertices = {
        // 前面（テクスチャを貼る面）
        { -0.5f, -0.5f, -0.5f, 0, 0.5f }, // 左下
        { -0.5f,  0.5f, -0.5f, 0, 0 }, // 左上
        {  0.5f,  0.5f, -0.5f, 0.5f, 0 }, // 右上
        {  0.5f, -0.5f, -0.5f, 0.5f, 0.5f }, // 右下
        // 右面（単色：UVは0,0で固定）
        { 0.5f, -0.5f, -0.5f, 0, 1 },
        { 0.5f,  0.5f, -0.5f, 0, 0 },
        { 0.5f,  0.5f,  0.5f, 1, 0 },
        { 0.5f, -0.5f,  0.5f, 1, 1 },
        // 後面
        { 0.5f, -0.5f,  0.5f, 0, 0 },
        { 0.5f,  0.5f,  0.5f, 0, 0 },
        { -0.5f,  0.5f,  0.5f, 0, 0 },
        { -0.5f, -0.5f,  0.5f, 0, 0 },
        // 左面
        { -0.5f, -0.5f,  0.5f, 0, 0 },
        { -0.5f,  0.5f,  0.5f, 0, 0 },
        { -0.5f,  0.5f, -0.5f, 0, 0 },
        { -0.5f, -0.5f, -0.5f, 0, 0 },
        // 上面
        { -0.5f,  0.5f, -0.5f, 0, 0 },
        { -0.5f,  0.5f,  0.5f, 0, 0 },
        { 0.5f,  0.5f,  0.5f, 0, 0 },
        { 0.5f,  0.5f, -0.5f, 0, 0 },
        // 下面
        { -0.5f, -0.5f, -0.5f, 0, 0 },
        { -0.5f, -0.5f,  0.5f, 0, 0 },
        { 0.5f, -0.5f,  0.5f, 0, 0 },
        { 0.5f, -0.5f, -0.5f, 0, 0 },
    };
    std::vector<uint16_t> indices = {
        // 各面 2三角形
        0,1,2, 0,2,3,      // 前面
        4,5,6, 4,6,7,      // 右面
        8,9,10, 8,10,11,   // 後面
        12,13,14, 12,14,15,// 左面
        16,17,18, 16,18,19,// 上面
        20,21,22, 20,22,23 // 下面
    };


    m_bufferManager.CreateVertexBuffer(m_deviceManager.GetDevice(), vertices);
    m_bufferManager.CreateIndexBuffer(m_deviceManager.GetDevice(), indices);
}
