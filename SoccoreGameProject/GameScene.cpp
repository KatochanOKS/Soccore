#include "GameScene.h"
#include "ObjectFactory.h"
#include "Transform.h"
#include "Animator.h"
#include "SkinnedMeshRenderer.h"
#include "StaticMeshRenderer.h"
#include "UIImage.h"      // �� �����ǉ��I

#include <DirectXMath.h>
using namespace DirectX;

GameScene::GameScene(EngineManager* engine) : engine(engine) {}

void GameScene::Start() {
    m_sceneObjects.clear();

    // ==== 1. UI�摜GameObject���� ====
    int logoTex = engine->GetTextureManager()->LoadTexture(L"assets/UI.png", engine->GetDeviceManager()->GetCommandList());
    GameObject* uiObj = new GameObject();
    auto* logo = uiObj->AddComponent<UIImage>();
    logo->texIndex = logoTex;
    logo->position = { 300, 100 };   // ��ʍ���(px)
    logo->size = { 400, 400 };
    logo->color = { 1, 1, 1, 1 };  // �������OK�I
    m_sceneObjects.push_back(uiObj);

    // ==== 2. �n�ʂƃL���[�u ====
    int groundTex = engine->GetTextureManager()->LoadTexture(L"assets/penguin2.png", engine->GetDeviceManager()->GetCommandList());
    m_sceneObjects.push_back(ObjectFactory::CreateCube(engine, { 0, -5.0f, 0 }, { 50, 0.2f, 50 }, groundTex, Colors::White));

    // ==== 3. �v���C���[ ====
    int bugEnemyTexIdx = engine->GetTextureManager()->LoadTexture(L"assets/Mutant.fbm/Mutant_diffuse.png", engine->GetDeviceManager()->GetCommandList());
    GameObject* player = ObjectFactory::CreateSkinningBaseModel(
        engine, "assets/Mutant.fbx", { 0, 0, 0 }, { 0.05f, 0.05f, 0.05f }, bugEnemyTexIdx, Colors::White);

    auto* animator = player->GetComponent<Animator>();
    std::vector<Animator::Keyframe> idleKeys;
    double idleLen;
    if (FbxModelLoader::LoadAnimationOnly("assets/Idle.fbx", idleKeys, idleLen)) {
        animator->AddAnimation("Idle", idleKeys);
    }
    std::vector<Animator::Keyframe> walkKeys;
    double walkLen;
    if (FbxModelLoader::LoadAnimationOnly("assets/Walking.fbx", walkKeys, walkLen)) {
        animator->AddAnimation("Walk", walkKeys);
    }
    m_sceneObjects.push_back(player);

    // �K�v�ɉ����đ��I�u�W�F�N�g���ǉ�
}

void GameScene::Update() {
    // �v���C���[�����ړ�����
    GameObject* player = nullptr;
    // �v���C���[GameObject��2�Ԗځim_sceneObjects[1]�j
    if (m_sceneObjects.size() > 1)
        player = m_sceneObjects[2];
    if (!player) return;

    auto* tr = player->GetComponent<Transform>();
    auto* animator = player->GetComponent<Animator>();
    float moveSpeed = 0.1f;
    bool isMoving = false;

    if (GetAsyncKeyState('W') & 0x8000) {
        tr->position.z += moveSpeed;
        tr->rotation.y = XMConvertToRadians(0.0f);
        isMoving = true;
    }
    if (GetAsyncKeyState('S') & 0x8000) {
        tr->position.z -= moveSpeed;
        tr->rotation.y = XMConvertToRadians(180.0f);
        isMoving = true;
    }
    if (GetAsyncKeyState('A') & 0x8000) {
        tr->position.x -= moveSpeed;
        tr->rotation.y = XMConvertToRadians(-90.0f);
        isMoving = true;
    }
    if (GetAsyncKeyState('D') & 0x8000) {
        tr->position.x += moveSpeed;
        tr->rotation.y = XMConvertToRadians(90.0f);
        isMoving = true;
    }

    if (animator) {
        if (isMoving && animator->currentAnim != "Walk") {
            animator->SetAnimation("Walk");
        }
        else if (!isMoving && animator->currentAnim != "Idle") {
            animator->SetAnimation("Idle");
        }
    }

    // �A�j���[�V�����i�s
    for (auto* obj : m_sceneObjects) {
        if (auto* animator = obj->GetComponent<Animator>())
            animator->Update(1.0f / 120.0f);
    }
}

void GameScene::Draw() {
    if (m_sceneObjects.empty()) return;

    // �v���C���[�ʒu�ŃJ����
    GameObject* player = nullptr;
    if (m_sceneObjects.size() > 1)
        player = m_sceneObjects[1];
    if (!player) return;
    auto* tr = player->GetComponent<Transform>();
    XMFLOAT3 playerPos = tr ? tr->position : XMFLOAT3{ 0,0,0 };

    XMFLOAT3 cameraOffset = { 0.0f, 5.0f, -20.0f };
    XMFLOAT3 cameraPos = {
        playerPos.x + cameraOffset.x,
        playerPos.y + cameraOffset.y,
        playerPos.z + cameraOffset.z
    };

    XMVECTOR eye = XMLoadFloat3(&cameraPos);
    XMVECTOR target = XMLoadFloat3(&playerPos);
    XMVECTOR up = XMVectorSet(0, 1, 0, 0);
    XMMATRIX view = XMMatrixLookAtLH(eye, target, up);

    XMMATRIX proj = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(60.0f),
        engine->GetSwapChainManager()->GetWidth() / (float)engine->GetSwapChainManager()->GetHeight(),
        0.1f, 100.0f
    );

    constexpr size_t CBV_SIZE = 256;
    void* mapped = nullptr;
    auto* cb = engine->GetBufferManager()->GetConstantBuffer();
    cb->Map(0, nullptr, &mapped);

    for (size_t i = 0; i < m_sceneObjects.size(); ++i) {
        GameObject* obj = m_sceneObjects[i];
        auto* tr = obj->GetComponent<Transform>();
        if (!tr) continue;

        ObjectCB cbData{};
        if (auto* smr = obj->GetComponent<SkinnedMeshRenderer>()) {
            cbData.Color = smr->color;
            cbData.UseTexture = smr->texIndex >= 0 ? 1 : 0;
        }
        else if (auto* mr = obj->GetComponent<StaticMeshRenderer>()) {
            cbData.Color = mr->color;
            cbData.UseTexture = mr->texIndex >= 0 ? 1 : 0;
        }
        cbData.WorldViewProj = XMMatrixTranspose(tr->GetWorldMatrix() * view * proj);
        memcpy((char*)mapped + CBV_SIZE * i, &cbData, sizeof(cbData));
    }
    cb->Unmap(0, nullptr);

    engine->GetRenderer()->BeginFrame();
    for (size_t i = 0; i < m_sceneObjects.size(); ++i)
        engine->GetRenderer()->DrawObject(m_sceneObjects[i], i, view, proj);
    engine->GetRenderer()->EndFrame();
}

// �f�X�g���N�^�Ō�n��
GameScene::~GameScene() {
    for (auto* obj : m_sceneObjects) {
        delete obj;
    }
    m_sceneObjects.clear();
}
