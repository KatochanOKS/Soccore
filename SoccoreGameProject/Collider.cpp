// Collider.cpp
#include "Collider.h"
#include "Transform.h"
#include "GameObject.h"
#include <cmath>
using namespace DirectX;

void Collider::AutoFitToTransform(Transform* tr) {
    size = tr->scale;
    center = { 0, 0, 0 };
}
void Collider::GetAABBWorld(XMFLOAT3& outMin, XMFLOAT3& outMax) {
    auto* tr = gameObject->GetComponent<Transform>();
    XMFLOAT3 pos = tr->position;
    XMFLOAT3 scl = tr->scale;
    float cx = (float)center.x, cy = (float)center.y, cz = (float)center.z;
    float sx = (float)size.x, sy = (float)size.y, sz = (float)size.z;
    float scx = (float)scl.x, scy = (float)scl.y, scz = (float)scl.z;

    outMin = XMFLOAT3(
        pos.x + cx * scx - (sx * 0.5f) * std::abs(scx),
        pos.y + cy * scy - (sy * 0.5f) * std::abs(scy),
        pos.z + cz * scz - (sz * 0.5f) * std::abs(scz)
    );
    outMax = XMFLOAT3(
        pos.x + cx * scx + (sx * 0.5f) * std::abs(scx),
        pos.y + cy * scy + (sy * 0.5f) * std::abs(scy),
        pos.z + cz * scz + (sz * 0.5f) * std::abs(scz)
    );
}
