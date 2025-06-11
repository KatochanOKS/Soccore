#pragma once
#include "GameObject.h"
#include "Transform.h"
#include "MeshRenderer.h"
#include "EngineManager.h"
#include "Colors.h"
#include "MeshLibrary.h"

class ObjectFactory {

public:
    static GameObject* CreateCube(EngineManager* engine, const XMFLOAT3& pos, const XMFLOAT3& scale, int texIdx = -1, const XMFLOAT4& color = Colors::White) {
        auto* obj = new GameObject();
        auto* tr = obj->AddComponent<Transform>();
        tr->position = pos;
        tr->scale = scale;

        auto* mr = obj->AddComponent<MeshRenderer>();
        mr->meshType = 0;
        mr->texIndex = texIdx;
        mr->color = color;

        // ★ 初回だけバッファ作成（共通バッファとして扱う）
        static bool initialized = false;
        if (!initialized) {
            std::vector<Vertex> vertices;
            std::vector<uint16_t> indices;
            MeshLibrary::GetCubeMesh(vertices, indices);
            engine->GetBufferManager()->CreateVertexBuffer(engine->GetDeviceManager()->GetDevice(), vertices);
            engine->GetBufferManager()->CreateIndexBuffer(engine->GetDeviceManager()->GetDevice(), indices);
            initialized = true;
        }

        engine->m_gameObjects.push_back(obj);
        return obj;
    }


    static GameObject* CreateModel(EngineManager* engine, const std::string& path, const XMFLOAT3& pos, const XMFLOAT3& scale, const XMFLOAT4& color = Colors::White) {
        auto* obj = new GameObject();
        auto* tr = obj->AddComponent<Transform>();
        tr->position = pos;
        tr->scale = scale;
        tr->rotation.y = XMConvertToRadians(180.0f); // ← ★これだけでOK！
        auto* mr = obj->AddComponent<MeshRenderer>();
        mr->meshType = 1;      // FBXモデル
        mr->texIndex = -1;
        mr->color = color;

        auto* modelData = engine->GetModelVertexInfo();
        if (modelData->vertices.empty()) {
            FbxModelLoader::Load(path, modelData);
            engine->GetModelBufferManager()->CreateVertexBuffer(engine->GetDeviceManager()->GetDevice(), modelData->vertices);
            engine->GetModelBufferManager()->CreateIndexBuffer(engine->GetDeviceManager()->GetDevice(), modelData->indices);
        }

        engine->m_gameObjects.push_back(obj);
        return obj;
    }


};
