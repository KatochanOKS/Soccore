#include "Collider.h"
#include "Transform.h"

// ���[���h���WAABB�i��]�͍l�����Ȃ�Cube�Ƃ��Čv�Z�j
void Collider::GetAABBWorld(const Transform* tr, DirectX::XMFLOAT3& outMin, DirectX::XMFLOAT3& outMax) const {
    // scale���|���Ȃ���Collider�̃T�C�Y�����̂܂܎g��
    float halfX = size.x * 0.5f;
    float halfY = size.y * 0.5f;
    float halfZ = size.z * 0.5f;

    outMin.x = tr->position.x + center.x - halfX;
    outMin.y = tr->position.y + center.y - halfY;
    outMin.z = tr->position.z + center.z - halfZ;

    outMax.x = tr->position.x + center.x + halfX;
    outMax.y = tr->position.y + center.y + halfY;
    outMax.z = tr->position.z + center.z + halfZ;
}

