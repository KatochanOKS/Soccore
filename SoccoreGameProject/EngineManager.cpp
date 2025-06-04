#include "EngineManager.h"
#include "BufferManager.h" // Vertexを使うため
#include <DirectXMath.h>
#include"GameObject.h"
using namespace DirectX;
using namespace Colors;
void EngineManager::Initialize() {
    // 定数バッファ用の構造体（16バイト境界のためパディング不要、float4x4+float4）
   
    m_deviceManager.Initialize();
    auto* device = m_deviceManager.GetDevice();
    auto* cmdQueue = m_deviceManager.GetCommandQueue();
    m_swapChainManager.Initialize(m_hWnd, device, cmdQueue, 1280, 720);
    m_depthBufferManager.Initialize(device, 1280, 720);
    m_pipelineManager.Initialize(device, L"assets/VertexShader.cso", L"assets/PixelShader.cso");

    // 画像ロード
    m_textureManager.Initialize(device);
    m_texIdx = m_textureManager.LoadTexture(L"assets/penguin1.png", m_deviceManager.GetCommandList());
    m_cubeTexIdx= m_textureManager.LoadTexture(L"assets/penguin2.png", m_deviceManager.GetCommandList()); // 追加

    // GameObject生成（地面＋複数Cube）
    m_gameObjects.clear();

    // --- 地面オブジェクト ---
    auto* ground = new GameObject("Ground", 0, m_texIdx, Red);   // 地面：Ground.png
    ground->transform.position = XMFLOAT3(0, -1.0f, 0);
    ground->transform.scale = XMFLOAT3(50.0f, 0.2f, 50.0f);
    m_gameObjects.push_back(ground);

    // --- キューブオブジェクト ---
    auto* cube1 = new GameObject("Cube1", 0, m_cubeTexIdx, White); // Cube.png
    cube1->transform.position = XMFLOAT3(-2, 0, 0);
    cube1->transform.scale = XMFLOAT3(1, 1, 1);
    m_gameObjects.push_back(cube1);

    auto* cube2 = new GameObject("Cube2", 0, m_cubeTexIdx, White); // Cube.png
    cube2->transform.position = XMFLOAT3(2, 2, 2);
    cube2->transform.scale = XMFLOAT3(1, 1, 1);
    m_gameObjects.push_back(cube2);

    // ...追加Cubeも同様に...

    // 定数バッファは「256バイト×オブジェクト数」
    constexpr size_t CBV_SIZE = 256;
    m_bufferManager.CreateConstantBuffer(device, CBV_SIZE * m_gameObjects.size());
    CreateTestCube(); // 立方体のバッファ作成（地面もスケールで流用）
}

void EngineManager::Start() {
    // ゲーム開始時の処理（使わなければ空でOK）
}

void EngineManager::Update() {
    // 毎フレームの更新処理（使わなければ空でOK）
}

// メイン描画関数
// エンジンの描画処理（完璧版！）
void EngineManager::Draw() {
    // 1. バックバッファ・コマンドリスト取得
    UINT backBufferIndex = m_swapChainManager.GetSwapChain()->GetCurrentBackBufferIndex();
    auto* cmdList = m_deviceManager.GetCommandList();

    // 2. バリア設定（Present→RenderTarget）
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = m_swapChainManager.GetBackBuffer(backBufferIndex);
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    cmdList->ResourceBarrier(1, &barrier);

    // 3. RTV/DSV設定
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_swapChainManager.GetRTVHeap()->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += backBufferIndex * m_swapChainManager.GetRTVHeapSize();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_depthBufferManager.GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
    cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // 4. 画面クリア
    const float clearColor[] = { 0.1f, 0.3f, 0.6f, 1.0f };
    cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // 5. ビューポート・シザー設定
    float width = static_cast<float>(m_swapChainManager.GetWidth());
    float height = static_cast<float>(m_swapChainManager.GetHeight());
    D3D12_VIEWPORT viewport = { 0, 0, width, height, 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, (LONG)width, (LONG)height };
    cmdList->RSSetViewports(1, &viewport);
    cmdList->RSSetScissorRects(1, &scissorRect);

    // 6. パイプライン・ルートシグネチャ・SRVヒープ設定
    cmdList->SetPipelineState(m_pipelineManager.GetPipelineState());
    cmdList->SetGraphicsRootSignature(m_pipelineManager.GetRootSignature());
    ID3D12DescriptorHeap* heaps[] = { m_textureManager.GetSRVHeap() };
    cmdList->SetDescriptorHeaps(_countof(heaps), heaps);

    // 7. カメラ行列
    static float angle = 0.0f;
    angle += 0.01f;
    XMMATRIX view = XMMatrixLookAtLH(
        XMVectorSet(0, 9, -20, 1), // カメラ位置
        XMVectorSet(0, 0, 0, 1),   // 注視点
        XMVectorSet(0, 1, 0, 0)    // 上方向
    );
    XMMATRIX proj = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(60.0f),
        width / height,
        0.1f, 100.0f
    );

    // 8. 回転値をTransformへ（地面もCubeもここで回転可能！）
    for (auto* obj : m_gameObjects) {
        // 例：すべてのオブジェクトを回転させる場合
        obj->transform.rotation.y = angle;

         if (obj->name == "Ground") obj->transform.rotation.y = 0;
         else obj->transform.rotation.y = angle; 
    }

    // 9. 定数バッファMap（256バイトアライン）
    void* mapped = nullptr;
    HRESULT hr = m_bufferManager.GetConstantBuffer()->Map(0, nullptr, &mapped);
    if (FAILED(hr) || !mapped) return;
    
    // CBVサイズは256固定
    constexpr size_t CBV_SIZE = 256;

    for (size_t i = 0; i < m_gameObjects.size(); ++i) {
        // 1. 各GameObjectの取得
        GameObject* obj = m_gameObjects[i];

        // 2. 定数バッファ（ObjectCB）を準備
        ObjectCB cb;
        XMMATRIX world = obj->transform.GetWorldMatrix(); // ワールド行列を取得

        // 行列をGPU用に転置し、View/Projをかけてセット
        cb.WorldViewProj = XMMatrixTranspose(world * view * proj);

        // オブジェクト固有の色情報をセット
        cb.Color = obj->color;

        // 定数バッファ領域へデータコピー（各オブジェクト分ずらして書き込む）
        memcpy((char*)mapped + CBV_SIZE * i, &cb, sizeof(cb));

        // 3. 定数バッファのGPUアドレスを計算してルートCBVにセット
        D3D12_GPU_VIRTUAL_ADDRESS cbvAddr = m_bufferManager.GetConstantBufferGPUAddress() + CBV_SIZE * i;
        cmdList->SetGraphicsRootConstantBufferView(1, cbvAddr); // ルートパラメータ1にバインド

        // 4. テクスチャが設定されている場合のみSRVバインド
        if (obj->texIndex >= 0) {
            // テクスチャSRVをルートパラメータ0にセット
            cmdList->SetGraphicsRootDescriptorTable(0, m_textureManager.GetSRV(obj->texIndex));
        }
        // 5. 描画コマンド（プリミティブ・バッファバインド）
        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // 頂点バッファビューをセット
        D3D12_VERTEX_BUFFER_VIEW vbv = m_bufferManager.GetVertexBufferView();
        D3D12_INDEX_BUFFER_VIEW ibv = m_bufferManager.GetIndexBufferView();
        cmdList->IASetVertexBuffers(0, 1, &vbv);
        cmdList->IASetIndexBuffer(&ibv);

        // インデックスド描画（36頂点＝12三角形＝立方体想定）
        cmdList->DrawIndexedInstanced(36, 1, 0, 0, 0);
    }


    m_bufferManager.GetConstantBuffer()->Unmap(0, nullptr);

    // 11. バリアでPRESENTに戻す
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    cmdList->ResourceBarrier(1, &barrier);

    // 12. コマンドリスト実行とPresent
    cmdList->Close();
    ID3D12CommandList* commandLists[] = { cmdList };
    m_deviceManager.GetCommandQueue()->ExecuteCommandLists(1, commandLists);
    m_deviceManager.WaitForGpu();
    m_swapChainManager.GetSwapChain()->Present(1, 0);

    // 13. コマンドアロケータ/リストをリセットして次フレーム準備
    m_deviceManager.GetCommandAllocator()->Reset();
    cmdList->Reset(m_deviceManager.GetCommandAllocator(), nullptr);
}


// 終了時の後始末
// 終了時の後始末
void EngineManager::Shutdown() {
    // GameObjectの動的メモリをすべて解放
    for (auto* obj : m_gameObjects) delete obj;
    m_gameObjects.clear();

    // DX12リソースのクリーンアップ
    m_deviceManager.Cleanup();
    m_swapChainManager.Cleanup();
    // 必要に応じて他のManager（TextureManagerなど）もCleanup()
}


// 頂点・インデックスバッファを作成し、立方体モデルデータを格納
void EngineManager::CreateTestCube() {
    std::vector<Vertex> vertices = {
        // 前面（テクスチャを貼る面）
        { -0.5f, -0.5f, -0.5f, 0, 0 }, // 左下
        { -0.5f,  0.5f, -0.5f, 0, 0 }, // 左上
        {  0.5f,  0.5f, -0.5f, 0, 0 }, // 右上
        {  0.5f, -0.5f, -0.5f, 0, 0 }, // 右下
        // 右面（単色：UVは0,0で固定）
        { 0.5f, -0.5f, -0.5f, 0, 0},
        { 0.5f,  0.5f, -0.5f, 0, 0 },
        { 0.5f,  0.5f,  0.5f, 0, 0 },
        { 0.5f, -0.5f,  0.5f, 0, 0 },
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
        { -0.5f,  0.5f, -0.5f, 0, 1 },
        { -0.5f,  0.5f,  0.5f, 0, 0 },
        { 0.5f,  0.5f,  0.5f, 1, 0 },
        { 0.5f,  0.5f, -0.5f, 1, 1 },
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
