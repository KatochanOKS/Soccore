#include "EngineManager.h"
#include "Transform.h"
#include "StaticMeshRenderer.h"   // ← 追加
#include "SkinnedMeshRenderer.h"  // ← 追加
#include "Colors.h"
#include "ObjectFactory.h"
#include <chrono>
#include <memory>
#include "FbxModelLoader.h"

using namespace DirectX;
using namespace Colors;

void EngineManager::Initialize() {
    m_deviceManager.Initialize();
    auto* device = m_deviceManager.GetDevice();
    auto* cmdQueue = m_deviceManager.GetCommandQueue();
    m_swapChainManager.Initialize(m_hWnd, device, cmdQueue, 1280, 720);
    m_depthBufferManager.Initialize(device, 1280, 720);
    m_pipelineManager.Initialize(
        device,
        L"assets/VertexShader.cso", L"assets/PixelShader.cso",
        L"assets/SkinningVS.cso", L"assets/SkinningPS.cso"
    );

    m_textureManager.Initialize(device);
    ID3D12GraphicsCommandList* cmdList = m_deviceManager.GetCommandList();
    int groundTex = m_textureManager.LoadTexture(L"assets/penguin2.png", cmdList);
    int playerTex = m_textureManager.LoadTexture(L"assets/penguin1.png", cmdList);
    int cubeTex = m_textureManager.LoadTexture(L"assets/penguin2.png", cmdList);
    int enemyTex = m_textureManager.LoadTexture(L"assets/penguin2.png", cmdList);
    int bugEnemyTexIdx = m_textureManager.LoadTexture(L"assets/Mutant.fbm/Mutant_diffuse.png", cmdList);
    m_gameObjects.clear();

    // 地面とキューブ設置
    ObjectFactory::CreateCube(this, { 0, -5.0f, 0 }, { 50, 0.2f, 50 }, groundTex, White);
    ObjectFactory::CreateCube(this, { 0,  0.0f, 0 }, { 1, 1, 1 }, playerTex, White);
    ObjectFactory::CreateCube(this, { -2, 0.0f, 0 }, { 1, 1, 1 }, cubeTex, White);
    ObjectFactory::CreateCube(this, { 2, 2.0f, 2 }, { 1, 1, 1 }, cubeTex, White);

    // ★プレイヤー用スキニングベースモデル作成
    GameObject* player = ObjectFactory::CreateSkinningBaseModel(
        this, "assets/Mutant.fbx", { 0, 0, 0 }, { 0.05f, 0.05f, 0.05f }, bugEnemyTexIdx, Colors::White);

    // ★アニメーション登録
    auto* animator = player->GetComponent<Animator>();
    std::vector<Animator::Keyframe> idleKeys;
    double idleLength;
    if (FbxModelLoader::LoadAnimationOnly("assets/Idle.fbx", idleKeys, idleLength)) {
        animator->AddAnimation("Idle", idleKeys);
    }
    std::vector<Animator::Keyframe> walkKeys;
    double walkLength;
    if (FbxModelLoader::LoadAnimationOnly("assets/Walking.fbx", walkKeys, walkLength)) {
        animator->AddAnimation("Walk", walkKeys);
    }

    constexpr size_t CBV_SIZE = 256;
    m_bufferManager.CreateConstantBuffer(device, CBV_SIZE * m_gameObjects.size());

    m_renderer.Initialize(
        &m_deviceManager,
        &m_swapChainManager,
        &m_depthBufferManager,
        &m_pipelineManager,
        &m_textureManager,
        &m_bufferManager,
        &m_bufferManager,
        GetModelVertexInfo()
    );
}

void EngineManager::Start() {}

void EngineManager::Update() {
    for (auto* obj : m_gameObjects) {
        auto* animator = obj->GetComponent<Animator>();
        if (animator) animator->Update(1.0f / 120.0f);
    }

    // --- プレイヤー制御 ---
    auto* player = m_gameObjects.back(); // 最後に追加したのがプレイヤーと仮定
    auto* tr = player->GetComponent<Transform>();
    auto* animator = player->GetComponent<Animator>();
    float moveSpeed = 0.1f;
    bool isMoving = false;

    if (GetAsyncKeyState('W') & 0x8000) {
        tr->position.z += moveSpeed;
        tr->rotation.y = XMConvertToRadians(0.0f);
        isMoving = true;
    }
    if (GetAsyncKeyState('S') & 0x8000) {
        tr->position.z -= moveSpeed;
        tr->rotation.y = XMConvertToRadians(180.0f);
        isMoving = true;
    }
    if (GetAsyncKeyState('A') & 0x8000) {
        tr->position.x -= moveSpeed;
        tr->rotation.y = XMConvertToRadians(-90.0f);
        isMoving = true;
    }
    if (GetAsyncKeyState('D') & 0x8000) {
        tr->position.x += moveSpeed;
        tr->rotation.y = XMConvertToRadians(90.0f);
        isMoving = true;
    }

    if (animator) {
        if (isMoving && animator->currentAnim != "Walk") {
            animator->SetAnimation("Walk");
        }
        else if (!isMoving && animator->currentAnim != "Idle") {
            animator->SetAnimation("Idle");
        }
    }
}

void EngineManager::Draw() {
    auto* player = m_gameObjects.back();
    auto* tr = player->GetComponent<Transform>();
    XMFLOAT3 playerPos = tr->position;

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

    constexpr size_t CBV_SIZE = 256;
    void* mapped = nullptr;
    m_bufferManager.GetConstantBuffer()->Map(0, nullptr, &mapped);
    for (size_t i = 0; i < m_gameObjects.size(); ++i) {
        GameObject* obj = m_gameObjects[i];
        auto* tr = obj->GetComponent<Transform>();

        ObjectCB cb{};
        // 型ごとにRenderer情報を取得
        if (auto* smr = obj->GetComponent<SkinnedMeshRenderer>()) {
            cb.Color = smr->color;
            cb.UseTexture = (smr->texIndex >= 0 ? 1 : 0);
        } else if (auto* mr = obj->GetComponent<StaticMeshRenderer>()) {
            cb.Color = mr->color;
            cb.UseTexture = (mr->texIndex >= 0 ? 1 : 0);
        } else {
            continue;
        }
        cb.WorldViewProj = XMMatrixTranspose(tr->GetWorldMatrix() * view * proj);

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
}
