#pragma once
#include <string>
#include <DirectXMath.h>
#include "Colors.h"

class EngineManager;
class GameObject;

class ObjectFactory {
public:
    // Cube生成（地面や壁など）
    static GameObject* CreateCube(
        EngineManager* engine,
        const DirectX::XMFLOAT3& pos,
        const DirectX::XMFLOAT3& scale,
        int texIdx = -1,
        const DirectX::XMFLOAT4& color = Colors::White,
        const DirectX::XMFLOAT3& colliderCenter = { 0,0,0 },
        const DirectX::XMFLOAT3& colliderSize = { -1,-1,-1 },
        const std::string& tag = "Ground",
        const std::string& name = "GroundCube"
    );

    // サッカーボール等の球体
    static GameObject* CreateBall(
        EngineManager* engine,
        const DirectX::XMFLOAT3& pos,
        const DirectX::XMFLOAT3& scale,
        int texIdx = -1,
        const DirectX::XMFLOAT4& color = Colors::White,
        const DirectX::XMFLOAT3& colliderCenter = { 0,0,0 },
        const DirectX::XMFLOAT3& colliderSize = { -1,-1,-1 },
        const std::string& tag = "Ball",
        const std::string& name = "SoccerBall"
    );

    // 静的FBXモデル
    static GameObject* CreateModel(
        EngineManager* engine,
        const std::string& path,
        const DirectX::XMFLOAT3& pos,
        const DirectX::XMFLOAT3& scale,
        int texIdx = -1,
        const DirectX::XMFLOAT4& color = Colors::White,
        const DirectX::XMFLOAT3& colliderCenter = { 0,0,0 },
        const DirectX::XMFLOAT3& colliderSize = { -1,-1,-1 },
        const std::string& tag = "Model",
        const std::string& name = ""
    );

    // スキニングモデル（アニメ付キャラ）
    static GameObject* CreateSkinningBaseModel(
        EngineManager* engine,
        const std::string& fbxPath,
        const DirectX::XMFLOAT3& pos,
        const DirectX::XMFLOAT3& scale,
        int texIdx = -1,
        const DirectX::XMFLOAT4& color = Colors::White,
        const DirectX::XMFLOAT3& colliderCenter = { 0,1.0f,0 },
        const DirectX::XMFLOAT3& colliderSize = { 0.5f,2.0f,0.5f },
        const std::string& tag = "Character",
        const std::string& name = ""
    );

    // スカイドーム
    static GameObject* CreateSkyDome(
        EngineManager* engine,
        int texIdx,
        float radius = 500.0f,
        const std::string& tag = "Sky",
        const std::string& name = "SkyDome"
    );
};
