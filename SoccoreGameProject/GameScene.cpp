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
#include <algorithm>
#include <DirectXMath.h>
#include <cmath>
#include <Windows.h>
#undef min
#undef max

#include "imgui.h"


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
GameScene::GameScene(EngineManager* engine)
    : engine(engine)
	, m_StageManage(engine) // ステージ管理初期化
{}

/// <summary>
/// シーン開始時の初期化処理
/// </summary>
void GameScene::Start() {
    m_SceneObjects.clear();
    sceneChanged = false;  // シーン切り替えフラグ初期化
    m_UIManager.InitUI(engine, m_SceneObjects); // UI初期化
    m_PlayerManager.InitPlayers(engine, m_SceneObjects); // プレイヤー管理初期化
    m_StageManage.InitStage(m_SceneObjects); // ステージ初期化
    RegisterAnimations(); // アニメーション登録

    // これが無いと Controller や UI に scene が入らない
    for (auto* obj : m_SceneObjects) obj->scene = this;
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

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Once);
    ImGui::SetNextWindowBgAlpha(0.35f);
    ImGuiWindowFlags fpsFlags = ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;
    if (ImGui::Begin("Performance", nullptr, fpsFlags)) {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::Text("FPS: %.1f", io.Framerate);
        ImGui::Text("Frame Time: %.3f ms", 1000.0f / io.Framerate);
    }
    ImGui::End();

    ImGui::Begin("Mouse Debug");
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("MousePos: %.1f, %.1f", io.MousePos.x, io.MousePos.y);
    ImGui::Text("DisplaySize: %.1f, %.1f", io.DisplaySize.x, io.DisplaySize.y);
    ImGui::Text("FramebufferScale: %.2f, %.2f", io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    ImGui::End();

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
            cbData.Color = smr->m_Color;
            cbData.UseTexture = smr->m_TexIndex >= 0 ? 1 : 0;
        }
        else if (auto* mr = obj->GetComponent<StaticMeshRenderer>()) {
            cbData.Color = mr->m_Color;
            cbData.UseTexture = mr->m_TexIndex >= 0 ? 1 : 0;
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
/// アニメーションの登録処理
/// </summary>
void GameScene::RegisterAnimations() {
    auto* animator1 = FindByName("Player1")->GetComponent<Animator>();
    auto* animator2 = FindByName("Player2")->GetComponent<Animator>();
    m_AnimationManager.RegisterPlayerAnimations(animator1, animator2);
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
////namespace {
//int x;
//}
//は内部的に
//
//cpp コードをコピーする static int x;
//とほぼ同じ意味。

namespace {

    enum class SlotRole { Big, Bell, Replay, Bar, Power, Miss };

    static SlotRole ParseSlotResult(const std::string& s) {
        if (s == "BIG")           return SlotRole::Big;
        if (s == "ベル揃い")      return SlotRole::Bell;
        if (s == "リプレイ")      return SlotRole::Replay;
        if (s == "BAR揃い")       return SlotRole::Bar;
        if (s == "力揃い")        return SlotRole::Power;
        return SlotRole::Miss;
    }

    template<class TComp>
    void SafeDamage(GameObject* obj, float dmg) {
        if (!obj) return;
        if (auto* c = obj->GetComponent<TComp>()) c->TakeDamage(dmg);
    }

}

void GameScene::ApplySlotEffect(const std::string& result, bool fromPlayer2)
{
    auto* p1 = m_PlayerManager.GetPlayer1();
    auto* p2 = m_PlayerManager.GetPlayer2();

    auto* c1 = p1 ? p1->GetComponent<Player1Component>() : nullptr;
    auto* c2 = p2 ? p2->GetComponent<Player2Component>() : nullptr;

    if (!c1 || !c2)
        return;

    // スロット役は1回だけ判定
    auto role = ParseSlotResult(result);

    const float DMG_SMALL  = 0.5f;
    const float DMG_MEDIUM = 1.0f;
    const float DMG_LARGE  = 1.5f;
    const float DMG_HUGE   = 3.0f;

    // 攻撃側 / 被攻撃側を決定（型を分けて保持）
    Player1Component* attacker1 = nullptr;
    Player2Component* attacker2 = nullptr;
    Player1Component* target1   = nullptr;
    Player2Component* target2   = nullptr;

    if (fromPlayer2)
    {
        attacker2 = c2;
        target1   = c1;
    }
    else
    {
        attacker1 = c1;
        target2   = c2;
    }

    switch (role)
    {
    case SlotRole::Big:
        if (attacker1)
        {
            attacker1->ExecuteAttack("Punch");
            target2->TakeDamage(DMG_HUGE);
        }
        else
        {
            attacker2->ExecuteAttack("Punch");
            target1->TakeDamage(DMG_HUGE);
        }
        break;

    case SlotRole::Bar:
        if (attacker1)
        {
            attacker1->ExecuteAttack("Punch");
            target2->TakeDamage(DMG_LARGE);
        }
        else
        {
            attacker2->ExecuteAttack("Punch");
            target1->TakeDamage(DMG_LARGE);
        }
        break;

    case SlotRole::Power:
        if (attacker1)
        {
            attacker1->ExecuteAttack("Punch");
            target2->TakeDamage(DMG_MEDIUM);
        }
        else
        {
            attacker2->ExecuteAttack("Punch");
            target1->TakeDamage(DMG_MEDIUM);
        }
        break;

    case SlotRole::Bell:
        if (attacker1)
        {
            attacker1->ExecuteAttack("Kick");
            target2->TakeDamage(DMG_MEDIUM);
        }
        else
        {
            attacker2->ExecuteAttack("Kick");
            target1->TakeDamage(DMG_MEDIUM);
        }
        break;

    case SlotRole::Replay:
        if (attacker1)
        {
            attacker1->hp = std::min(attacker1->maxHp, attacker1->hp + 0.10f);
        }
        else
        {
            attacker2->hp = std::min(attacker2->maxHp, attacker2->hp + 0.10f);
        }
        break;

    case SlotRole::Miss:
    default:
        break;
    }
}
