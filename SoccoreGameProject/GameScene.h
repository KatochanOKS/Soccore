#pragma once
#include "Scene.h"
#include "EngineManager.h"
#include "ReelComponent.h"
#include "GameUIManager.h"
#include "PlayerManager.h"
#include <vector>
#include <string>
#include <DirectXMath.h>

/// <summary>
/// �Q�[���̃��C���V�[���B�X�e�[�W�EUI�E�v���C���[�E�A�j���[�V�����Ȃǂ̏������E�Ǘ��E�`��E�X�V��S������B
/// </summary>
class GameScene : public Scene {
public:
    /// <summary>
    /// �Q�[���V�[���̃R���X�g���N�^
    /// </summary>
    
    GameScene(EngineManager* engine);

    /// <summary>
    /// �Q�[���V�[���̃f�X�g���N�^
    /// </summary>
    
    ~GameScene() override;

    /// <summary>
    /// �V�[���J�n���̏���������
    /// </summary>
    
    void Start() override;

    /// <summary>
    /// ���t���[���̍X�V����
    /// </summary>
    
    void Update() override;

    /// <summary>
    /// ���t���[���̕`�揈��
    /// </summary>
    
    void Draw() override;

    /// <summary>
    /// �w�肵���^�O������GameObject����������
    /// </summary>
    
    GameObject* FindByTag(const std::string& tag);

    /// <summary>
    /// �w�肵�����O������GameObject����������
    /// </summary>
    
    GameObject* FindByName(const std::string& name);

    std::vector<GameObject*> m_SceneObjects; ///< �V�[�����̑SGameObject���X�g

private:
    /// <summary>
    /// UI�v�f�̏���������
    /// </summary>
    
    void InitUI();

    /// <summary>
    /// �X�e�[�W�̏���������
    /// </summary>
    
    void InitStage();

    /// <summary>
    /// �v���C���[�̏���������
    /// </summary>
    
    void InitPlayers();

    /// <summary>
    /// �A�j���[�V�����̓o�^����
    /// </summary>
    
    void RegisterAnimations();

    EngineManager* engine = nullptr; ///< �G���W���Ǘ�
	GameUIManager m_UIManager;    ///< UI�Ǘ�
	PlayerManager m_PlayerManager;   ///< �v���C���[�Ǘ�

    bool m_IsP1DyingEnded = false;   ///< �v���C���[1���S���o�I���t���O
    bool m_IsP2DyingEnded = false;   ///< �v���C���[2���S���o�I���t���O
};