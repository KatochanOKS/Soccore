#include "GameScene.h"
#include "GameObject.h"
#include "ObjectFactory.h"
#include "Transform.h"
#include "Animator.h"
#include "SkinnedMeshRenderer.h"
#include "StaticMeshRenderer.h"
#include "UIImage.h"
#include "Collider.h"
#include "Player1Component.h"
#include "Player2Component.h"
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

    // �X�J�C�h�[��
    int skyTex = engine->GetTextureManager()->LoadTexture(
        L"assets/SkyDome.png",
        engine->GetDeviceManager()->GetCommandList()
    );
    auto* sky = ObjectFactory::CreateSkyDome(engine, skyTex, 600.0f, "Sky", "SkyDome");
    m_sceneObjects.push_back(sky);

    // UI�i���S�T���v���j
    int logoTex = engine->GetTextureManager()->LoadTexture(L"assets/Green3.png", engine->GetDeviceManager()->GetCommandList());
    GameObject* uiObj1 = new GameObject();
    auto* logo = uiObj1->AddComponent<UIImage>();
    logo->texIndex = logoTex;
    logo->size = { 500, 50 }; // 1280��1278
    logo->position = { 0, 100 };

    logo->color = { 1, 1, 1, 1 };
    uiObj1->tag = "UI";
    uiObj1->name = "HP1";
    m_sceneObjects.push_back(uiObj1);

    GameObject* uiObj2 = new GameObject();
    auto* logo2 = uiObj2->AddComponent<UIImage>();
    logo2->texIndex = logoTex;
     logo2->size = { 500, 50 }; // 1280��1278
    logo2->position = { 780, 50 };
    logo2->color = { 1, 1, 1, 1 };
    uiObj2->tag = "UI";
    uiObj2->name = "HP2";
    m_sceneObjects.push_back(uiObj2);
    // �i���X�e�[�W�̏��i�����Ŕ���Cube�j
    m_sceneObjects.push_back(ObjectFactory::CreateCube(
        engine, { 0.0f, -0.5f, 0.0f }, { 10.0f, 1.0f, 3.0f }, -1, Colors::Gray, { 0,0,0 }, { -1,-1,-1 }, "Ground", "GroundFloor"
    ));

    //m_sceneObjects.push_back(ObjectFactory::CreateCube(
    //    engine, { -2.5f, 0.85f, 0.0f }, { 1.0f, 1.7f, 1.0f }, -1, Colors::Red, { 0,0.85f,0 }, { 1.0,1.7f,1 }, "Ground", "Block"
    //));

    // �v���C���[1�i�����j--------------------------------------------------------
    int p1TexIdx = engine->GetTextureManager()->LoadTexture(L"assets/Mutant.fbm/Mutant_diffuse.png", engine->GetDeviceManager()->GetCommandList());
    GameObject* player1 = ObjectFactory::CreateSkinningBaseModel(
        engine, "assets/Mutant.fbx",
        { -2.5f, 0.0f, 0.0f },    // ���ɔz�u
        { 0.01f, 0.01f, 0.01f },
        p1TexIdx, Colors::White,
        { 0,0.85f,0 }, { 1.5f,1.7f,1.5f }, "Player", "Player1"
    );
    player1->AddComponent<Player1Component>();
    player1->GetComponent<Transform>()->rotation.y = XMConvertToRadians(90.0f); // �E����

    // �v���C���[2�i�E���j--------------------------------------------------------
	int p2TexIdx = engine->GetTextureManager()->LoadTexture(L"assets/MMA2/SkeletonzombieTAvelange.fbm/skeletonZombie_diffuse.png", engine->GetDeviceManager()->GetCommandList());
    GameObject* player2 = ObjectFactory::CreateSkinningBaseModel(
        engine, "assets/MMA2/SkeletonzombieTAvelange.fbx",
        { 2.5f, 0.0f, 0.0f },     // �E�ɔz�u
        { 0.01f, 0.01f, 0.01f },
        p2TexIdx, Colors::White,
        { 0,0.9f,0 }, { 1.5f,1.8f,1.5f }, "Enemy", "Player2"
    );
    player2->AddComponent<Player2Component>(); // ���ɗ���PlayerComponent�i2P�����AI�ɐؑ�OK�j
    player2->GetComponent<Transform>()->rotation.y = XMConvertToRadians(-90.0f); // ������

    m_sceneObjects.push_back(player1);
    m_sceneObjects.push_back(player2);

    auto* smr1 = player1->GetComponent<SkinnedMeshRenderer>();
    auto* smr2 = player2->GetComponent<SkinnedMeshRenderer>();
    char buf[256];

    OutputDebugStringA("--- PLAYER1 boneNames ---\n");
    for (size_t i = 0; i < smr1->skinVertexInfo->boneNames.size(); ++i) {
        sprintf_s(buf, "[%02zu] %s\n", i, smr1->skinVertexInfo->boneNames[i].c_str());
        OutputDebugStringA(buf);
    }

    OutputDebugStringA("--- PLAYER2 boneNames ---\n");
    for (size_t i = 0; i < smr2->skinVertexInfo->boneNames.size(); ++i) {
        sprintf_s(buf, "[%02zu] %s\n", i, smr2->skinVertexInfo->boneNames[i].c_str());
        OutputDebugStringA(buf);
    }

    // ----- �A�j���[�V�����o�^ -----
    // assets/MMA/*.fbx
    struct AnimEntry { const char* file; const char* name; };
    AnimEntry anims1[] = {
         { "assets/MMA/BodyBlock.fbx",    "BodyBlock"   },
         { "assets/MMA/MmaKick.fbx",      "Kick"        },
         { "assets/MMA/MutantDying.fbx",  "Dying"       },
         { "assets/MMA/Punching.fbx",   "Punch"       },
         { "assets/MMA/BouncingFightIdle.fbx",             "Idle"        },
         { "assets/MMA/Walking.fbx",          "Walk"        },
    };

    AnimEntry anims2[] = {
        { "assets/MMA2/OutwardBlock.fbx",    "BodyBlock"   },
        { "assets/MMA2/MmaKick.fbx",         "Kick"        },
        { "assets/MMA2/DyingBackwards.fbx",  "Dying"       },
        { "assets/MMA2/CrossPunch.fbx",        "Punch"       },
        { "assets/MMA2/BouncingFightIdle2.fbx",     "Idle"        },
        { "assets/MMA2/WalkingPlayer2.fbx",  "Walk"        },
        { "assets/MMA2/Kicking.fbx",  "Kick2"        },
    };

    // �v���C���[1�̃A�j���o�^
    auto* animator1 = player1->GetComponent<Animator>();
    for (auto& anim1 : anims1) {
        std::vector<Animator::Keyframe> keys;
        double animLen;
        if (FbxModelLoader::LoadAnimationOnly(anim1.file, keys, animLen)) {
            animator1->AddAnimation(anim1.name, keys);
        }
    }

    // �v���C���[2�̃A�j���o�^
    auto* animator2 = player2->GetComponent<Animator>();
    for (auto& anim2 : anims2) {
        std::vector<Animator::Keyframe> keys;
        double animLen;
        if (FbxModelLoader::LoadAnimationOnly(anim2.file, keys, animLen)) {
            animator2->AddAnimation(anim2.name, keys);
        }
    }

    // �V�[���Q��
    player1->scene = this;
    player2->scene = this;

    if (skyTex < 0) { OutputDebugStringA("SkyDome texture load failed!\n"); }
    else { OutputDebugStringA("SkyDome created!\n"); }
}

void GameScene::Update() {
    // --- �SComponent��Update ---
    for (auto* obj : m_sceneObjects) {
        for (auto* comp : obj->components) comp->Update();
    }

    // --- �S�A�j���[�^�[Update ---
    for (auto* obj : m_sceneObjects) {
        if (auto* animator = obj->GetComponent<Animator>())
            animator->Update(1.0f / 120.0f);
    }



    // --- �R���C�_�[AABB���m�ōU������ ---
    GameObject* player1 = FindByName("Player1");
    GameObject* player2 = FindByName("Player2");
    if (player1 && player2) {
        auto* tr1 = player1->GetComponent<Transform>();
        auto* tr2 = player2->GetComponent<Transform>();
        auto* col1 = player1->GetComponent<Collider>();
        auto* col2 = player2->GetComponent<Collider>();
        if (!tr1 || !tr2 || !col1 || !col2) {
            OutputDebugStringA("[DEBUG] Transform/Collider�擾���s\n");
            return;
        }

        XMFLOAT3 minA, maxA, minB, maxB;
        col1->GetAABBWorld(tr1, minA, maxA);
        col2->GetAABBWorld(tr2, minB, maxB);

        // --- �v���C���[1�̍U�� ---
        auto* anim1 = player1->GetComponent<Animator>();
        char buf[256];
        sprintf_s(buf, "[DEBUG] Player1 currentAnim = %s, playing = %d\n", anim1->currentAnim.c_str(), anim1->isPlaying ? 1 : 0);
        OutputDebugStringA(buf);

        if (anim1 && (anim1->currentAnim == "Punch" || anim1->currentAnim == "Kick")) {
            OutputDebugStringA("[DEBUG] Player1�U���A�j����! �R���C�_�[��������{\n");
            if (CheckAABBOverlap(minA, maxA, minB, maxB)) {
                OutputDebugStringA("Player1�̍U����Player2��AABB��HIT!\n");
            }
            else {
                OutputDebugStringA("[DEBUG] Player1�U���A�j��������AABB�q�b�g����\n");
            }
        }

        auto* comp1 = player1->GetComponent<Player1Component>();
        auto* comp2 = player2->GetComponent<Player2Component>();
        // �U���������
        if (CheckAABBOverlap(minA, maxA, minB, maxB)) {
            if (!comp2->isGuarding) { // �K�[�h���ȊO�ł���HP����
                comp2->hp -= 0.0005f;
                if (comp2->hp < 0.0f) comp2->hp = 0.0f;
            }
        }


        sprintf_s(buf, "[DEBUG] minA=(%.2f,%.2f,%.2f) maxA=(%.2f,%.2f,%.2f)\n", minA.x, minA.y, minA.z, maxA.x, maxA.y, maxA.z);
        OutputDebugStringA(buf);
        sprintf_s(buf, "[DEBUG] minB=(%.2f,%.2f,%.2f) maxB=(%.2f,%.2f,%.2f)\n", minB.x, minB.y, minB.z, maxB.x, maxB.y, maxB.z);
        OutputDebugStringA(buf);


        // --- �v���C���[2�����l ---
        auto* anim2 = player2->GetComponent<Animator>();
        sprintf_s(buf, "[DEBUG] Player2 currentAnim = %s, playing = %d\n", anim2->currentAnim.c_str(), anim2->isPlaying ? 1 : 0);
        OutputDebugStringA(buf);

        if (anim2 && (anim2->currentAnim == "Punch" || anim2->currentAnim == "Kick")) {
            OutputDebugStringA("[DEBUG] Player2�U���A�j����! �R���C�_�[��������{\n");
            if (CheckAABBOverlap(minB, maxB, minA, maxA)) {
                OutputDebugStringA("Player2�̍U����Player1��AABB��HIT!\n");
            }
            else {
                OutputDebugStringA("[DEBUG] Player2�U���A�j��������AABB�q�b�g����\n");
            }
        }
    }
}



void GameScene::Draw() {
    Camera* cam = engine->GetCamera();

    // �T�C�h�r���[�F���ォ�牡�����iX���j������
    // ��F{X=0, Y=3, Z=10} ���� {X=0, Y=1, Z=0} �𒍎�
    cam->SetPosition({ 0.0f, 3.0f, -5.0f });   // Z=10�͉�ʉ��i��O�ł�OK�j
    cam->SetTarget({ 0.0f, 1.0f, 0.0f });      // �X�e�[�W����������

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
