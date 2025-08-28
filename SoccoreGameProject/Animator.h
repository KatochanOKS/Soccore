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

    bool loop = true;  // �f�t�H���g�̓��[�v�Đ�
    bool isPlaying = true;  // ���Đ�����

    // --- �R���X�g���N�^ ---
    Animator();

    // --- �A�j��/�{�[��/�o�C���h�|�[�Y�̃Z�b�g�imove�Z�}���e�B�N�X�Ή��j ---
    void SetAnimations(
        std::unordered_map<std::string, std::vector<Keyframe>>&& anims,
        std::vector<std::string>&& bones,
        std::vector<DirectX::XMMATRIX>&& bindPoseMatrices
    );

    // --- �A�j���[�V�����̐؂�ւ� ---
    void SetAnimation(const std::string& animName, bool loop = true);

    // --- �t���[�����ƂɃA�j����i�߂� ---
    void Update(float deltaTime);

    // --- ���݂̃{�[���s��i�`��p�j ---
    const std::vector<DirectX::XMMATRIX>& GetCurrentPose() const;

    // --- �X�L�j���O�p�i�o�C���h�|�[�Y�␳���j ---
    std::vector<DirectX::XMMATRIX> GetSkinnedPose(const std::vector<DirectX::XMMATRIX>& bindPoses) const;

    // --- �A�j���[�V�����ǉ��i�P�̂ł��ǉ��\�j ---
    void AddAnimation(const std::string& name, const std::vector<Keyframe>& keyframes);

private:
    // �����I�ȁu�Đ����t���[���ԍ��v�v�Z
    size_t FindKeyframeIndex(const std::vector<Keyframe>& frames, double time) const;
};
