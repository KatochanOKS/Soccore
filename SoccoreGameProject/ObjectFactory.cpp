#include "ObjectFactory.h"
#include "GameObject.h"
#include "Transform.h"
#include "MeshRenderer.h"
#include "Animator.h"
#include "EngineManager.h"
#include "MeshLibrary.h"
#include "FbxModelLoader.h"

using namespace DirectX;

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

    auto* mr = obj->AddComponent<MeshRenderer>();
    mr->meshType = 0;
    mr->texIndex = texIdx;
    mr->color = color;

    // ���񂾂��o�b�t�@�쐬�i���ʃo�b�t�@�Ƃ��Ĉ����j
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

    auto* mr = obj->AddComponent<MeshRenderer>();
    mr->meshType = 1;
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
// 3. �X�L�j���O�i�A�j���[�V�����t��FBX�j���f������
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

    auto* mr = obj->AddComponent<MeshRenderer>();
    mr->meshType = 2; // �X�L�����b�V������p
    mr->texIndex = texIndex;
    mr->color = color;

    // �X�L�j���O���Ǎ�
    auto* skinInfo = new FbxModelLoader::SkinningVertexInfo();
    if (!FbxModelLoader::LoadSkinningModel(fbxPath, skinInfo)) {
        delete obj;
        return nullptr;
    }
    mr->skinVertexInfo = skinInfo;

    // �o�b�t�@�����i�^��p�o�[�W�����ɕ����ĂˁI�j
    mr->modelBuffer = new BufferManager();
    mr->modelBuffer->CreateSkinningVertexBuffer(engine->GetDeviceManager()->GetDevice(), skinInfo->vertices);
    mr->modelBuffer->CreateIndexBuffer(engine->GetDeviceManager()->GetDevice(), skinInfo->indices);

    // Animator�Z�b�g�A�b�v
    auto* animator = obj->AddComponent<Animator>();
    if (!skinInfo->animations.empty()) {
        std::unordered_map<std::string, std::vector<Animator::Keyframe>> animMap;
        for (const auto& anim : skinInfo->animations)
            animMap[anim.name] = anim.keyframes;
        animator->SetAnimations(animMap, skinInfo->boneNames);
    }
    mr->animator = animator; // MeshRenderer�ɂ��Z�b�g�i�`��p�j

    engine->m_gameObjects.push_back(obj);
    return obj;
}
