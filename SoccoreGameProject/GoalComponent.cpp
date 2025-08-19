#include "GoalComponent.h"
#include "GameObject.h"
#include "Transform.h"
#include "Collider.h"
#include <DirectXMath.h>
#include <windows.h>
using namespace DirectX;

void GoalComponent::Update() {
}

bool GoalComponent::CheckGoal(const XMFLOAT3& ballPos) {
    auto* tr = gameObject->GetComponent<Transform>();
    auto* col = gameObject->GetComponent<Collider>();
    if (!tr || !col) return false;

    XMFLOAT3 minW, maxW;
    col->GetAABBWorld(tr, minW, maxW);

    char buf[256];
    sprintf_s(buf,
        "GoalColliderAABB: min(%.2f,%.2f,%.2f) max(%.2f,%.2f,%.2f) center(%.2f,%.2f,%.2f) size(%.2f,%.2f,%.2f)\n",
        minW.x, minW.y, minW.z, maxW.x, maxW.y, maxW.z,
        col->center.x, col->center.y, col->center.z,
        col->size.x, col->size.y, col->size.z
    );
    OutputDebugStringA(buf);
    // ”»’è
    return (ballPos.x >= minW.x && ballPos.x <= maxW.x &&
        ballPos.y >= minW.y && ballPos.y <= maxW.y &&
        ballPos.z >= minW.z && ballPos.z <= maxW.z);



}
