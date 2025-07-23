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

//--------------------------------------
// FBX�̈ꎞ�C���X�^���X�\����
//--------------------------------------
struct FbxModelInstance {
    FbxManager* manager = nullptr;
    FbxScene* scene = nullptr;
    std::vector<std::string> boneNames;
    std::vector<DirectX::XMMATRIX> bindPoses;
    double animationLength = 0.0;
    ~FbxModelInstance() {
        if (scene) scene->Destroy();
        if (manager) manager->Destroy();
    }
};

//--------------------------------------
// FBX���f�����[�_�[�N���X
//--------------------------------------
class FbxModelLoader
{
public:
    FbxModelLoader();

    // -------------------------------
    // �ÓI���f���i��X�L���j�p ���_�E�C���f�b�N�X�z��
    // -------------------------------
    struct VertexInfo {
        std::vector<Vertex> vertices;
        std::vector<unsigned short> indices;
    };

    // -------------------------------
    // �X�L�j���O�p�f�[�^�\����
    // -------------------------------
    struct SkinningVertexInfo {
        std::vector<SkinningVertex> vertices;           // �X�L�j���O���_�z��
        std::vector<unsigned short> indices;            // �C���f�b�N�X�z��
        std::vector<std::string> boneNames;             // �{�[�������X�g
        std::vector<DirectX::XMMATRIX> bindPoses;       // �o�C���h�|�[�Y�s��
        // --- �A�j���[�V������� ---
        struct Animation {
            std::string name;
            double length; // �A�j�����i�b�j
            std::vector<Animator::Keyframe> keyframes;
        };
        std::vector<Animation> animations;              // �A�j���[�V�����z��
    };

    // -------------------------------
    // �ÓI���f���ǂݍ��݁i�����j
    // -------------------------------
    static bool Load(const std::string& filePath, VertexInfo* vertexInfo);

    // -------------------------------
    // �X�L�j���O�Ή����f����FBX�ǂݍ��݁i�V�K�j
    // -------------------------------
    // @filePath : FBX�t�@�C���p�X
    // @outInfo  : SkinningVertexInfo�i�[��
    // return    : ������true
    // -------------------------------
    static bool LoadSkinningModel(const std::string& filePath, SkinningVertexInfo* outInfo);

private:
    // �����̃��[�e�B���e�B�֐��Q
    static bool IsExistNormalUVInfo(const std::vector<float>& vertexInfo);
    static std::vector<float> CreateVertexInfo(const std::vector<float>& vertex, const FbxVector4& normalVec4, const FbxVector2& uvVec2);
    static int CreateNewVertexIndex(const std::vector<float>& vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2,
        std::vector<std::vector<float>>& vertexInfoList, int oldIndex, std::vector<std::array<int, 2>>& oldNewIndexPairList);
    static bool IsSetNormalUV(const std::vector<float> vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2);
};
