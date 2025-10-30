#include "ObjectFactory.h"
#include "GameObject.h"
#include "Transform.h"
#include "StaticMeshRenderer.h"
#include "SkinnedMeshRenderer.h"
#include "Animator.h"
#include "EngineManager.h"
#include "MeshLibrary.h"
#include "FbxModelLoader.h"
#include "Collider.h"
#include"ReelComponent.h"

using namespace DirectX;

// ヘルパー：ColliderをTransformに合わせて自動設定
inline void AutoFitCollider(Collider* col, Transform* tr) {
    col->center = { 0,0,0 };
    col->size = tr->scale;
}

//---------------------------
// Cube（地面・壁など）
//---------------------------
GameObject* ObjectFactory::CreateCube(
    EngineManager* engine,
    const XMFLOAT3& pos,
    const XMFLOAT3& scale,
    int texIdx,
    const XMFLOAT4& color,
    const XMFLOAT3& colliderCenter,
    const XMFLOAT3& colliderSize,
    const std::string& tag,
    const std::string& name
) {
    auto* obj = new GameObject();
    obj->tag = tag;
    obj->name = name;

    auto* tr = obj->AddComponent<Transform>();
    tr->position = pos;
    tr->scale = scale;

    auto* mr = obj->AddComponent<StaticMeshRenderer>();
    mr->texIndex = texIdx;
    mr->color = color;

    // FBXじゃない場合は共通バッファをセット
    mr->modelBuffer = engine->GetCubeBufferManager();
    mr->vertexInfo = nullptr; // Cube等は個別のvertexInfo不要（共通バッファを使うだけ）

    auto* col = obj->AddComponent<Collider>();
    if (colliderSize.x < 0 || colliderSize.y < 0 || colliderSize.z < 0) {
        AutoFitCollider(col, tr);
    }
    else {
        col->center = colliderCenter;
        col->size = colliderSize;
    }
    return obj;
}

//---------------------------
// Model（静的FBXモデル）
//---------------------------
GameObject* ObjectFactory::CreateModel(
    EngineManager* engine,
    const std::string& path,
    const XMFLOAT3& pos,
    const XMFLOAT3& scale,
    int texIdx,
    const XMFLOAT4& color,
    const XMFLOAT3& colliderCenter,
    const XMFLOAT3& colliderSize,
    const std::string& tag,
    const std::string& name
) {
    auto* obj = new GameObject();
    obj->tag = tag;
    obj->name = name.empty() ? path : name;

    auto* tr = obj->AddComponent<Transform>();
    tr->position = pos;
    tr->scale = scale;

    auto* mr = obj->AddComponent<StaticMeshRenderer>();
    mr->texIndex = texIdx;
    mr->color = color;

    mr->vertexInfo = new FbxModelLoader::VertexInfo();
    if (!FbxModelLoader::Load(path, mr->vertexInfo)) {
        delete obj;
        return nullptr;
    }
    mr->modelBuffer = new BufferManager();
    mr->modelBuffer->CreateVertexBuffer(engine->GetDeviceManager()->GetDevice(), mr->vertexInfo->vertices);
    mr->modelBuffer->CreateIndexBuffer(engine->GetDeviceManager()->GetDevice(), mr->vertexInfo->indices);

    auto* col = obj->AddComponent<Collider>();
    if (colliderSize.x < 0 || colliderSize.y < 0 || colliderSize.z < 0) {
        AutoFitCollider(col, tr);
    }
    else {
        col->center = colliderCenter;
        col->size = colliderSize;
    }
    return obj;
}

//---------------------------
// SkinningModel（スキニング・アニメ対応）
//---------------------------
GameObject* ObjectFactory::CreateSkinningBaseModel(
    EngineManager* engine,
    const std::string& fbxPath,
    const XMFLOAT3& pos,
    const XMFLOAT3& scale,
    int texIdx,
    const XMFLOAT4& color,
    const XMFLOAT3& colliderCenter,
    const XMFLOAT3& colliderSize,
    const std::string& tag,
    const std::string& name
) {
    auto* obj = new GameObject();
    obj->tag = tag;
    obj->name = name.empty() ? fbxPath : name;

    auto* tr = obj->AddComponent<Transform>();
    tr->position = pos;
    tr->scale = scale;

    auto* smr = obj->AddComponent<SkinnedMeshRenderer>();
    smr->texIndex = texIdx;
    smr->color = color;

    auto* col = obj->AddComponent<Collider>();
    if (colliderSize.x < 0 || colliderSize.y < 0 || colliderSize.z < 0) {
        col->center = colliderCenter;
        col->size = colliderSize;
    }
    else {
        col->center = colliderCenter;
        col->size = colliderSize;
    }

    auto* skinInfo = new FbxModelLoader::SkinningVertexInfo();
    if (!FbxModelLoader::LoadSkinningModel(fbxPath, skinInfo)) {
        delete obj;
        return nullptr;
    }
    smr->skinVertexInfo = skinInfo;
    smr->modelBuffer = new BufferManager();
    smr->modelBuffer->CreateSkinningVertexBuffer(engine->GetDeviceManager()->GetDevice(), skinInfo->vertices);
    smr->modelBuffer->CreateIndexBuffer(engine->GetDeviceManager()->GetDevice(), skinInfo->indices);

    // ==== ここから追加！ ====
// ワールド＋ボーン行列80個ぶんのサイズ（256+80*64=5376バイト）でOK
    smr->boneCB = new BufferManager();
    smr->boneCB->CreateConstantBuffer(
        engine->GetDeviceManager()->GetDevice(),
        256 + sizeof(DirectX::XMMATRIX) * 80
    );

    auto* animator = obj->AddComponent<Animator>();
    animator->boneNames = skinInfo->boneNames;
    animator->bindPoses = skinInfo->bindPoses;
    smr->animator = animator;
    return obj;
}

//---------------------------
// SkyDome
//---------------------------
GameObject* ObjectFactory::CreateSkyDome(
    EngineManager* engine,
    int texIdx,
    float radius,
    const std::string& tag,
    const std::string& name
) {
    auto* obj = new GameObject();
    obj->tag = tag;
    obj->name = name;

    auto* tr = obj->AddComponent<Transform>();
    tr->position = { 0,0,0 };
    tr->scale = { radius, radius, radius };

    auto* mr = obj->AddComponent<StaticMeshRenderer>();
    mr->texIndex = texIdx;
    mr->color = Colors::White;
    mr->isSkySphere = true;

    static bool inited = false;
    if (!inited) {
        std::vector<Vertex> v; std::vector<uint16_t> i;
        MeshLibrary::GetSphereMesh(v, i, 32, 64);
        auto* dev = engine->GetDeviceManager()->GetDevice();
        engine->GetSkyBufferManager()->CreateVertexBuffer(dev, v);
        engine->GetSkyBufferManager()->CreateIndexBuffer(dev, i);
        inited = true;
    }

    return obj;
}

GameObject* ObjectFactory::CreateCylinderReel(
    EngineManager* engine,
    const XMFLOAT3& pos,
    const XMFLOAT3& scale,
    int texIdx,
    const XMFLOAT4& color,
    const std::string& tag,
    const std::string& name
) {
    auto* obj = new GameObject();
    obj->tag = tag;
    obj->name = name;

    auto* tr = obj->AddComponent<Transform>();
    tr->position = pos;
    tr->scale = scale;
    tr->rotation.z = -DirectX::XM_PIDIV2; // ← マイナスで逆方向
    auto* mr = obj->AddComponent<StaticMeshRenderer>(); // StaticMeshRenderer流用
    mr->texIndex = texIdx;
    mr->color = color;

    // ...前略
    auto* reel = obj->AddComponent<ReelComponent>();
    reel->m_IsSpinning = true;


    // 頂点・インデックスを生成
    mr->vertexInfo = new FbxModelLoader::VertexInfo();
    MeshLibrary::GetCylinderMesh(mr->vertexInfo->vertices, mr->vertexInfo->indices, 32);
    mr->modelBuffer = new BufferManager();
    mr->modelBuffer->CreateVertexBuffer(engine->GetDeviceManager()->GetDevice(), mr->vertexInfo->vertices);
    mr->modelBuffer->CreateIndexBuffer(engine->GetDeviceManager()->GetDevice(), mr->vertexInfo->indices);

    return obj;
}
