#include "Collider.h"
#include "Transform.h"
#include "GameObject.h"
#include <algorithm>
using namespace DirectX;

void Collider::GetAABBWorld(XMFLOAT3& outMin, XMFLOAT3& outMax) {
    auto* tr = gameObject->GetComponent<Transform>();
    XMFLOAT3 pos = tr->position;
    XMFLOAT3 scl = tr->scale;

    outMin = {
        pos.x + (center.x - size.x * 0.5f) * scl.x,
        pos.y + (center.y - size.y * 0.5f) * scl.y,
        pos.z + (center.z - size.z * 0.5f) * scl.z
    };
    outMax = {
        pos.x + (center.x + size.x * 0.5f) * scl.x,
        pos.y + (center.y + size.y * 0.5f) * scl.y,
        pos.z + (center.z + size.z * 0.5f) * scl.z
    };
}
