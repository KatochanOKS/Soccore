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

using namespace DirectX;

//-------------------------------------------------------------------
// ObjectFactoryは「ゲームオブジェクト生成の道具箱」
// 各種GameObjectの生成＋初期化（必要なコンポーネントやパラメータ設定）を一括で担当
// すべてstatic関数なので、インスタンス化不要！
//-------------------------------------------------------------------

//---------------------------------------------
// 1. Cube（静的メッシュ：地面や壁など）の生成
//---------------------------------------------
GameObject* ObjectFactory::CreateCube(
    EngineManager* engine,
    const XMFLOAT3& pos,
    const XMFLOAT3& scale,
    int texIdx,
    const XMFLOAT4& color
) {
    auto* obj = new GameObject();

    // Transform設定
    auto* tr = obj->AddComponent<Transform>();
    tr->position = pos;
    tr->scale = scale;  // 見た目と当たり判定の拡大縮小はscaleで！

    // 描画
    auto* mr = obj->AddComponent<StaticMeshRenderer>();
    mr->texIndex = texIdx;
    mr->color = color;

    // ★ Collider（中心0, サイズ1固定、scaleで見た目拡大OK！）
    auto* col = obj->AddComponent<Collider>();
    col->center = { 0, 0, 0 };
    col->size = { 1, 1, 1 }; // Meshが±0.5範囲なので1でOK

    // バッファ（初回のみ生成）
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
// 2. 静的FBXモデル（建物・オブジェ等）の生成
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
    tr->rotation.y = XMConvertToRadians(180.0f); // FBXモデルの向き補正（必要なら）

    // 描画コンポーネント追加
    auto* mr = obj->AddComponent<StaticMeshRenderer>();
    mr->texIndex = texIndex;
    mr->color = color;

    // FBXファイルから頂点・インデックスデータを読み込み
    mr->vertexInfo = new FbxModelLoader::VertexInfo();
    if (!FbxModelLoader::Load(path, mr->vertexInfo)) {
        delete obj; // ロード失敗時はオブジェクトも破棄
        return nullptr;
    }
    // バッファ生成
    mr->modelBuffer = new BufferManager();
    mr->modelBuffer->CreateVertexBuffer(engine->GetDeviceManager()->GetDevice(), mr->vertexInfo->vertices);
    mr->modelBuffer->CreateIndexBuffer(engine->GetDeviceManager()->GetDevice(), mr->vertexInfo->indices);

    engine->m_gameObjects.push_back(obj);
    return obj;
}

//---------------------------------------------
// 3. スキニング（アニメ付きFBXキャラクター）モデル生成
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

    // スキニングメッシュ描画用コンポーネント
    auto* smr = obj->AddComponent<SkinnedMeshRenderer>();
    smr->texIndex = texIndex;
    smr->color = color;

    // FBXファイルからスキニング情報（ボーン、ウェイト、アニメ等）を読み込み
    auto* skinInfo = new FbxModelLoader::SkinningVertexInfo();
    if (!FbxModelLoader::LoadSkinningModel(fbxPath, skinInfo)) {
        delete obj; // ロード失敗時はオブジェクトも破棄
        return nullptr;
    }
    smr->skinVertexInfo = skinInfo;

    // バッファ生成
    smr->modelBuffer = new BufferManager();
    smr->modelBuffer->CreateSkinningVertexBuffer(engine->GetDeviceManager()->GetDevice(), skinInfo->vertices);
    smr->modelBuffer->CreateIndexBuffer(engine->GetDeviceManager()->GetDevice(), skinInfo->indices);

    // アニメーション制御用コンポーネント
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
// 4. スキンベース（素体のみ、アニメ登録は後で）モデル生成
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

    // プレイヤーなどキャラクター用のコライダーもここで付与
    auto* pcol = obj->AddComponent<Collider>();
    pcol->center = {0, 1.0f, 0};   // 足元から頭まで
pcol->size   = {0.5f, 2.0f, 0.5f};


    // FBXからボーン・バインドポーズだけ取得（アニメは後付けOK）
    auto* skinInfo = new FbxModelLoader::SkinningVertexInfo();
    if (!FbxModelLoader::LoadSkinningModel(fbxPath, skinInfo)) {
        delete obj;
        return nullptr;
    }
    smr->skinVertexInfo = skinInfo;

    smr->modelBuffer = new BufferManager();
    smr->modelBuffer->CreateSkinningVertexBuffer(engine->GetDeviceManager()->GetDevice(), skinInfo->vertices);
    smr->modelBuffer->CreateIndexBuffer(engine->GetDeviceManager()->GetDevice(), skinInfo->indices);

    // Animator追加（アニメは外から追加可能なように最小限のみセット）
    auto* animator = obj->AddComponent<Animator>();
    animator->boneNames = skinInfo->boneNames;
    animator->bindPoses = skinInfo->bindPoses;
    smr->animator = animator;

    engine->m_gameObjects.push_back(obj);
    return obj;
}

//---------------------------------------------
// （サッカーボール等、オリジナルオブジェクトを追加したい場合はここに追加していく！）
//---------------------------------------------
//---------------------------------------------
// 5. Ball（サッカーボールなど、転がせる物体）の生成
//---------------------------------------------
GameObject* ObjectFactory::CreateBall(
    EngineManager* engine,
    const XMFLOAT3& pos,
    const XMFLOAT3& scale,
    int texIndex,
    const XMFLOAT4& color
) {
    auto* obj = new GameObject();

    // Transform（球の半径はscaleで調整！）
    auto* tr = obj->AddComponent<Transform>();
    tr->position = pos;
    tr->scale = scale;

    // 描画
    auto* mr = obj->AddComponent<StaticMeshRenderer>();
    mr->texIndex = texIndex;
    mr->color = color;

    // ★ Collider（必ず1,1,1！scaleで拡大するのでOK）
    auto* col = obj->AddComponent<Collider>();
    col->center = { 0, 0, 0 };
    col->size = { 1, 1, 1 };

    // バッファ（初回のみ生成）
    static bool ballMeshInitialized = false;
    if (!ballMeshInitialized) {
        std::vector<Vertex> vertices;
        std::vector<uint16_t> indices;
        MeshLibrary::GetCubeMesh(vertices, indices); // 仮でCube流用
        engine->GetBufferManager()->CreateVertexBuffer(engine->GetDeviceManager()->GetDevice(), vertices);
        engine->GetBufferManager()->CreateIndexBuffer(engine->GetDeviceManager()->GetDevice(), indices);
        ballMeshInitialized = true;
    }

    engine->m_gameObjects.push_back(obj);
    return obj;
}

// ObjectFactory.cpp（実装を追加）
GameObject* ObjectFactory::CreateSkyDome(EngineManager* engine, int texIndex, float radius) {
    auto* obj = new GameObject();

    // 位置は後でカメラに追従させるので初期値0でOK
    auto* tr = obj->AddComponent<Transform>();
    tr->position = { 0,0,0 };
    tr->scale = { radius, radius, radius }; // 大きめの球で包む

    auto* mr = obj->AddComponent<StaticMeshRenderer>();
    mr->texIndex = texIndex;
    mr->color = Colors::White;
    mr->isSkySphere = true; // ← 特殊扱いフラグ

    // 初回だけ、共有の頂点/インデックスバッファを作る（既存のBufferManagerを流用）
    static bool inited = false;
    if (!inited) {
        std::vector<Vertex> v; std::vector<uint16_t> i;
        MeshLibrary::GetSphereMesh(v, i, 32, 64); // 分割はお好みで
        auto* dev = engine->GetDeviceManager()->GetDevice();
        engine->GetSkyBufferManager()->CreateVertexBuffer(dev, v);
        engine->GetSkyBufferManager()->CreateIndexBuffer(dev, i);
        inited = true;
    }

    engine->m_gameObjects.push_back(obj);
    return obj;
}

