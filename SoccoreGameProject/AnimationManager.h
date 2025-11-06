#pragma once
#pragma once
#include "Animator.h"
#include "FbxModelLoader.h"
#include <string>
#include <vector>

class AnimationManager {
public:
    void RegisterPlayerAnimations(Animator* animator1, Animator* animator2);
};