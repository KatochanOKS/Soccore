#pragma once
#include "Component.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <DirectXMath.h>

//--------------------------------------
// FBX�A�j���[�V�����Đ��p�R���|�[�l���g
//--------------------------------------
class Animator : public Component {
public:
    // �L�[�t���[�����i�e�������ƂɑS�{�[���̍s��Z�b�g�j
    struct Keyframe {
        double time; // �Đ������i�b�P�ʁj
        std::vector<DirectX::XMMATRIX> pose; // �S�{�[���̎p���s��
    };

    // �A�j���[�V�������Ƃ̃L�[�t���[���z��
    std::unordered_map<std::string, std::vector<Keyframe>> animations; // �A�j�������L�[�t���[���z��

    std::vector<std::string> boneNames;                   // �{�[�������X�g
    std::vector<DirectX::XMMATRIX> boneMatrices;           // ���̑S�{�[���̎p��

    std::vector<DirectX::XMMATRIX> bindPoses; // ���o�C���h�|�[�Y��ۑ����Ă���


    std::string currentAnim;  // ���݂̃A�j����
    double currentTime = 0.0; // �A�j���Đ��ʒu
    bool isPlaying = true;    // �Đ����t���O

    Animator();

    void SetAnimations(
        const std::unordered_map<std::string, std::vector<Keyframe>>& anims,
        const std::vector<std::string>& bones,
        const std::vector<DirectX::XMMATRIX>& bindPoseMatrices
    );


    // �Đ��A�j���[�V������؂�ւ���iWalk, Jump���j
    void SetAnimation(const std::string& animName);

    // ���t���[���A���݂̃{�[���s����X�V����i������Update�Ƃ͓Ɨ��j
    void Update(float deltaTime);

    // ���݂̃{�[���s��i�`��p�j
    const std::vector<DirectX::XMMATRIX>& GetCurrentPose() const;

    // �A�j���[�V�����p�� �~ BindPose�␳���s�����X�L�j���O�s���Ԃ�
    std::vector<DirectX::XMMATRIX> GetSkinnedPose(const std::vector<DirectX::XMMATRIX>& bindPoses) const;

};
