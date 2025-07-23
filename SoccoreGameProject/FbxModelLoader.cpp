#include "FbxModelLoader.h"
#include "BufferManager.h"
#include <DirectXMath.h>
using namespace DirectX;
#include <algorithm>
#include <map>
#include <tuple>
FbxModelLoader::FbxModelLoader()
{
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
            vertices.push_back(Vertex{
                vi[0], vi[1], vi[2],
                vi[3], vi[4], vi[5],
                vi[6], 1.0f - vi[7] // ← ここを変更 FBX/Blender/Maya等 → vは下から上が0→1 FBX/Blender/Maya等 → vは下から上が0→1
                });


        }
        // インデックスはオフセットを付けてallIndicesへ
        for (auto idx : indices) {
            allIndices.push_back(idx + indexOffset);
        }
        // 頂点もallVerticesへ
        allVertices.insert(allVertices.end(), vertices.begin(), vertices.end());
        indexOffset += static_cast<unsigned short>(vertices.size());
    }

    // マネージャー、シーンの破棄
    scene->Destroy();
    manager->Destroy();
    *vertexInfo = { allVertices, allIndices };
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

// FBXの行列をDirectXに変換
static XMMATRIX FbxAMatrixToXMMATRIX(const FbxAMatrix& m) {
    XMFLOAT4X4 xf;
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            xf.m[r][c] = (float)m.Get(r, c);
    return XMLoadFloat4x4(&xf);
}

// SkinningVertex属性を比較するための構造体
struct SkinVtxKey {
    float x, y, z;
    float nx, ny, nz;
    float u, v;
    uint32_t boneIndices[4];
    float boneWeights[4];

    // 完全一致比較
    bool operator<(const SkinVtxKey& rhs) const {
        // tupleで全部比較
        return std::tie(x, y, z, nx, ny, nz, u, v,
            boneIndices[0], boneIndices[1], boneIndices[2], boneIndices[3],
            boneWeights[0], boneWeights[1], boneWeights[2], boneWeights[3])
            < std::tie(rhs.x, rhs.y, rhs.z, rhs.nx, rhs.ny, rhs.nz, rhs.u, rhs.v,
                rhs.boneIndices[0], rhs.boneIndices[1], rhs.boneIndices[2], rhs.boneIndices[3],
                rhs.boneWeights[0], rhs.boneWeights[1], rhs.boneWeights[2], rhs.boneWeights[3]);
    }
};

bool FbxModelLoader::LoadSkinningModel(const std::string& filePath, SkinningVertexInfo* outInfo)
{
    // --- FBX初期化 ---
    FbxManager* manager = FbxManager::Create();
    FbxImporter* importer = FbxImporter::Create(manager, "");
    if (!importer->Initialize(filePath.c_str(), -1, manager->GetIOSettings())) {
        manager->Destroy();
        return false;
    }
    FbxScene* scene = FbxScene::Create(manager, "");
    importer->Import(scene);
    importer->Destroy();
    FbxGeometryConverter conv(manager);
    conv.Triangulate(scene, true);

    outInfo->vertices.clear();
    outInfo->indices.clear();
    outInfo->boneNames.clear();
    outInfo->bindPoses.clear();

    int meshCount = scene->GetSrcObjectCount<FbxMesh>();
    bool bonesInitialized = false;

    unsigned int indexOffset = 0;
    for (int m = 0; m < meshCount; ++m) {
        FbxMesh* mesh = scene->GetSrcObject<FbxMesh>(m);
        if (!mesh) continue;
        int ctrlCount = mesh->GetControlPointsCount();
        int skinCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
        int polyCount = mesh->GetPolygonCount();

        // --- コントロールポイントごとにボーン情報 ---
        struct BoneWeight { int boneIdx; float weight; };
        std::vector<std::vector<BoneWeight>> ctrlptBones(ctrlCount);

        int clusterCount = 0;
        if (skinCount > 0) {
            FbxSkin* skin = static_cast<FbxSkin*>(mesh->GetDeformer(0, FbxDeformer::eSkin));
            clusterCount = skin->GetClusterCount();

            if (!bonesInitialized) {
                outInfo->boneNames.resize(clusterCount);
                outInfo->bindPoses.resize(clusterCount);
            }

            for (int c = 0; c < clusterCount; ++c) {
                FbxCluster* cluster = skin->GetCluster(c);
                FbxNode* link = cluster->GetLink();
                std::string boneName = link ? link->GetName() : "Bone" + std::to_string(c);

                if (!bonesInitialized) {
                    outInfo->boneNames[c] = boneName;
                    outInfo->bindPoses[c] = FbxAMatrixToXMMATRIX(link->EvaluateGlobalTransform().Inverse());
                }

                int* indices = cluster->GetControlPointIndices();
                double* weights = cluster->GetControlPointWeights();
                int idxCount = cluster->GetControlPointIndicesCount();

                for (int i = 0; i < idxCount; ++i) {
                    int ctrlIdx = indices[i];
                    float w = (float)weights[i];
                    if (ctrlIdx >= 0 && ctrlIdx < ctrlCount)
                        ctrlptBones[ctrlIdx].push_back({ c, w });
                }
            }
            bonesInitialized = true;
        }

        // --- 頂点重複排除マップ ---
        std::map<SkinVtxKey, uint16_t> vtxMap;
        std::vector<SkinningVertex> vertices;
        std::vector<uint16_t> indices;

        FbxStringList uvSets;
        mesh->GetUVSetNames(uvSets);
        const char* uvSet = (uvSets.GetCount() > 0) ? uvSets[0] : "";

        for (int pi = 0; pi < polyCount; ++pi) {
            int polySize = mesh->GetPolygonSize(pi);
            for (int vi = 0; vi < polySize; ++vi) {
                int ctrlIdx = mesh->GetPolygonVertex(pi, vi);
                FbxVector4 pos = mesh->GetControlPointAt(ctrlIdx);

                // 法線
                FbxVector4 normal;
                mesh->GetPolygonVertexNormal(pi, vi, normal);

                // UV
                FbxVector2 uv = { 0,0 }; bool unmapped = false;
                mesh->GetPolygonVertexUV(pi, vi, uvSet, uv, unmapped);

                // --- ボーン情報 ---
                auto& boneList = ctrlptBones[ctrlIdx];
                std::sort(boneList.begin(), boneList.end(), [](auto& a, auto& b) {return a.weight > b.weight; });

                SkinningVertex v = {};
                v.x = (float)pos[0]; v.y = (float)pos[1]; v.z = (float)pos[2];
                v.nx = (float)normal[0]; v.ny = (float)normal[1]; v.nz = (float)normal[2];
                v.u = (float)uv[0];
                v.v = 1.0f - (float)uv[1]; // FBX下から上が0→1

                float sumW = 0.0f;
                for (int k = 0; k < 4 && k < (int)boneList.size(); ++k) {
                    v.boneIndices[k] = boneList[k].boneIdx;
                    v.boneWeights[k] = boneList[k].weight;
                    sumW += v.boneWeights[k];
                }
                if (sumW > 0.0f)
                    for (int k = 0; k < 4; ++k)
                        v.boneWeights[k] /= sumW;
                else {
                    v.boneIndices[0] = 0; v.boneWeights[0] = 1.0f;
                    for (int k = 1; k < 4; ++k) { v.boneIndices[k] = 0; v.boneWeights[k] = 0.0f; }
                }

                // --- 重複頂点判定用キーを生成 ---
                SkinVtxKey key;
                key.x = v.x; key.y = v.y; key.z = v.z;
                key.nx = v.nx; key.ny = v.ny; key.nz = v.nz;
                key.u = v.u; key.v = v.v;
                for (int k = 0; k < 4; ++k) {
                    key.boneIndices[k] = v.boneIndices[k];
                    key.boneWeights[k] = v.boneWeights[k];
                }

                // --- mapで重複頂点を管理 ---
                auto it = vtxMap.find(key);
                if (it != vtxMap.end()) {
                    indices.push_back(it->second);
                }
                else {
                    uint16_t newIdx = (uint16_t)vertices.size();
                    vtxMap[key] = newIdx;
                    vertices.push_back(v);
                    indices.push_back(newIdx);
                }
            }
        }

        // --- 複数メッシュ対応 ---
        for (auto idx : indices)
            outInfo->indices.push_back(idx + indexOffset);
        outInfo->vertices.insert(outInfo->vertices.end(), vertices.begin(), vertices.end());
        indexOffset += (unsigned int)vertices.size();
    }

    scene->Destroy();
    manager->Destroy();
    return true;
}



// --- FBXを一度だけロードしてキャッシュ ---
FbxModelInstance* FbxModelLoader::LoadAndCache(const std::string& fbxPath)
{
    auto* instance = new FbxModelInstance();
    instance->manager = FbxManager::Create();
    instance->scene = FbxScene::Create(instance->manager, "");
    FbxImporter* importer = FbxImporter::Create(instance->manager, "");
    if (!importer->Initialize(fbxPath.c_str(), -1, instance->manager->GetIOSettings())) {
        importer->Destroy();
        delete instance;
        return nullptr;
    }
    importer->Import(instance->scene);
    importer->Destroy();

    // ボーン情報セット
    int meshCount = instance->scene->GetSrcObjectCount<FbxMesh>();
    bool bonesInitialized = false;
    for (int m = 0; m < meshCount; ++m) {
        FbxMesh* mesh = instance->scene->GetSrcObject<FbxMesh>(m);
        if (!mesh) continue;
        int skinCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
        if (skinCount > 0 && !bonesInitialized) {
            FbxSkin* skin = static_cast<FbxSkin*>(mesh->GetDeformer(0, FbxDeformer::eSkin));
            int clusterCount = skin->GetClusterCount();
            instance->boneNames.resize(clusterCount);
            instance->bindPoses.resize(clusterCount);
            for (int c = 0; c < clusterCount; ++c) {
                FbxCluster* cluster = skin->GetCluster(c);
                FbxNode* link = cluster->GetLink();
                std::string boneName = link ? link->GetName() : "Bone" + std::to_string(c);
                instance->boneNames[c] = boneName;
                instance->bindPoses[c] = FbxAMatrixToXMMATRIX(link->EvaluateGlobalTransform().Inverse());
            }
            bonesInitialized = true;
        }
    }
    // アニメ長取得
    instance->animationLength = 2.5;
    FbxAnimStack* animStack = instance->scene->GetCurrentAnimationStack();
    if (animStack) {
        FbxTakeInfo* takeInfo = instance->scene->GetTakeInfo(animStack->GetName());
        if (takeInfo) {
            FbxTimeSpan span = takeInfo->mLocalTimeSpan;
            instance->animationLength = span.GetDuration().GetSecondDouble();
        }
    }
    if (instance->boneNames.empty()) {
        delete instance;
        return nullptr;
    }
    return instance;
}

// --- キャッシュ済みインスタンスからボーン行列計算 ---
void FbxModelLoader::CalcCurrentBoneMatrices(
    FbxModelInstance* instance,
    double currentTime,
    std::vector<DirectX::XMMATRIX>& outMatrices
) {
    if (!instance) return;
    if (instance->animationLength > 0.0)
        currentTime = fmod(currentTime, instance->animationLength);

    FbxAnimStack* animStack = instance->scene->GetCurrentAnimationStack();
    if (!animStack) return;
    FbxAnimLayer* animLayer = animStack->GetMember<FbxAnimLayer>(0);
    if (!animLayer) return;

    FbxTime time;
    time.SetSecondDouble(currentTime);

    for (int i = 0; i < (int)instance->boneNames.size(); ++i) {
        FbxNode* boneNode = instance->scene->FindNodeByName(instance->boneNames[i].c_str());
        if (!boneNode) {
            outMatrices[i] = DirectX::XMMatrixIdentity();
            continue;
        }
        FbxAMatrix globalTransform = boneNode->EvaluateGlobalTransform(time);
        DirectX::XMMATRIX current = FbxAMatrixToXMMATRIX(globalTransform);
        outMatrices[i] = DirectX::XMMatrixTranspose(instance->bindPoses[i] * current);
    }
}
