#include "Player2Component.h"
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

void Player2Component::LoadConfigFromLua() {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    if (luaL_dofile(L, "assets/scripts/player2_config.lua") != LUA_OK) {
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

    // �f�o�b�O�o��
    char buf[128];
    sprintf_s(buf, "Player2Config: maxHp=%.1f, hp=%.1f, speed=%.3f, name=%s\n", maxHp, hp, moveSpeed, name.c_str());
    OutputDebugStringA(buf);
}

void Player2Component::Start() {
    // �ŏ��̓Ǎ����ɍX�V�������o����
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesExA("assets/scripts/player2_config.lua", GetFileExInfoStandard, &data)) {
        lastWriteTime = data.ftLastWriteTime;
    }
    LoadConfigFromLua();
}
void Player2Component::Update() {

    // Lua�t�@�C���X�V�Ď��i���������[�h�I�j
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesExA("assets/scripts/player2_config.lua", GetFileExInfoStandard, &data)) {
        if (CompareFileTime(&lastWriteTime, &data.ftLastWriteTime) != 0) {
            lastWriteTime = data.ftLastWriteTime;
            LoadConfigFromLua();
            OutputDebugStringA("Lua�z�b�g�����[�h!\n");
        }
    }

    auto* tr = gameObject->GetComponent<Transform>();
    auto* animator = gameObject->GetComponent<Animator>();
    if (!tr || !animator) return;

    // ���͎擾�i2P: U=�K�[�h, P=�L�b�N, L=�p���`, ���ňړ��j
    bool kickKey = (GetAsyncKeyState('P') & 0x8000);
    bool punchKey = (GetAsyncKeyState('L') & 0x8000);
    bool guardKey = (GetAsyncKeyState('U') & 0x8000);
    bool moveL = (GetAsyncKeyState(VK_LEFT) & 0x8000);
    bool moveR = (GetAsyncKeyState(VK_RIGHT) & 0x8000);
    bool moveU = (GetAsyncKeyState(VK_UP) & 0x8000);
    bool moveD = (GetAsyncKeyState(VK_DOWN) & 0x8000);
    bool isMoving = moveL || moveR || moveU || moveD;

    static bool prevKickKey = false, prevPunchKey = false, prevGuardKey = false;

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
            // ���ړ�
            if (moveL) tr->position.x -= moveSpeed;
            if (moveR) tr->position.x += moveSpeed;
            if (moveU) tr->position.z -= moveSpeed;
            if (moveD) tr->position.z += moveSpeed;
        }
        break;

    case PlayerState::Attack:
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
        reactionTimer -= 1.0f / 60.0f;
        if (reactionTimer <= 0.0f) {
            state = PlayerState::Idle;
            animator->SetAnimation("Idle", true);
        }
        break;

    case PlayerState::Dying:
        // ���S���͉������Ȃ�
        break;

    case PlayerState::Win:
        // �������
        break;
    }

    // HP�o�[UI�i�����܂܁j
    const float delaySpeed = 0.001f;
    if (delayedHp > hp) {
        delayedHp -= delaySpeed;
        if (delayedHp < hp) delayedHp = hp;
    }
    else {
        delayedHp = hp;
    }

    const float HPBAR_MAX_WIDTH = 500.0f;
    const float HPBAR_RIGHT_EDGE = 1280.0f - HPBAR_MAX_WIDTH; // �E���Ƀo�[���Œ肵�����ꍇ

    GameObject* hpRedBarObj = gameObject->scene->FindByName("HP2Red");
    if (hpRedBarObj) {
        auto* redBar = hpRedBarObj->GetComponent<UIImage>();
        float redWidth = HPBAR_MAX_WIDTH * (delayedHp / maxHp);
        if (redWidth < 0) redWidth = 0;
        redBar->m_Size.x = redWidth;
        redBar->m_Position.x = HPBAR_RIGHT_EDGE + (HPBAR_MAX_WIDTH - redWidth); // �E�[�Œ�
    }

    GameObject* hpBarObj = gameObject->scene->FindByName("HP2");
    if (hpBarObj) {
        auto* bar = hpBarObj->GetComponent<UIImage>();
        float barWidth = HPBAR_MAX_WIDTH * (hp / maxHp);
        if (barWidth < 0) barWidth = 0;
        bar->m_Size.x = barWidth;
        bar->m_Position.x = HPBAR_RIGHT_EDGE + (HPBAR_MAX_WIDTH - barWidth); // �E�[�Œ�
    }


    prevKickKey = kickKey;
    prevPunchKey = punchKey;
    prevGuardKey = guardKey;
}

// ��_���[�W�󂯂�Ƃ�
void Player2Component::TakeDamage(float amount) {
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
        reactionTimer = 1.5f;
        auto* animator = gameObject->GetComponent<Animator>();
        if (animator) animator->SetAnimation("Reaction", false);
    }
}
