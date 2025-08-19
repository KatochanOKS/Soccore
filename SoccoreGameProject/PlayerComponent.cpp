#include "GameObject.h"
#include "Transform.h"
#include "Animator.h"
#include "PlayerComponent.h"
#include "Transform.h"
#include "Animator.h"
#include <DirectXMath.h>
#include <windows.h>

using namespace DirectX;

void PlayerComponent::Update() {
    auto* tr = gameObject->GetComponent<Transform>();
    auto* animator = gameObject->GetComponent<Animator>();
    bool isMoving = false;

    // ���͏���
    if (GetAsyncKeyState('W') & 0x8000) {
        tr->position.z += moveSpeed;
        tr->rotation.y = XMConvertToRadians(0.0f);
        isMoving = true;
    }
    if (GetAsyncKeyState('S') & 0x8000) {
        tr->position.z -= moveSpeed;
        tr->rotation.y = XMConvertToRadians(180.0f);
        isMoving = true;
    }
    if (GetAsyncKeyState('A') & 0x8000) {
        tr->position.x -= moveSpeed;
        tr->rotation.y = XMConvertToRadians(-90.0f);
        isMoving = true;
    }
    if (GetAsyncKeyState('D') & 0x8000) {
        tr->position.x += moveSpeed;
        tr->rotation.y = XMConvertToRadians(90.0f);
        isMoving = true;
    }

    // �A�j���[�V��������
    if (animator) {
        if (isMoving && animator->currentAnim != "Walk") {
            animator->SetAnimation("Walk");
        }
        else if (!isMoving && animator->currentAnim != "Idle") {
            animator->SetAnimation("Idle");
        }
    }
}
