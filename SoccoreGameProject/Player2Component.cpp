#include "Player2Component.h"
#include "Transform.h"
#include "Animator.h"
#include "GameObject.h"
#include "UIImage.h"
#include "GameScene.h"
#include <windows.h>

void Player2Component::Update() {
    auto* tr = gameObject->GetComponent<Transform>();
    auto* animator = gameObject->GetComponent<Animator>();
    if (!tr || !animator) return;

    // 入力取得（2P: U=ガード, P=キック, L=パンチ, 矢印で移動）
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
            // 矢印移動
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
        // 死亡時は何もしない
        break;

    case PlayerState::Win:
        // 勝利状態
        break;
    }

    // HPバーUI（既存まま）
    const float delaySpeed = 0.001f;
    if (delayedHp > hp) {
        delayedHp -= delaySpeed;
        if (delayedHp < hp) delayedHp = hp;
    }
    else {
        delayedHp = hp;
    }

    const float HPBAR_MAX_WIDTH = 500.0f;
    const float HPBAR_RIGHT_EDGE = 1280.0f;

    GameObject* hpRedBarObj = gameObject->scene->FindByName("HP2Red");
    if (hpRedBarObj) {
        auto* redBar = hpRedBarObj->GetComponent<UIImage>();
        float redWidth = HPBAR_MAX_WIDTH * delayedHp;
        if (redWidth < 0) redWidth = 0;
        redBar->size.x = redWidth;
        redBar->position.x = HPBAR_RIGHT_EDGE - redWidth;
    }

    GameObject* hpBarObj = gameObject->scene->FindByName("HP2");
    if (hpBarObj) {
        auto* bar = hpBarObj->GetComponent<UIImage>();
        float barWidth = HPBAR_MAX_WIDTH * hp;
        if (barWidth < 0) barWidth = 0;
        bar->size.x = barWidth;
        bar->position.x = HPBAR_RIGHT_EDGE - barWidth;
    }

    prevKickKey = kickKey;
    prevPunchKey = punchKey;
    prevGuardKey = guardKey;
}

// 被ダメージ受けるとき
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
        reactionTimer = 2.0f;
        auto* animator = gameObject->GetComponent<Animator>();
        if (animator) animator->SetAnimation("Reaction", false);
    }
}
