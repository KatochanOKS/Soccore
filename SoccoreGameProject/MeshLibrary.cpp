// MeshLibrary.cpp
#include "MeshLibrary.h"

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
