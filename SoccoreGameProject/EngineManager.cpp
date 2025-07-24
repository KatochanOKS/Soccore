#include "EngineManager.h"
#include "Transform.h"
#include "MeshRenderer.h"
#include "Colors.h"
#include "ObjectFactory.h" // ★追加aaaaaaaaaaaaaaaaaaaaaaaaaaa
#include <chrono>  // ← これを必ず追加！
#include <memory>

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

    ID3D12GraphicsCommandList* cmdList = m_deviceManager.GetCommandList();
    int groundTex = m_textureManager.LoadTexture(L"assets/penguin2.png", cmdList);
    int playerTex = m_textureManager.LoadTexture(L"assets/penguin1.png", cmdList);
    int cubeTex = m_textureManager.LoadTexture(L"assets/penguin2.png", cmdList);
    int enemyTex = m_textureManager.LoadTexture(L"assets/penguin2.png", cmdList);
    int bossTexIdx = m_textureManager.LoadTexture(L"assets/MixamoModel.fbm/Boss_diffuse.png", cmdList);

    m_gameObjects.clear();

    // ---- Unity風配置 ----
    ObjectFactory::CreateCube(this, { 0, -1.0f, 0 }, { 50, 0.2f, 50 }, groundTex, White);      // 地面
    ObjectFactory::CreateCube(this, { 0,  0.0f, 0 }, { 1, 1, 1 }, playerTex, White);           // プレイヤー
    ObjectFactory::CreateCube(this, { -2,  0.0f, 0 }, { 1, 1, 1 }, cubeTex, White);             // Cube1
    ObjectFactory::CreateCube(this, { 2,  2.0f, 2 }, { 1, 1, 1 }, cubeTex, White);             // Cube2
    ObjectFactory::CreateModel(this, "assets/MixamoModel.fbx", { 0,0,0 }, { 0.05f,0.05f,0.05f }, bossTexIdx, White);
    OutputDebugStringA("★CreateSkinningModel呼び出し直前\n");
    // 定数バッファ

    // アニメーション付きFBXのロード
    FbxModelLoader::SkinningVertexInfo skinInfo;
    bool result = FbxModelLoader::LoadSkinningModel("assets/UnarmedWalkForward.fbx", &skinInfo);
    if (result) {
        OutputDebugStringA("[Test] アニメーション付きモデルロード成功！\n");
        for (auto& anim : skinInfo.animations) {
            char msg[256];
            sprintf_s(msg, "[Test] anim name: %s, length: %.2f, frame count: %zu\n",
                anim.name.c_str(), anim.length, anim.keyframes.size());
            OutputDebugStringA(msg);
        }

        // ★ここでAnimatorインスタンス生成
        m_animator = std::make_unique<Animator>();

        // vector→unordered_mapに変換
        std::unordered_map<std::string, std::vector<Animator::Keyframe>> anims;
        for (auto& anim : skinInfo.animations)
            anims[anim.name] = anim.keyframes;

        m_animator->SetAnimations(anims, skinInfo.boneNames);

        // 初期アニメ再生セット（例："mixamo.com"や"Armature|Walk"等）
        m_animator->SetAnimation(skinInfo.animations[0].name);

        // Debug
        char msg2[128];
        sprintf_s(msg2, "[Debug] アニメセット: %s\n", skinInfo.animations[0].name.c_str());
        OutputDebugStringA(msg2);
    }
    else {
        OutputDebugStringA("[Test] ロード失敗！\n");
    }



    constexpr size_t CBV_SIZE = 256;
    m_bufferManager.CreateConstantBuffer(device, CBV_SIZE * m_gameObjects.size());


    m_renderer.Initialize(
        &m_deviceManager,
        &m_swapChainManager,
        &m_depthBufferManager,
        &m_pipelineManager,
        &m_textureManager,
        &m_bufferManager,         // Cube用バッファ
        &m_bufferManager,         // ★定数バッファ（共通で使ってるならこれでOK）
        GetModelVertexInfo()
    );

}


void EngineManager::Start() {}
void EngineManager::Update() {

    if (m_animator) {
        m_animator->Update(1.0f / 60.0f);
        // --- デバッグ：再生中のアニメと現在時刻をprint ---
        char msg[128];
        sprintf_s(msg, "[Debug] anim=%s, time=%.3f\n", m_animator->currentAnim.c_str(), m_animator->currentTime);
        OutputDebugStringA(msg);
    }


    auto* player = m_gameObjects[4]; // 2番目がプレイヤーCubeの場合
    auto* tr = player->GetComponent<Transform>();
    float moveSpeed = 0.1f;
    if (GetAsyncKeyState('W') & 0x8000) tr->position.z += moveSpeed;
    if (GetAsyncKeyState('S') & 0x8000) tr->position.z -= moveSpeed;
    if (GetAsyncKeyState('A') & 0x8000) tr->position.x -= moveSpeed;
    if (GetAsyncKeyState('D') & 0x8000) tr->position.x += moveSpeed;


}

void EngineManager::Draw() {
    // --- 追従したいキャラのTransform取得
    auto* player = m_gameObjects[4]; // 番号は自分のプレイヤーCubeに合わせてね
    auto* tr = player->GetComponent<Transform>();
    XMFLOAT3 playerPos = tr->position;

    // --- カメラ位置を決める（後ろ＆上）
    XMFLOAT3 cameraOffset = { 0.0f, 5.0f, -20.0f };
    XMFLOAT3 cameraPos = {
        playerPos.x + cameraOffset.x,
        playerPos.y + cameraOffset.y,
        playerPos.z + cameraOffset.z
    };

    XMVECTOR eye = XMLoadFloat3(&cameraPos);
    XMVECTOR target = XMLoadFloat3(&playerPos);
    XMVECTOR up = XMVectorSet(0, 1, 0, 0);
    XMMATRIX view = XMMatrixLookAtLH(eye, target, up);

    XMMATRIX proj = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(60.0f),
        m_swapChainManager.GetWidth() / (float)m_swapChainManager.GetHeight(),
        0.1f, 100.0f
    );

    // 定数バッファ書き込み（Transformに基づくWorld行列）
    void* mapped = nullptr;
    constexpr size_t CBV_SIZE = 256;
    m_bufferManager.GetConstantBuffer()->Map(0, nullptr, &mapped);
    for (size_t i = 0; i < m_gameObjects.size(); ++i) {
        GameObject* obj = m_gameObjects[i];
        auto* tr = obj->GetComponent<Transform>();
        auto* mr = obj->GetComponent<MeshRenderer>();
        if (!tr || !mr) continue;

        ObjectCB cb;
        XMMATRIX world = tr->GetWorldMatrix();
        cb.WorldViewProj = XMMatrixTranspose(world * view * proj);
        cb.Color = mr->color;
        cb.UseTexture = (mr->texIndex >= 0 ? 1 : 0);
        memcpy((char*)mapped + CBV_SIZE * i, &cb, sizeof(cb));
    }
    m_bufferManager.GetConstantBuffer()->Unmap(0, nullptr);

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
