#include "ObjectFactory.h"
#include "GameObject.h"
#include "Transform.h"
#include "StaticMeshRenderer.h"   // �� �ǉ��I
#include "SkinnedMeshRenderer.h"  // �� �ǉ��I
#include "Animator.h"
#include "EngineManager.h"
#include "MeshLibrary.h"
#include "FbxModelLoader.h"

using namespace DirectX;

//---------------------------------------------
// 1. Cube�i�ÓI���b�V���j�̐���
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

    // ��: auto* mr = obj->AddComponent<MeshRenderer>();
    auto* mr = obj->AddComponent<StaticMeshRenderer>();
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

//---------------------------------------------
// 2. ���ʂ�FBX�i�ÓI���b�V���j�̐���
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

    auto* smr = obj->AddComponent<SkinnedMeshRenderer>();
    smr->texIndex = texIndex;
    smr->color = color;

    // �X�L�j���O���Ǎ�
    auto* skinInfo = new FbxModelLoader::SkinningVertexInfo();
    if (!FbxModelLoader::LoadSkinningModel(fbxPath, skinInfo)) {
        delete obj;
        return nullptr;
    }
    smr->skinVertexInfo = skinInfo;

    // �o�b�t�@����
    smr->modelBuffer = new BufferManager();
    smr->modelBuffer->CreateSkinningVertexBuffer(engine->GetDeviceManager()->GetDevice(), skinInfo->vertices);
    smr->modelBuffer->CreateIndexBuffer(engine->GetDeviceManager()->GetDevice(), skinInfo->indices);

    // Animator��ǉ�
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
// 4. �X�L���x�[�X�i�A�j���o�^�O�̑f�̃��f���j
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

    // ���f���E�{�[���E�o�C���h�|�[�Y�̂ݓǂݍ���
    auto* skinInfo = new FbxModelLoader::SkinningVertexInfo();
    if (!FbxModelLoader::LoadSkinningModel(fbxPath, skinInfo)) {
        delete obj;
        return nullptr;
    }
    smr->skinVertexInfo = skinInfo;

    smr->modelBuffer = new BufferManager();
    smr->modelBuffer->CreateSkinningVertexBuffer(engine->GetDeviceManager()->GetDevice(), skinInfo->vertices);
    smr->modelBuffer->CreateIndexBuffer(engine->GetDeviceManager()->GetDevice(), skinInfo->indices);

    // Animator�ǉ��i�A�j���o�^�͊O����\�j
    auto* animator = obj->AddComponent<Animator>();
    animator->boneNames = skinInfo->boneNames;
    animator->bindPoses = skinInfo->bindPoses;
    smr->animator = animator;

    engine->m_gameObjects.push_back(obj);
    return obj;
}
