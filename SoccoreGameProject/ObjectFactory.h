#pragma once
#include <string>
#include <DirectXMath.h>
#include "Colors.h"

class EngineManager;
class GameObject;

class ObjectFactory {
public:
    // ------------------------------------------------------------
    // ���ׂĂ̊֐��́ustatic�v�I
    // �� �C���X�^���X����炸�A�ǂ�����ł��uObjectFactory::CreateXXX()�v�ŌĂׂ�
    //    �i����E���[�e�B���e�B�N���X�Ƃ��Ďg�����߁j
    //
    // �Ԃ�l�́uGameObject*�v�I
    // �� new�œ��I���������I�u�W�F�N�g�̃A�h���X��Ԃ�
    //    �i�Ԃ��ꂽ�|�C���^�͌Ăяo������vector���ɓo�^���ĊǗ�����j
    // ------------------------------------------------------------

    // ��F�L���[�u�^�̃I�u�W�F�N�g�𐶐�
    static GameObject* CreateCube(
        EngineManager* engine,
        const DirectX::XMFLOAT3& pos,
        const DirectX::XMFLOAT3& scale,
        int texIdx = -1,
        const DirectX::XMFLOAT4& color = Colors::White
    );

    // ��FFBX���f���i�ÓI�j�̐���
    static GameObject* CreateModel(
        EngineManager* engine,
        const std::string& path,
        const DirectX::XMFLOAT3& pos,
        const DirectX::XMFLOAT3& scale,
        int texIndex = -1,
        const DirectX::XMFLOAT4& color = Colors::White
    );

    // ��F�A�j���Ή��i�X�L�j���O�j���f���̐���
    static GameObject* CreateSkinningModel(
        EngineManager* engine,
        const std::string& fbxPath,
        const DirectX::XMFLOAT3& pos,
        const DirectX::XMFLOAT3& scale,
        int texIndex = -1,
        const DirectX::XMFLOAT4& color = Colors::White
    );

    // ��F�A�j���o�^�O�̃X�L���Ή����f��
    static GameObject* CreateSkinningBaseModel(
        EngineManager* engine,
        const std::string& fbxPath,
        const DirectX::XMFLOAT3& pos,
        const DirectX::XMFLOAT3& scale,
        int texIndex = -1,
        const DirectX::XMFLOAT4& color = Colors::White
    );

    // ��F�T�b�J�[�{�[���̐����i�T�C�Y��scale�Œ����j
    static GameObject* CreateBall(
        EngineManager* engine,
        const DirectX::XMFLOAT3& pos,
        const DirectX::XMFLOAT3& scale,
        int texIndex = -1,
        const DirectX::XMFLOAT4& color = Colors::White
    );

	// ��F�X�J�C�h�[���p�̋��̃��b�V������
    // ObjectFactory.h�i�錾��ǉ��j
    static GameObject* CreateSkyDome(EngineManager* engine, int texIndex, float radius = 500.0f);

};
