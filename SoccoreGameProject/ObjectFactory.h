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


    static GameObject* CreateSkinningModel(
        EngineManager* engine,
        const std::string& path,
        const XMFLOAT3& pos,
        const XMFLOAT3& scale,
        int texIndex = -1,
        const XMFLOAT4& color = Colors::White)
    {
        auto* obj = new GameObject();
        auto* tr = obj->AddComponent<Transform>();
        tr->position = pos;
        tr->scale = scale;
        tr->rotation.y = XMConvertToRadians(180.0f);

        auto* mr = obj->AddComponent<MeshRenderer>();
        mr->meshType = 2; // 2=スキニングFBXと定義
        mr->texIndex = texIndex;
        mr->color = color;
        OutputDebugStringA(("★FBXロード対象: " + path + "\n").c_str());

        // 頂点データ取得
        mr->skinInfo = new FbxModelLoader::SkinningVertexInfo();
        if (!FbxModelLoader::LoadSkinningModel(path, mr->skinInfo)) {
            OutputDebugStringA("★[SKIN_LOAD_ERROR] FBX LoadSkinningModel failed!\n");
            delete obj;
            return nullptr;
        }
        char buf[128];
        sprintf_s(buf, "★Skinning FBX load: %s Vtx=%zu Idx=%zu Bone=%zu\n",
            path.c_str(), mr->skinInfo->vertices.size(), mr->skinInfo->indices.size(), mr->skinInfo->boneNames.size());
        OutputDebugStringA(buf);

        // 例えば各頂点のboneIndicesとboneWeightsの先頭3つだけ出す
        for (int i = 0; i < 3 && i < mr->skinInfo->vertices.size(); ++i) {
            const auto& v = mr->skinInfo->vertices[i];
            char b[256];
            sprintf_s(b, "Vtx%d: BoneIdx={%u,%u,%u,%u} Weights={%.3f,%.3f,%.3f,%.3f}\n",
                i, v.boneIndices[0], v.boneIndices[1], v.boneIndices[2], v.boneIndices[3],
                v.boneWeights[0], v.boneWeights[1], v.boneWeights[2], v.boneWeights[3]);
            OutputDebugStringA(b);
        }

        mr->skinBuffer = new BufferManager();
        mr->skinBuffer->CreateSkinningVertexBuffer(engine->GetDeviceManager()->GetDevice(), mr->skinInfo->vertices);
        mr->skinBuffer->CreateIndexBuffer(engine->GetDeviceManager()->GetDevice(), mr->skinInfo->indices);

        // --- ボーンバッファの生成（追加） ---
        mr->boneCount = mr->skinInfo->boneNames.size();
        if (mr->boneCount == 0) {
            // デバッグログ出すと分かりやすい
            OutputDebugStringA("[BoneError] FBXにボーンが1つもありません\n");
            delete obj; // もしくはreturn nullptr;
            return nullptr;
        }
        mr->boneMatrices.resize(mr->boneCount, DirectX::XMMatrixIdentity());
        mr->boneBuffer = new BufferManager();
        mr->boneBuffer->CreateBoneConstantBuffer(engine->GetDeviceManager()->GetDevice(), mr->boneCount);

        engine->m_gameObjects.push_back(obj);
        return obj;
    }
};
