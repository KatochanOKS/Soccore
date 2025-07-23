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

// --- グローバルにキャッシュ用struct ---
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

class FbxModelLoader
{
public:
    FbxModelLoader();

    struct VertexInfo {
        std::vector<Vertex> vertices;
        std::vector<unsigned short> indices;
    };
    struct SkinningVertexInfo {
        std::vector<SkinningVertex> vertices;
        std::vector<uint16_t> indices;
        std::vector<std::string> boneNames;
        std::vector<DirectX::XMMATRIX> bindPoses;
    };

    static bool Load(const std::string& filePath, VertexInfo* vertexInfo);

    static bool LoadSkinningModel(
        const std::string& filePath,
        SkinningVertexInfo* outInfo
    );

    // ★FBXファイルからキャッシュインスタンス生成
    static FbxModelInstance* LoadAndCache(const std::string& filePath);

    // ★キャッシュ済みシーンからアニメ時刻でボーン行列計算
    static void CalcCurrentBoneMatrices(
        FbxModelInstance* instance,
        double currentTime,
        std::vector<DirectX::XMMATRIX>& outMatrices
    );

private:
    static bool IsExistNormalUVInfo(const std::vector<float>& vertexInfo);
    static std::vector<float> CreateVertexInfo(const std::vector<float>& vertex, const FbxVector4& normalVec4, const FbxVector2& uvVec2);
    static int CreateNewVertexIndex(const std::vector<float>& vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2,
        std::vector<std::vector<float>>& vertexInfoList, int oldIndex, std::vector<std::array<int, 2>>& oldNewIndexPairList);
    static bool IsSetNormalUV(const std::vector<float> vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2);
};
