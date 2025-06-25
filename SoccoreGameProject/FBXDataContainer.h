#pragma once
#include <fbxsdk.h>
#include <vector>
#include <DirectXMath.h>

class FBXDataContainer {
public:
    FBXDataContainer();
    ~FBXDataContainer();

    bool Load(const std::wstring& fbxPath);             // FBX�t�@�C����ǂݍ���
    int GetAnimeFrames() const;                         // �A�j���[�V�����̑S�t���[����
    void UpdateBoneMatrix(int frame);                   // �w��t���[���̃{�[���p���ɍX�V

    const std::vector<DirectX::XMMATRIX>& GetFinalBoneMatrices() const { return m_finalMatrices; }

private:
    void ExtractBoneHierarchy(FbxNode* node);
    void ComputeBoneMatrices(int frame);

    struct Bone {
        std::string name;
        FbxNode* node;
        DirectX::XMMATRIX offsetMatrix;     // bindPose�t�s��
    };

    FbxManager* m_manager = nullptr;
    FbxScene* m_scene = nullptr;
    FbxAnimLayer* m_animLayer = nullptr;

    std::vector<Bone> m_bones;
    std::vector<DirectX::XMMATRIX> m_finalMatrices;
    int m_totalFrames = 0;
};
