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


    static GameObject* CreateModel(
        EngineManager* engine,
        const std::string& path,
        const XMFLOAT3& pos,
        const XMFLOAT3& scale,
        int texIndex = -1,                      // ← 追加
        const XMFLOAT4& color = Colors::White
    ) {
        auto* obj = new GameObject();
        auto* tr = obj->AddComponent<Transform>();
        tr->position = pos;
        tr->scale = scale;
        tr->rotation.y = XMConvertToRadians(180.0f);

        auto* mr = obj->AddComponent<MeshRenderer>();
        mr->meshType = 1;
        mr->texIndex = texIndex;                // ← ここで指定！
        mr->color = color;

        mr->vertexInfo = new FbxModelLoader::VertexInfo();
        if (!FbxModelLoader::Load(path, mr->vertexInfo)) {
            delete obj;
            return nullptr;
        }
        mr->modelBuffer = new BufferManager();
        mr->modelBuffer->CreateVertexBuffer(engine->GetDeviceManager()->GetDevice(), mr->vertexInfo->vertices);
        mr->modelBuffer->CreateIndexBuffer(engine->GetDeviceManager()->GetDevice(), mr->vertexInfo->indices);

        engine->m_gameObjects.push_back(obj);
        return obj;
    }
};
