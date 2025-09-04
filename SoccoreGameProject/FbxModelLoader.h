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

//----------------------------------------------------
// FBXファイルからモデル・アニメ・スキニング情報を抽出するクラス
//----------------------------------------------------
class FbxModelLoader
{
public:
    FbxModelLoader();

    //----------------------------------------
    // 1. 静的モデル（ボーンなし・アニメなし）の頂点＆インデックス格納用
    //----------------------------------------
    struct VertexInfo {
        std::vector<Vertex> vertices;
        std::vector<unsigned short> indices;
    };

    //----------------------------------------
    // 2. スキニング対応モデルの情報格納用
    //----------------------------------------
    struct SkinningVertexInfo {
        std::vector<SkinningVertex> vertices;        // スキニング対応頂点リスト
        std::vector<unsigned short> indices;         // インデックス配列
        std::vector<std::string> boneNames;          // ボーン名リスト
        std::vector<DirectX::XMMATRIX> bindPoses;    // ボーンごとのバインドポーズ行列
        struct Animation {
            std::string name;                        // アニメ名
            double length;                           // アニメ長（秒）
            std::vector<Animator::Keyframe> keyframes;
        };
        std::vector<Animation> animations;           // 複数アニメを格納
    };

    //----------------------------------------
    // 3. 静的モデル読込
    //----------------------------------------
    // FBXからボーンなし頂点/インデックスを抽出
    static bool Load(const std::string& filePath, VertexInfo* vertexInfo);

    //----------------------------------------
    // 4. スキニングモデル読込
    //----------------------------------------
    // FBXからボーン名/バインドポーズ/スキン頂点/アニメをすべて抽出
    static bool LoadSkinningModel(const std::string& filePath, SkinningVertexInfo* outInfo);

    // アニメーションだけを抽出したい場合
    static bool LoadAnimationOnly(const std::string& fbxPath, std::vector<Animator::Keyframe>& outKeyframes, double& outLength);

    // --- バイナリキャッシュ保存/読込 ---
    static bool SaveSkinningBin(const std::string& path, const SkinningVertexInfo* info);
    static bool LoadSkinningBin(const std::string& path, SkinningVertexInfo* info);
    static bool SaveAnimationBin(const std::string& path, const std::vector<Animator::Keyframe>& keyframes, double length);
    static bool LoadAnimationBin(const std::string& path, std::vector<Animator::Keyframe>& keyframes, double& length);
private:
    // 以降は内部ユーティリティ関数
    static bool IsExistNormalUVInfo(const std::vector<float>& vertexInfo);
    static std::vector<float> CreateVertexInfo(const std::vector<float>& vertex, const FbxVector4& normalVec4, const FbxVector2& uvVec2);
    static int CreateNewVertexIndex(const std::vector<float>& vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2,
        std::vector<std::vector<float>>& vertexInfoList, int oldIndex, std::vector<std::array<int, 2>>& oldNewIndexPairList);
    static bool IsSetNormalUV(const std::vector<float> vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2);
};
