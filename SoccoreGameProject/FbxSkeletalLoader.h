#pragma once
#include "SkeletalMesh.h"
#include "AnimationClip.h"

class FbxSkeletalLoader {
public:
    static bool LoadMesh(const std::string& filePath, SkeletalMesh& meshOut);
    static AnimationClip* LoadAnimation(const std::string& filePath, const std::vector<Bone>& bones);
};
