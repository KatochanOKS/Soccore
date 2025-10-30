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
#include "EngineManager.h"
#include "GameOverScene.h"
#include "PlayerManager.h"
#include <DirectXMath.h>
#include <cmath>
using namespace DirectX;

static bool sceneChanged = false;

/// <summary>
/// 指定したタグを持つGameObjectを検索する
/// </summary>
GameObject* GameScene::FindByTag(const std::string& tag) {
    for (auto* obj : m_SceneObjects) {
        if (obj->tag == tag) return obj;
    }
    return nullptr;
}

/// <summary>
/// 指定した名前を持つGameObjectを検索する
/// </summary>
GameObject* GameScene::FindByName(const std::string& name) {
    for (auto* obj : m_SceneObjects) {
        if (obj->name == name) return obj;
    }
    return nullptr;
}

/// <summary>
/// 2つのAABBの重なり判定を行う（攻撃判定用）
/// </summary>
bool CheckAABBOverlap(const XMFLOAT3& minA, const XMFLOAT3& maxA,
    const XMFLOAT3& minB, const XMFLOAT3& maxB) {
    return (minA.x <= maxB.x && maxA.x >= minB.x) &&
        (minA.y <= maxB.y && maxA.y >= minB.y) &&
        (minA.z <= maxB.z && maxA.z >= minB.z);
}

/// <summary>
/// ゲームシーンのコンストラクタ
/// </summary>
GameScene::GameScene(EngineManager* engine) : engine(engine) {}

/// <summary>
/// シーン開始時の初期化処理
/// </summary>
void GameScene::Start() {
    m_SceneObjects.clear();
    sceneChanged = false;  // シーン切り替えフラグ初期化
    m_UIManager.InitUI(engine, m_SceneObjects); // UI初期化
    m_PlayerManager.InitPlayers(engine, m_SceneObjects); // プレイヤー管理初期化
    InitStage();     // ステージ生成
    RegisterAnimations(); // アニメーション登録

    // プレイヤーのシーン参照セット
    FindByName("Player1")->scene = this;
    FindByName("Player2")->scene = this;
}

/// <summary>
/// 毎フレームの更新処理
/// </summary>
void GameScene::Update() {
    // 全ComponentのUpdate
    for (auto* obj : m_SceneObjects) {
        for (auto* comp : obj->components) comp->Update();
    }
    // 全アニメーターUpdate
    for (auto* obj : m_SceneObjects) {
        if (auto* animator = obj->GetComponent<Animator>())
            animator->Update(1.0f / 120.0f);
    }

    // プレイヤー同士の攻撃判定・死亡判定・ゲームオーバー判定はPlayerManagerに委譲
    m_PlayerManager.UpdatePlayers();

    // ゲームオーバーシーンへの遷移判定
    if (!sceneChanged && (m_PlayerManager.IsP1DyingEnded() || m_PlayerManager.IsP2DyingEnded())) {
        sceneChanged = true;
        engine->ChangeScene(std::make_unique<GameOverScene>(engine));
        return;
    }
}

/// <summary>
/// 毎フレームの描画処理
/// </summary>
void GameScene::Draw() {
    Camera* cam = engine->GetCamera();

    // サイドビューカメラ設定
    cam->SetPosition({ 0.0f, 2.0f, -5.0f });
    cam->SetTarget({ 0.0f, 2.0f, 0.0f });

    XMMATRIX view = cam->GetViewMatrix();
    XMMATRIX proj = cam->GetProjectionMatrix(
        engine->GetSwapChainManager()->GetWidth(),
        engine->GetSwapChainManager()->GetHeight()
    );

    // スカイドームの位置をカメラに追従
    auto* sky = FindByTag("Sky");
    if (sky) {
        auto* skyTr = sky->GetComponent<Transform>();
        if (skyTr) skyTr->position = cam->GetPosition();
    }

    constexpr size_t CBV_SIZE = 256;
    void* mapped = nullptr;
    auto* cb = engine->GetBufferManager()->GetConstantBuffer();
    cb->Map(0, nullptr, &mapped);

    // 各オブジェクトの定数バッファ更新
    for (size_t i = 0; i < m_SceneObjects.size(); ++i) {
        GameObject* obj = m_SceneObjects[i];
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
    // 通常オブジェクト描画
    for (size_t i = 0; i < m_SceneObjects.size(); ++i) {
        auto* obj = m_SceneObjects[i];
        if (!obj->GetComponent<UIImage>()) {
            engine->GetRenderer()->DrawObject(obj, i, view, proj);
        }
    }
    // UIオブジェクト描画（最後に描画）
    for (size_t i = 0; i < m_SceneObjects.size(); ++i) {
        auto* obj = m_SceneObjects[i];
        if (obj->GetComponent<UIImage>()) {
            engine->GetRenderer()->DrawObject(obj, i, view, proj);
        }
    }
    engine->GetRenderer()->EndFrame();
}

/// <summary>
/// ステージの初期化処理
/// </summary>
void GameScene::InitStage() {
    m_SceneObjects.push_back(ObjectFactory::CreateCube(
        engine, { 0.0f, -0.5f, 0.0f }, { 10.0f, 1.0f, 3.0f }, -1, Colors::Gray, { 0,0,0 }, { -1,-1,-1 }, "Ground", "GroundFloor"
    ));

    int reelTex = engine->GetTextureManager()->LoadTexture(L"assets/Slot/Reel.png", engine->GetDeviceManager()->GetCommandList());

    // スロットリール3本を横並びで生成
    for (int i = 0; i < 3; ++i) {
        float x = -2.0f + i * 2.0f;
        auto* reel = ObjectFactory::CreateCylinderReel(
            engine,
            { x, 2.0f, 0.0f },
            { 2.0f, 2.0f, 2.0f },
            reelTex,
            Colors::White,
            "Reel",
            "SlotReel" + std::to_string(i + 1)
        );
        m_SceneObjects.push_back(reel);
    }
}

/// <summary>
/// アニメーションの登録処理
/// </summary>
void GameScene::RegisterAnimations() {
    struct AnimEntry { const char* file; const char* name; };
    AnimEntry anims1[] = {
        { "assets/MMA/BodyBlock.fbx",    "BodyBlock"   },
        { "assets/MMA/MmaKick.fbx",      "Kick"        },
        { "assets/MMA/MutantDying.fbx",  "Dying"       },
        { "assets/MMA/Punching.fbx",     "Punch"       },
        { "assets/MMA/BouncingFightIdle.fbx", "Idle"   },
        { "assets/MMA/Walking.fbx",      "Walk"        },
        { "assets/MMA/TakingPunch.fbx",  "Reaction"    },
    };
    AnimEntry anims2[] = {
        { "assets/MMA2/OutwardBlock.fbx",    "BodyBlock"   },
        { "assets/MMA2/MmaKick.fbx",         "Kick"        },
        { "assets/MMA2/DyingBackwards.fbx",  "Dying"       },
        { "assets/MMA2/Punching2P.fbx",      "Punch"       },
        { "assets/MMA2/BouncingFightIdle2.fbx", "Idle"     },
        { "assets/MMA2/WalkingPlayer2.fbx",  "Walk"        },
        { "assets/MMA2/Kicking.fbx",         "Kick2"       },
        { "assets/MMA2/ZombieReactionHit.fbx", "Reaction"  },
    };

    auto* animator1 = FindByName("Player1")->GetComponent<Animator>();
    for (auto& anim1 : anims1) {
        std::vector<Animator::Keyframe> keys;
        double animLen;
        if (FbxModelLoader::LoadAnimationOnly(anim1.file, keys, animLen)) {
            animator1->AddAnimation(anim1.name, keys);
        }
    }
    auto* animator2 = FindByName("Player2")->GetComponent<Animator>();
    for (auto& anim2 : anims2) {
        std::vector<Animator::Keyframe> keys;
        double animLen;
        if (FbxModelLoader::LoadAnimationOnly(anim2.file, keys, animLen)) {
            animator2->AddAnimation(anim2.name, keys);
        }
    }
}

/// <summary>
/// ゲームシーンのデストラクタ
/// </summary>
GameScene::~GameScene() {
    for (auto* obj : m_SceneObjects) {
        delete obj;
    }
    m_SceneObjects.clear();
}