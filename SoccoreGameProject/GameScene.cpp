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
#include "EngineManager.h"
#include "GameOverScene.h"
#include "PlayerManager.h"
#include <DirectXMath.h>
#include <cmath>
using namespace DirectX;

static bool sceneChanged = false;

/// <summary>
/// �w�肵���^�O������GameObject����������
/// </summary>
GameObject* GameScene::FindByTag(const std::string& tag) {
    for (auto* obj : m_SceneObjects) {
        if (obj->tag == tag) return obj;
    }
    return nullptr;
}

/// <summary>
/// �w�肵�����O������GameObject����������
/// </summary>
GameObject* GameScene::FindByName(const std::string& name) {
    for (auto* obj : m_SceneObjects) {
        if (obj->name == name) return obj;
    }
    return nullptr;
}

/// <summary>
/// 2��AABB�̏d�Ȃ蔻����s���i�U������p�j
/// </summary>
bool CheckAABBOverlap(const XMFLOAT3& minA, const XMFLOAT3& maxA,
    const XMFLOAT3& minB, const XMFLOAT3& maxB) {
    return (minA.x <= maxB.x && maxA.x >= minB.x) &&
        (minA.y <= maxB.y && maxA.y >= minB.y) &&
        (minA.z <= maxB.z && maxA.z >= minB.z);
}

/// <summary>
/// �Q�[���V�[���̃R���X�g���N�^
/// </summary>
GameScene::GameScene(EngineManager* engine) : engine(engine) {}

/// <summary>
/// �V�[���J�n���̏���������
/// </summary>
void GameScene::Start() {
    m_SceneObjects.clear();
    sceneChanged = false;  // �V�[���؂�ւ��t���O������
    m_UIManager.InitUI(engine, m_SceneObjects); // UI������
    m_PlayerManager.InitPlayers(engine, m_SceneObjects); // �v���C���[�Ǘ�������
    InitStage();     // �X�e�[�W����
    RegisterAnimations(); // �A�j���[�V�����o�^

    // �v���C���[�̃V�[���Q�ƃZ�b�g
    FindByName("Player1")->scene = this;
    FindByName("Player2")->scene = this;
}

/// <summary>
/// ���t���[���̍X�V����
/// </summary>
void GameScene::Update() {
    // �SComponent��Update
    for (auto* obj : m_SceneObjects) {
        for (auto* comp : obj->components) comp->Update();
    }
    // �S�A�j���[�^�[Update
    for (auto* obj : m_SceneObjects) {
        if (auto* animator = obj->GetComponent<Animator>())
            animator->Update(1.0f / 120.0f);
    }

    // �v���C���[���m�̍U������E���S����E�Q�[���I�[�o�[�����PlayerManager�ɈϏ�
    m_PlayerManager.UpdatePlayers();

    // �Q�[���I�[�o�[�V�[���ւ̑J�ڔ���
    if (!sceneChanged && (m_PlayerManager.IsP1DyingEnded() || m_PlayerManager.IsP2DyingEnded())) {
        sceneChanged = true;
        engine->ChangeScene(std::make_unique<GameOverScene>(engine));
        return;
    }
}

/// <summary>
/// ���t���[���̕`�揈��
/// </summary>
void GameScene::Draw() {
    Camera* cam = engine->GetCamera();

    // �T�C�h�r���[�J�����ݒ�
    cam->SetPosition({ 0.0f, 2.0f, -5.0f });
    cam->SetTarget({ 0.0f, 2.0f, 0.0f });

    XMMATRIX view = cam->GetViewMatrix();
    XMMATRIX proj = cam->GetProjectionMatrix(
        engine->GetSwapChainManager()->GetWidth(),
        engine->GetSwapChainManager()->GetHeight()
    );

    // �X�J�C�h�[���̈ʒu���J�����ɒǏ]
    auto* sky = FindByTag("Sky");
    if (sky) {
        auto* skyTr = sky->GetComponent<Transform>();
        if (skyTr) skyTr->position = cam->GetPosition();
    }

    constexpr size_t CBV_SIZE = 256;
    void* mapped = nullptr;
    auto* cb = engine->GetBufferManager()->GetConstantBuffer();
    cb->Map(0, nullptr, &mapped);

    // �e�I�u�W�F�N�g�̒萔�o�b�t�@�X�V
    for (size_t i = 0; i < m_SceneObjects.size(); ++i) {
        GameObject* obj = m_SceneObjects[i];
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
    // �ʏ�I�u�W�F�N�g�`��
    for (size_t i = 0; i < m_SceneObjects.size(); ++i) {
        auto* obj = m_SceneObjects[i];
        if (!obj->GetComponent<UIImage>()) {
            engine->GetRenderer()->DrawObject(obj, i, view, proj);
        }
    }
    // UI�I�u�W�F�N�g�`��i�Ō�ɕ`��j
    for (size_t i = 0; i < m_SceneObjects.size(); ++i) {
        auto* obj = m_SceneObjects[i];
        if (obj->GetComponent<UIImage>()) {
            engine->GetRenderer()->DrawObject(obj, i, view, proj);
        }
    }
    engine->GetRenderer()->EndFrame();
}

/// <summary>
/// �X�e�[�W�̏���������
/// </summary>
void GameScene::InitStage() {
    m_SceneObjects.push_back(ObjectFactory::CreateCube(
        engine, { 0.0f, -0.5f, 0.0f }, { 10.0f, 1.0f, 3.0f }, -1, Colors::Gray, { 0,0,0 }, { -1,-1,-1 }, "Ground", "GroundFloor"
    ));

    int reelTex = engine->GetTextureManager()->LoadTexture(L"assets/Slot/Reel.png", engine->GetDeviceManager()->GetCommandList());

    // �X���b�g���[��3�{�������тŐ���
    for (int i = 0; i < 3; ++i) {
        float x = -2.0f + i * 2.0f;
        auto* reel = ObjectFactory::CreateCylinderReel(
            engine,
            { x, 2.0f, 0.0f },
            { 2.0f, 2.0f, 2.0f },
            reelTex,
            Colors::White,
            "Reel",
            "SlotReel" + std::to_string(i + 1)
        );
        m_SceneObjects.push_back(reel);
    }
}

/// <summary>
/// �A�j���[�V�����̓o�^����
/// </summary>
void GameScene::RegisterAnimations() {
    struct AnimEntry { const char* file; const char* name; };
    AnimEntry anims1[] = {
        { "assets/MMA/BodyBlock.fbx",    "BodyBlock"   },
        { "assets/MMA/MmaKick.fbx",      "Kick"        },
        { "assets/MMA/MutantDying.fbx",  "Dying"       },
        { "assets/MMA/Punching.fbx",     "Punch"       },
        { "assets/MMA/BouncingFightIdle.fbx", "Idle"   },
        { "assets/MMA/Walking.fbx",      "Walk"        },
        { "assets/MMA/TakingPunch.fbx",  "Reaction"    },
    };
    AnimEntry anims2[] = {
        { "assets/MMA2/OutwardBlock.fbx",    "BodyBlock"   },
        { "assets/MMA2/MmaKick.fbx",         "Kick"        },
        { "assets/MMA2/DyingBackwards.fbx",  "Dying"       },
        { "assets/MMA2/Punching2P.fbx",      "Punch"       },
        { "assets/MMA2/BouncingFightIdle2.fbx", "Idle"     },
        { "assets/MMA2/WalkingPlayer2.fbx",  "Walk"        },
        { "assets/MMA2/Kicking.fbx",         "Kick2"       },
        { "assets/MMA2/ZombieReactionHit.fbx", "Reaction"  },
    };

    auto* animator1 = FindByName("Player1")->GetComponent<Animator>();
    for (auto& anim1 : anims1) {
        std::vector<Animator::Keyframe> keys;
        double animLen;
        if (FbxModelLoader::LoadAnimationOnly(anim1.file, keys, animLen)) {
            animator1->AddAnimation(anim1.name, keys);
        }
    }
    auto* animator2 = FindByName("Player2")->GetComponent<Animator>();
    for (auto& anim2 : anims2) {
        std::vector<Animator::Keyframe> keys;
        double animLen;
        if (FbxModelLoader::LoadAnimationOnly(anim2.file, keys, animLen)) {
            animator2->AddAnimation(anim2.name, keys);
        }
    }
}

/// <summary>
/// �Q�[���V�[���̃f�X�g���N�^
/// </summary>
GameScene::~GameScene() {
    for (auto* obj : m_SceneObjects) {
        delete obj;
    }
    m_SceneObjects.clear();
}