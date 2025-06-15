#pragma once
#include "SkeletalMesh.h"

class FbxSkeletalLoader {
public:
    static bool Load(const std::string& filePath, SkeletalMesh& meshOut);
    // AnimationClipÇ‡Ç±Ç±Ç≈ê∂ê¨ó\íË
};
