#include "Collider.h"
#include "Transform.h"

// ���[���h���WAABB�i��]�͍l�����Ȃ�Cube�Ƃ��Čv�Z�j
void Collider::GetAABBWorld(const Transform* tr, DirectX::XMFLOAT3& outMin, DirectX::XMFLOAT3& outMax) const {
    // ���[�J����Ԃ�AABB�����[���h���W��
    float halfX = size.x * 0.5f * tr->scale.x;
    float halfY = size.y * 0.5f * tr->scale.y;
    float halfZ = size.z * 0.5f * tr->scale.z;

    outMin.x = tr->position.x + center.x * tr->scale.x - halfX;
    outMin.y = tr->position.y + center.y * tr->scale.y - halfY;
    outMin.z = tr->position.z + center.z * tr->scale.z - halfZ;

    outMax.x = tr->position.x + center.x * tr->scale.x + halfX;
    outMax.y = tr->position.y + center.y * tr->scale.y + halfY;
    outMax.z = tr->position.z + center.z * tr->scale.z + halfZ;
}
