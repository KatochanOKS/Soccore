#include "GameScene.h"
#include"GameObject.h"
#include "ObjectFactory.h"
#include "Transform.h"
#include "Animator.h"
#include "SkinnedMeshRenderer.h"
#include "StaticMeshRenderer.h"
#include "UIImage.h"
#include"Collider.h"
#include <DirectXMath.h>
using namespace DirectX;

// GameSceneのコンストラクタ。EngineManager（リソース管理の中枢）を参照保持
GameScene::GameScene(EngineManager* engine) : engine(engine) {}

// ゲームオブジェクトの初期化と配置
void GameScene::Start() {
    m_sceneObjects.clear(); // シーン開始時に全オブジェクト消去（リセット）

    int skyTex = engine->GetTextureManager()->LoadTexture(
        L"assets/SkyDome.png",
        engine->GetDeviceManager()->GetCommandList()
    );
    auto* sky = ObjectFactory::CreateSkyDome(engine, skyTex, 600.0f);
    m_sceneObjects.push_back(sky);

    // ==== 1. UI画像のGameObject生成 ====
    int logoTex = engine->GetTextureManager()->LoadTexture(L"assets/UI.png", engine->GetDeviceManager()->GetCommandList());
    GameObject* uiObj = new GameObject();
    auto* logo = uiObj->AddComponent<UIImage>();
    logo->texIndex = logoTex;
    logo->position = { 300, 100 };   // 画面左上(px)
    logo->size = { 400, 400 };
    logo->color = { 1, 1, 1, 1 };   // 完全不透明
    m_sceneObjects.push_back(uiObj);

    // ==== 2. 地面オブジェクト（赤いキューブ） ====
    int groundTex = engine->GetTextureManager()->LoadTexture(L"assets/penguin2.png", engine->GetDeviceManager()->GetCommandList());
    // 地面キューブのY座標は-5.0、サイズは(100, 0.2, 100)で超広い
    m_sceneObjects.push_back(ObjectFactory::CreateCube(engine, { 0, -5.0f, 0 }, { 100, 0.2f, 100 }, -1, Colors::Red));

    // ==== 3. サンプルボール（青いキューブ） ====
    auto* ball = ObjectFactory::CreateBall(engine, { 0, 2, 10 }, { 5.0f, 5.0f, 5.0f }, -1, Colors::Blue);
    m_sceneObjects.push_back(ball);

    // ==== 4. プレイヤーキャラクター（FBXスキニングモデル） ====
    int bugEnemyTexIdx = engine->GetTextureManager()->LoadTexture(L"assets/Mutant.fbm/Mutant_diffuse.png", engine->GetDeviceManager()->GetCommandList());
    // center.y = 1.5, size.y = 3.0の前提で作成
    // Y=5からスタート（＝地面から10.0上）で自然に落下するのも確認できる
    GameObject* player = ObjectFactory::CreateSkinningBaseModel(
        engine, "assets/Mutant.fbx", { 0, 5, 0 }, { 0.05f, 0.05f, 0.05f }, bugEnemyTexIdx, Colors::White);

    // アニメーション読み込み（Idle, Walk。拡張可）
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
        OutputDebugStringA("スカイドームのテクスチャ読み込みに失敗しました！\n");
    }
    else {
        OutputDebugStringA("スカイドームの生成成功！\n");
    }

    // --- 必要があればここに追加オブジェクトを登録 ---
}

void GameScene::Update() {
    // ===== 1. プレイヤーの入力による移動とアニメ制御 =====
    if (m_sceneObjects.size() <= 3) return;
    GameObject* player = m_sceneObjects[4];
    auto* tr = player->GetComponent<Transform>();
    auto* animator = player->GetComponent<Animator>();

    float moveSpeed = 0.1f;
    bool isMoving = false;
    // WASDキー入力でXZ平面を移動
    if (GetAsyncKeyState('W') & 0x8000) { tr->position.z += moveSpeed; tr->rotation.y = XMConvertToRadians(0.0f); isMoving = true; }
    if (GetAsyncKeyState('S') & 0x8000) { tr->position.z -= moveSpeed; tr->rotation.y = XMConvertToRadians(180.0f); isMoving = true; }
    if (GetAsyncKeyState('A') & 0x8000) { tr->position.x -= moveSpeed; tr->rotation.y = XMConvertToRadians(-90.0f); isMoving = true; }
    if (GetAsyncKeyState('D') & 0x8000) { tr->position.x += moveSpeed; tr->rotation.y = XMConvertToRadians(90.0f); isMoving = true; }

    // 移動中はWalk、静止時はIdleアニメ
    if (animator) {
        if (isMoving && animator->currentAnim != "Walk") {
            animator->SetAnimation("Walk");
        }
        else if (!isMoving && animator->currentAnim != "Idle") {
            animator->SetAnimation("Idle");
        }
    }

    // ===== 2. プレイヤーの重力・接地（地面補正） =====
    auto* playerCol = player->GetComponent<Collider>();
    GameObject* ground = m_sceneObjects[2];
    auto* groundCol = ground->GetComponent<Collider>();

    if (playerCol && groundCol) {
        // --- 各コライダーの世界座標AABBを取得 ---
        DirectX::XMFLOAT3 pMin, pMax, gMin, gMax;
        playerCol->GetAABBWorld(pMin, pMax);
        groundCol->GetAABBWorld(gMin, gMax);

        // --- 地面のXZ範囲内か？（XZ軸上の当たり判定。地面からはみ出すと落下する） ---
        bool onGroundXZ =
            tr->position.x > gMin.x && tr->position.x < gMax.x &&
            tr->position.z > gMin.z && tr->position.z < gMax.z;

        // --- 足元Y座標と地面Y座標（gMax.yは地面の上面） ---
        float halfHeight = playerCol->size.y * 0.5f * tr->scale.y;
        float footY = tr->position.y + (playerCol->center.y * tr->scale.y) - halfHeight;
        float groundY = gMax.y;

        // --- 【重要】前フレームの足元YとXZ判定を保持 ---
        static float prevFootY = 0.0f;
        static bool wasOnGroundXZ = false;
        static bool firstUpdate = true;
        static const float epsilon = 0.01f; // 接地判定の許容誤差

        // --- ゲーム開始直後だけXZ範囲＋Yで必ず吸着 ---
        if (firstUpdate) {
            if (onGroundXZ && footY <= groundY + epsilon) {
                // 「初期配置で既に地面をまたいでいたら」必ず吸着
                tr->position.y += (groundY - footY);
            }
            firstUpdate = false;
        }
        else {
            bool willLand = false;
            if (onGroundXZ && wasOnGroundXZ) {
                // --- 【最重要】前フレームと今フレームの間で地面Yを“通過”したら吸着 ---
                // （上→下・下→上、どちらもピタリ止まる＝すり抜け絶対防止）
                if ((prevFootY - groundY) * (footY - groundY) <= 0.0f) {
                    willLand = true;
                }
            }
            if (willLand) {
                // 着地時だけピタッと止める
                tr->position.y += (groundY - footY);
            }
            else {
                // それ以外は常に重力で落下
                tr->position.y -= 0.2f;
            }
        }
        // --- 状態保存（次フレーム判定用） ---
        prevFootY = footY;
        wasOnGroundXZ = onGroundXZ;

        // --- デバッグ出力 ---
        char buf[256];
        sprintf_s(buf, "PosY=%.2f, FootY=%.2f, GroundY=%.2f, onXZ=%d\n",
            tr->position.y, footY, groundY, (int)onGroundXZ);
        OutputDebugStringA(buf);
    }

    // ===== 3. アニメーション進行（毎フレーム120分の1秒ずつ進める） =====
    for (auto* obj : m_sceneObjects) {
        if (auto* animator = obj->GetComponent<Animator>())
            animator->Update(1.0f / 120.0f);
    }
}

// ===== 描画処理（プレイヤーを中心にカメラ追従。すべてのGameObjectを描画） =====
void GameScene::Draw() {
    if (m_sceneObjects.empty()) return;

    // --- プレイヤー座標にカメラを追従 ---
    GameObject* player = nullptr;
    if (m_sceneObjects.size() > 1)
        player = m_sceneObjects[4];
    if (!player) return;
    auto* tr = player->GetComponent<Transform>();
    XMFLOAT3 playerPos = tr ? tr->position : XMFLOAT3{ 0,0,0 };

    XMFLOAT3 cameraOffset = { 0.0f, 5.0f, -20.0f }; // カメラは斜め上後方から
    XMFLOAT3 cameraPos = {
        playerPos.x + cameraOffset.x,
        playerPos.y + cameraOffset.y,
        playerPos.z + cameraOffset.z
    };
    char buf[128];
    sprintf_s(buf, "CameraPos: %.2f, %.2f, %.2f\n", cameraPos.x, cameraPos.y, cameraPos.z);
    OutputDebugStringA(buf);

    XMVECTOR eye = XMLoadFloat3(&cameraPos);
    XMVECTOR target = XMLoadFloat3(&playerPos);
    XMVECTOR up = XMVectorSet(0, 8, 0, 0);
    XMMATRIX view = XMMatrixLookAtLH(eye, target, up);

    XMMATRIX proj = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(70.0f),
        engine->GetSwapChainManager()->GetWidth() / (float)engine->GetSwapChainManager()->GetHeight(),
        0.1f, 1000.0f // ← farを1000.0fなど大きくする
    );

    // cameraPos が決まった直後あたり
    if (!m_sceneObjects.empty()) {
        auto* skyTr = m_sceneObjects[0]->GetComponent<Transform>(); // 先頭をSkyDomeにしている想定
        if (skyTr) skyTr->position = cameraPos;
    }


    // --- 各GameObjectの定数バッファ（CBV）を更新 ---
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

    // --- 描画フレーム開始 ---
    engine->GetRenderer()->BeginFrame();
    for (size_t i = 0; i < m_sceneObjects.size(); ++i)
        engine->GetRenderer()->DrawObject(m_sceneObjects[i], i, view, proj);
    engine->GetRenderer()->EndFrame();
}

// ===== デストラクタでメモリ解放 =====
GameScene::~GameScene() {
    for (auto* obj : m_sceneObjects) {
        delete obj;
    }
    m_sceneObjects.clear();
}
