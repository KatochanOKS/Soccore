// MeshLibrary.cpp
#include "MeshLibrary.h"
#include<DirectXMath.h>
void MeshLibrary::GetCubeMesh(std::vector<Vertex>& vertices, std::vector<uint16_t>& indices) {
    vertices = {
        // 前面 (Z-)
        { -0.5f, -0.5f, -0.5f,  0, 0, -1, 0, 1 },
        { -0.5f,  0.5f, -0.5f,  0, 0, -1, 0, 0 },
        {  0.5f,  0.5f, -0.5f,  0, 0, -1, 1, 0 },
        {  0.5f, -0.5f, -0.5f,  0, 0, -1, 1, 1 },

        // 右面 (X+)
        { 0.5f, -0.5f, -0.5f,   1, 0, 0, 0, 1 },
        { 0.5f,  0.5f, -0.5f,   1, 0, 0, 0, 0 },
        { 0.5f,  0.5f,  0.5f,   1, 0, 0, 1, 0 },
        { 0.5f, -0.5f,  0.5f,   1, 0, 0, 1, 1 },

        // 後面 (Z+)
        { 0.5f, -0.5f,  0.5f,   0, 0, 1, 0, 1 },
        { 0.5f,  0.5f,  0.5f,   0, 0, 1, 0, 0 },
        { -0.5f,  0.5f,  0.5f,  0, 0, 1, 1, 0 },
        { -0.5f, -0.5f,  0.5f,  0, 0, 1, 1, 1 },

        // 左面 (X-)
        { -0.5f, -0.5f,  0.5f,  -1, 0, 0, 0, 1 },
        { -0.5f,  0.5f,  0.5f,  -1, 0, 0, 0, 0 },
        { -0.5f,  0.5f, -0.5f,  -1, 0, 0, 1, 0 },
        { -0.5f, -0.5f, -0.5f,  -1, 0, 0, 1, 1 },

        // 上面 (Y+)
        { -0.5f,  0.5f, -0.5f,  0, 1, 0, 0, 1 },
        { -0.5f,  0.5f,  0.5f,  0, 1, 0, 0, 0 },
        {  0.5f,  0.5f,  0.5f,  0, 1, 0, 1, 0 },
        {  0.5f,  0.5f, -0.5f,  0, 1, 0, 1, 1 },

        // 下面 (Y-)
        { -0.5f, -0.5f, -0.5f,  0, -1, 0, 0, 1 },
        { -0.5f, -0.5f,  0.5f,  0, -1, 0, 0, 0 },
        {  0.5f, -0.5f,  0.5f,  0, -1, 0, 1, 0 },
        {  0.5f, -0.5f, -0.5f,  0, -1, 0, 1, 1 },
    };
    indices = {
        0,1,2, 0,2,3,
        4,5,6, 4,6,7,
        8,9,10, 8,10,11,
        12,13,14, 12,14,15,
        16,17,18, 16,18,19,
        20,21,22, 20,22,23
    };
}

void MeshLibrary::GetQuadMesh2D(std::vector<Vertex>& vertices, std::vector<uint16_t>& indices) {
    // 画面XY平面に貼り付ける四角（左上基準、NDC想定で中心0,0）
    vertices = {
        { -0.5f,  0.5f, 0.0f, 0,0,-1, 0,0 }, // 左上
        {  0.5f,  0.5f, 0.0f, 0,0,-1, 1,0 }, // 右上
        {  0.5f, -0.5f, 0.0f, 0,0,-1, 1,1 }, // 右下
        { -0.5f, -0.5f, 0.0f, 0,0,-1, 0,1 }, // 左下
    };
    indices = { 0,1,2, 0,2,3 };
}

// MeshLibrary.cpp（末尾に追加）
void MeshLibrary::GetSphereMesh(std::vector<Vertex>& vertices, std::vector<uint16_t>& indices, int latDiv, int lonDiv) {
    vertices.clear(); indices.clear();
    const float r = 1.0f;

    for (int lat = 0; lat <= latDiv; ++lat) {
        float t = (float)lat / latDiv;
        float theta = t * DirectX::XM_PI;              // 0..PI
        float sinT = sinf(theta), cosT = cosf(theta);

        for (int lon = 0; lon <= lonDiv; ++lon) {
            float p = (float)lon / lonDiv;
            float phi = p * DirectX::XM_2PI;           // 0..2PI
            float sinP = sinf(phi), cosP = cosf(phi);

            Vertex v{};
            v.x = r * sinT * cosP;
            v.y = r * cosT;
            v.z = r * sinT * sinP;

            // ★内側から見るので法線は内向き（-pos）
            v.nx = -v.x; v.ny = -v.y; v.nz = -v.z;

            // equirectangular想定のUV
            v.u = p;      // 0..1
            v.v = t;      // 0..1

            vertices.push_back(v);
        }
    }
    for (int lat = 0; lat < latDiv; ++lat) {
        for (int lon = 0; lon < lonDiv; ++lon) {
            int i0 = lat * (lonDiv + 1) + lon;
            int i1 = i0 + (lonDiv + 1);

            indices.push_back(i0);
            indices.push_back(i1);
            indices.push_back(i0 + 1);

            indices.push_back(i0 + 1);
            indices.push_back(i1);
            indices.push_back(i1 + 1);
        }
    }
}

// MeshLibrary.cpp
void MeshLibrary::GetSphereMesh(std::vector<Vertex>& outVertices, std::vector<uint16_t>& outIndices, float radius, int slice, int stack) {
    using namespace DirectX;
    outVertices.clear();
    outIndices.clear();

    for (int y = 0; y <= stack; ++y) {
        float phi = XM_PI * y / stack;
        for (int x = 0; x <= slice; ++x) {
            float theta = XM_2PI * x / slice;
            XMFLOAT3 pos = {
                radius * sinf(phi) * cosf(theta),
                radius * cosf(phi),
                radius * sinf(phi) * sinf(theta)
            };
            XMFLOAT3 normal = pos;
            XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&normal));
            XMStoreFloat3(&normal, n);

            float u = (float)x / slice;
            float v = 1.0f - (float)y / stack;

            outVertices.push_back({
                pos.x, pos.y, pos.z,
                normal.x, normal.y, normal.z,
                u, v
                });
        }
    }

    for (int y = 0; y < stack; ++y) {
        for (int x = 0; x < slice; ++x) {
            int i0 = y * (slice + 1) + x;
            int i1 = i0 + 1;
            int i2 = i0 + (slice + 1);
            int i3 = i2 + 1;
            outIndices.push_back(i0);
            outIndices.push_back(i2);
            outIndices.push_back(i1);
            outIndices.push_back(i1);
            outIndices.push_back(i2);
            outIndices.push_back(i3);
        }
    }
}
