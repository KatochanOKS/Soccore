#include "FbxModelLoader.h"
#include "BufferManager.h"
#include <DirectXMath.h>
using namespace DirectX;
#include <algorithm>

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

// スキニングモデル読込
bool FbxModelLoader::LoadSkinningModel(const std::string& filePath, SkinningVertexInfo* outInfo)
{
    // FBX初期化
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

    // 初期化
    outInfo->vertices.clear();
    outInfo->indices.clear();
    outInfo->boneNames.clear();
    outInfo->bindPoses.clear();

    int meshCount = scene->GetSrcObjectCount<FbxMesh>();
    char buf[256];
    sprintf_s(buf, "[FBX DEBUG] meshCount = %d\n", meshCount);
    OutputDebugStringA(buf);

    bool bonesInitialized = false; // ボーン名/bindPoseを一度だけセットする

    unsigned int indexOffset = 0; // 複数メッシュ用の頂点オフセット

    for (int m = 0; m < meshCount; ++m) {
        FbxMesh* mesh = scene->GetSrcObject<FbxMesh>(m);
        if (!mesh) continue;
        int ctrlCount = mesh->GetControlPointsCount();
        int skinCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
        int polyCount = mesh->GetPolygonCount();

        sprintf_s(buf, "[FBX DEBUG] Mesh %d: skinCount=%d, ctrlPts=%d, polys=%d\n", m, skinCount, ctrlCount, polyCount);
        OutputDebugStringA(buf);

        // --- コントロールポイントごとにボーン情報を集める ---
        struct BoneWeight { int boneIdx; float weight; };
        std::vector<std::vector<BoneWeight>> ctrlptBones(ctrlCount);

        // スキン取得
        int clusterCount = 0;
        if (skinCount > 0) {
            FbxSkin* skin = static_cast<FbxSkin*>(mesh->GetDeformer(0, FbxDeformer::eSkin));
            clusterCount = skin->GetClusterCount();

            sprintf_s(buf, "[FBX DEBUG] Mesh %d: clusterCount=%d\n", m, clusterCount);
            OutputDebugStringA(buf);

            // ボーン名/bindPoseは最初のスキニングメッシュでのみセット（多重登録防止）
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
                sprintf_s(buf, "[FBX DEBUG] Cluster %d: idxCount=%d (%s)\n", c, idxCount, boneName.c_str());
                OutputDebugStringA(buf);

                for (int i = 0; i < idxCount; ++i) {
                    int ctrlIdx = indices[i];
                    float w = (float)weights[i];
                    if (ctrlIdx >= 0 && ctrlIdx < ctrlCount)
                        ctrlptBones[ctrlIdx].push_back({ c, w });
                    else {
                        sprintf_s(buf, "[FBX WARNING] ctrlIdx %d out of range for ctrlCount=%d\n", ctrlIdx, ctrlCount);
                        OutputDebugStringA(buf);
                    }
                }
            }
            bonesInitialized = true;
        }
        else {
            OutputDebugStringA("[FBX DEBUG] No skin found for this mesh.\n");
        }

        // --- 頂点ごとにSkinningVertex構築 ---
        FbxStringList uvSets;
        mesh->GetUVSetNames(uvSets);
        const char* uvSet = (uvSets.GetCount() > 0) ? uvSets[0] : "";

        std::vector<SkinningVertex> vertices;
        std::vector<uint16_t> indices;

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

                // 頂点生成
                SkinningVertex v = {};
                v.x = (float)pos[0]; v.y = (float)pos[1]; v.z = (float)pos[2];
                v.nx = (float)normal[0]; v.ny = (float)normal[1]; v.nz = (float)normal[2];
                v.u = (float)uv[0]; v.v = 1.0f - (float)uv[1];

                // --- ボーン情報をセット ---
                auto& boneList = ctrlptBones[ctrlIdx];
                std::sort(boneList.begin(), boneList.end(), [](auto& a, auto& b) {return a.weight > b.weight; });
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
                    v.boneIndices[0] = 0;
                    v.boneWeights[0] = 1.0f;
                    for (int k = 1; k < 4; ++k) {
                        v.boneIndices[k] = 0;
                        v.boneWeights[k] = 0.0f;
                    }
                }

                indices.push_back((uint16_t)vertices.size());
                vertices.push_back(v);
            }
        }

        // --- 複数メッシュ対応：indexOffsetを使ってずらしてから追加！ ---
        for (auto idx : indices) {
            outInfo->indices.push_back(idx + indexOffset);
        }
        outInfo->vertices.insert(outInfo->vertices.end(), vertices.begin(), vertices.end());
        indexOffset += vertices.size();
    }

    scene->Destroy();
    manager->Destroy();
    return true;
}



/// --- FBXを一度だけロードして必要情報をキャッシュする関数 ---
// （ファイル名から呼ばれ、初回だけメモリ上に読み込んで返す）
FbxModelInstance* FbxModelLoader::LoadAndCache(const std::string& fbxPath)
{
    auto* instance = new FbxModelInstance();

    // FBXの初期化（管理クラス・シーン生成）
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

    // --- 複数メッシュ対応：最初のスキンメッシュからボーン・バインドポーズ配列取得 ---
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
                // bindPose（逆行列）をキャッシュ
                instance->bindPoses[c] = FbxAMatrixToXMMATRIX(link->EvaluateGlobalTransform().Inverse());
            }
            bonesInitialized = true;
        }
    }

    // --- アニメーションの長さを取得（取得できなければデフォルト2.5秒） ---
    instance->animationLength = 2.5; // 仮の値
    FbxAnimStack* animStack = instance->scene->GetCurrentAnimationStack();
    if (animStack) {
        FbxTakeInfo* takeInfo = instance->scene->GetTakeInfo(animStack->GetName());
        if (takeInfo) {
            FbxTimeSpan span = takeInfo->mLocalTimeSpan;
            instance->animationLength = span.GetDuration().GetSecondDouble();
        }
    }

    // ボーンが1つもないモデルは失敗
    if (instance->boneNames.empty()) {
        delete instance;
        return nullptr;
    }

    return instance;
}


// --- キャッシュ済みインスタンス＋再生時刻で、ボーン最終行列配列を計算する関数 ---
void FbxModelLoader::CalcCurrentBoneMatrices(
    FbxModelInstance* instance,
    double currentTime,
    std::vector<DirectX::XMMATRIX>& outMatrices
) {
    if (!instance) return;

    // --- アニメーションのループ再生に対応 ---
    if (instance->animationLength > 0.0)
        currentTime = fmod(currentTime, instance->animationLength);

    // --- FBXアニメーションレイヤー取得 ---
    FbxAnimStack* animStack = instance->scene->GetCurrentAnimationStack();
    if (!animStack) return;
    FbxAnimLayer* animLayer = animStack->GetMember<FbxAnimLayer>(0);
    if (!animLayer) return;

    FbxTime time;
    time.SetSecondDouble(currentTime);

    // --- 全ボーンについて、グローバル変換×バインドポーズ逆行列を計算 ---
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
