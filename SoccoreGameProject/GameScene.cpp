#include "GameScene.h"
#include "GameObject.h"
#include "ObjectFactory.h"
#include "Transform.h"
#include "Animator.h"
#include "SkinnedMeshRenderer.h"
#include "StaticMeshRenderer.h"
#include "UIImage.h"
#include "Collider.h"
#include <DirectXMath.h>
using namespace DirectX;

// タグ検索
GameObject* GameScene::FindByTag(const std::string& tag) {
    for (auto* obj : m_sceneObjects) {
        if (obj->tag == tag) return obj;
    }
    return nullptr;
}

// 名前検索
GameObject* GameScene::FindByName(const std::string& name) {
    for (auto* obj : m_sceneObjects) {
        if (obj->name == name) return obj;
    }
    return nullptr;
}

// AABB同士の重なり判定
bool CheckAABBOverlap(const XMFLOAT3& minA, const XMFLOAT3& maxA,
    const XMFLOAT3& minB, const XMFLOAT3& maxB) {
    return (minA.x <= maxB.x && maxA.x >= minB.x) &&
        (minA.y <= maxB.y && maxA.y >= minB.y) &&
        (minA.z <= maxB.z && maxA.z >= minB.z);
}

GameScene::GameScene(EngineManager* engine) : engine(engine) {}

void GameScene::Start() {
    m_sceneObjects.clear();

    int skyTex = engine->GetTextureManager()->LoadTexture(
        L"assets/SkyDome.png",
        engine->GetDeviceManager()->GetCommandList()
    );
    auto* sky = ObjectFactory::CreateSkyDome(engine, skyTex, 600.0f, "Sky", "SkyDome");
    m_sceneObjects.push_back(sky);

    int logoTex = engine->GetTextureManager()->LoadTexture(L"assets/UI.png", engine->GetDeviceManager()->GetCommandList());
    GameObject* uiObj = new GameObject();
    auto* logo = uiObj->AddComponent<UIImage>();
    logo->texIndex = logoTex;
    logo->position = { 300, 100 };
    logo->size = { 400, 400 };
    logo->color = { 1, 1, 1, 1 };
    uiObj->tag = "UI";
    uiObj->name = "LogoUI";
    m_sceneObjects.push_back(uiObj);

    // 地面
    m_sceneObjects.push_back(ObjectFactory::CreateCube(
        engine, { 0, -5.0f, 0 }, { 50, 0.2f, 50 }, -1, Colors::Red, { 0,0,0 }, { -1,-1,-1 }, "Ground", "GroundFloor"
    ));

    // 旧テスト用ボール
    auto* oldBall = ObjectFactory::CreateBall(
        engine, { 0, 2, 10 }, { 5.0f, 5.0f, 5.0f }, -1, Colors::Blue, { 0,0,0 }, { -1,-1,-1 }, "Ball", "OldTestBall"
    );
    m_sceneObjects.push_back(oldBall);

    // プレイヤー
    int bugEnemyTexIdx = engine->GetTextureManager()->LoadTexture(L"assets/Mutant.fbm/Mutant_diffuse.png", engine->GetDeviceManager()->GetCommandList());
    GameObject* player = ObjectFactory::CreateSkinningBaseModel(
        engine, "assets/Mutant.fbx", { 0, 5, 0 }, { 0.01f, 0.01f, 0.01f }, bugEnemyTexIdx, Colors::White,
        { 0,1.0f,0 }, { 0.5f,2.0f,0.5f }, "Player", "Player1"
    );

    // サッカーボール（FBXモデルで本物）
    int ballTexIdx = engine->GetTextureManager()->LoadTexture(L"assets/soccer_ball/textures/football_ball_BaseColor.png", engine->GetDeviceManager()->GetCommandList());
    GameObject* soccerBall = ObjectFactory::CreateModel(
        engine,
        "assets/soccer_ball/football.fbx",
        { 0, 5.5f, 0 },
        { 0.05f, 0.05f, 0.05f },
        ballTexIdx,
        Colors::White,
        { 0,0,0 }, { -1,-1,-1 }, "Ball", "SoccerBall1"
    );
    m_sceneObjects.push_back(soccerBall);

    // スタジアム
    int studiumTexIdx = engine->GetTextureManager()->LoadTexture(L"assets/football/textures/Football_BaseColor.png", engine->GetDeviceManager()->GetCommandList());
    GameObject* stadium = ObjectFactory::CreateModel(
        engine,
        "assets/Soccore/SccoreStudium.fbx",
        { 0, 5.0f, 0 },
        { 1.0f, 1.0f, 1.0f },
        studiumTexIdx,
        Colors::White,
        { 0,0,0 }, { -1,-1,-1 }, "Stadium", "MainStadium"
    );
    stadium->GetComponent<Transform>()->rotation.y = XMConvertToRadians(90.0f);
    m_sceneObjects.push_back(stadium);

    // ゴール1
    int goalTexIdx = engine->GetTextureManager()->LoadTexture(L"assets/football/textures/Goal_BaseColor.png", engine->GetDeviceManager()->GetCommandList());
    GameObject* goal1 = ObjectFactory::CreateModel(
        engine,
        "assets/Soccore/Goal.fbx",
        { -20.0f, 5.0f, 0 },
        { 1.5f, 1.5f, 1.5f },
        goalTexIdx,
        Colors::White,
        { 0,0,0 }, { -1,-1,-1 }, "Goal", "Goal1"
    );
    goal1->GetComponent<Transform>()->rotation.y = XMConvertToRadians(90.0f);
    m_sceneObjects.push_back(goal1);

    // ゴール2
    GameObject* goal2 = ObjectFactory::CreateModel(
        engine,
        "assets/Soccore/Goal.fbx",
        { 20.0f, 5.0f, 0 },
        { 1.5f, 1.5f, 1.5f },
        goalTexIdx,
        Colors::White,
        { 0,0,0 }, { -1,-1,-1 }, "Goal", "Goal2"
    );
    goal2->GetComponent<Transform>()->rotation.y = XMConvertToRadians(-90.0f);
    m_sceneObjects.push_back(goal2);

    // アニメーション
    auto* animator = player->GetComponent<Animator>();
    std::vector<Animator::Keyframe> idleKeys;
    double idleLen;
    if (FbxModelLoader::LoadAnimationOnly("assets/Idle.fbx", idleKeys, idleLen)) {
        animator->AddAnimation("Idle", idleKeys);
    }
    std::vector<Animator::Keyframe> walkKeys;
    double walkLen;
    if (FbxModelLoader::LoadAnimationOnly("assets/Walking.fbx", walkKeys, walkLen)) {
        animator->AddAnimation("Walk", walkKeys);
    }
    m_sceneObjects.push_back(player);

    if (skyTex < 0) {
        OutputDebugStringA("SkyDome texture load failed!\n");
    }
    else {
        OutputDebugStringA("SkyDome created!\n");
    }
}

void GameScene::Update() {
    // --- 各主要オブジェクトを「タグ・名前」で取得 ---
    auto* player = FindByTag("Player");
    auto* soccerBall = FindByName("SoccerBall1");
    if (!player || !soccerBall) return;

    auto* tr = player->GetComponent<Transform>();
    auto* animator = player->GetComponent<Animator>();
    Camera* cam = engine->GetCamera();

    float moveSpeed = 0.1f;
    bool isMoving = false;

    // プレイヤー操作
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

    // プレイヤーとボールの距離で蹴る判定
    XMFLOAT3 playerPos = tr->position;
    XMFLOAT3 ballPos = soccerBall->GetComponent<Transform>()->position;
    float dx = ballPos.x - playerPos.x;
    float dz = ballPos.z - playerPos.z;
    float distSq = dx * dx + dz * dz;
    if ((GetAsyncKeyState(VK_SPACE) & 0x8000) && distSq < 25.0f) {
        float angleRad = tr->rotation.y;
        float kickSpeed = 0.5f;
        m_ballVelocity.x = sinf(angleRad) * kickSpeed;
        m_ballVelocity.z = cosf(angleRad) * kickSpeed;
        OutputDebugStringA("Kick!\n");
    }

    // ボール移動処理
    auto* ballTr = soccerBall->GetComponent<Transform>();
    ballTr->position.x += m_ballVelocity.x;
    ballTr->position.z += m_ballVelocity.z;

    // 摩擦
    m_ballVelocity.x *= 0.95f;
    m_ballVelocity.z *= 0.95f;
    if (fabs(m_ballVelocity.x) < 0.001f) m_ballVelocity.x = 0;
    if (fabs(m_ballVelocity.z) < 0.001f) m_ballVelocity.z = 0;

    // カメラ角度制限
    if (cam->cameraYaw > 90.0f) cam->cameraYaw = 90.0f;
    if (cam->cameraYaw < -90.0f) cam->cameraYaw = -90.0f;

    // アニメーション切替
    if (animator) {
        if (isMoving && animator->currentAnim != "Walk") {
            animator->SetAnimation("Walk");
        }
        else if (!isMoving && animator->currentAnim != "Idle") {
            animator->SetAnimation("Idle");
        }
    }

    // アニメ更新
    for (auto* obj : m_sceneObjects) {
        if (auto* animator = obj->GetComponent<Animator>())
            animator->Update(1.0f / 120.0f);
    }
}

void GameScene::Draw() {
    auto* player = FindByTag("Player");
    if (!player) return;
    auto* tr = player->GetComponent<Transform>();
    if (!tr) return;
    XMFLOAT3 playerPos = tr->position;

    Camera* cam = engine->GetCamera();
    XMFLOAT3 offset = { 0.0f, 5.0f, -15.0f };
    XMFLOAT3 cameraPos = {
        playerPos.x + offset.x,
        playerPos.y + offset.y,
        playerPos.z + offset.z
    };
    cam->SetPosition(cameraPos);
    cam->SetTarget(playerPos);

    XMMATRIX view = cam->GetViewMatrix();
    XMMATRIX proj = cam->GetProjectionMatrix(
        engine->GetSwapChainManager()->GetWidth(),
        engine->GetSwapChainManager()->GetHeight()
    );

    // スカイドーム追従
    auto* sky = FindByTag("Sky");
    if (sky) {
        auto* skyTr = sky->GetComponent<Transform>();
        if (skyTr) skyTr->position = cam->GetPosition();
    }

    constexpr size_t CBV_SIZE = 256;
    void* mapped = nullptr;
    auto* cb = engine->GetBufferManager()->GetConstantBuffer();
    cb->Map(0, nullptr, &mapped);

    for (size_t i = 0; i < m_sceneObjects.size(); ++i) {
        GameObject* obj = m_sceneObjects[i];
        auto* tr = obj->GetComponent<Transform>();
        if (!tr) continue;

        ObjectCB cbData{};
        if (auto* smr = obj->GetComponent<SkinnedMeshRenderer>()) {
            cbData.Color = smr->color;
            cbData.UseTexture = smr->texIndex >= 0 ? 1 : 0;
        }
        else if (auto* mr = obj->GetComponent<StaticMeshRenderer>()) {
            cbData.Color = mr->color;
            cbData.UseTexture = mr->texIndex >= 0 ? 1 : 0;
        }
        cbData.WorldViewProj = XMMatrixTranspose(tr->GetWorldMatrix() * view * proj);
        memcpy((char*)mapped + CBV_SIZE * i, &cbData, sizeof(cbData));
    }
    cb->Unmap(0, nullptr);

    engine->GetRenderer()->BeginFrame();
    // 通常オブジェクト
    for (size_t i = 0; i < m_sceneObjects.size(); ++i) {
        auto* obj = m_sceneObjects[i];
        if (!obj->GetComponent<UIImage>()) {
            engine->GetRenderer()->DrawObject(obj, i, view, proj);
        }
    }
    // UIは最後
    for (size_t i = 0; i < m_sceneObjects.size(); ++i) {
        auto* obj = m_sceneObjects[i];
        if (obj->GetComponent<UIImage>()) {
            engine->GetRenderer()->DrawObject(obj, i, view, proj);
        }
    }
    engine->GetRenderer()->EndFrame();
}

GameScene::~GameScene() {
    for (auto* obj : m_sceneObjects) {
        delete obj;
    }
    m_sceneObjects.clear();
}
