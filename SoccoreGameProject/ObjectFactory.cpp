#include "ObjectFactory.h"
#include "GameObject.h"
#include "Transform.h"
#include "StaticMeshRenderer.h"   // ← 追加！
#include "SkinnedMeshRenderer.h"  // ← 追加！
#include "Animator.h"
#include "EngineManager.h"
#include "MeshLibrary.h"
#include "FbxModelLoader.h"

using namespace DirectX;

//---------------------------------------------
// 1. Cube（静的メッシュ）の生成
//---------------------------------------------
GameObject* ObjectFactory::CreateCube(
    EngineManager* engine,
    const XMFLOAT3& pos,
    const XMFLOAT3& scale,
    int texIdx,
    const XMFLOAT4& color
) {
    auto* obj = new GameObject();
    auto* tr = obj->AddComponent<Transform>();
    tr->position = pos;
    tr->scale = scale;

    // 旧: auto* mr = obj->AddComponent<MeshRenderer>();
    auto* mr = obj->AddComponent<StaticMeshRenderer>();
    mr->texIndex = texIdx;
    mr->color = color;

    // 初回だけバッファ作成（共通バッファとして扱う）
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

//---------------------------------------------
// 2. 普通のFBX（静的メッシュ）の生成
//---------------------------------------------
GameObject* ObjectFactory::CreateModel(
    EngineManager* engine,
    const std::string& path,
    const XMFLOAT3& pos,
    const XMFLOAT3& scale,
    int texIndex,
    const XMFLOAT4& color
) {
    auto* obj = new GameObject();
    auto* tr = obj->AddComponent<Transform>();
    tr->position = pos;
    tr->scale = scale;
    tr->rotation.y = XMConvertToRadians(180.0f);

    auto* mr = obj->AddComponent<StaticMeshRenderer>();
    mr->texIndex = texIndex;
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

//---------------------------------------------
// 3. スキニング（アニメーション付きFBX）モデル生成
//---------------------------------------------
GameObject* ObjectFactory::CreateSkinningModel(
    EngineManager* engine,
    const std::string& fbxPath,
    const XMFLOAT3& pos,
    const XMFLOAT3& scale,
    int texIndex,
    const XMFLOAT4& color
) {
    auto* obj = new GameObject();
    auto* tr = obj->AddComponent<Transform>();
    tr->position = pos;
    tr->scale = scale;

    auto* smr = obj->AddComponent<SkinnedMeshRenderer>();
    smr->texIndex = texIndex;
    smr->color = color;

    // スキニング情報読込
    auto* skinInfo = new FbxModelLoader::SkinningVertexInfo();
    if (!FbxModelLoader::LoadSkinningModel(fbxPath, skinInfo)) {
        delete obj;
        return nullptr;
    }
    smr->skinVertexInfo = skinInfo;

    // バッファ生成
    smr->modelBuffer = new BufferManager();
    smr->modelBuffer->CreateSkinningVertexBuffer(engine->GetDeviceManager()->GetDevice(), skinInfo->vertices);
    smr->modelBuffer->CreateIndexBuffer(engine->GetDeviceManager()->GetDevice(), skinInfo->indices);

    // Animatorを追加
    auto* animator = obj->AddComponent<Animator>();
    if (!skinInfo->animations.empty()) {
        std::unordered_map<std::string, std::vector<Animator::Keyframe>> animMap;
        for (const auto& anim : skinInfo->animations)
            animMap[anim.name] = anim.keyframes;
        animator->SetAnimations(animMap, skinInfo->boneNames, skinInfo->bindPoses);
    }
    smr->animator = animator;

    engine->m_gameObjects.push_back(obj);
    return obj;
}

//---------------------------------------------
// 4. スキンベース（アニメ登録前の素体モデル）
//---------------------------------------------
GameObject* ObjectFactory::CreateSkinningBaseModel(
    EngineManager* engine,
    const std::string& fbxPath,
    const XMFLOAT3& pos,
    const XMFLOAT3& scale,
    int texIndex,
    const XMFLOAT4& color
) {
    auto* obj = new GameObject();
    auto* tr = obj->AddComponent<Transform>();
    tr->position = pos;
    tr->scale = scale;

    auto* smr = obj->AddComponent<SkinnedMeshRenderer>();
    smr->texIndex = texIndex;
    smr->color = color;

    // モデル・ボーン・バインドポーズのみ読み込む
    auto* skinInfo = new FbxModelLoader::SkinningVertexInfo();
    if (!FbxModelLoader::LoadSkinningModel(fbxPath, skinInfo)) {
        delete obj;
        return nullptr;
    }
    smr->skinVertexInfo = skinInfo;

    smr->modelBuffer = new BufferManager();
    smr->modelBuffer->CreateSkinningVertexBuffer(engine->GetDeviceManager()->GetDevice(), skinInfo->vertices);
    smr->modelBuffer->CreateIndexBuffer(engine->GetDeviceManager()->GetDevice(), skinInfo->indices);

    // Animator追加（アニメ登録は外から可能）
    auto* animator = obj->AddComponent<Animator>();
    animator->boneNames = skinInfo->boneNames;
    animator->bindPoses = skinInfo->bindPoses;
    smr->animator = animator;

    engine->m_gameObjects.push_back(obj);
    return obj;
}
