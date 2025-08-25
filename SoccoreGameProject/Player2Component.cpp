#include "Player2Component.h"
#include "Transform.h"
#include "Animator.h"
#include "GameObject.h"
#include "UIImage.h"
#include "GameScene.h"    // ←これを必ず書く！
#include <windows.h>

void Player2Component::Update() {
    auto* tr = gameObject->GetComponent<Transform>();
    auto* animator = gameObject->GetComponent<Animator>();
    if (!tr || !animator) return;

    static bool prevKickKey = false;
    static bool prevPunchKey = false;
    static bool prevGuardKey = false;
    bool isMoving = false;

    // キーアサインは自由！ここではUキーでガード
    bool kickKey = (GetAsyncKeyState('P') & 0x8000); // キック
    bool punchKey = (GetAsyncKeyState('L') & 0x8000); // パンチ
    bool guardKey = (GetAsyncKeyState('U') & 0x8000); // ガード

    // 攻撃アニメ中か
    bool isAttackAnim = (animator->currentAnim == "Kick" || animator->currentAnim == "Punch");

    isGuarding = guardKey; // これだけでOK！

    // 1. ガード優先（押しっぱなしで再生）
    if (guardKey) {
        if (animator->currentAnim != "BodyBlock" || !animator->isPlaying) {
            animator->SetAnimation("BodyBlock", true); // ガードアニメ名を合わせて！
        }
        // ガード中は他の操作を一切受け付けない
    }
    // 2. 攻撃アニメ優先
    else if (isAttackAnim && animator->isPlaying) {
        // 攻撃アニメ中は何もしない
    }
    else if (isAttackAnim && !animator->isPlaying) {
        animator->SetAnimation("Idle", true);
    }
    // 3. 通常操作
    else {
        if (kickKey && !prevKickKey) {
            animator->SetAnimation("Kick", false);
        }
        else if (punchKey && !prevPunchKey) {
            animator->SetAnimation("Punch", false);
        }
        else {
            if (GetAsyncKeyState(VK_LEFT) & 0x8000) { tr->position.x -= moveSpeed; isMoving = true; }
            if (GetAsyncKeyState(VK_RIGHT) & 0x8000) { tr->position.x += moveSpeed; isMoving = true; }
            if (GetAsyncKeyState(VK_UP) & 0x8000) { tr->position.z -= moveSpeed; isMoving = true; }
            if (GetAsyncKeyState(VK_DOWN) & 0x8000) { tr->position.z += moveSpeed; isMoving = true; }

            if (isMoving) {
                if (animator->currentAnim != "Walk") animator->SetAnimation("Walk", true);
            }
            else {
                if (animator->currentAnim != "Idle") animator->SetAnimation("Idle", true);
            }
        }
    }
    // ======= HPバーの横幅をHPに応じて変化させる =======
   // HPバーの最大横幅（Startで指定したsize.xと同じ値を使う）
    const float HPBAR_MAX_WIDTH = 500.0f;
    const float HPBAR_RIGHT_EDGE = 1280.0f; // HPバーの右端位置（画面右寄りに調整）

    GameObject* hpBarObj = gameObject->scene->FindByName("HP2");
    if (hpBarObj) {
        auto* bar = hpBarObj->GetComponent<UIImage>();
        float barWidth = HPBAR_MAX_WIDTH * hp;
        if (barWidth < 0) barWidth = 0;
        bar->size.x = barWidth;

        // 右端が常に同じ位置（HPBAR_RIGHT_EDGE）になるように左端（position.x）をずらす
        bar->position.x = HPBAR_RIGHT_EDGE - barWidth;
    }

        // 入力状態を更新
        prevKickKey = kickKey;
        prevPunchKey = punchKey;
        prevGuardKey = guardKey;

}

