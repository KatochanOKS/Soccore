#include "EngineManager.h"
#include "Colors.h"
using namespace DirectX;
using namespace Colors;

void EngineManager::Initialize() {
    m_deviceManager.Initialize();
    auto* device = m_deviceManager.GetDevice();
    auto* cmdQueue = m_deviceManager.GetCommandQueue();
    m_swapChainManager.Initialize(m_hWnd, device, cmdQueue, 1280, 720);
    m_depthBufferManager.Initialize(device, 1280, 720);
    m_pipelineManager.Initialize(device, L"assets/VertexShader.cso", L"assets/PixelShader.cso");

    m_textureManager.Initialize(device);
    m_texIdx = m_textureManager.LoadTexture(L"assets/penguin1.png", m_deviceManager.GetCommandList());
    m_cubeTexIdx = m_textureManager.LoadTexture(L"assets/penguin2.png", m_deviceManager.GetCommandList());

    m_gameObjects.clear();

    // FBXモデル読込&バッファ作成
    if (FbxModelLoader::Load("assets/MixamoModel.fbx", &m_modelVertexInfo)) {
        m_modelBufferManager.CreateVertexBuffer(device, m_modelVertexInfo.vertices);
        m_modelBufferManager.CreateIndexBuffer(device, m_modelVertexInfo.indices);
    }

    // FBXモデル用GameObject
    auto* fbxObj = new GameObject("FbxModel", 1, -1, Red);
    fbxObj->transform.position = XMFLOAT3(0, 0, 0);
    fbxObj->transform.scale = XMFLOAT3(0.05f, 0.05f, 0.05f);
    fbxObj->meshType = 1;
    m_gameObjects.push_back(fbxObj);

    // 地面
    auto* ground = new GameObject("Ground", 0, m_texIdx, White);
    ground->transform.position = XMFLOAT3(0, -1.0f, 0);
    ground->transform.scale = XMFLOAT3(50.0f, 0.2f, 50.0f);
    m_gameObjects.push_back(ground);

    // キューブ
    auto* cube1 = new GameObject("Cube1", 0, m_cubeTexIdx, White);
    cube1->transform.position = XMFLOAT3(-2, 0, 0);
    cube1->transform.scale = XMFLOAT3(1, 1, 1);
    m_gameObjects.push_back(cube1);

    auto* cube2 = new GameObject("Cube2", 0, m_cubeTexIdx, White);
    cube2->transform.position = XMFLOAT3(2, 2, 2);
    cube2->transform.scale = XMFLOAT3(1, 1, 1);
    m_gameObjects.push_back(cube2);

    // ...他のオブジェクトも同様に追加...

    // 定数バッファ
    constexpr size_t CBV_SIZE = 256;
    m_bufferManager.CreateConstantBuffer(device, CBV_SIZE * m_gameObjects.size());

    // キューブ用頂点・インデックスバッファ
    CreateTestCube();

    // Rendererに各マネージャを渡して初期化
    m_renderer.Initialize(
        &m_deviceManager,
        &m_swapChainManager,
        &m_depthBufferManager,
        &m_pipelineManager,
        &m_textureManager,
        &m_bufferManager,
        &m_modelBufferManager,
        &m_modelVertexInfo
    );
}

void EngineManager::Start() {}
void EngineManager::Update() {}

void EngineManager::Draw() {
    static float angle = 0.0f;
    angle += 0.01f;
    XMMATRIX view = XMMatrixLookAtLH(
        XMVectorSet(0, 9, -20, 1),
        XMVectorSet(0, 0, 0, 1),
        XMVectorSet(0, 1, 0, 0)
    );
    XMMATRIX proj = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(60.0f),
        m_swapChainManager.GetWidth() / (float)m_swapChainManager.GetHeight(),
        0.1f, 100.0f
    );

    // 回転値
    for (auto* obj : m_gameObjects) {
        if (obj->name == "Ground") obj->transform.rotation.y = 0;
        else obj->transform.rotation.y = angle;
    }

    // 定数バッファ書き込み（各オブジェクト分）
    void* mapped = nullptr;
    constexpr size_t CBV_SIZE = 256;
    m_bufferManager.GetConstantBuffer()->Map(0, nullptr, &mapped);
    for (size_t i = 0; i < m_gameObjects.size(); ++i) {
        GameObject* obj = m_gameObjects[i];
        ObjectCB cb;
        XMMATRIX world = obj->transform.GetWorldMatrix();
        cb.WorldViewProj = XMMatrixTranspose(world * view * proj);
        cb.Color = obj->color;
        cb.UseTexture = (obj->texIndex >= 0 ? 1 : 0);
        memcpy((char*)mapped + CBV_SIZE * i, &cb, sizeof(cb));
    }
    m_bufferManager.GetConstantBuffer()->Unmap(0, nullptr);

    // 描画処理（Rendererに委譲）
    m_renderer.BeginFrame();
    for (size_t i = 0; i < m_gameObjects.size(); ++i)
        m_renderer.DrawObject(m_gameObjects[i], i, view, proj);
    m_renderer.EndFrame();
}

void EngineManager::Shutdown() {
    for (auto* obj : m_gameObjects) delete obj;
    m_gameObjects.clear();
    m_deviceManager.Cleanup();
    m_swapChainManager.Cleanup();
    // 必要に応じて他ManagerもCleanup
}

void EngineManager::CreateTestCube() {
    std::vector<Vertex> vertices = {
        // 前面 (z: -0.5, 法線 0,0,-1)
        { -0.5f, -0.5f, -0.5f,  0, 0, -1, 0, 0 },
        { -0.5f,  0.5f, -0.5f,  0, 0, -1, 0, 1 },
        {  0.5f,  0.5f, -0.5f,  0, 0, -1, 1, 1 },
        {  0.5f, -0.5f, -0.5f,  0, 0, -1, 1, 0 },
        // 右面 (x: 0.5, 法線 1,0,0)
        { 0.5f, -0.5f, -0.5f,   1, 0, 0,  0, 0 },
        { 0.5f,  0.5f, -0.5f,   1, 0, 0,  0, 1 },
        { 0.5f,  0.5f,  0.5f,   1, 0, 0,  1, 1 },
        { 0.5f, -0.5f,  0.5f,   1, 0, 0,  1, 0 },
        // 後面 (z: 0.5, 法線 0,0,1)
        { 0.5f, -0.5f,  0.5f,   0, 0, 1,  0, 0 },
        { 0.5f,  0.5f,  0.5f,   0, 0, 1,  0, 1 },
        { -0.5f,  0.5f,  0.5f,  0, 0, 1,  1, 1 },
        { -0.5f, -0.5f,  0.5f,  0, 0, 1,  1, 0 },
        // 左面 (x: -0.5, 法線 -1,0,0)
        { -0.5f, -0.5f,  0.5f,  -1, 0, 0, 0, 0 },
        { -0.5f,  0.5f,  0.5f,  -1, 0, 0, 0, 1 },
        { -0.5f,  0.5f, -0.5f,  -1, 0, 0, 1, 1 },
        { -0.5f, -0.5f, -0.5f,  -1, 0, 0, 1, 0 },
        // 上面 (y: 0.5, 法線 0,1,0)
        { -0.5f,  0.5f, -0.5f,  0, 1, 0, 0, 1 },
        { -0.5f,  0.5f,  0.5f,  0, 1, 0, 0, 0 },
        {  0.5f,  0.5f,  0.5f,  0, 1, 0, 1, 0 },
        {  0.5f,  0.5f, -0.5f,  0, 1, 0, 1, 1 },
        // 下面 (y: -0.5, 法線 0,-1,0)
        { -0.5f, -0.5f, -0.5f,  0, -1, 0, 0, 0 },
        { -0.5f, -0.5f,  0.5f,  0, -1, 0, 0, 1 },
        {  0.5f, -0.5f,  0.5f,  0, -1, 0, 1, 1 },
        {  0.5f, -0.5f, -0.5f,  0, -1, 0, 1, 0 },
    };
    std::vector<uint16_t> indices = {
        0,1,2, 0,2,3,
        4,5,6, 4,6,7,
        8,9,10, 8,10,11,
        12,13,14, 12,14,15,
        16,17,18, 16,18,19,
        20,21,22, 20,22,23
    };
    m_bufferManager.CreateVertexBuffer(m_deviceManager.GetDevice(), vertices);
    m_bufferManager.CreateIndexBuffer(m_deviceManager.GetDevice(), indices);
}
