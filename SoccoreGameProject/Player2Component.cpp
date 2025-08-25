#include "Player2Component.h"
#include "Transform.h"
#include "Animator.h"
#include "GameObject.h"
#include "UIImage.h"
#include "GameScene.h"    // �������K�������I
#include <windows.h>

void Player2Component::Update() {
    auto* tr = gameObject->GetComponent<Transform>();
    auto* animator = gameObject->GetComponent<Animator>();
    if (!tr || !animator) return;

    static bool prevKickKey = false;
    static bool prevPunchKey = false;
    static bool prevGuardKey = false;
    bool isMoving = false;

    // �L�[�A�T�C���͎��R�I�����ł�U�L�[�ŃK�[�h
    bool kickKey = (GetAsyncKeyState('P') & 0x8000); // �L�b�N
    bool punchKey = (GetAsyncKeyState('L') & 0x8000); // �p���`
    bool guardKey = (GetAsyncKeyState('U') & 0x8000); // �K�[�h

    // �U���A�j������
    bool isAttackAnim = (animator->currentAnim == "Kick" || animator->currentAnim == "Punch");

    isGuarding = guardKey; // ���ꂾ����OK�I

    // 1. �K�[�h�D��i�������ςȂ��ōĐ��j
    if (guardKey) {
        if (animator->currentAnim != "BodyBlock" || !animator->isPlaying) {
            animator->SetAnimation("BodyBlock", true); // �K�[�h�A�j���������킹�āI
        }
        // �K�[�h���͑��̑������؎󂯕t���Ȃ�
    }
    // 2. �U���A�j���D��
    else if (isAttackAnim && animator->isPlaying) {
        // �U���A�j�����͉������Ȃ�
    }
    else if (isAttackAnim && !animator->isPlaying) {
        animator->SetAnimation("Idle", true);
    }
    // 3. �ʏ푀��
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
    // ======= HP�o�[�̉�����HP�ɉ����ĕω������� =======
   // HP�o�[�̍ő剡���iStart�Ŏw�肵��size.x�Ɠ����l���g���j
    const float HPBAR_MAX_WIDTH = 500.0f;
    const float HPBAR_RIGHT_EDGE = 1280.0f; // HP�o�[�̉E�[�ʒu�i��ʉE���ɒ����j

    GameObject* hpBarObj = gameObject->scene->FindByName("HP2");
    if (hpBarObj) {
        auto* bar = hpBarObj->GetComponent<UIImage>();
        float barWidth = HPBAR_MAX_WIDTH * hp;
        if (barWidth < 0) barWidth = 0;
        bar->size.x = barWidth;

        // �E�[����ɓ����ʒu�iHPBAR_RIGHT_EDGE�j�ɂȂ�悤�ɍ��[�iposition.x�j�����炷
        bar->position.x = HPBAR_RIGHT_EDGE - barWidth;
    }

        // ���͏�Ԃ��X�V
        prevKickKey = kickKey;
        prevPunchKey = punchKey;
        prevGuardKey = guardKey;

}

