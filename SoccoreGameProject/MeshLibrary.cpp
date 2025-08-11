// MeshLibrary.cpp
#include "MeshLibrary.h"
#include<DirectXMath.h>
void MeshLibrary::GetCubeMesh(std::vector<Vertex>& vertices, std::vector<uint16_t>& indices) {
    vertices = {
        // �O�� (Z-)
        { -0.5f, -0.5f, -0.5f,  0, 0, -1, 0, 1 },
        { -0.5f,  0.5f, -0.5f,  0, 0, -1, 0, 0 },
        {  0.5f,  0.5f, -0.5f,  0, 0, -1, 1, 0 },
        {  0.5f, -0.5f, -0.5f,  0, 0, -1, 1, 1 },

        // �E�� (X+)
        { 0.5f, -0.5f, -0.5f,   1, 0, 0, 0, 1 },
        { 0.5f,  0.5f, -0.5f,   1, 0, 0, 0, 0 },
        { 0.5f,  0.5f,  0.5f,   1, 0, 0, 1, 0 },
        { 0.5f, -0.5f,  0.5f,   1, 0, 0, 1, 1 },

        // ��� (Z+)
        { 0.5f, -0.5f,  0.5f,   0, 0, 1, 0, 1 },
        { 0.5f,  0.5f,  0.5f,   0, 0, 1, 0, 0 },
        { -0.5f,  0.5f,  0.5f,  0, 0, 1, 1, 0 },
        { -0.5f, -0.5f,  0.5f,  0, 0, 1, 1, 1 },

        // ���� (X-)
        { -0.5f, -0.5f,  0.5f,  -1, 0, 0, 0, 1 },
        { -0.5f,  0.5f,  0.5f,  -1, 0, 0, 0, 0 },
        { -0.5f,  0.5f, -0.5f,  -1, 0, 0, 1, 0 },
        { -0.5f, -0.5f, -0.5f,  -1, 0, 0, 1, 1 },

        // ��� (Y+)
        { -0.5f,  0.5f, -0.5f,  0, 1, 0, 0, 1 },
        { -0.5f,  0.5f,  0.5f,  0, 1, 0, 0, 0 },
        {  0.5f,  0.5f,  0.5f,  0, 1, 0, 1, 0 },
        {  0.5f,  0.5f, -0.5f,  0, 1, 0, 1, 1 },

        // ���� (Y-)
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
    // ���XY���ʂɓ\��t����l�p�i�����ANDC�z��Œ��S0,0�j
    vertices = {
        { -0.5f,  0.5f, 0.0f, 0,0,-1, 0,0 }, // ����
        {  0.5f,  0.5f, 0.0f, 0,0,-1, 1,0 }, // �E��
        {  0.5f, -0.5f, 0.0f, 0,0,-1, 1,1 }, // �E��
        { -0.5f, -0.5f, 0.0f, 0,0,-1, 0,1 }, // ����
    };
    indices = { 0,1,2, 0,2,3 };
}

// MeshLibrary.cpp�i�����ɒǉ��j
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

            // ���������猩��̂Ŗ@���͓������i-pos�j
            v.nx = -v.x; v.ny = -v.y; v.nz = -v.z;

            // equirectangular�z���UV
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
