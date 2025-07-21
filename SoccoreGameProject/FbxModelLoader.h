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

// --- FBXキャッシュ用構造体 ---
// これは「FBXモデルを1回だけロードし、必要な情報をずっと保持するための箱」です
struct FbxModelInstance {
	FbxManager* manager = nullptr;            // FBX SDK 管理インスタンス
	FbxScene* scene = nullptr;                // FBXシーンデータ
	std::vector<std::string> boneNames;       // ボーン名リスト
	std::vector<DirectX::XMMATRIX> bindPoses; // ボーンのバインドポーズ行列
	double animationLength = 0.0;             // アニメーションの総再生時間（秒）

	// --- デストラクタでFBXリソースの解放（必須!!）---
	~FbxModelInstance() {
		if (scene)    scene->Destroy();
		if (manager)  manager->Destroy();
	}
};


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

	// FBXのアニメーションデータをキャッシュする構造体
	struct FbxModelInstance {
		FbxManager* manager = nullptr;            // FBX管理クラス
		FbxScene* scene = nullptr;                // FBXシーン
		std::vector<std::string> boneNames;       // ボーン名リスト
		std::vector<DirectX::XMMATRIX> bindPoses; // ボーンごとのバインドポーズ
		double animationLength = 0.0;             // アニメーションの長さ（秒）
	};

	static bool Load(const std::string& filePath, VertexInfo* vertexInfo);

	static bool LoadSkinningModel(
		const std::string& filePath,
		SkinningVertexInfo* outInfo
	);

	// ★FBXをロードしてキャッシュ構造体を作成（初回だけ）
	static FbxModelInstance* LoadAndCache(const std::string& filePath);

	// ★キャッシュ済みインスタンスからアニメーション時刻でボーン行列を計算
	static void CalcCurrentBoneMatrices(
		FbxModelInstance* instance,             // キャッシュインスタンス
		double currentTime,                     // 再生したい時刻（秒）
		std::vector<DirectX::XMMATRIX>& outMatrices // 計算結果格納先
	);

private:
	static bool IsExistNormalUVInfo(const std::vector<float>& vertexInfo);
	static std::vector<float> CreateVertexInfo(const std::vector<float>& vertex, const FbxVector4& normalVec4, const FbxVector2& uvVec2);
	static int CreateNewVertexIndex(const std::vector<float>& vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2,
		std::vector<std::vector<float>>& vertexInfoList, int oldIndex, std::vector<std::array<int, 2>>& oldNewIndexPairList);
	static bool IsSetNormalUV(const std::vector<float> vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2);
};
