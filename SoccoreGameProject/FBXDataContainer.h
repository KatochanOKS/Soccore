#pragma once
#include <fbxsdk.h>
#include <vector>
#include <DirectXMath.h>

class FBXDataContainer {
public:
    FBXDataContainer();
    ~FBXDataContainer();

    bool Load(const std::wstring& fbxPath);             // FBXファイルを読み込む
    int GetAnimeFrames() const;                         // アニメーションの全フレーム数
    void UpdateBoneMatrix(int frame);                   // 指定フレームのボーン姿勢に更新

    const std::vector<DirectX::XMMATRIX>& GetFinalBoneMatrices() const { return m_finalMatrices; }

private:
    void ExtractBoneHierarchy(FbxNode* node);
    void ComputeBoneMatrices(int frame);

    struct Bone {
        std::string name;
        FbxNode* node;
        DirectX::XMMATRIX offsetMatrix;     // bindPose逆行列
    };

    FbxManager* m_manager = nullptr;
    FbxScene* m_scene = nullptr;
    FbxAnimLayer* m_animLayer = nullptr;

    std::vector<Bone> m_bones;
    std::vector<DirectX::XMMATRIX> m_finalMatrices;
    int m_totalFrames = 0;
};
