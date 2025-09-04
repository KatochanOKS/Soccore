#pragma once
#include <fbxsdk.h>
#include <vector>
#include <array>
#include <string>
#include <DirectXMath.h>
#pragma comment(lib, "libfbxsdk-md.lib")
#pragma comment(lib, "libxml2-md.lib")
#pragma comment(lib, "zlib-md.lib")
#include "BufferManager.h"
#include "Animator.h" // Animator::Keyframe���g������

//----------------------------------------------------
// FBX�t�@�C�����烂�f���E�A�j���E�X�L�j���O���𒊏o����N���X
//----------------------------------------------------
class FbxModelLoader
{
public:
    FbxModelLoader();

    //----------------------------------------
    // 1. �ÓI���f���i�{�[���Ȃ��E�A�j���Ȃ��j�̒��_���C���f�b�N�X�i�[�p
    //----------------------------------------
    struct VertexInfo {
        std::vector<Vertex> vertices;
        std::vector<unsigned short> indices;
    };

    //----------------------------------------
    // 2. �X�L�j���O�Ή����f���̏��i�[�p
    //----------------------------------------
    struct SkinningVertexInfo {
        std::vector<SkinningVertex> vertices;        // �X�L�j���O�Ή����_���X�g
        std::vector<unsigned short> indices;         // �C���f�b�N�X�z��
        std::vector<std::string> boneNames;          // �{�[�������X�g
        std::vector<DirectX::XMMATRIX> bindPoses;    // �{�[�����Ƃ̃o�C���h�|�[�Y�s��
        struct Animation {
            std::string name;                        // �A�j����
            double length;                           // �A�j�����i�b�j
            std::vector<Animator::Keyframe> keyframes;
        };
        std::vector<Animation> animations;           // �����A�j�����i�[
    };

    //----------------------------------------
    // 3. �ÓI���f���Ǎ�
    //----------------------------------------
    // FBX����{�[���Ȃ����_/�C���f�b�N�X�𒊏o
    static bool Load(const std::string& filePath, VertexInfo* vertexInfo);

    //----------------------------------------
    // 4. �X�L�j���O���f���Ǎ�
    //----------------------------------------
    // FBX����{�[����/�o�C���h�|�[�Y/�X�L�����_/�A�j�������ׂĒ��o
    static bool LoadSkinningModel(const std::string& filePath, SkinningVertexInfo* outInfo);

    // �A�j���[�V���������𒊏o�������ꍇ
    static bool LoadAnimationOnly(const std::string& fbxPath, std::vector<Animator::Keyframe>& outKeyframes, double& outLength);

    // --- �o�C�i���L���b�V���ۑ�/�Ǎ� ---
    static bool SaveSkinningBin(const std::string& path, const SkinningVertexInfo* info);
    static bool LoadSkinningBin(const std::string& path, SkinningVertexInfo* info);
    static bool SaveAnimationBin(const std::string& path, const std::vector<Animator::Keyframe>& keyframes, double length);
    static bool LoadAnimationBin(const std::string& path, std::vector<Animator::Keyframe>& keyframes, double& length);
private:
    // �ȍ~�͓������[�e�B���e�B�֐�
    static bool IsExistNormalUVInfo(const std::vector<float>& vertexInfo);
    static std::vector<float> CreateVertexInfo(const std::vector<float>& vertex, const FbxVector4& normalVec4, const FbxVector2& uvVec2);
    static int CreateNewVertexIndex(const std::vector<float>& vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2,
        std::vector<std::vector<float>>& vertexInfoList, int oldIndex, std::vector<std::array<int, 2>>& oldNewIndexPairList);
    static bool IsSetNormalUV(const std::vector<float> vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2);
};
