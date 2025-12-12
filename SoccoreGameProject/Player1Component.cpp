#include "Player1Component.h"

#include "Animator.h"
#include "GameObject.h"
#include "GameScene.h"
#include "UIImage.h"

#include <windows.h>

extern "C"
{
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

/* ===============================
   Lua 設定読み込み
================================ */

void Player1Component::LoadConfigFromLua()
{
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    if (luaL_dofile(L, "assets/scripts/player1_config.lua") != LUA_OK)
    {
        OutputDebugStringA(lua_tostring(L, -1));
        lua_close(L);
        return;
    }

    lua_getglobal(L, "maxHp");
    if (lua_isnumber(L, -1))
        maxHp = (float)lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_getglobal(L, "hp");
    if (lua_isnumber(L, -1))
        hp = (float)lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_close(L);

    delayedHp = hp;
}

/* ===============================
   初期化
================================ */

void Player1Component::Start()
{
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesExA("assets/scripts/player1_config.lua", GetFileExInfoStandard, &data))
    {
        lastWriteTime = data.ftLastWriteTime;
    }

    LoadConfigFromLua();
    state = PlayerState::Idle;
}

/* ===============================
   毎フレーム更新（入力なし）
================================ */

void Player1Component::Update()
{
    // Luaホットリロード
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesExA("assets/scripts/player1_config.lua", GetFileExInfoStandard, &data))
    {
        if (CompareFileTime(&lastWriteTime, &data.ftLastWriteTime) != 0)
        {
            lastWriteTime = data.ftLastWriteTime;
            LoadConfigFromLua();
        }
    }

    auto* animator = gameObject->GetComponent<Animator>();
    if (!animator)
        return;

    switch (state)
    {
    case PlayerState::Idle:
        if (animator->currentAnim != "Idle")
            animator->SetAnimation("Idle", true);
        break;

    case PlayerState::Attack:
        if (!animator->isPlaying)
        {
            state = PlayerState::Idle;
            animator->SetAnimation("Idle", true);
        }
        break;

    case PlayerState::Reaction:
        reactionTimer -= 1.0f / 60.0f;
        if (reactionTimer <= 0.0f)
        {
            state = PlayerState::Idle;
            animator->SetAnimation("Idle", true);
        }
        break;

    case PlayerState::Dying:
        // 何もしない（GameScene側で遷移管理）
        break;

    default:
        break;
    }

    UpdateHpBar();
}

/* ===============================
   外部命令：攻撃実行
================================ */

void Player1Component::ExecuteAttack(const std::string& animName)
{
    if (state == PlayerState::Dying)
        return;

    state = PlayerState::Attack;

    if (auto* animator = gameObject->GetComponent<Animator>())
    {
        animator->SetAnimation(animName, false);
    }
}

/* ===============================
   外部命令：ダメージ
================================ */

void Player1Component::TakeDamage(float amount)
{
    if (state == PlayerState::Dying)
        return;

    hp -= amount;
    if (hp <= 0.0f)
    {
        hp    = 0.0f;
        state = PlayerState::Dying;
        if (auto* animator = gameObject->GetComponent<Animator>())
            animator->SetAnimation("Dying", false);
    }
    else
    {
        state         = PlayerState::Reaction;
        reactionTimer = 0.8f;
        if (auto* animator = gameObject->GetComponent<Animator>())
            animator->SetAnimation("Reaction", false);
    }
}

/* ===============================
   HPバー更新（演出）
================================ */

void Player1Component::UpdateHpBar()
{
    const float delaySpeed = 0.005f;

    if (delayedHp > hp)
    {
        delayedHp -= delaySpeed;
        if (delayedHp < hp)
            delayedHp = hp;
    }
    else
    {
        delayedHp = hp;
    }

    constexpr float MAX_WIDTH = 500.0f;

    if (auto* red = gameObject->scene->FindByName("HP1Red"))
    {
        if (auto* img = red->GetComponent<UIImage>())
            img->m_Size.x = MAX_WIDTH * (delayedHp / maxHp);
    }

    if (auto* bar = gameObject->scene->FindByName("HP1"))
    {
        if (auto* img = bar->GetComponent<UIImage>())
            img->m_Size.x = MAX_WIDTH * (hp / maxHp);
    }
}
