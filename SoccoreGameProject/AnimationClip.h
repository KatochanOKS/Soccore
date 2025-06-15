#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <DirectXMath.h>

class AnimationClip
{
public:
	float duration = 0.0f;
	float frameRate = 30.0f;

	std::unordered_map<std::string, std::vector<DirectX::XMMATRIX>> boneKeyframes;

};