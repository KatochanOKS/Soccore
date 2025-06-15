#pragma once
#include <vector>
#include <DirectXMath.h>
#include "AnimationClip.h"


class Animator {
public:

	AnimationClip* currentClip = nullptr;
	float currentTime = 0.0f;

void SetAnimationClip(AnimationClip* clip) {
	currentClip = clip;
   currentTime = 0.0f; // Reset time when changing clips
	}

void Update(float deltaTime) {
	if (!currentClip) return;
	currentTime += deltaTime;
	if (currentTime > currentClip->duration)
		currentTime = 0.0f; // Loop the animation
}

std::vector<DirectX::XMMATRIX>  GetCurrentPoseMatrices();



};