#pragma once
#include <string>
#include <DirectXMath.h>
#include "Colors.h"

class EngineManager;
class GameObject;

class ObjectFactory {
public:
    static GameObject* CreateCube(EngineManager* engine, const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& scale, int texIdx = -1, const DirectX::XMFLOAT4& color = Colors::White);
    static GameObject* CreateModel(EngineManager* engine, const std::string& path, const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& scale, int texIndex = -1, const DirectX::XMFLOAT4& color = Colors::White);
    static GameObject* CreateSkinningModel(EngineManager* engine, const std::string& fbxPath, const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& scale, int texIndex = -1, const DirectX::XMFLOAT4& color = Colors::White);
    static GameObject* CreateSkinningBaseModel(EngineManager* engine,const std::string& fbxPath,const DirectX::XMFLOAT3& pos,const DirectX::XMFLOAT3& scale,int texIndex = -1,const DirectX::XMFLOAT4& color = Colors::White
    );

};
