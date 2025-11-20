#include "Player1Component.h"
#include "Transform.h"
#include "Animator.h"
#include "GameObject.h"
#include "UIImage.h"
#include "GameScene.h"
#include <windows.h>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

// LuaパラメータをPlayer1Componentに適用する関数
void Player1Component::LoadConfigFromLua() {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    if (luaL_dofile(L, "assets/scripts/player1_config.lua") != LUA_OK) {
        const char* err = lua_tostring(L, -1);
        OutputDebugStringA(err);
        lua_close(L);
        return;
    }

    // maxHp
    lua_getglobal(L, "maxHp");
    if (lua_isnumber(L, -1)) maxHp = (float)lua_tonumber(L, -1);
    lua_pop(L, 1);

    // hp
    lua_getglobal(L, "hp");
    if (lua_isnumber(L, -1)) hp = (float)lua_tonumber(L, -1);
    lua_pop(L, 1);

    // speed
    lua_getglobal(L, "speed");
    if (lua_isnumber(L, -1)) moveSpeed = (float)lua_tonumber(L, -1);
    lua_pop(L, 1);

    // name
    lua_getglobal(L, "name");
    if (lua_isstring(L, -1)) {
        const char* name_utf8 = lua_tostring(L, -1);
        name = name_utf8;
    }
    lua_pop(L, 1);

    lua_close(L);

    // デバッグ表示
    char buf[128];
    sprintf_s(buf, "Player1Config: maxHp=%.1f, hp=%.1f, speed=%.3f, name=%s\n", maxHp, hp, moveSpeed, name.c_str());
    OutputDebugStringA(buf);
}


void Player1Component::Start() {
    // 最初の読込時に更新時刻も覚える
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesExA("assets/scripts/player1_config.lua", GetFileExInfoStandard, &data)) {
        lastWriteTime = data.ftLastWriteTime;
    }
    LoadConfigFromLua();
}

void Player1Component::Update() {

    // Luaファイル更新監視（自動リロード！）
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesExA("assets/scripts/player1_config.lua", GetFileExInfoStandard, &data)) {
        if (CompareFileTime(&lastWriteTime, &data.ftLastWriteTime) != 0) {
            lastWriteTime = data.ftLastWriteTime;
            LoadConfigFromLua();
            OutputDebugStringA("Luaホットリロード!\n");
        }
    }

    auto* tr = gameObject->GetComponent<Transform>();
    auto* animator = gameObject->GetComponent<Animator>();
    if (!tr || !animator) return;

    // キー入力
    bool kickKey = (GetAsyncKeyState('K') & 0x8000);
    bool punchKey = (GetAsyncKeyState('J') & 0x8000);
    bool guardKey = (GetAsyncKeyState('G') & 0x8000);
    bool moveL = (GetAsyncKeyState('A') & 0x8000);
    bool moveR = (GetAsyncKeyState('D') & 0x8000);
    bool isMoving = moveL || moveR;

    static bool prevKickKey = false, prevPunchKey = false, prevGuardKey = false;

    // 状態遷移とアニメ制御
    switch (state) {
    case PlayerState::Idle:
        isGuarding = false;
        if (guardKey) {
            state = PlayerState::Guard;
            animator->SetAnimation("BodyBlock", true);
            isGuarding = true;
        }
        else if (kickKey && !prevKickKey) {
            state = PlayerState::Attack;
            animator->SetAnimation("Kick", false);
        }
        else if (punchKey && !prevPunchKey) {
            state = PlayerState::Attack;
            animator->SetAnimation("Punch", false);
        }
        else if (isMoving) {
            state = PlayerState::Move;
            animator->SetAnimation("Walk", true);
        }
        else {
            if (animator->currentAnim != "Idle")
                animator->SetAnimation("Idle", true);
        }
        break;

    case PlayerState::Move:
        isGuarding = false;
        if (guardKey) {
            state = PlayerState::Guard;
            animator->SetAnimation("BodyBlock", true);
            isGuarding = true;
        }
        else if (kickKey && !prevKickKey) {
            state = PlayerState::Attack;
            animator->SetAnimation("Kick", false);
        }
        else if (punchKey && !prevPunchKey) {
            state = PlayerState::Attack;
            animator->SetAnimation("Punch", false);
        }
        else if (!isMoving) {
            state = PlayerState::Idle;
            animator->SetAnimation("Idle", true);
        }
        else {
            // 移動
            if (moveL) tr->position.x -= moveSpeed;
            if (moveR) tr->position.x += moveSpeed;
        }
        break;

    case PlayerState::Attack:
        // 攻撃アニメ終了後Idle
        if (!animator->isPlaying) {
            state = PlayerState::Idle;
            animator->SetAnimation("Idle", true);
        }
        break;

    case PlayerState::Guard:
        isGuarding = true;
        if (!guardKey) {
            state = PlayerState::Idle;
            animator->SetAnimation("Idle", true);
            isGuarding = false;
        }
        break;

    case PlayerState::Reaction:
        // ダメージ演出（一定時間でIdleに戻る）
        reactionTimer -= 0.5f / 60.0f;
        if (reactionTimer <= 0.0f) {
            state = PlayerState::Idle;
            animator->SetAnimation("Idle", true);
        }
        break;

    case PlayerState::Dying:
        
        break;

    case PlayerState::Win:
        // 勝利状態
        break;
    }

    // HPバーUIの更新（既存のまま）
    const float delaySpeed = 0.005f;
    if (delayedHp > hp) {
        delayedHp -= delaySpeed;
        if (delayedHp < hp) delayedHp = hp;
    }
    else {
        delayedHp = hp;
    }

    const float HPBAR_MAX_WIDTH = 500.0f;
    const float HPBAR_LEFT_EDGE = 0.0f;

    GameObject* hpRedBarObj = gameObject->scene->FindByName("HP1Red");
    if (hpRedBarObj) {
        auto* redBar = hpRedBarObj->GetComponent<UIImage>();
        float redWidth = HPBAR_MAX_WIDTH * (delayedHp / maxHp);   // ←修正ポイント
        if (redWidth < 0) redWidth = 0;
        redBar->m_Size.x = redWidth;
        redBar->m_Position.x = HPBAR_LEFT_EDGE;
    }

    GameObject* hpBarObj = gameObject->scene->FindByName("HP1");
    if (hpBarObj) {
        auto* bar = hpBarObj->GetComponent<UIImage>();
        float barWidth = HPBAR_MAX_WIDTH * (hp / maxHp);          // ←修正ポイント
        if (barWidth < 0) barWidth = 0;
        bar->m_Size.x = barWidth;
        bar->m_Position.x = HPBAR_LEFT_EDGE;
    }


    // 入力状態保存
    prevKickKey = kickKey;
    prevPunchKey = punchKey;
    prevGuardKey = guardKey;
}

// 被ダメージ受けるときGameScene等から呼び出す
void Player1Component::TakeDamage(float amount) {
    if (state == PlayerState::Dying) return;
    hp -= amount;
    if (hp <= 0.0f) {
        hp = 0.0f;
        state = PlayerState::Dying;
        auto* animator = gameObject->GetComponent<Animator>();
        if (animator) animator->SetAnimation("Dying", false);
    }
    else {
        state = PlayerState::Reaction;
        reactionTimer = 0.8f; // リアクション時間
        auto* animator = gameObject->GetComponent<Animator>();
        if (animator) animator->SetAnimation("Reaction", false);
    }
}
