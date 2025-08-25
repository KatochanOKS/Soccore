#include "Player1Component.h"
#include "Transform.h"
#include "Animator.h"
#include "GameObject.h"
#include <windows.h>

void Player1Component::Update() {
    Transform* tr = gameObject->GetComponent<Transform>();
    Animator* animator = gameObject->GetComponent<Animator>();
    if (!tr || !animator) return;

    static bool prevKickKey = false;
    static bool prevPunchKey = false;
    static bool prevGuardKey = false;
    bool isMoving = false;

    // 現在の入力取得
    bool kickKey = (GetAsyncKeyState('K') & 0x8000);
    bool punchKey = (GetAsyncKeyState('J') & 0x8000);
    bool guardKey = (GetAsyncKeyState('G') & 0x8000);

    // 攻撃アニメ中かどうか
    bool isAttackAnim = (animator->currentAnim == "Kick" || animator->currentAnim == "Punch");

    isGuarding = guardKey; // これだけでOK！

    // ★★★ 1. ガード優先判定（押しっぱなしで再生・途中割込禁止）★★★
    if (guardKey) {
        // すでにGuardアニメ中以外のときだけ切替
        if (animator->currentAnim != "BodyBlock" || !animator->isPlaying) {
            animator->SetAnimation("BodyBlock", true); // ループ再生
        }
        // ガード中は他の操作（移動・攻撃）を一切受け付けない
    }
    // ★★★ 2. 攻撃アニメ優先（ガード中以外で攻撃アニメ中）★★★
    else if (isAttackAnim && animator->isPlaying) {
        // 攻撃アニメ中は何もしない（割込禁止）
    }
    else if (isAttackAnim && !animator->isPlaying) {
        // 攻撃アニメ終了後はIdleに戻す
        animator->SetAnimation("Idle", true);
    }
    // ★★★ 3. 通常操作 ★★★
    else {
        // 攻撃入力（押した瞬間だけ受付。ループなし！）
        if (kickKey && !prevKickKey) {
            animator->SetAnimation("Kick", false);  // 1回だけ再生
        }
        else if (punchKey && !prevPunchKey) {
            animator->SetAnimation("Punch", false); // 1回だけ再生
        }
        else {
            // 通常移動（攻撃・ガードアニメ中以外）
            if (GetAsyncKeyState('A') & 0x8000) { tr->position.x -= moveSpeed; isMoving = true; }
            if (GetAsyncKeyState('D') & 0x8000) { tr->position.x += moveSpeed; isMoving = true; }
            if (GetAsyncKeyState('W') & 0x8000) { tr->position.z -= moveSpeed; isMoving = true; }
            if (GetAsyncKeyState('S') & 0x8000) { tr->position.z += moveSpeed; isMoving = true; }

            if (isMoving) {
                if (animator->currentAnim != "Walk") animator->SetAnimation("Walk", true);
            }
            else {
                if (animator->currentAnim != "Idle") animator->SetAnimation("Idle", true);
            }
        }
    }

    // 入力状態を更新
    prevKickKey = kickKey;
    prevPunchKey = punchKey;
    prevGuardKey = guardKey;
}
