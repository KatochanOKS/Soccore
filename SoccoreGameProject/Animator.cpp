#include "Animator.h"

//--------------------------------------
// �R���X�g���N�^
//--------------------------------------
Animator::Animator() {}

//--------------------------------------
// �A�j���[�V�����f�[�^�E�{�[�������Z�b�g�i���������ɌĂԁj
//--------------------------------------
void Animator::SetAnimations(const std::unordered_map<std::string, std::vector<Keyframe>>& anims, const std::vector<std::string>& bones) {
    animations = anims;
    boneNames = bones;
    if (!anims.empty())
        currentAnim = anims.begin()->first; // �ŏ��̃A�j���ɃZ�b�g
    currentTime = 0.0;
    isPlaying = true;
    boneMatrices.clear();
    if (!boneNames.empty())
        boneMatrices.resize(boneNames.size(), DirectX::XMMatrixIdentity());
}

//--------------------------------------
// �Đ��A�j���[�V������؂�ւ���iWalk, Jump���j
//--------------------------------------
void Animator::SetAnimation(const std::string& animName) {
    if (animations.find(animName) != animations.end()) {
        currentAnim = animName;
        currentTime = 0.0;
    }
}

//--------------------------------------
// ���t���[���A���݂̃{�[���s����X�V����
//--------------------------------------
void Animator::Update(float deltaTime) {
    if (!isPlaying || animations.count(currentAnim) == 0) return;
    const auto& frames = animations[currentAnim];
    if (frames.empty()) return;

    // ���Ԃ�i�߂ă��[�v
    currentTime += deltaTime;
    double animLength = frames.back().time;
    if (currentTime > animLength)
        currentTime = fmod(currentTime, animLength);

    // ���ݎ����ɍł��߂��L�[�t���[����T��
    size_t frameIdx = 0;
    while (frameIdx + 1 < frames.size() && frames[frameIdx + 1].time <= currentTime)
        ++frameIdx;

    // �L�[�t���[������{�[���s��Z�b�g
    boneMatrices = frames[frameIdx].pose;
}

//--------------------------------------
// ���݂̃{�[���s��i�`��p�j��Ԃ�
//--------------------------------------
const std::vector<DirectX::XMMATRIX>& Animator::GetCurrentPose() const {
    return boneMatrices;
}
