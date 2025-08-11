#include "GameScene.h"
#include"GameObject.h"
#include "ObjectFactory.h"
#include "Transform.h"
#include "Animator.h"
#include "SkinnedMeshRenderer.h"
#include "StaticMeshRenderer.h"
#include "UIImage.h"
#include"Collider.h"
#include <DirectXMath.h>
using namespace DirectX;

// GameScene�̃R���X�g���N�^�BEngineManager�i���\�[�X�Ǘ��̒����j���Q�ƕێ�
GameScene::GameScene(EngineManager* engine) : engine(engine) {}

// �Q�[���I�u�W�F�N�g�̏������Ɣz�u
void GameScene::Start() {
    m_sceneObjects.clear(); // �V�[���J�n���ɑS�I�u�W�F�N�g�����i���Z�b�g�j

    int skyTex = engine->GetTextureManager()->LoadTexture(
        L"assets/SkyDome.png",
        engine->GetDeviceManager()->GetCommandList()
    );
    auto* sky = ObjectFactory::CreateSkyDome(engine, skyTex, 600.0f);
    m_sceneObjects.push_back(sky);

    // ==== 1. UI�摜��GameObject���� ====
    int logoTex = engine->GetTextureManager()->LoadTexture(L"assets/UI.png", engine->GetDeviceManager()->GetCommandList());
    GameObject* uiObj = new GameObject();
    auto* logo = uiObj->AddComponent<UIImage>();
    logo->texIndex = logoTex;
    logo->position = { 300, 100 };   // ��ʍ���(px)
    logo->size = { 400, 400 };
    logo->color = { 1, 1, 1, 1 };   // ���S�s����
    m_sceneObjects.push_back(uiObj);

    // ==== 2. �n�ʃI�u�W�F�N�g�i�Ԃ��L���[�u�j ====
    int groundTex = engine->GetTextureManager()->LoadTexture(L"assets/penguin2.png", engine->GetDeviceManager()->GetCommandList());
    // �n�ʃL���[�u��Y���W��-5.0�A�T�C�Y��(100, 0.2, 100)�Œ��L��
    m_sceneObjects.push_back(ObjectFactory::CreateCube(engine, { 0, -5.0f, 0 }, { 100, 0.2f, 100 }, -1, Colors::Red));

    // ==== 3. �T���v���{�[���i���L���[�u�j ====
    auto* ball = ObjectFactory::CreateBall(engine, { 0, 2, 10 }, { 5.0f, 5.0f, 5.0f }, -1, Colors::Blue);
    m_sceneObjects.push_back(ball);

    // ==== 4. �v���C���[�L�����N�^�[�iFBX�X�L�j���O���f���j ====
    int bugEnemyTexIdx = engine->GetTextureManager()->LoadTexture(L"assets/Mutant.fbm/Mutant_diffuse.png", engine->GetDeviceManager()->GetCommandList());
    // center.y = 1.5, size.y = 3.0�̑O��ō쐬
    // Y=5����X�^�[�g�i���n�ʂ���10.0��j�Ŏ��R�ɗ�������̂��m�F�ł���
    GameObject* player = ObjectFactory::CreateSkinningBaseModel(
        engine, "assets/Mutant.fbx", { 0, 5, 0 }, { 0.05f, 0.05f, 0.05f }, bugEnemyTexIdx, Colors::White);

    // �A�j���[�V�����ǂݍ��݁iIdle, Walk�B�g���j
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


    if (skyTex < 0) {
        OutputDebugStringA("�X�J�C�h�[���̃e�N�X�`���ǂݍ��݂Ɏ��s���܂����I\n");
    }
    else {
        OutputDebugStringA("�X�J�C�h�[���̐��������I\n");
    }

    // --- �K�v������΂����ɒǉ��I�u�W�F�N�g��o�^ ---
}

void GameScene::Update() {
    // ===== 1. �v���C���[�̓��͂ɂ��ړ��ƃA�j������ =====
    if (m_sceneObjects.size() <= 3) return;
    GameObject* player = m_sceneObjects[4];
    auto* tr = player->GetComponent<Transform>();
    auto* animator = player->GetComponent<Animator>();

    float moveSpeed = 0.1f;
    bool isMoving = false;
    // WASD�L�[���͂�XZ���ʂ��ړ�
    if (GetAsyncKeyState('W') & 0x8000) { tr->position.z += moveSpeed; tr->rotation.y = XMConvertToRadians(0.0f); isMoving = true; }
    if (GetAsyncKeyState('S') & 0x8000) { tr->position.z -= moveSpeed; tr->rotation.y = XMConvertToRadians(180.0f); isMoving = true; }
    if (GetAsyncKeyState('A') & 0x8000) { tr->position.x -= moveSpeed; tr->rotation.y = XMConvertToRadians(-90.0f); isMoving = true; }
    if (GetAsyncKeyState('D') & 0x8000) { tr->position.x += moveSpeed; tr->rotation.y = XMConvertToRadians(90.0f); isMoving = true; }

    // �ړ�����Walk�A�Î~����Idle�A�j��
    if (animator) {
        if (isMoving && animator->currentAnim != "Walk") {
            animator->SetAnimation("Walk");
        }
        else if (!isMoving && animator->currentAnim != "Idle") {
            animator->SetAnimation("Idle");
        }
    }

    // ===== 2. �v���C���[�̏d�́E�ڒn�i�n�ʕ␳�j =====
    auto* playerCol = player->GetComponent<Collider>();
    GameObject* ground = m_sceneObjects[2];
    auto* groundCol = ground->GetComponent<Collider>();

    if (playerCol && groundCol) {
        // --- �e�R���C�_�[�̐��E���WAABB���擾 ---
        DirectX::XMFLOAT3 pMin, pMax, gMin, gMax;
        playerCol->GetAABBWorld(pMin, pMax);
        groundCol->GetAABBWorld(gMin, gMax);

        // --- �n�ʂ�XZ�͈͓����H�iXZ����̓����蔻��B�n�ʂ���͂ݏo���Ɨ�������j ---
        bool onGroundXZ =
            tr->position.x > gMin.x && tr->position.x < gMax.x &&
            tr->position.z > gMin.z && tr->position.z < gMax.z;

        // --- ����Y���W�ƒn��Y���W�igMax.y�͒n�ʂ̏�ʁj ---
        float halfHeight = playerCol->size.y * 0.5f * tr->scale.y;
        float footY = tr->position.y + (playerCol->center.y * tr->scale.y) - halfHeight;
        float groundY = gMax.y;

        // --- �y�d�v�z�O�t���[���̑���Y��XZ�����ێ� ---
        static float prevFootY = 0.0f;
        static bool wasOnGroundXZ = false;
        static bool firstUpdate = true;
        static const float epsilon = 0.01f; // �ڒn����̋��e�덷

        // --- �Q�[���J�n���ゾ��XZ�͈́{Y�ŕK���z�� ---
        if (firstUpdate) {
            if (onGroundXZ && footY <= groundY + epsilon) {
                // �u�����z�u�Ŋ��ɒn�ʂ��܂����ł�����v�K���z��
                tr->position.y += (groundY - footY);
            }
            firstUpdate = false;
        }
        else {
            bool willLand = false;
            if (onGroundXZ && wasOnGroundXZ) {
                // --- �y�ŏd�v�z�O�t���[���ƍ��t���[���̊ԂŒn��Y���g�ʉ߁h������z�� ---
                // �i�と���E������A�ǂ�����s�^���~�܂遁���蔲����Ζh�~�j
                if ((prevFootY - groundY) * (footY - groundY) <= 0.0f) {
                    willLand = true;
                }
            }
            if (willLand) {
                // ���n�������s�^�b�Ǝ~�߂�
                tr->position.y += (groundY - footY);
            }
            else {
                // ����ȊO�͏�ɏd�͂ŗ���
                tr->position.y -= 0.2f;
            }
        }
        // --- ��ԕۑ��i���t���[������p�j ---
        prevFootY = footY;
        wasOnGroundXZ = onGroundXZ;

        // --- �f�o�b�O�o�� ---
        char buf[256];
        sprintf_s(buf, "PosY=%.2f, FootY=%.2f, GroundY=%.2f, onXZ=%d\n",
            tr->position.y, footY, groundY, (int)onGroundXZ);
        OutputDebugStringA(buf);
    }

    // ===== 3. �A�j���[�V�����i�s�i���t���[��120����1�b���i�߂�j =====
    for (auto* obj : m_sceneObjects) {
        if (auto* animator = obj->GetComponent<Animator>())
            animator->Update(1.0f / 120.0f);
    }
}

// ===== �`�揈���i�v���C���[�𒆐S�ɃJ�����Ǐ]�B���ׂĂ�GameObject��`��j =====
void GameScene::Draw() {
    if (m_sceneObjects.empty()) return;

    // --- �v���C���[���W�ɃJ������Ǐ] ---
    GameObject* player = nullptr;
    if (m_sceneObjects.size() > 1)
        player = m_sceneObjects[4];
    if (!player) return;
    auto* tr = player->GetComponent<Transform>();
    XMFLOAT3 playerPos = tr ? tr->position : XMFLOAT3{ 0,0,0 };

    XMFLOAT3 cameraOffset = { 0.0f, 5.0f, -20.0f }; // �J�����͎΂ߏ�������
    XMFLOAT3 cameraPos = {
        playerPos.x + cameraOffset.x,
        playerPos.y + cameraOffset.y,
        playerPos.z + cameraOffset.z
    };
    char buf[128];
    sprintf_s(buf, "CameraPos: %.2f, %.2f, %.2f\n", cameraPos.x, cameraPos.y, cameraPos.z);
    OutputDebugStringA(buf);

    XMVECTOR eye = XMLoadFloat3(&cameraPos);
    XMVECTOR target = XMLoadFloat3(&playerPos);
    XMVECTOR up = XMVectorSet(0, 8, 0, 0);
    XMMATRIX view = XMMatrixLookAtLH(eye, target, up);

    XMMATRIX proj = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(70.0f),
        engine->GetSwapChainManager()->GetWidth() / (float)engine->GetSwapChainManager()->GetHeight(),
        0.1f, 1000.0f // �� far��1000.0f�ȂǑ傫������
    );

    // cameraPos �����܂������゠����
    if (!m_sceneObjects.empty()) {
        auto* skyTr = m_sceneObjects[0]->GetComponent<Transform>(); // �擪��SkyDome�ɂ��Ă���z��
        if (skyTr) skyTr->position = cameraPos;
    }


    // --- �eGameObject�̒萔�o�b�t�@�iCBV�j���X�V ---
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

    // --- �`��t���[���J�n ---
    engine->GetRenderer()->BeginFrame();
    for (size_t i = 0; i < m_sceneObjects.size(); ++i)
        engine->GetRenderer()->DrawObject(m_sceneObjects[i], i, view, proj);
    engine->GetRenderer()->EndFrame();
}

// ===== �f�X�g���N�^�Ń�������� =====
GameScene::~GameScene() {
    for (auto* obj : m_sceneObjects) {
        delete obj;
    }
    m_sceneObjects.clear();
}
