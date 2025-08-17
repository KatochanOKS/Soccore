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

GameScene::GameScene(EngineManager* engine) : engine(engine) {}

// AABB同士の重なり判定
bool CheckAABBOverlap(const DirectX::XMFLOAT3& minA, const DirectX::XMFLOAT3& maxA,
    const DirectX::XMFLOAT3& minB, const DirectX::XMFLOAT3& maxB) {
    return (minA.x <= maxB.x && maxA.x >= minB.x) &&
        (minA.y <= maxB.y && maxA.y >= minB.y) &&
        (minA.z <= maxB.z && maxA.z >= minB.z);
}


void GameScene::Start() {
    m_sceneObjects.clear();

    int skyTex = engine->GetTextureManager()->LoadTexture(
        L"assets/SkyDome.png",
        engine->GetDeviceManager()->GetCommandList()
    );
    auto* sky = ObjectFactory::CreateSkyDome(engine, skyTex, 600.0f);
    m_sceneObjects.push_back(sky); //0

    int logoTex = engine->GetTextureManager()->LoadTexture(L"assets/UI.png", engine->GetDeviceManager()->GetCommandList());
    GameObject* uiObj = new GameObject();
    auto* logo = uiObj->AddComponent<UIImage>();
    logo->texIndex = logoTex;
    logo->position = { 300, 100 };
    logo->size = { 400, 400 };
    logo->color = { 1, 1, 1, 1 };
    m_sceneObjects.push_back(uiObj); //1

    int groundTex = engine->GetTextureManager()->LoadTexture(L"assets/penguin2.png", engine->GetDeviceManager()->GetCommandList());
	m_sceneObjects.push_back(ObjectFactory::CreateCube(engine, { 0, -5.0f, 0 }, { 50, 0.2f, 50 }, -1, Colors::Red)); //2

    auto* oldBall = ObjectFactory::CreateBall(engine, { 0, 2, 10 }, { 5.0f, 5.0f, 5.0f }, -1, Colors::Blue);
    m_sceneObjects.push_back(oldBall); //3

    int bugEnemyTexIdx = engine->GetTextureManager()->LoadTexture(L"assets/Mutant.fbm/Mutant_diffuse.png", engine->GetDeviceManager()->GetCommandList());
    GameObject* player = ObjectFactory::CreateSkinningBaseModel(
        engine, "assets/Mutant.fbx", { 0, 5, 0 }, { 0.01f, 0.01f, 0.01f }, bugEnemyTexIdx, Colors::White
    );

	int ballTexIdx = engine->GetTextureManager()->LoadTexture(L"assets/soccer_ball/textures/football_ball_BaseColor.png", engine->GetDeviceManager()->GetCommandList());

    GameObject* soccoreBall = ObjectFactory::CreateModel(
        engine,
        "assets/soccer_ball/football.fbx",
        { 0, 5.5f, 0 },
        { 0.05f, 0.05f, 0.05f },
        ballTexIdx,
        Colors::White
    );
	m_sceneObjects.push_back(soccoreBall); //4

	int studiumTexIdx = engine->GetTextureManager()->LoadTexture(L"assets/football/textures/Football_BaseColor.png", engine->GetDeviceManager()->GetCommandList());

    GameObject* soccoreStudium = ObjectFactory::CreateModel(
        engine,
        "assets/Soccore/SccoreStudium.fbx",
        { 0, 5.0f, 0 },
        { 1.0f, 1.0f, 1.0f },
        studiumTexIdx,
        Colors::White
    );
    soccoreStudium->GetComponent<Transform>()->rotation.y = DirectX::XMConvertToRadians(90.0f);  // ←Y軸90°

    m_sceneObjects.push_back(soccoreStudium); //5

	int goalTexIdx = engine->GetTextureManager()->LoadTexture(L"assets/football/textures/Goal_BaseColor.png", engine->GetDeviceManager()->GetCommandList());

    GameObject* goal = ObjectFactory::CreateModel(
        engine,
        "assets/Soccore/Goal.fbx",
        { -20.0f, 5.0f, 0 },
        { 1.5f, 1.5f, 1.5f },
        goalTexIdx,
        Colors::White
	);
    goal->GetComponent<Transform>()->rotation.y = DirectX::XMConvertToRadians(90.0f);  // ←Y軸90°
	m_sceneObjects.push_back(goal); //6

    GameObject* goal2 = ObjectFactory::CreateModel(
        engine,
        "assets/Soccore/Goal.fbx",
        { 20.0f, 5.0f, 0 },
        { 1.5f, 1.5f, 1.5f },
        goalTexIdx,
        Colors::White
    );
    goal2->GetComponent<Transform>()->rotation.y = DirectX::XMConvertToRadians(-90.0f);  // ←Y軸90°
    m_sceneObjects.push_back(goal2); //7
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
    m_sceneObjects.push_back(player); //8

    if (skyTex < 0) {
        OutputDebugStringA("SkyDome texture load failed!\n");
    }
    else {
        OutputDebugStringA("SkyDome created!\n");
    }
}

void GameScene::Update() {
    if (m_sceneObjects.size() <= 7) return;
    GameObject* player = m_sceneObjects[8];
    auto* tr = player->GetComponent<Transform>();
    auto* animator = player->GetComponent<Animator>();
    Camera* cam = engine->GetCamera();

    float moveSpeed = 0.1f;
    float rotateSpeed = 1.0f;
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


    // プレイヤーとボールの距離を測定
    XMFLOAT3 playerPos = tr->position;
	GameObject* m_soccerBall = m_sceneObjects[4]; // サッカーボールのオブジェクト
    XMFLOAT3 ballPos = m_soccerBall->GetComponent<Transform>()->position;

    float dx = ballPos.x - playerPos.x;
    float dz = ballPos.z - playerPos.z;
    float distSq = dx * dx + dz * dz;

    // --- スペースでボールを蹴る処理（ある程度近づいたら） ---
    if ((GetAsyncKeyState(VK_SPACE) & 0x8000) && distSq < 25.0f) {  // ← 半径約5mまでOKに
        float angleRad = tr->rotation.y;
        float kickSpeed = 0.5f;

        // 正しい
        m_ballVelocity.x = sinf(angleRad) * kickSpeed;
        m_ballVelocity.z = cosf(angleRad) * kickSpeed;

        OutputDebugStringA("Kick!\n");
    }

    // --- ボール移動処理（速度に基づいて動く） ---
    auto* ballTr = m_soccerBall->GetComponent<Transform>();
    ballTr->position.x += m_ballVelocity.x;
    ballTr->position.z += m_ballVelocity.z;

    // 摩擦（徐々に止まる）
    m_ballVelocity.x *= 0.95f;
    m_ballVelocity.z *= 0.95f;

    // 速度が小さくなったら完全に止める
    if (fabs(m_ballVelocity.x) < 0.001f) m_ballVelocity.x = 0;
    if (fabs(m_ballVelocity.z) < 0.001f) m_ballVelocity.z = 0;



    // カメラ角度を -90 ～ +90 に制限
    if (cam->cameraYaw > 90.0f) cam->cameraYaw = 90.0f;
    if (cam->cameraYaw < -90.0f) cam->cameraYaw = -90.0f;

    // アニメーション切り替え
    if (animator) {
        if (isMoving && animator->currentAnim != "Walk") {
            animator->SetAnimation("Walk");
        }
        else if (!isMoving && animator->currentAnim != "Idle") {
            animator->SetAnimation("Idle");
        }
    }

    // アニメーション更新
    for (auto* obj : m_sceneObjects) {
        if (auto* animator = obj->GetComponent<Animator>())
            animator->Update(1.0f / 120.0f);
    }

}


void GameScene::Draw() {
    if (m_sceneObjects.size() <= 7) return;
    GameObject* player = m_sceneObjects[8];
    auto* tr = player->GetComponent<Transform>();
    if (!tr) return;
    XMFLOAT3 playerPos = tr->position;

    Camera* cam = engine->GetCamera();
    // ↓カメラの位置をプレイヤーの「真後ろ＋真上」に直接固定
    XMFLOAT3 offset = { 0.0f, 5.0f, -15.0f }; // 高め・後ろから
    XMFLOAT3 cameraPos = {
        playerPos.x + offset.x,
        playerPos.y + offset.y,
        playerPos.z + offset.z
    };
    cam->SetPosition(cameraPos);
    cam->SetTarget(playerPos); // プレイヤーを見る

    XMMATRIX view = cam->GetViewMatrix();
    XMMATRIX proj = cam->GetProjectionMatrix(
        engine->GetSwapChainManager()->GetWidth(),
        engine->GetSwapChainManager()->GetHeight()
    );

    if (!m_sceneObjects.empty()) {
        auto* skyTr = m_sceneObjects[0]->GetComponent<Transform>();
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
    // --- 通常オブジェクト（UI以外） ---
        for (size_t i = 0; i < m_sceneObjects.size(); ++i) {
            auto* obj = m_sceneObjects[i];
            if (!obj->GetComponent<UIImage>()) {
                engine->GetRenderer()->DrawObject(obj, i, view, proj);
            }
        }

    // --- UIは最後に描画する ---
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
