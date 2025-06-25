#pragma once
#include <fbxsdk.h>
#include <vector>
#include <array>
#include <string>
#pragma comment(lib, "libfbxsdk-md.lib")
#pragma comment(lib, "libxml2-md.lib")
#pragma comment(lib, "zlib-md.lib")
#include "BufferManager.h" // BufferManagerのヘッダーファイルをインクルード

// --- ボーン情報構造体 ---
struct Bone {
    std::string name;           // ボーン名
    int parentIndex;            // 親ボーンのインデックス（親なしは-1）
    FbxAMatrix bindPoseMatrix;  // 初期バインドポーズ行列
};

// --- 頂点＆インデックス＆ボーン情報をまとめる構造体 ---
struct VertexInfo {
    std::vector<Vertex> vertices;              // 頂点配列（BufferManagerのVertex型）
    std::vector<unsigned short> indices;       // インデックス配列
    std::vector<Bone> bones;                   // ボーン配列
};

class FbxModelLoader
{
public:
    FbxModelLoader();

    static bool Load(const std::string& filePath, VertexInfo* vertexInfo);

private:
    static bool IsExistNormalUVInfo(const std::vector<float>& vertexInfo);
    static std::vector<float> CreateVertexInfo(const std::vector<float>& vertex, const FbxVector4& normalVec4, const FbxVector2& uvVec2);
    static int CreateNewVertexIndex(const std::vector<float>& vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2,
        std::vector<std::vector<float>>& vertexInfoList, int oldIndex, std::vector<std::array<int, 2>>& oldNewIndexPairList);
    static bool IsSetNormalUV(const std::vector<float> vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2);
};
