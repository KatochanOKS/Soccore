#include "PlayerManager.h"
#include "Player1Component.h"
#include "Player2Component.h"
#include "ObjectFactory.h"
#include "Transform.h"
#include "Animator.h"
#include "Collider.h"
#include <DirectXMath.h>
#include <cmath>
using namespace DirectX;

void PlayerManager::InitPlayers(EngineManager* engine, std::vector<GameObject*>& sceneObjects) {
    // 1P¶¬
    int p1TexIdx = engine->GetTextureManager()->LoadTexture(L"assets/Mutant.fbm/Mutant_diffuse.png", engine->GetDeviceManager()->GetCommandList());
    m_Player1 = ObjectFactory::CreateSkinningBaseModel(
        engine, "assets/Mutant.fbx",
        { -0.7f, 0.0f, 0.0f },
        { 0.01f, 0.01f, 0.01f },
        p1TexIdx, Colors::White,
        { 0,0.85f,0 }, { 1.5f,1.7f,1.5f }, "Player", "Player1"
    );
    m_Player1->AddComponent<Player1Component>();
    m_Player1->GetComponent<Transform>()->rotation.y = XMConvertToRadians(90.0f);
    if (auto* comp1 = m_Player1->GetComponent<Player1Component>()) comp1->Start();

    // 2P¶¬
    int p2TexIdx = engine->GetTextureManager()->LoadTexture(L"assets/MMA2/SkeletonzombieTAvelange.fbm/skeletonZombie_diffuse.png", engine->GetDeviceManager()->GetCommandList());
    m_Player2 = ObjectFactory::CreateSkinningBaseModel(
        engine, "assets/MMA2/SkeletonzombieTAvelange.fbx",
        { 0.7f, 0.0f, 0.0f },
        { 0.01f, 0.01f, 0.01f },
        p2TexIdx, Colors::White,
        { 0,0.9f,0 }, { 1.5f,1.8f,1.5f }, "Enemy", "Player2"
    );
    m_Player2->AddComponent<Player2Component>();
    m_Player2->GetComponent<Transform>()->rotation.y = XMConvertToRadians(-90.0f);
    if (auto* comp2 = m_Player2->GetComponent<Player2Component>()) comp2->Start();

    sceneObjects.push_back(m_Player1);
    sceneObjects.push_back(m_Player2);
}

GameObject* PlayerManager::GetPlayer1() {
    return m_Player1;
}
GameObject* PlayerManager::GetPlayer2() {
    return m_Player2;
}
GameObject* PlayerManager::GetPlayerByName(const std::string& name) {
    if (m_Player1 && m_Player1->name == name) return m_Player1;
    if (m_Player2 && m_Player2->name == name) return m_Player2;
    return nullptr;
}

void PlayerManager::UpdatePlayers() {
    // ƒvƒŒƒCƒ„[“¯Žm‚ÌUŒ‚”»’èEŽ€–S”»’è‚È‚Ç
    if (!m_Player1 || !m_Player2) return;

    auto* tr1 = m_Player1->GetComponent<Transform>();
    auto* tr2 = m_Player2->GetComponent<Transform>();
    auto* col1 = m_Player1->GetComponent<Collider>();
    auto* col2 = m_Player2->GetComponent<Collider>();
    if (!tr1 || !tr2 || !col1 || !col2) return;

    XMFLOAT3 minA, maxA, minB, maxB;
    col1->GetAABBWorld(tr1, minA, maxA);
    col2->GetAABBWorld(tr2, minB, maxB);

    auto* anim1 = m_Player1->GetComponent<Animator>();
    auto* comp1 = m_Player1->GetComponent<Player1Component>();
    auto* anim2 = m_Player2->GetComponent<Animator>();
    auto* comp2 = m_Player2->GetComponent<Player2Component>();

    // ƒvƒŒƒCƒ„[1‚ÌUŒ‚”»’è
    static bool prevHitP1toP2 = false;
    bool isP1AttackAnim = (anim1 && (anim1->currentAnim == "Punch" || anim1->currentAnim == "Kick") && anim1->isPlaying);
    bool hitP1toP2 = isP1AttackAnim &&
        (minA.x <= maxB.x && maxA.x >= minB.x) &&
        (minA.y <= maxB.y && maxA.y >= minB.y) &&
        (minA.z <= maxB.z && maxA.z >= minB.z);

    if (hitP1toP2 && !prevHitP1toP2) {
        if (comp2 && !comp2->isGuarding) {
            comp2->TakeDamage(2.0f);
        }
    }
    prevHitP1toP2 = hitP1toP2;

    // ƒvƒŒƒCƒ„[2‚ÌUŒ‚”»’è
    static bool prevHitP2toP1 = false;
    bool isP2AttackAnim = (anim2 && (anim2->currentAnim == "Punch" || anim2->currentAnim == "Kick") && anim2->isPlaying);
    bool hitP2toP1 = isP2AttackAnim &&
        (minB.x <= maxA.x && maxB.x >= minA.x) &&
        (minB.y <= maxA.y && maxB.y >= minA.y) &&
        (minB.z <= maxA.z && maxB.z >= minA.z);

    if (hitP2toP1 && !prevHitP2toP1) {
        if (comp1 && !comp1->isGuarding) {
            comp1->TakeDamage(3.0f);
        }
    }
    prevHitP2toP1 = hitP2toP1;

    // Ž€–S‰‰oI—¹ŒŸo
    if (comp1 && comp1->state == PlayerState::Dying) {
        if (anim1 && !anim1->isPlaying) {
            m_IsP1DyingEnded = true;
        }
    }
    if (comp2 && comp2->state == PlayerState::Dying) {
        if (anim2 && !anim2->isPlaying) {
            m_IsP2DyingEnded = true;
        }
    }
}