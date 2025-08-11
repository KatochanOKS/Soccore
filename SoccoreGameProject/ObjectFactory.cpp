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
// ObjectFactory�́u�Q�[���I�u�W�F�N�g�����̓���v
// �e��GameObject�̐����{�������i�K�v�ȃR���|�[�l���g��p�����[�^�ݒ�j���ꊇ�ŒS��
// ���ׂ�static�֐��Ȃ̂ŁA�C���X�^���X���s�v�I
//-------------------------------------------------------------------

//---------------------------------------------
// 1. Cube�i�ÓI���b�V���F�n�ʂ�ǂȂǁj�̐���
//---------------------------------------------
GameObject* ObjectFactory::CreateCube(
    EngineManager* engine,
    const XMFLOAT3& pos,
    const XMFLOAT3& scale,
    int texIdx,
    const XMFLOAT4& color
) {
    auto* obj = new GameObject();

    // Transform�ݒ�
    auto* tr = obj->AddComponent<Transform>();
    tr->position = pos;
    tr->scale = scale;  // �����ڂƓ����蔻��̊g��k����scale�ŁI

    // �`��
    auto* mr = obj->AddComponent<StaticMeshRenderer>();
    mr->texIndex = texIdx;
    mr->color = color;

    // �� Collider�i���S0, �T�C�Y1�Œ�Ascale�Ō����ڊg��OK�I�j
    auto* col = obj->AddComponent<Collider>();
    col->center = { 0, 0, 0 };
    col->size = { 1, 1, 1 }; // Mesh���}0.5�͈͂Ȃ̂�1��OK

    // �o�b�t�@�i����̂ݐ����j
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
// 2. �ÓIFBX���f���i�����E�I�u�W�F���j�̐���
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
    tr->rotation.y = XMConvertToRadians(180.0f); // FBX���f���̌����␳�i�K�v�Ȃ�j

    // �`��R���|�[�l���g�ǉ�
    auto* mr = obj->AddComponent<StaticMeshRenderer>();
    mr->texIndex = texIndex;
    mr->color = color;

    // FBX�t�@�C�����璸�_�E�C���f�b�N�X�f�[�^��ǂݍ���
    mr->vertexInfo = new FbxModelLoader::VertexInfo();
    if (!FbxModelLoader::Load(path, mr->vertexInfo)) {
        delete obj; // ���[�h���s���̓I�u�W�F�N�g���j��
        return nullptr;
    }
    // �o�b�t�@����
    mr->modelBuffer = new BufferManager();
    mr->modelBuffer->CreateVertexBuffer(engine->GetDeviceManager()->GetDevice(), mr->vertexInfo->vertices);
    mr->modelBuffer->CreateIndexBuffer(engine->GetDeviceManager()->GetDevice(), mr->vertexInfo->indices);

    engine->m_gameObjects.push_back(obj);
    return obj;
}

//---------------------------------------------
// 3. �X�L�j���O�i�A�j���t��FBX�L�����N�^�[�j���f������
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

    // �X�L�j���O���b�V���`��p�R���|�[�l���g
    auto* smr = obj->AddComponent<SkinnedMeshRenderer>();
    smr->texIndex = texIndex;
    smr->color = color;

    // FBX�t�@�C������X�L�j���O���i�{�[���A�E�F�C�g�A�A�j�����j��ǂݍ���
    auto* skinInfo = new FbxModelLoader::SkinningVertexInfo();
    if (!FbxModelLoader::LoadSkinningModel(fbxPath, skinInfo)) {
        delete obj; // ���[�h���s���̓I�u�W�F�N�g���j��
        return nullptr;
    }
    smr->skinVertexInfo = skinInfo;

    // �o�b�t�@����
    smr->modelBuffer = new BufferManager();
    smr->modelBuffer->CreateSkinningVertexBuffer(engine->GetDeviceManager()->GetDevice(), skinInfo->vertices);
    smr->modelBuffer->CreateIndexBuffer(engine->GetDeviceManager()->GetDevice(), skinInfo->indices);

    // �A�j���[�V��������p�R���|�[�l���g
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
// 4. �X�L���x�[�X�i�f�̂̂݁A�A�j���o�^�͌�Łj���f������
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

    // �v���C���[�ȂǃL�����N�^�[�p�̃R���C�_�[�������ŕt�^
    auto* pcol = obj->AddComponent<Collider>();
    pcol->center = {0, 1.0f, 0};   // �������瓪�܂�
pcol->size   = {0.5f, 2.0f, 0.5f};


    // FBX����{�[���E�o�C���h�|�[�Y�����擾�i�A�j���͌�t��OK�j
    auto* skinInfo = new FbxModelLoader::SkinningVertexInfo();
    if (!FbxModelLoader::LoadSkinningModel(fbxPath, skinInfo)) {
        delete obj;
        return nullptr;
    }
    smr->skinVertexInfo = skinInfo;

    smr->modelBuffer = new BufferManager();
    smr->modelBuffer->CreateSkinningVertexBuffer(engine->GetDeviceManager()->GetDevice(), skinInfo->vertices);
    smr->modelBuffer->CreateIndexBuffer(engine->GetDeviceManager()->GetDevice(), skinInfo->indices);

    // Animator�ǉ��i�A�j���͊O����ǉ��\�Ȃ悤�ɍŏ����̂݃Z�b�g�j
    auto* animator = obj->AddComponent<Animator>();
    animator->boneNames = skinInfo->boneNames;
    animator->bindPoses = skinInfo->bindPoses;
    smr->animator = animator;

    engine->m_gameObjects.push_back(obj);
    return obj;
}

//---------------------------------------------
// �i�T�b�J�[�{�[�����A�I���W�i���I�u�W�F�N�g��ǉ��������ꍇ�͂����ɒǉ����Ă����I�j
//---------------------------------------------
//---------------------------------------------
// 5. Ball�i�T�b�J�[�{�[���ȂǁA�]�����镨�́j�̐���
//---------------------------------------------
GameObject* ObjectFactory::CreateBall(
    EngineManager* engine,
    const XMFLOAT3& pos,
    const XMFLOAT3& scale,
    int texIndex,
    const XMFLOAT4& color
) {
    auto* obj = new GameObject();

    // Transform�i���̔��a��scale�Œ����I�j
    auto* tr = obj->AddComponent<Transform>();
    tr->position = pos;
    tr->scale = scale;

    // �`��
    auto* mr = obj->AddComponent<StaticMeshRenderer>();
    mr->texIndex = texIndex;
    mr->color = color;

    // �� Collider�i�K��1,1,1�Iscale�Ŋg�傷��̂�OK�j
    auto* col = obj->AddComponent<Collider>();
    col->center = { 0, 0, 0 };
    col->size = { 1, 1, 1 };

    // �o�b�t�@�i����̂ݐ����j
    static bool ballMeshInitialized = false;
    if (!ballMeshInitialized) {
        std::vector<Vertex> vertices;
        std::vector<uint16_t> indices;
        MeshLibrary::GetCubeMesh(vertices, indices); // ����Cube���p
        engine->GetBufferManager()->CreateVertexBuffer(engine->GetDeviceManager()->GetDevice(), vertices);
        engine->GetBufferManager()->CreateIndexBuffer(engine->GetDeviceManager()->GetDevice(), indices);
        ballMeshInitialized = true;
    }

    engine->m_gameObjects.push_back(obj);
    return obj;
}

// ObjectFactory.cpp�i������ǉ��j
GameObject* ObjectFactory::CreateSkyDome(EngineManager* engine, int texIndex, float radius) {
    auto* obj = new GameObject();

    // �ʒu�͌�ŃJ�����ɒǏ]������̂ŏ����l0��OK
    auto* tr = obj->AddComponent<Transform>();
    tr->position = { 0,0,0 };
    tr->scale = { radius, radius, radius }; // �傫�߂̋��ŕ��

    auto* mr = obj->AddComponent<StaticMeshRenderer>();
    mr->texIndex = texIndex;
    mr->color = Colors::White;
    mr->isSkySphere = true; // �� ���ꈵ���t���O

    // ���񂾂��A���L�̒��_/�C���f�b�N�X�o�b�t�@�����i������BufferManager�𗬗p�j
    static bool inited = false;
    if (!inited) {
        std::vector<Vertex> v; std::vector<uint16_t> i;
        MeshLibrary::GetSphereMesh(v, i, 32, 64); // �����͂��D�݂�
        auto* dev = engine->GetDeviceManager()->GetDevice();
        engine->GetSkyBufferManager()->CreateVertexBuffer(dev, v);
        engine->GetSkyBufferManager()->CreateIndexBuffer(dev, i);
        inited = true;
    }

    engine->m_gameObjects.push_back(obj);
    return obj;
}

