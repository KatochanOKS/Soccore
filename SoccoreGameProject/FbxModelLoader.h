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
#include "Animator.h" // Animator::Keyframeを使うため

//--------------------------------------
// FBXの一時インスタンス構造体
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
// FBXモデルローダークラス
//--------------------------------------
class FbxModelLoader
{
public:
    FbxModelLoader();

    // -------------------------------
    // 静的モデル（非スキン）用 頂点・インデックス配列
    // -------------------------------
    struct VertexInfo {
        std::vector<Vertex> vertices;
        std::vector<unsigned short> indices;
    };

    // -------------------------------
    // スキニング用データ構造体
    // -------------------------------
    struct SkinningVertexInfo {
        std::vector<SkinningVertex> vertices;           // スキニング頂点配列
        std::vector<unsigned short> indices;            // インデックス配列
        std::vector<std::string> boneNames;             // ボーン名リスト
        std::vector<DirectX::XMMATRIX> bindPoses;       // バインドポーズ行列
        // --- アニメーション情報 ---
        struct Animation {
            std::string name;
            double length; // アニメ長（秒）
            std::vector<Animator::Keyframe> keyframes;
        };
        std::vector<Animation> animations;              // アニメーション配列
    };

    // -------------------------------
    // 静的モデル読み込み（既存）
    // -------------------------------
    static bool Load(const std::string& filePath, VertexInfo* vertexInfo);

    // -------------------------------
    // スキニング対応モデルのFBX読み込み（新規）
    // -------------------------------
    // @filePath : FBXファイルパス
    // @outInfo  : SkinningVertexInfo格納先
    // return    : 成功でtrue
    // -------------------------------
    static bool LoadSkinningModel(const std::string& filePath, SkinningVertexInfo* outInfo);

private:
    // 既存のユーティリティ関数群
    static bool IsExistNormalUVInfo(const std::vector<float>& vertexInfo);
    static std::vector<float> CreateVertexInfo(const std::vector<float>& vertex, const FbxVector4& normalVec4, const FbxVector2& uvVec2);
    static int CreateNewVertexIndex(const std::vector<float>& vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2,
        std::vector<std::vector<float>>& vertexInfoList, int oldIndex, std::vector<std::array<int, 2>>& oldNewIndexPairList);
    static bool IsSetNormalUV(const std::vector<float> vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2);
};
