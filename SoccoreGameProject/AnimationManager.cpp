#include "pch.h"
#include "AnimationManager.h"

void AnimationManager::RegisterPlayerAnimations(Animator* animator1, Animator* animator2) {
    struct AnimEntry { const char* file; const char* name; };
    AnimEntry anims1[] = {
        { "assets/MMA/BodyBlock.fbx",    "BodyBlock"   },
        { "assets/MMA/MmaKick.fbx",      "Kick"        },
        { "assets/MMA/MutantDying.fbx",  "Dying"       },
        { "assets/MMA/Punching.fbx",     "Punch"       },
        { "assets/MMA/BouncingFightIdle.fbx", "Idle"   },
        { "assets/MMA/Walking.fbx",      "Walk"        },
        { "assets/MMA/TakingPunch.fbx",  "Reaction"    },
    };
    AnimEntry anims2[] = {
        { "assets/MMA2/OutwardBlock.fbx",    "BodyBlock"   },
        { "assets/MMA2/MmaKick.fbx",         "Kick"        },
        { "assets/MMA2/DyingBackwards.fbx",  "Dying"       },
        { "assets/MMA2/Punching2P.fbx",      "Punch"       },
        { "assets/MMA2/BouncingFightIdle2.fbx", "Idle"     },
        { "assets/MMA2/WalkingPlayer2.fbx",  "Walk"        },
        { "assets/MMA2/Kicking.fbx",         "Kick2"       },
        { "assets/MMA2/ZombieReactionHit.fbx", "Reaction"  },
    };

    for (auto& anim1 : anims1) {
        std::vector<Animator::Keyframe> keys;
        double animLen;
        if (FbxModelLoader::LoadAnimationOnly(anim1.file, keys, animLen)) {
            animator1->AddAnimation(anim1.name, keys);
        }
    }
    for (auto& anim2 : anims2) {
        std::vector<Animator::Keyframe> keys;
        double animLen;
        if (FbxModelLoader::LoadAnimationOnly(anim2.file, keys, animLen)) {
            animator2->AddAnimation(anim2.name, keys);
        }
    }
}