#include "FbxModelLoader.h"
#include "BufferManager.h"
#include <DirectXMath.h>
#include <algorithm>
#include <map>
#include <tuple>
#include <iostream>   // std::cout, もしくはOutputDebugStringA
#include <functional>  // ★ここを必ず追加！
using namespace DirectX;
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

#include "FbxModelLoader.h"
// ...（他のインクルードも必要に応じて）...

//----------------------------
// スキニング＆アニメ付きFBXの読み込み
//----------------------------
//bool FbxModelLoader::LoadSkinningModel(const std::string& filePath, SkinningVertexInfo* outInfo)
//{
//    //----------------------------------------
//    // 1. FBX SDKマネージャ、インポータ、シーンの生成
//    //----------------------------------------
//    // FbxManager* manager = FbxManager::Create();
//    // FbxImporter* importer = FbxImporter::Create(manager, "");
//    // importer->Initialize(filePath.c_str(), -1, manager->GetIOSettings());
//    // FbxScene* scene = FbxScene::Create(manager, "");
//    // importer->Import(scene);
//    // importer->Destroy();
//
//    //----------------------------------------
//    // 2. メッシュの三角形化
//    //----------------------------------------
//    // FbxGeometryConverter geometryConverter(manager);
//    // geometryConverter.Triangulate(scene, true);
//
//    //----------------------------------------
//    // 3. ボーン階層/名前/バインドポーズ行列の抽出
//    //----------------------------------------
//    // - シーン内の全骨ノードを走査し、boneNames, bindPosesをoutInfoに格納
//
//    //----------------------------------------
//    // 4. 各頂点ごとに
//    //    ・座標, 法線, UV
//    //    ・ボーンインデックス, ボーンウェイト
//    // を抽出し、SkinningVertex配列を生成
//    //----------------------------------------
//    // - 各FbxMeshのコントロールポイントから必要情報を取得
//
//    //----------------------------------------
//    // 5. インデックス配列も生成
//    //----------------------------------------
//    // - ポリゴン情報から
//
//    //----------------------------------------
//    // 6. 各アニメーション（Take/Clip）ごとに
//    //    ・全キーフレームごとに全ボーンの変換行列配列
//    //    ・Animator::Keyframe配列としてoutInfo->animationsに格納
//    //----------------------------------------
//    // - アニメーションスタックやレイヤを走査
//
//    //----------------------------------------
//    // 7. すべてoutInfoにセット完了後、FBXオブジェクト開放
//    //----------------------------------------
//    // scene->Destroy();
//    // manager->Destroy();
//
//    //----------------------------------------
//    // 8. 成功でtrue、失敗時はfalse
//    //----------------------------------------
//    return false; // ※まだ仮実装。細かい実装は後で段階的に
//}

//----------------------------
// スキニング＆アニメ付きFBXの読み込み
//----------------------------
bool FbxModelLoader::LoadSkinningModel(const std::string& filePath, SkinningVertexInfo* outInfo)
{
    // --- 1. FBX SDKマネージャ作成 ---
    OutputDebugStringA("[FBX] 1. FbxManager作成\n");
    FbxManager* manager = FbxManager::Create();
    if (!manager) {
        OutputDebugStringA("[FBX] FbxManager作成失敗\n");
        return false;
    }

    // --- 2. インポータ作成＆初期化 ---
    OutputDebugStringA("[FBX] 2. FbxImporter作成＆初期化\n");
    FbxImporter* importer = FbxImporter::Create(manager, "");
    if (!importer->Initialize(filePath.c_str(), -1, manager->GetIOSettings())) {
        OutputDebugStringA("[FBX] FbxImporter初期化失敗\n");
        return false;
    }

    // --- 3. シーン生成＆インポート ---
    OutputDebugStringA("[FBX] 3. シーン生成＆インポート\n");
    FbxScene* scene = FbxScene::Create(manager, "");
    if (!importer->Import(scene)) {
        OutputDebugStringA("[FBX] インポート失敗\n");
        importer->Destroy();
        manager->Destroy();
        return false;
    }
    importer->Destroy();

    // --- 4. 三角形化 ---
    OutputDebugStringA("[FBX] 4. メッシュの三角形化\n");
    FbxGeometryConverter geometryConverter(manager);
    if (!geometryConverter.Triangulate(scene, true)) {
        OutputDebugStringA("[FBX] 三角形化失敗\n");
        scene->Destroy();
        manager->Destroy();
        return false;
    }

    // --- 5. 読み込んだシーンのサマリを出力（ノード数など） ---
    FbxNode* rootNode = scene->GetRootNode();
    if (!rootNode) {
        OutputDebugStringA("[FBX] ルートノードがありません\n");
        scene->Destroy();
        manager->Destroy();
        return false;
    }

    char msg[256];
    sprintf_s(msg, "[FBX] RootNode名: %s, 子ノード数: %d\n",
        rootNode->GetName(), rootNode->GetChildCount());
    OutputDebugStringA(msg);

    // --- 6. とりあえず全ノード名を再帰で出してみる ---
 // 先に宣言だけ
    std::function<void(FbxNode*, int)> PrintNodeNames;

    // その後、初期化
    PrintNodeNames = [&](FbxNode* node, int depth) {
        std::string indent(depth * 2, ' ');
        char nodeMsg[256];
        sprintf_s(nodeMsg, "%s- %s\n", indent.c_str(), node->GetName());
        OutputDebugStringA(nodeMsg);
        for (int i = 0; i < node->GetChildCount(); ++i)
            PrintNodeNames(node->GetChild(i), depth + 1);
        };

    // --- 7. ボーンノードの抽出・リストアップ ---
    OutputDebugStringA("[FBX] --- ボーンノード一覧 ---\n");

    // ボーン判定のための関数（たとえばMixamoならノード名に"mixamorig:"が含まれるものがボーン）
    auto IsBoneNode = [](FbxNode* node) -> bool {
        std::string name = node->GetName();
        // Mixamoのボーン名は "mixamorig:" から始まる
        return name.find("mixamorig:") != std::string::npos;
        };

    // 再帰的にボーンノード名を出力＆boneNamesに追加
    std::function<void(FbxNode*, int)> ListBoneNodes;
    ListBoneNodes = [&](FbxNode* node, int depth) {
        if (IsBoneNode(node)) {
            std::string indent(depth * 2, ' ');
            char msg[256];
            sprintf_s(msg, "%s- %s\n", indent.c_str(), node->GetName());
            OutputDebugStringA(msg);
            outInfo->boneNames.push_back(node->GetName());
        }
        for (int i = 0; i < node->GetChildCount(); ++i)
            ListBoneNodes(node->GetChild(i), depth + 1);
        };
    ListBoneNodes(rootNode, 0);

    char msg2[128];
    sprintf_s(msg2, "[FBX] ボーン数 = %zu\n", outInfo->boneNames.size());
    OutputDebugStringA(msg2);


    // --- 8. 各ボーンのバインドポーズ行列取得 ---
    OutputDebugStringA("[FBX] --- バインドポーズ行列（初期姿勢）取得 ---\n");

    outInfo->bindPoses.clear();
    auto* pose = scene->GetPose(0);
    if (pose && pose->IsBindPose()) {
        OutputDebugStringA("[FBX] BindPoseから行列取得\n");
        for (const std::string& boneName : outInfo->boneNames) {
            FbxNode* boneNode = scene->FindNodeByName(boneName.c_str());
            if (!boneNode) {
                char err[128];
                sprintf_s(err, "[FBX] ボーンノードが見つからない: %s\n", boneName.c_str());
                OutputDebugStringA(err);
                outInfo->bindPoses.push_back(DirectX::XMMatrixIdentity());
                continue;
            }
            // Pose配列をループして一致するノードを探す
            FbxMatrix mat;
            bool found = false;
            for (int pi = 0; pi < pose->GetCount(); ++pi) {
                if (pose->GetNode(pi) == boneNode) {
                    mat = pose->GetMatrix(pi);
                    found = true;
                    break;
                }
            }
            if (!found) {
                char err[128];
                sprintf_s(err, "[FBX] BindPoseが見つからない: %s\n", boneName.c_str());
                OutputDebugStringA(err);
                outInfo->bindPoses.push_back(DirectX::XMMatrixIdentity());
                continue;
            }

            DirectX::XMMATRIX dxMat = DirectX::XMMatrixIdentity();
            for (int r = 0; r < 4; ++r)
                for (int c = 0; c < 4; ++c)
                    dxMat.r[r].m128_f32[c] = static_cast<float>(mat.Get(r, c));
            outInfo->bindPoses.push_back(dxMat);

            char msg[256];
            sprintf_s(msg, "[FBX] Bone: %s, BindPose: (%.2f, %.2f, %.2f)\n",
                boneName.c_str(),
                (float)mat.Get(0, 3), (float)mat.Get(1, 3), (float)mat.Get(2, 3));
            OutputDebugStringA(msg);
        }
    }
    else {
        // Fallback: EvaluateGlobalTransform
        OutputDebugStringA("[FBX] BindPoseが見つからない！EvaluateGlobalTransformで代用\n");
        for (const std::string& boneName : outInfo->boneNames) {
            FbxNode* boneNode = scene->FindNodeByName(boneName.c_str());
            if (!boneNode) {
                char err[128];
                sprintf_s(err, "[FBX] ボーンノードが見つからない: %s\n", boneName.c_str());
                OutputDebugStringA(err);
                outInfo->bindPoses.push_back(DirectX::XMMatrixIdentity());
                continue;
            }
            FbxAMatrix bindPoseMatrix = boneNode->EvaluateGlobalTransform();
            DirectX::XMMATRIX dxMat = DirectX::XMMatrixIdentity();
            for (int r = 0; r < 4; ++r)
                for (int c = 0; c < 4; ++c)
                    dxMat.r[r].m128_f32[c] = static_cast<float>(bindPoseMatrix.Get(r, c));
            outInfo->bindPoses.push_back(dxMat);

            char msg[256];
            sprintf_s(msg, "[FBX] Bone: %s, FallbackBindPose: (%.2f, %.2f, %.2f)\n",
                boneName.c_str(),
                (float)bindPoseMatrix.Get(0, 3), (float)bindPoseMatrix.Get(1, 3), (float)bindPoseMatrix.Get(2, 3));
            OutputDebugStringA(msg);
        }
    }


    // --- 9. 各頂点ごとにボーンインデックス＆ウェイトを格納 ---
    OutputDebugStringA("[FBX] --- スキニング頂点情報抽出 ---\n");

    outInfo->vertices.clear();
    outInfo->indices.clear();

    // 全メッシュ走査
    int meshCount = scene->GetSrcObjectCount<FbxMesh>();
    unsigned short indexOffset = 0;
    for (int m = 0; m < meshCount; ++m) {
        auto mesh = scene->GetSrcObject<FbxMesh>(m);
        if (!mesh) continue;

        // --- コントロールポイント（頂点）ごとにボーンインデックスとウェイトを配列化 ---
        // ボーンインデックス: boneNames配列の中で一致したらそのインデックス
        int controlPointCount = mesh->GetControlPointsCount();

        // 各コントロールポイントに対して最大4つのインデックス・ウェイトを割り当て
        struct BoneWeight {
            int indices[4] = { 0,0,0,0 };
            float weights[4] = { 0,0,0,0 };
        };
        std::vector<BoneWeight> boneWeights(controlPointCount);

        // デフォーム情報（スキン）を抽出
        int skinCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
        for (int s = 0; s < skinCount; ++s) {
            FbxSkin* skin = (FbxSkin*)mesh->GetDeformer(s, FbxDeformer::eSkin);
            int clusterCount = skin->GetClusterCount();
            for (int c = 0; c < clusterCount; ++c) {
                FbxCluster* cluster = skin->GetCluster(c);
                std::string boneName = cluster->GetLink()->GetName();
                // boneNames内インデックス取得
                auto it = std::find(outInfo->boneNames.begin(), outInfo->boneNames.end(), boneName);
                int boneIdx = (it != outInfo->boneNames.end()) ? std::distance(outInfo->boneNames.begin(), it) : -1;
                if (boneIdx < 0) continue;

                int* indices = cluster->GetControlPointIndices();
                double* weights = cluster->GetControlPointWeights();
                int indexCount = cluster->GetControlPointIndicesCount();
                for (int i = 0; i < indexCount; ++i) {
                    int ctrlIdx = indices[i];
                    double weight = weights[i];
                    // 最大4つまで詰める（既に4つ埋まっていたら無視）
                    for (int j = 0; j < 4; ++j) {
                        if (boneWeights[ctrlIdx].weights[j] == 0.0f) {
                            boneWeights[ctrlIdx].indices[j] = boneIdx;
                            boneWeights[ctrlIdx].weights[j] = (float)weight;
                            break;
                        }
                    }
                }
            }
        }

        // 頂点配列を生成
        // FBXの頂点は「コントロールポイント＋法線・UVごとに複製」されるため、ポリゴン単位で処理
        for (int polIndex = 0; polIndex < mesh->GetPolygonCount(); polIndex++) {
            for (int polVertexIndex = 0; polVertexIndex < mesh->GetPolygonSize(polIndex); polVertexIndex++) {
                int ctrlIdx = mesh->GetPolygonVertex(polIndex, polVertexIndex);

                // --- 位置 ---
                auto point = mesh->GetControlPointAt(ctrlIdx);

                // --- 法線 ---
                FbxVector4 normalVec4;
                mesh->GetPolygonVertexNormal(polIndex, polVertexIndex, normalVec4);

                // --- UV ---
                FbxStringList uvSetNameList;
                mesh->GetUVSetNames(uvSetNameList);
                const char* uvSetName = (uvSetNameList.GetCount() > 0) ? uvSetNameList.GetStringAt(0) : "";
                FbxVector2 uvVec2(0, 0);
                bool isUnMapped = false;
                mesh->GetPolygonVertexUV(polIndex, polVertexIndex, uvSetName, uvVec2, isUnMapped);

                // --- スキニング情報 ---
                BoneWeight& bw = boneWeights[ctrlIdx];

                // --- 頂点データをSkinningVertex型で構築 ---
                SkinningVertex v;
                v.x = (float)point[0];
                v.y = (float)point[1];
                v.z = (float)point[2];
                v.nx = (float)normalVec4[0];
                v.ny = (float)normalVec4[1];
                v.nz = (float)normalVec4[2];
                v.u = (float)uvVec2[0];
                v.v = 1.0f - (float)uvVec2[1]; // v座標は上下反転
                for (int i = 0; i < 4; ++i) {
                    v.boneIndices[i] = bw.indices[i];
                    v.boneWeights[i] = bw.weights[i];
                }
                outInfo->vertices.push_back(v);
                outInfo->indices.push_back((unsigned short)outInfo->vertices.size() - 1);

                    char uvmsg[256];
                sprintf_s(uvmsg, "[FBX][UV] Vtx[%d] Pos=(%.2f, %.2f, %.2f) UV=(%.3f, %.3f)\n",
                    (int)outInfo->vertices.size() - 1, v.x, v.y, v.z, v.u, v.v);
                OutputDebugStringA(uvmsg);

                char msg[128];
                sprintf_s(msg, "[FBX] Vtx[%d]: BoneIdx=(%d,%d,%d,%d) Weight=(%.2f,%.2f,%.2f,%.2f)\n",
                    (int)outInfo->vertices.size() - 1,
                    v.boneIndices[0], v.boneIndices[1], v.boneIndices[2], v.boneIndices[3],
                    v.boneWeights[0], v.boneWeights[1], v.boneWeights[2], v.boneWeights[3]);
                OutputDebugStringA(msg);
            }
        }
    }


    // --- 10. アニメーション情報（キーフレーム）の取得 ---
    OutputDebugStringA("[FBX] --- アニメーション情報抽出 ---\n");

    outInfo->animations.clear();

    int animStackCount = scene->GetSrcObjectCount<FbxAnimStack>();
    for (int stackIdx = 0; stackIdx < animStackCount; ++stackIdx) {
        FbxAnimStack* animStack = scene->GetSrcObject<FbxAnimStack>(stackIdx);
        std::string animName = animStack->GetName();
        OutputDebugStringA(("[FBX] アニメ名: " + animName + "\n").c_str());

        // アニメーションレイヤを取得（通常1つでOK）
        FbxAnimLayer* animLayer = animStack->GetMember<FbxAnimLayer>(0);
        if (!animLayer) continue;

        // 開始～終了時間を取得
        FbxTimeSpan timeSpan = animStack->GetLocalTimeSpan();
        FbxTime start = timeSpan.GetStart();
        FbxTime end = timeSpan.GetStop();

        double startSec = start.GetSecondDouble();
        double endSec = end.GetSecondDouble();
        double frameRate = 30.0; // 仮に30FPS（Mixamoはほぼこの値）
        int numFrames = int((endSec - startSec) * frameRate) + 1;

        OutputDebugStringA(("[FBX] フレーム数: " + std::to_string(numFrames) + "\n").c_str());

        // キーフレーム配列
        std::vector<Animator::Keyframe> keyframes;

        // 全フレーム分
        for (int f = 0; f < numFrames; ++f) {
            double sec = startSec + f / frameRate;
            FbxTime t;
            t.SetSecondDouble(sec);

            // 各ボーンの行列
            std::vector<DirectX::XMMATRIX> framePoses;
            for (const std::string& boneName : outInfo->boneNames) {
                FbxNode* boneNode = scene->FindNodeByName(boneName.c_str());
                if (!boneNode) {
                    framePoses.push_back(DirectX::XMMatrixIdentity());
                    continue;
                }
                // アニメ時刻でのグローバル変換を取得
                FbxAMatrix mat = boneNode->EvaluateGlobalTransform(t);
                DirectX::XMMATRIX dxMat = DirectX::XMMatrixIdentity();
                for (int r = 0; r < 4; ++r)
                    for (int c = 0; c < 4; ++c)
                        dxMat.r[r].m128_f32[c] = static_cast<float>(mat.Get(r, c));
                framePoses.push_back(dxMat);
            }
            // キーフレーム登録
            Animator::Keyframe kf;
            kf.time = sec;
            kf.pose = framePoses;
            keyframes.push_back(kf);
        }

        // Animation構造体にまとめて登録
        FbxModelLoader::SkinningVertexInfo::Animation anim;
        anim.name = animName;
        anim.length = endSec - startSec;
        anim.keyframes = keyframes;
        outInfo->animations.push_back(anim);
    }



    // --- 今回はここまで ---
    OutputDebugStringA("[FBX] === FBXロード初期段階完了 ===\n");

    // 解放
    scene->Destroy();
    manager->Destroy();

    return true; // まずは「ファイル読めてノード一覧がprintできる」ことを目標に！
}


