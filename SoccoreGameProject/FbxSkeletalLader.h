#pragma once
#include "SkeletalMesh.h"

class FbxSkeletalLoader {
public:
    static bool Load(const std::string& filePath, SkeletalMesh& meshOut);
    // AnimationClipもここで生成予定
};
