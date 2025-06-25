#include "FbxModelLoader.h"
#include "BufferManager.h"
#include <map>        // 追加（ボーン名→インデックス用）
#include <string>     // 追加
FbxModelLoader::FbxModelLoader()
{
}
// FBXノードを再帰的にたどってボーン情報を抽出する関数
void GetBonesRecursive(FbxNode* node, int parentIdx,
    std::vector<Bone>& outBones,
    std::map<std::string, int>& boneNameToIdx)
{
    if (!node) return;

    // このノードが「スケルトン（ボーン）」か判定
    if (node->GetNodeAttribute() &&
        node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton) {
        Bone bone;
        bone.name = node->GetName();    // ボーン名
        bone.parentIndex = parentIdx;   // 親インデックス
        bone.bindPoseMatrix = node->EvaluateGlobalTransform(); // バインドポーズ行列
        int newIdx = static_cast<int>(outBones.size());
        outBones.push_back(bone);
        boneNameToIdx[bone.name] = newIdx;
        parentIdx = newIdx; // 子の親として渡す
    }
    // 子ノードを再帰探索
    for (int i = 0; i < node->GetChildCount(); ++i) {
        GetBonesRecursive(node->GetChild(i), parentIdx, outBones, boneNameToIdx);
    }
}


bool FbxModelLoader::Load(const std::string& filePath, VertexInfo* vertexInfo)
{
    auto manager = FbxManager::Create();
    auto importer = FbxImporter::Create(manager, "");
    if (!importer->Initialize(filePath.c_str(), -1, manager->GetIOSettings()))
        return false;
    auto scene = FbxScene::Create(manager, "");
    importer->Import(scene);
    importer->Destroy();
    FbxGeometryConverter geometryConverter(manager);
    if (!geometryConverter.Triangulate(scene, true))
        return false;

    // ボーン階層を再帰で収集
    std::vector<Bone> bones;
    std::map<std::string, int> boneNameToIndex;
    GetBonesRecursive(scene->GetRootNode(), -1, bones, boneNameToIndex);

    // 全体で使うボーン情報
    std::map<std::string, int> boneNameToIndex; // ボーン名→インデックス
    std::vector<std::string> boneNames;         // ボーン名リスト（インデックス対応）


    // ======= 全メッシュ取得 =======
    int meshCount = scene->GetSrcObjectCount<FbxMesh>();
    std::vector<Vertex> allVertices;
    std::vector<unsigned short> allIndices;
    unsigned short indexOffset = 0;

    for (int m = 0; m < meshCount; ++m) {
        auto mesh = scene->GetSrcObject<FbxMesh>(m);
        if (!mesh) continue;

        // UVセット名の取得
        FbxStringList uvSetNameList;
        mesh->GetUVSetNames(uvSetNameList);
        if (uvSetNameList.GetCount() == 0) continue;
        const char* uvSetName = uvSetNameList.GetStringAt(0);

        // コントロールポイント数＝FBX内部の頂点数
        int controlPointCount = mesh->GetControlPointsCount();
        // 頂点ごとにスキン情報（最大4つのボーンとウェイト）を保存する構造体
        struct SkinData {
            std::vector<std::pair<int, float>> boneWeightPairs; // (ボーン番号, ウェイト)
        };
        std::vector<SkinData> skinDatas(controlPointCount); // 頂点数分

        // ----- スキン情報の抽出 -----
        int deformerCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
        for (int deformerIdx = 0; deformerIdx < deformerCount; ++deformerIdx) {
            FbxSkin* skin = (FbxSkin*)mesh->GetDeformer(deformerIdx, FbxDeformer::eSkin);
            if (!skin) continue;
            // すべてのクラスタ（＝ボーン）について
            for (int clusterIdx = 0; clusterIdx < skin->GetClusterCount(); ++clusterIdx) {
                FbxCluster* cluster = skin->GetCluster(clusterIdx);
                // ボーン名の取得
                std::string boneName = cluster->GetLink()->GetName();
                // ボーンインデックス割り当て（初出なら追加）
                int boneIdx = 0;
                if (boneNameToIndex.count(boneName) == 0) {
                    boneIdx = static_cast<int>(boneNames.size());
                    boneNames.push_back(boneName);
                    boneNameToIndex[boneName] = boneIdx;
                }
                else {
                    boneIdx = boneNameToIndex[boneName];
                }

                int* indices = cluster->GetControlPointIndices();
                double* weights = cluster->GetControlPointWeights();
                int indicesCount = cluster->GetControlPointIndicesCount();
                for (int i = 0; i < indicesCount; ++i) {
                    int vertexIdx = indices[i];
                    float weight = static_cast<float>(weights[i]);
                    if (weight <= 0.0f) continue;
                    // 4つまで登録
                    if (skinDatas[vertexIdx].boneWeightPairs.size() < 4) {
                        skinDatas[vertexIdx].boneWeightPairs.push_back({ boneIdx, weight });
                    }
                }
            }
        }
        // ----- ここまでで各頂点に最大4つまでの(ボーン番号, ウェイト)が記録される -----

        // 頂点座標
        std::vector<std::vector<float>> vertexInfoList;
        for (int i = 0; i < mesh->GetControlPointsCount(); i++) {
            auto point = mesh->GetControlPointAt(i);
            std::vector<float> vertex;
            vertex.push_back(point[0]);
            vertex.push_back(point[1]);
            vertex.push_back(point[2]);
            vertexInfoList.push_back(vertex);
        }
        // 頂点毎の情報
        std::vector<unsigned short> indices;
        std::vector<std::array<int, 2>> oldNewIndexPairList;
        for (int polIndex = 0; polIndex < mesh->GetPolygonCount(); polIndex++) {
            for (int polVertexIndex = 0; polVertexIndex < mesh->GetPolygonSize(polIndex); polVertexIndex++) {
                auto vertexIndex = mesh->GetPolygonVertex(polIndex, polVertexIndex);
                std::vector<float> vertexInfo = vertexInfoList[vertexIndex];
                FbxVector4 normalVec4;
                mesh->GetPolygonVertexNormal(polIndex, polVertexIndex, normalVec4);
                FbxVector2 uvVec2;
                bool isUnMapped;
                mesh->GetPolygonVertexUV(polIndex, polVertexIndex, uvSetName, uvVec2, isUnMapped);
                if (!IsExistNormalUVInfo(vertexInfo)) {
                    vertexInfoList[vertexIndex] = CreateVertexInfo(vertexInfo, normalVec4, uvVec2);
                }
                else if (!IsSetNormalUV(vertexInfo, normalVec4, uvVec2)) {
                    vertexIndex = CreateNewVertexIndex(vertexInfo, normalVec4, uvVec2, vertexInfoList, vertexIndex, oldNewIndexPairList);
                }
                indices.push_back(vertexIndex);
            }
        }
        // 頂点データを構築してオフセット加算
        std::vector<Vertex> vertices;
        for (int i = 0; i < vertexInfoList.size(); i++) {
            std::vector<float>& vi = vertexInfoList[i];

            Vertex v = {
                vi[0], vi[1], vi[2],    // 位置
                vi[3], vi[4], vi[5],    // 法線
                vi[6], 1.0f - vi[7]     // UV（FBXはvが逆なので1-v）
            };

            // ---- スキニング（ボーン）情報をセット ----
            auto& pairs = skinDatas[i].boneWeightPairs;
            float totalWeight = 0.0f;
            for (int k = 0; k < pairs.size() && k < 4; ++k) {
                v.boneIndices[k] = pairs[k].first;   // ボーンインデックス
                v.boneWeights[k] = pairs[k].second;  // ウェイト
                totalWeight += pairs[k].second;
            }
            // ウェイトの合計が1.0になるよう正規化（重要！）
            if (totalWeight > 0.0f) {
                for (int k = 0; k < 4; ++k)
                    v.boneWeights[k] /= totalWeight;
            }
            // ---- ここまで ----

            vertices.push_back(v);


        }
        // インデックスはオフセットを付けてallIndicesへ
        for (auto idx : indices) {
            allIndices.push_back(idx + indexOffset);
        }
        // 頂点もallVerticesへ
        allVertices.insert(allVertices.end(), vertices.begin(), vertices.end());
        indexOffset += static_cast<unsigned short>(vertices.size());
    }

    *vertexInfo = { allVertices, allIndices, bones }; // ここでボーンも渡す

    // マネージャー、シーンの破棄
    scene->Destroy();
    manager->Destroy();
    return true;
}

// --- 残りはそのまま ---
bool FbxModelLoader::IsExistNormalUVInfo(const std::vector<float>& vertexInfo)
{
    return vertexInfo.size() == 8;
}
std::vector<float> FbxModelLoader::CreateVertexInfo(const std::vector<float>& vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2)
{
    std::vector<float> newVertexInfo;
    newVertexInfo.push_back(vertexInfo[0]);
    newVertexInfo.push_back(vertexInfo[1]);
    newVertexInfo.push_back(vertexInfo[2]);
    newVertexInfo.push_back(normalVec4[0]);
    newVertexInfo.push_back(normalVec4[1]);
    newVertexInfo.push_back(normalVec4[2]);
    newVertexInfo.push_back(uvVec2[0]);
    newVertexInfo.push_back(uvVec2[1]);
    return newVertexInfo;
}
int FbxModelLoader::CreateNewVertexIndex(const std::vector<float>& vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2,
    std::vector<std::vector<float>>& vertexInfoList, int oldIndex, std::vector<std::array<int, 2>>& oldNewIndexPairList)
{
    for (int i = 0; i < oldNewIndexPairList.size(); i++) {
        int newIndex = oldNewIndexPairList[i][1];
        if (oldIndex == oldNewIndexPairList[i][0]
            && IsSetNormalUV(vertexInfoList[newIndex], normalVec4, uvVec2)) {
            return newIndex;
        }
    }
    std::vector<float> newVertexInfo = CreateVertexInfo(vertexInfo, normalVec4, uvVec2);
    vertexInfoList.push_back(newVertexInfo);
    int newIndex = vertexInfoList.size() - 1;
    std::array<int, 2> oldNewIndexPair{ oldIndex , newIndex };
    oldNewIndexPairList.push_back(oldNewIndexPair);
    return newIndex;
}
bool FbxModelLoader::IsSetNormalUV(const std::vector<float> vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2)
{
    return fabs(vertexInfo[3] - normalVec4[0]) < FLT_EPSILON
        && fabs(vertexInfo[4] - normalVec4[1]) < FLT_EPSILON
        && fabs(vertexInfo[5] - normalVec4[2]) < FLT_EPSILON
        && fabs(vertexInfo[6] - uvVec2[0]) < FLT_EPSILON
        && fabs(vertexInfo[7] - uvVec2[1]) < FLT_EPSILON;
}
