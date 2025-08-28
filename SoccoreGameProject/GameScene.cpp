#include "GameScene.h"
#include "GameObject.h"
#include "ObjectFactory.h"
#include "Transform.h"
#include "Animator.h"
#include "SkinnedMeshRenderer.h"
#include "StaticMeshRenderer.h"
#include "UIImage.h"
#include "Collider.h"
#include "Player1Component.h"
#include "Player2Component.h"
#include "SoccerBallComponent.h"
#include "GoalComponent.h"
#include <DirectXMath.h>
#include <cmath>
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

    // スカイドーム
    int skyTex = engine->GetTextureManager()->LoadTexture(
        L"assets/SkyDome.png",
        engine->GetDeviceManager()->GetCommandList()
    );
    auto* sky = ObjectFactory::CreateSkyDome(engine, skyTex, 600.0f, "Sky", "SkyDome");
    m_sceneObjects.push_back(sky);

    // HPバーUI -----------------------------------------------

    // 1P用 赤残像バー（左上）
    int logoTexRed = engine->GetTextureManager()->LoadTexture(L"assets/Red.png", engine->GetDeviceManager()->GetCommandList());
    GameObject* hp1RedObj = new GameObject();
    auto* hp1Red = hp1RedObj->AddComponent<UIImage>();
    hp1Red->texIndex = logoTexRed;
    hp1Red->size = { 500, 50 };
    hp1Red->position = { 0, 50 };
    hp1Red->color = { 1, 1, 1, 1 };
    hp1RedObj->tag = "UI";
    hp1RedObj->name = "HP1Red";
    m_sceneObjects.push_back(hp1RedObj);

    // 1P用 本体バー（左上）
    int logoTex = engine->GetTextureManager()->LoadTexture(L"assets/Green3.png", engine->GetDeviceManager()->GetCommandList());
    GameObject* hp1Obj = new GameObject();
    auto* hp1 = hp1Obj->AddComponent<UIImage>();
    hp1->texIndex = logoTex;
    hp1->size = { 500, 50 };
    hp1->position = { 0, 50 };
    hp1->color = { 1, 1, 1, 1 };
    hp1Obj->tag = "UI";
    hp1Obj->name = "HP1";
    m_sceneObjects.push_back(hp1Obj);

    // 2P用 赤残像バー（右上）
    GameObject* hp2RedObj = new GameObject();
    auto* hp2Red = hp2RedObj->AddComponent<UIImage>();
    hp2Red->texIndex = logoTexRed;
    hp2Red->size = { 500, 50 };
    hp2Red->position = { 780, 50 };
    hp2Red->color = { 1, 1, 1, 1 };
    hp2RedObj->tag = "UI";
    hp2RedObj->name = "HP2Red";
    m_sceneObjects.push_back(hp2RedObj);

    // 2P用 本体バー（右上）
    GameObject* hp2Obj = new GameObject();
    auto* hp2 = hp2Obj->AddComponent<UIImage>();
    hp2->texIndex = logoTex;
    hp2->size = { 500, 50 };
    hp2->position = { 780, 50 };
    hp2->color = { 1, 1, 1, 1 };
    hp2Obj->tag = "UI";
    hp2Obj->name = "HP2";
    m_sceneObjects.push_back(hp2Obj);

    // 格闘ステージの床
    m_sceneObjects.push_back(ObjectFactory::CreateCube(
        engine, { 0.0f, -0.5f, 0.0f }, { 10.0f, 1.0f, 3.0f }, -1, Colors::Gray, { 0,0,0 }, { -1,-1,-1 }, "Ground", "GroundFloor"
    ));

    // --- プレイヤー生成・アニメ登録 ---
    // 1P
    int p1TexIdx = engine->GetTextureManager()->LoadTexture(L"assets/Mutant.fbm/Mutant_diffuse.png", engine->GetDeviceManager()->GetCommandList());
    GameObject* player1 = ObjectFactory::CreateSkinningBaseModel(
        engine, "assets/Mutant.fbx",
        { -2.5f, 0.0f, 0.0f },
        { 0.01f, 0.01f, 0.01f },
        p1TexIdx, Colors::White,
        { 0,0.85f,0 }, { 1.5f,1.7f,1.5f }, "Player", "Player1"
    );
    player1->AddComponent<Player1Component>();
    player1->GetComponent<Transform>()->rotation.y = XMConvertToRadians(90.0f);

    // 2P
    int p2TexIdx = engine->GetTextureManager()->LoadTexture(L"assets/MMA2/SkeletonzombieTAvelange.fbm/skeletonZombie_diffuse.png", engine->GetDeviceManager()->GetCommandList());
    GameObject* player2 = ObjectFactory::CreateSkinningBaseModel(
        engine, "assets/MMA2/SkeletonzombieTAvelange.fbx",
        { 2.5f, 0.0f, 0.0f },
        { 0.01f, 0.01f, 0.01f },
        p2TexIdx, Colors::White,
        { 0,0.9f,0 }, { 1.5f,1.8f,1.5f }, "Enemy", "Player2"
    );
    player2->AddComponent<Player2Component>();
    player2->GetComponent<Transform>()->rotation.y = XMConvertToRadians(-90.0f);

    m_sceneObjects.push_back(player1);
    m_sceneObjects.push_back(player2);

    // --- デバッグ表示（ボーン名）---
    auto* smr1 = player1->GetComponent<SkinnedMeshRenderer>();
    auto* smr2 = player2->GetComponent<SkinnedMeshRenderer>();
    char buf[256];

    OutputDebugStringA("--- PLAYER1 boneNames ---\n");
    for (size_t i = 0; i < smr1->skinVertexInfo->boneNames.size(); ++i) {
        sprintf_s(buf, "[%02zu] %s\n", i, smr1->skinVertexInfo->boneNames[i].c_str());
        OutputDebugStringA(buf);
    }
    OutputDebugStringA("--- PLAYER2 boneNames ---\n");
    for (size_t i = 0; i < smr2->skinVertexInfo->boneNames.size(); ++i) {
        sprintf_s(buf, "[%02zu] %s\n", i, smr2->skinVertexInfo->boneNames[i].c_str());
        OutputDebugStringA(buf);
    }

    // --- アニメ登録 ---
    struct AnimEntry { const char* file; const char* name; };
    AnimEntry anims1[] = {
        { "assets/MMA/BodyBlock.fbx",    "BodyBlock"   },
        { "assets/MMA/MmaKick.fbx",      "Kick"        },
        { "assets/MMA/MutantDying.fbx",  "Dying"       },
        { "assets/MMA/Punching.fbx",     "Punch"       },
        { "assets/MMA/BouncingFightIdle.fbx", "Idle"   },
        { "assets/MMA/Walking.fbx",      "Walk"        },
        { "assets/MMA/TakingPunch.fbx",      "Reaction"        },
    };
    AnimEntry anims2[] = {
        { "assets/MMA2/OutwardBlock.fbx",    "BodyBlock"   },
        { "assets/MMA2/MmaKick.fbx",         "Kick"        },
        { "assets/MMA2/DyingBackwards.fbx",  "Dying"       },
        { "assets/MMA2/Punching2P.fbx",      "Punch"       },
        { "assets/MMA2/BouncingFightIdle2.fbx", "Idle"     },
        { "assets/MMA2/WalkingPlayer2.fbx",  "Walk"        },
        { "assets/MMA2/Kicking.fbx",         "Kick2"       },
        { "assets/MMA2/ZombieReactionHit.fbx",         "Reaction"       },
    };
    // 1P
    auto* animator1 = player1->GetComponent<Animator>();
    for (auto& anim1 : anims1) {
        std::vector<Animator::Keyframe> keys;
        double animLen;
        if (FbxModelLoader::LoadAnimationOnly(anim1.file, keys, animLen)) {
            animator1->AddAnimation(anim1.name, keys);
        }
    }
    // 2P
    auto* animator2 = player2->GetComponent<Animator>();
    for (auto& anim2 : anims2) {
        std::vector<Animator::Keyframe> keys;
        double animLen;
        if (FbxModelLoader::LoadAnimationOnly(anim2.file, keys, animLen)) {
            animator2->AddAnimation(anim2.name, keys);
        }
    }

    // シーン参照セット
    player1->scene = this;
    player2->scene = this;

    if (skyTex < 0) { OutputDebugStringA("SkyDome texture load failed!\n"); }
    else { OutputDebugStringA("SkyDome created!\n"); }
}

void GameScene::Update() {
    // --- 全ComponentのUpdate ---
    for (auto* obj : m_sceneObjects) {
        for (auto* comp : obj->components) comp->Update();
    }
    // --- 全アニメーターUpdate ---
    for (auto* obj : m_sceneObjects) {
        if (auto* animator = obj->GetComponent<Animator>())
            animator->Update(1.0f / 120.0f);
    }

    // --- コライダーAABB同士で攻撃判定 ---
    GameObject* player1 = FindByName("Player1");
    GameObject* player2 = FindByName("Player2");
    if (player1 && player2) {
        auto* tr1 = player1->GetComponent<Transform>();
        auto* tr2 = player2->GetComponent<Transform>();
        auto* col1 = player1->GetComponent<Collider>();
        auto* col2 = player2->GetComponent<Collider>();
        if (!tr1 || !tr2 || !col1 || !col2) {
            OutputDebugStringA("[DEBUG] Transform/Collider取得失敗\n");
            return;
        }

        XMFLOAT3 minA, maxA, minB, maxB;
        col1->GetAABBWorld(tr1, minA, maxA);
        col2->GetAABBWorld(tr2, minB, maxB);

        // --- プレイヤー1の攻撃 ---
        auto* anim1 = player1->GetComponent<Animator>();
        char buf[256];
        sprintf_s(buf, "[DEBUG] Player1 currentAnim = %s, playing = %d\n", anim1->currentAnim.c_str(), anim1->isPlaying ? 1 : 0);
        OutputDebugStringA(buf);

        auto* comp1 = player1->GetComponent<Player1Component>();
        auto* comp2 = player2->GetComponent<Player2Component>();

        // 1P攻撃ヒット管理
        static bool prevHitP1toP2 = false;
        bool isP1AttackAnim = (anim1 && (anim1->currentAnim == "Punch" || anim1->currentAnim == "Kick") && anim1->isPlaying);
        bool hitP1toP2 = isP1AttackAnim && CheckAABBOverlap(minA, maxA, minB, maxB);

        // 1P攻撃ヒット管理
        if (hitP1toP2 && !prevHitP1toP2) {
            if (comp2 && !comp2->isGuarding) {
                comp2->TakeDamage(0.08f);   // ←必ずTakeDamage経由
            }
        }


        prevHitP1toP2 = hitP1toP2;

        // 2P攻撃ヒット管理
        auto* anim2 = player2->GetComponent<Animator>();
        static bool prevHitP2toP1 = false;
        bool isP2AttackAnim = (anim2 && (anim2->currentAnim == "Punch" || anim2->currentAnim == "Kick") && anim2->isPlaying);
        bool hitP2toP1 = isP2AttackAnim && CheckAABBOverlap(minB, maxB, minA, maxA);

        // 2P攻撃ヒット管理
        if (hitP2toP1 && !prevHitP2toP1) {
            if (comp1 && !comp1->isGuarding) {
                comp1->TakeDamage(0.08f);   // ←必ずTakeDamage経由
            }
        }

        prevHitP2toP1 = hitP2toP1;
    }
}

void GameScene::Draw() {
    Camera* cam = engine->GetCamera();

    // サイドビュー
    cam->SetPosition({ 0.0f, 3.0f, -5.0f });
    cam->SetTarget({ 0.0f, 1.0f, 0.0f });

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
