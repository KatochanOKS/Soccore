#pragma once
#include <fbxsdk.h>
#include <vector>
#include <array>
#include <string>
#pragma comment(lib, "libfbxsdk-md.lib")
#pragma comment(lib, "libxml2-md.lib")
#pragma comment(lib, "zlib-md.lib")
#include "BufferManager.h" // BufferManagerのヘッダーファイルをインクルード
#include <DirectXMath.h>
class FbxModelLoader
{
public:
	FbxModelLoader();
	struct VertexInfo {
		std::vector<Vertex> vertices; // ← ここで BufferManagerのVertex型を使う
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

	// アニメーションの現在のボーン行列を計算する関数
	static void CalcCurrentBoneMatrices(
		const std::string& fbxPath,                  // FBXファイルのパス
		double currentTime,                          // 再生時間（秒）
		std::vector<DirectX::XMMATRIX>& outMatrices, // 計算したボーン行列を詰める配列
		const std::vector<std::string>& boneNames,   // ボーン名リスト
		const std::vector<DirectX::XMMATRIX>& bindPoses // バインドポーズ行列リスト
	);

private:
	static bool IsExistNormalUVInfo(const std::vector<float>& vertexInfo);
	static std::vector<float> CreateVertexInfo(const std::vector<float>& vertex, const FbxVector4& normalVec4, const FbxVector2& uvVec2);
	static int CreateNewVertexIndex(const std::vector<float>& vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2,
		std::vector<std::vector<float>>& vertexInfoList, int oldIndex, std::vector<std::array<int, 2>>& oldNewIndexPairList);
	static bool IsSetNormalUV(const std::vector<float> vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2);
};
