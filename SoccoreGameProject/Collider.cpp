#include "Collider.h"
#include "Transform.h"
#include "GameObject.h"
#include<windows.h>
#include <algorithm>
#include <cmath>  // fabsf
using namespace DirectX;

void Collider::GetAABBWorld(XMFLOAT3& outMin, XMFLOAT3& outMax) {
    auto* tr = gameObject->GetComponent<Transform>();
    XMFLOAT3 pos = tr->position;
    XMFLOAT3 scl = tr->scale;

    // すべて(float)でキャストする
    float cx = (float)center.x, cy = (float)center.y, cz = (float)center.z;
    float sx = (float)size.x, sy = (float)size.y, sz = (float)size.z;
    float scx = (float)scl.x, scy = (float)scl.y, scz = (float)scl.z;

    outMin = XMFLOAT3(
        pos.x + cx * scx - (sx * 0.5f) * fabsf(scx),
        pos.y + cy * scy - (sy * 0.5f) * fabsf(scy),
        pos.z + cz * scz - (sz * 0.5f) * fabsf(scz)
    );
    outMax = XMFLOAT3(
        pos.x + cx * scx + (sx * 0.5f) * fabsf(scx),
        pos.y + cy * scy + (sy * 0.5f) * fabsf(scy),
        pos.z + cz * scz + (sz * 0.5f) * fabsf(scz)
    );

    // デバッグ出力（ここで異常値チェック）
    char buf[256];
    sprintf_s(buf, "AABBmin:(%.2f,%.2f,%.2f) max:(%.2f,%.2f,%.2f)\n",
        outMin.x, outMin.y, outMin.z, outMax.x, outMax.y, outMax.z);
    OutputDebugStringA(buf);
}


