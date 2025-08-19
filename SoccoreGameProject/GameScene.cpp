#include "GameScene.h"
#include "GameObject.h"
#include "ObjectFactory.h"
#include "Transform.h"
#include "Animator.h"
#include "SkinnedMeshRenderer.h"
#include "StaticMeshRenderer.h"
#include "UIImage.h"
#include "Collider.h"
#include "PlayerComponent.h"
#include "SoccerBallComponent.h"
#include "GoalComponent.h"
#include <DirectXMath.h>
#include <cmath>
using namespace DirectX;

// �^�O����
GameObject* GameScene::FindByTag(const std::string& tag) {
    for (auto* obj : m_sceneObjects) {
        if (obj->tag == tag) return obj;
    }
    return nullptr;
}

// ���O����
GameObject* GameScene::FindByName(const std::string& name) {
    for (auto* obj : m_sceneObjects) {
        if (obj->name == name) return obj;
    }
    return nullptr;
}

// AABB���m�̏d�Ȃ蔻��
bool CheckAABBOverlap(const XMFLOAT3& minA, const XMFLOAT3& maxA,
    const XMFLOAT3& minB, const XMFLOAT3& maxB) {
    return (minA.x <= maxB.x && maxA.x >= minB.x) &&
        (minA.y <= maxB.y && maxA.y >= minB.y) &&
        (minA.z <= maxB.z && maxA.z >= minB.z);
}

GameScene::GameScene(EngineManager* engine) : engine(engine) {}

void GameScene::Start() {
    m_sceneObjects.clear();

    int skyTex = engine->GetTextureManager()->LoadTexture(
        L"assets/SkyDome.png",
        engine->GetDeviceManager()->GetCommandList()
    );
    auto* sky = ObjectFactory::CreateSkyDome(engine, skyTex, 600.0f, "Sky", "SkyDome");
    m_sceneObjects.push_back(sky);

    int logoTex = engine->GetTextureManager()->LoadTexture(L"assets/UI.png", engine->GetDeviceManager()->GetCommandList());
    GameObject* uiObj = new GameObject();
    auto* logo = uiObj->AddComponent<UIImage>();
    logo->texIndex = logoTex;
    logo->position = { 300, 100 };
    logo->size = { 400, 400 };
    logo->color = { 1, 1, 1, 1 };
    uiObj->tag = "UI";
    uiObj->name = "LogoUI";
    m_sceneObjects.push_back(uiObj);

    // �n��
    m_sceneObjects.push_back(ObjectFactory::CreateCube(
        engine, { -19.5f, 1.0f, 0 }, { 1, 2, 3.0f }, -1, Colors::Red, { 0,0,0 }, { -1,-1,-1 }, "Ground", "GroundFloor"
    ));

    // �v���C���[
    int bugEnemyTexIdx = engine->GetTextureManager()->LoadTexture(L"assets/Mutant.fbm/Mutant_diffuse.png", engine->GetDeviceManager()->GetCommandList());
    GameObject* player = ObjectFactory::CreateSkinningBaseModel(
        engine, "assets/Mutant.fbx", { 0, 0, 0 }, { 0.01f, 0.01f, 0.01f }, bugEnemyTexIdx, Colors::White,
        { 0,0.0f,0 }, { 0.5f,2.0f,0.5f }, "Player", "Player1"
    );
    player->AddComponent<PlayerComponent>();

    // �T�b�J�[�{�[��
    int ballTexIdx = engine->GetTextureManager()->LoadTexture(L"assets/soccer_ball/textures/football_ball_BaseColor.png", engine->GetDeviceManager()->GetCommandList());
    GameObject* soccerBall = ObjectFactory::CreateModel(
        engine,
        "assets/soccer_ball/football.fbx",
        { -19.5f, 0.5f, -1.0 },
        { 0.05f, 0.05f, 0.05f },
        ballTexIdx,
        Colors::White,
        { 0,0,0 }, { -1,-1,-1 }, "Ball", "SoccerBall1"
    );
    soccerBall->AddComponent<SoccerBallComponent>();
    m_sceneObjects.push_back(soccerBall);

    // �X�^�W�A��
    int studiumTexIdx = engine->GetTextureManager()->LoadTexture(L"assets/football/textures/Football_BaseColor.png", engine->GetDeviceManager()->GetCommandList());
    GameObject* stadium = ObjectFactory::CreateModel(
        engine,
        "assets/Soccore/SccoreStudium.fbx",
        { 0, 0.0f, 0 },
        { 1.0f, 1.0f, 1.0f },
        studiumTexIdx,
        Colors::White,
        { 0,0,0 }, { -1,-1,-1 }, "Stadium", "MainStadium"
    );
    stadium->GetComponent<Transform>()->rotation.y = XMConvertToRadians(90.0f);
    m_sceneObjects.push_back(stadium);

    // �S�[��1
    int goalTexIdx = engine->GetTextureManager()->LoadTexture(L"assets/football/textures/Goal_BaseColor.png", engine->GetDeviceManager()->GetCommandList());
    GameObject* goal1 = ObjectFactory::CreateModel(
        engine,
        "assets/Soccore/Goal.fbx",
        { -20.0f, 0.0f, 0 },
        { 1.0f, 1.0f, 1.0f },
        goalTexIdx,
        Colors::White,
        { 0,0,0 }, { -1,-1,-1 }, "Goal", "Goal1"
    );
    goal1->GetComponent<Transform>()->rotation.y = XMConvertToRadians(90.0f);
    auto* goal1collider = goal1->GetComponent<Collider>();
    if (goal1collider) {
        goal1collider->center = { 0, 0.0f, 0 }; // ��: �S�[���̐^�񒆂����S
        goal1collider->size = { 2.0f, 2.0f, 3.0f }; // ��: ����3m, ����2.5m, ���s��4m�iBlender�ȂǂŒ��ׂ��l�j
    }
    goal1->AddComponent<GoalComponent>();
    m_sceneObjects.push_back(goal1);

    // �S�[��2
    GameObject* goal2 = ObjectFactory::CreateModel(
        engine,
        "assets/Soccore/Goal.fbx",
        { 20.0f, 0.0f, 0 },
        { 1.0f, 1.0f, 1.0f },
        goalTexIdx,
        Colors::White,
        { 0,0,0 }, { -1,-1,-1 }, "Goal", "Goal2"
    );
    goal2->GetComponent<Transform>()->rotation.y = XMConvertToRadians(-90.0f);
    auto* goal2collider = goal2->GetComponent<Collider>();
    if (goal2collider) {
        goal2collider->center = { 0, 0.0f, 0 }; // ��: �S�[���̐^�񒆂����S
        goal2collider->size = { 2.0f, 2.0f, 3.0f }; // ��: ����3m, ����2.5m, ���s��4m�iBlender�ȂǂŒ��ׂ��l�j
    }
    goal2->AddComponent<GoalComponent>();
    m_sceneObjects.push_back(goal2);

    // �A�j���[�V����
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
        OutputDebugStringA("SkyDome texture load failed!\n");
    }
    else {
        OutputDebugStringA("SkyDome created!\n");
    }
}

void GameScene::Update() {
    // --- �SComponent��Update ---
    for (auto* obj : m_sceneObjects) {
        for (auto* comp : obj->components) comp->Update();
    }



    // --- �v���C���[���{�[���擾 ---
    auto* player = FindByTag("Player");
    auto* soccerBall = FindByName("SoccerBall1");
    if (!player || !soccerBall) return;



    auto* tr = player->GetComponent<Transform>();
    if (!tr) return;
    auto* ballComp = soccerBall->GetComponent<SoccerBallComponent>();
    if (!ballComp) return;
    auto* ballTr = soccerBall->GetComponent<Transform>();
    XMFLOAT3 ballPos = ballTr->position;

    ballTr->position.x = tr->position.x;
    ballTr->position.y = tr->position.y +0.3f; // �����Ȃ̂ŁA�̂̍������班��������i�K�v�Ȃ�l�����j
    ballTr->position.z = tr->position.z;

    // --- �v���C���[�ƃ{�[���̋����ŃL�b�N���� ---
    XMFLOAT3 playerPos = tr->position;
    float dx = ballPos.x - playerPos.x;
    float dz = ballPos.z - playerPos.z;
    float distSq = dx * dx + dz * dz;

    if ((GetAsyncKeyState(VK_SPACE) & 0x8000) && distSq < 25.0f) {
        float angleRad = tr->rotation.y;
        float kickSpeed = 0.01f;
        ballComp->Kick(angleRad, kickSpeed);
        OutputDebugStringA("Kick!\n");
    }

    // --- �S�A�j���[�^�[Update ---
    for (auto* obj : m_sceneObjects) {
        if (auto* animator = obj->GetComponent<Animator>())
            animator->Update(1.0f / 120.0f);
    }

    // --- �S�[������ ---
    auto* goal1 = FindByName("Goal1");
    auto* goal2 = FindByName("Goal2");
    bool scored = false;
    std::string scoredGoalName = "";

    if (goal1 && goal1->GetComponent<GoalComponent>()->CheckGoal(ballPos)) {
        scored = true;
        scoredGoalName = "Goal1";
    }
    if (goal2 && goal2->GetComponent<GoalComponent>()->CheckGoal(ballPos)) {
        scored = true;
        scoredGoalName = "Goal2";
    }

    if (scored) {
        OutputDebugStringA((scoredGoalName + " GOAL!!!------------------------------------------------------------------------------------------------------------------------------------------------------------\n").c_str());
        // --- �{�[����^�񒆂Ƀ��Z�b�g ---
        auto* ballTr = soccerBall->GetComponent<Transform>();
        ballTr->position = { 0.0f, 0.5f, 0.0f };  // �����͒n�ʂ�菭���������Ȃǒ���
        auto* ballComp = soccerBall->GetComponent<SoccerBallComponent>();
        if (ballComp) ballComp->velocity = { 0,0,0 }; // �{�[�����x���[����
        // ���_���Z�≉�o�������Œǉ��\
    }


    // �{�[�����W
    char buf[256];
    sprintf_s(buf, "BallPos: %.2f, %.2f, %.2f\n", ballPos.x, ballPos.y, ballPos.z);
    OutputDebugStringA(buf);

    // �S�[��1�̈ʒu�ƃX�P�[��
    auto* goal1Tr = goal1->GetComponent<Transform>();
    sprintf_s(buf, "Goal1Pos: %.2f, %.2f, %.2f  Scale: %.2f, %.2f, %.2f\n",
        goal1Tr->position.x, goal1Tr->position.y, goal1Tr->position.z,
        goal1Tr->scale.x, goal1Tr->scale.y, goal1Tr->scale.z);
    OutputDebugStringA(buf);

}


void GameScene::Draw() {
    auto* player = FindByTag("Player");
    if (!player) return;
    auto* tr = player->GetComponent<Transform>();
    if (!tr) return;
    XMFLOAT3 playerPos = tr->position;

    Camera* cam = engine->GetCamera();
    XMFLOAT3 offset = { 0.0f, 5.0f, -15.0f };
    XMFLOAT3 cameraPos = {
        playerPos.x + offset.x,
        playerPos.y + offset.y,
        playerPos.z + offset.z
    };
    cam->SetPosition(cameraPos);
    cam->SetTarget(playerPos);

    XMMATRIX view = cam->GetViewMatrix();
    XMMATRIX proj = cam->GetProjectionMatrix(
        engine->GetSwapChainManager()->GetWidth(),
        engine->GetSwapChainManager()->GetHeight()
    );

    // �X�J�C�h�[���Ǐ]
    auto* sky = FindByTag("Sky");
    if (sky) {
        auto* skyTr = sky->GetComponent<Transform>();
        if (skyTr) skyTr->position = cam->GetPosition();
    }

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
    // �ʏ�I�u�W�F�N�g
    for (size_t i = 0; i < m_sceneObjects.size(); ++i) {
        auto* obj = m_sceneObjects[i];
        if (!obj->GetComponent<UIImage>()) {
            engine->GetRenderer()->DrawObject(obj, i, view, proj);
        }
    }
    // UI�͍Ō�
    for (size_t i = 0; i < m_sceneObjects.size(); ++i) {
        auto* obj = m_sceneObjects[i];
        if (obj->GetComponent<UIImage>()) {
            engine->GetRenderer()->DrawObject(obj, i, view, proj);
        }
    }
    engine->GetRenderer()->EndFrame();
}

GameScene::~GameScene() {
    for (auto* obj : m_sceneObjects) {
        delete obj;
    }
    m_sceneObjects.clear();
}
