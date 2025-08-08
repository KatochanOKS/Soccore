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
    // GameObject�{�̂𐶐�
    auto* obj = new GameObject();

    // �ʒu�E�X�P�[���i�傫���j�ݒ�
    auto* tr = obj->AddComponent<Transform>();
    tr->position = pos;
    tr->scale = scale;

    // �`��p��StaticMeshRenderer��ǉ��E�e�N�X�`����F���Z�b�g
    auto* mr = obj->AddComponent<StaticMeshRenderer>();
    mr->texIndex = texIdx;
    mr->color = color;

    auto* col = obj->AddComponent<Collider>();
    col->center = { 0, 0, 0 };
    col->size = { 1, 1, 1 }; // ���K���u1,1,1�v�I�I�I


    // ����̂݁A���ʃo�b�t�@����x�����쐬�i����UP�j
    static bool initialized = false;
    if (!initialized) {
        std::vector<Vertex> vertices;
        std::vector<uint16_t> indices;
        MeshLibrary::GetCubeMesh(vertices, indices);
        engine->GetBufferManager()->CreateVertexBuffer(engine->GetDeviceManager()->GetDevice(), vertices);
        engine->GetBufferManager()->CreateIndexBuffer(engine->GetDeviceManager()->GetDevice(), indices);
        initialized = true;
    }

    // ������I�u�W�F�N�g���G���W���̊Ǘ����X�g�ɓo�^
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
    pcol->center = { 0, 1.0f, 0 }; // Y�����������グ�āu�����v�����
    pcol->size = { 0.5f, 2.0f, 0.5f }; // �g���ƕ����L�����ɍ��킹��

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
    // 1. GameObject�{�̂𐶐�
    auto* obj = new GameObject();

    // 2. �ʒu�E�傫�����Z�b�g�iscale�̓{�[�����a�x�[�X�Œ����I�j
    auto* tr = obj->AddComponent<Transform>();
    tr->position = pos;
    tr->scale = scale; // ��F{0.5f, 0.5f, 0.5f} �Ȃ璼�a1m�̋�

    // 3. �`��i�ÓI���b�V����OK�FCube or �����Sphere�ł��j
    auto* mr = obj->AddComponent<StaticMeshRenderer>();
    mr->texIndex = texIndex;
    mr->color = color;

    // �����ł�Cube���b�V���𗬗p�i�{�i�I�ɂ͋��̃��b�V�����j
    static bool ballMeshInitialized = false;
    if (!ballMeshInitialized) {
        std::vector<Vertex> vertices;
        std::vector<uint16_t> indices;
        MeshLibrary::GetCubeMesh(vertices, indices); // ��Cube�̂܂�
        engine->GetBufferManager()->CreateVertexBuffer(engine->GetDeviceManager()->GetDevice(), vertices);
        engine->GetBufferManager()->CreateIndexBuffer(engine->GetDeviceManager()->GetDevice(), indices);
        ballMeshInitialized = true;
    }

    // 4. �����蔻��iAABB��Collider��ǉ��j
    auto* col = obj->AddComponent<Collider>();
    col->center = { 0, 0, 0 };
    col->size = { 1.0f, 1.0f, 1.0f }; // ���a1m�����Bscale�ɍ��킹�Ă����Ă�������

    // 5. �G���W���̊Ǘ����X�g�ɓo�^
    engine->m_gameObjects.push_back(obj);

    return obj;
}
