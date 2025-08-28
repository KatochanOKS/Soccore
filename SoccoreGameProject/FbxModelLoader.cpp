#include "FbxModelLoader.h"
#include "BufferManager.h"
#include <DirectXMath.h>
#include <algorithm>
#include <map>
#include <tuple>
#include <iostream>
#include <functional>
using namespace DirectX;

//------------------------------------------------------------
// コンストラクタ
//------------------------------------------------------------
FbxModelLoader::FbxModelLoader() {}


//------------------------------------------------------------
// 静的メッシュの読み込み
//------------------------------------------------------------
bool FbxModelLoader::Load(const std::string& filePath, VertexInfo* vertexInfo)
{
    // FBXマネージャ・インポータ作成
    auto manager = FbxManager::Create();
    auto importer = FbxImporter::Create(manager, "");
    if (!importer->Initialize(filePath.c_str(), -1, manager->GetIOSettings()))
        return false;
    auto scene = FbxScene::Create(manager, "");
    importer->Import(scene);
    importer->Destroy();

    // メッシュの三角形化
    FbxGeometryConverter geometryConverter(manager);
    if (!geometryConverter.Triangulate(scene, true))
        return false;

    // 全メッシュ走査
    int meshCount = scene->GetSrcObjectCount<FbxMesh>();
    std::vector<Vertex> allVertices;
    std::vector<unsigned short> allIndices;
    unsigned short indexOffset = 0;

    for (int m = 0; m < meshCount; ++m) {
        auto mesh = scene->GetSrcObject<FbxMesh>(m);
        if (!mesh) continue;

        // UVセット名取得
        FbxStringList uvSetNameList;
        mesh->GetUVSetNames(uvSetNameList);
        if (uvSetNameList.GetCount() == 0) continue;
        const char* uvSetName = uvSetNameList.GetStringAt(0);

        // 頂点座標リスト
        std::vector<std::vector<float>> vertexInfoList;
        for (int i = 0; i < mesh->GetControlPointsCount(); i++) {
            auto point = mesh->GetControlPointAt(i);
            std::vector<float> vertex = {
                static_cast<float>(point[0]),
                static_cast<float>(point[1]),
                static_cast<float>(point[2])
            };
            vertexInfoList.push_back(vertex);
        }

        // 頂点ごとに法線・UV情報付与
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

        // 頂点データ構築
        std::vector<Vertex> vertices;
        for (int i = 0; i < vertexInfoList.size(); i++) {
            std::vector<float>& vi = vertexInfoList[i];
            vertices.push_back(Vertex{
                vi[0], vi[1], vi[2],
                vi[3], vi[4], vi[5],
                vi[6], 1.0f - vi[7] // vは上下反転
                });
        }
        // インデックス・頂点配列格納
        for (auto idx : indices) {
            allIndices.push_back(idx + indexOffset);
        }
        allVertices.insert(allVertices.end(), vertices.begin(), vertices.end());
        indexOffset += static_cast<unsigned short>(vertices.size());
    }

    // メモリ解放
    scene->Destroy();
    manager->Destroy();
    *vertexInfo = { allVertices, allIndices };
    return true;
}

//------------------------------------------------------------
// 頂点が法線・UVを持っているか
//------------------------------------------------------------
bool FbxModelLoader::IsExistNormalUVInfo(const std::vector<float>& vertexInfo)
{
    return vertexInfo.size() == 8;
}

//------------------------------------------------------------
// 頂点情報に法線・UV追加
//------------------------------------------------------------
std::vector<float> FbxModelLoader::CreateVertexInfo(const std::vector<float>& vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2)
{
    std::vector<float> newVertexInfo = vertexInfo;
    newVertexInfo.push_back(normalVec4[0]);
    newVertexInfo.push_back(normalVec4[1]);
    newVertexInfo.push_back(normalVec4[2]);
    newVertexInfo.push_back(uvVec2[0]);
    newVertexInfo.push_back(uvVec2[1]);
    return newVertexInfo;
}

//------------------------------------------------------------
// 新たな法線・UVを持つ頂点を新規追加
//------------------------------------------------------------
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

//------------------------------------------------------------
// 既存頂点が指定法線・UVと一致するか
//------------------------------------------------------------
bool FbxModelLoader::IsSetNormalUV(const std::vector<float> vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2)
{
    return fabs(vertexInfo[3] - normalVec4[0]) < FLT_EPSILON
        && fabs(vertexInfo[4] - normalVec4[1]) < FLT_EPSILON
        && fabs(vertexInfo[5] - normalVec4[2]) < FLT_EPSILON
        && fabs(vertexInfo[6] - uvVec2[0]) < FLT_EPSILON
        && fabs(vertexInfo[7] - uvVec2[1]) < FLT_EPSILON;
}

//------------------------------------------------------------
// スキニング対応モデルの読み込み
//------------------------------------------------------------
bool FbxModelLoader::LoadSkinningModel(const std::string& filePath, SkinningVertexInfo* outInfo)
{
    // --- FBXマネージャ ---
    OutputDebugStringA("[FBX] 1. FbxManager作成\n");
    FbxManager* manager = FbxManager::Create();
    if (!manager) { OutputDebugStringA("[FBX] FbxManager作成失敗\n"); return false; }

    // --- インポータ ---
    OutputDebugStringA("[FBX] 2. FbxImporter作成＆初期化\n");
    FbxImporter* importer = FbxImporter::Create(manager, "");
    if (!importer->Initialize(filePath.c_str(), -1, manager->GetIOSettings())) {
        OutputDebugStringA("[FBX] FbxImporter初期化失敗\n"); return false;
    }

    // --- シーン ---
    OutputDebugStringA("[FBX] 3. シーン生成＆インポート\n");
    FbxScene* scene = FbxScene::Create(manager, "");
    if (!importer->Import(scene)) {
        OutputDebugStringA("[FBX] インポート失敗\n");
        importer->Destroy(); manager->Destroy(); return false;
    }
    importer->Destroy();

    // --- 三角形化 ---
    OutputDebugStringA("[FBX] 4. メッシュの三角形化\n");
    FbxGeometryConverter geometryConverter(manager);
    if (!geometryConverter.Triangulate(scene, true)) {
        OutputDebugStringA("[FBX] 三角形化失敗\n"); scene->Destroy(); manager->Destroy(); return false;
    }

    // --- ルートノード ---
    FbxNode* rootNode = scene->GetRootNode();
    if (!rootNode) {
        OutputDebugStringA("[FBX] ルートノードがありません\n");
        scene->Destroy(); manager->Destroy(); return false;
    }

    // --- ボーン名抽出 ---
    outInfo->boneNames.clear();
    std::function<void(FbxNode*, int)> ListBoneNodes;
    ListBoneNodes = [&](FbxNode* node, int depth) {
        std::string name = node->GetName();
        if (name.find("mixamorig:") != std::string::npos)
            outInfo->boneNames.push_back(name);
        for (int i = 0; i < node->GetChildCount(); ++i)
            ListBoneNodes(node->GetChild(i), depth + 1);
        };
    ListBoneNodes(rootNode, 0);

    // --- バインドポーズ取得 ---
    outInfo->bindPoses.clear();
    auto* pose = scene->GetPose(0);
    if (pose && pose->IsBindPose()) {
        for (const std::string& boneName : outInfo->boneNames) {
            FbxNode* boneNode = scene->FindNodeByName(boneName.c_str());
            FbxMatrix mat;
            bool found = false;
            for (int i = 0; i < pose->GetCount(); ++i) {
                if (pose->GetNode(i) == boneNode) {
                    mat = pose->GetMatrix(i); found = true; break;
                }
            }
            DirectX::XMMATRIX dxMat = DirectX::XMMatrixIdentity();
            if (found) {
                for (int r = 0; r < 4; ++r)
                    for (int c = 0; c < 4; ++c)
                        dxMat.r[r].m128_f32[c] = (float)mat.Get(r, c);
            }
            outInfo->bindPoses.push_back(dxMat);
        }
    }
    else {
        for (const std::string& boneName : outInfo->boneNames) {
            FbxNode* boneNode = rootNode->FindChild(boneName.c_str(), true);
            if (!boneNode) { outInfo->bindPoses.push_back(DirectX::XMMatrixIdentity()); continue; }
            FbxAMatrix bindPoseMatrix = boneNode->EvaluateGlobalTransform();
            DirectX::XMMATRIX dxMat = DirectX::XMMatrixIdentity();
            for (int r = 0; r < 4; ++r)
                for (int c = 0; c < 4; ++c)
                    dxMat.r[r].m128_f32[c] = (float)bindPoseMatrix.Get(r, c);
            outInfo->bindPoses.push_back(dxMat);
        }
    }

    // --- スキニング頂点・インデックス抽出 ---
    outInfo->vertices.clear();
    outInfo->indices.clear();
    int meshCount = scene->GetSrcObjectCount<FbxMesh>();
    unsigned short indexOffset = 0;
    for (int m = 0; m < meshCount; ++m) {
        auto mesh = scene->GetSrcObject<FbxMesh>(m);
        if (!mesh) continue;
        int controlPointCount = mesh->GetControlPointsCount();

        struct BoneWeight {
            int indices[4] = { 0,0,0,0 };
            float weights[4] = { 0,0,0,0 };
        };
        std::vector<BoneWeight> boneWeights(controlPointCount);

        int skinCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
        for (int s = 0; s < skinCount; ++s) {
            FbxSkin* skin = (FbxSkin*)mesh->GetDeformer(s, FbxDeformer::eSkin);
            int clusterCount = skin->GetClusterCount();
            for (int c = 0; c < clusterCount; ++c) {
                FbxCluster* cluster = skin->GetCluster(c);
                std::string boneName = cluster->GetLink()->GetName();
                auto it = std::find(outInfo->boneNames.begin(), outInfo->boneNames.end(), boneName);
                int boneIdx = (it != outInfo->boneNames.end()) ? std::distance(outInfo->boneNames.begin(), it) : -1;
                if (boneIdx < 0) continue;

                int* indices = cluster->GetControlPointIndices();
                double* weights = cluster->GetControlPointWeights();
                int indexCount = cluster->GetControlPointIndicesCount();
                for (int i = 0; i < indexCount; ++i) {
                    int ctrlIdx = indices[i];
                    double weight = weights[i];
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
        for (int polIndex = 0; polIndex < mesh->GetPolygonCount(); polIndex++) {
            for (int polVertexIndex = 0; polVertexIndex < mesh->GetPolygonSize(polIndex); polVertexIndex++) {
                int ctrlIdx = mesh->GetPolygonVertex(polIndex, polVertexIndex);

                auto point = mesh->GetControlPointAt(ctrlIdx);
                FbxVector4 normalVec4;
                mesh->GetPolygonVertexNormal(polIndex, polVertexIndex, normalVec4);

                FbxStringList uvSetNameList;
                mesh->GetUVSetNames(uvSetNameList);
                const char* uvSetName = (uvSetNameList.GetCount() > 0) ? uvSetNameList.GetStringAt(0) : "";
                FbxVector2 uvVec2(0, 0);
                bool isUnMapped = false;
                mesh->GetPolygonVertexUV(polIndex, polVertexIndex, uvSetName, uvVec2, isUnMapped);

                BoneWeight& bw = boneWeights[ctrlIdx];
                SkinningVertex v;
                v.x = (float)point[0];
                v.y = (float)point[1];
                v.z = (float)point[2];
                v.nx = (float)normalVec4[0];
                v.ny = (float)normalVec4[1];
                v.nz = (float)normalVec4[2];
                v.u = (float)uvVec2[0];
                v.v = 1.0f - (float)uvVec2[1];
                for (int i = 0; i < 4; ++i) {
                    v.boneIndices[i] = bw.indices[i];
                    v.boneWeights[i] = bw.weights[i];
                }
                outInfo->vertices.push_back(v);
                outInfo->indices.push_back((unsigned short)outInfo->vertices.size() - 1);
            }
        }
    }

    // --- アニメーション情報（キーフレーム）抽出 ---
    outInfo->animations.clear();
    int animStackCount = scene->GetSrcObjectCount<FbxAnimStack>();
    for (int stackIdx = 0; stackIdx < animStackCount; ++stackIdx) {
        FbxAnimStack* animStack = scene->GetSrcObject<FbxAnimStack>(stackIdx);
        std::string animName = animStack->GetName();
        FbxAnimLayer* animLayer = animStack->GetMember<FbxAnimLayer>(0);
        if (!animLayer) continue;

        FbxTimeSpan timeSpan = animStack->GetLocalTimeSpan();
        FbxTime start = timeSpan.GetStart();
        FbxTime end = timeSpan.GetStop();

        double startSec = start.GetSecondDouble();
        double endSec = end.GetSecondDouble();
        double frameRate = 30.0;
        int numFrames = int((endSec - startSec) * frameRate) + 1;

        std::vector<Animator::Keyframe> keyframes;
        for (int f = 0; f < numFrames; ++f) {
            double sec = startSec + f / frameRate;
            FbxTime t;
            t.SetSecondDouble(sec);
            std::vector<DirectX::XMMATRIX> framePoses;
            for (const std::string& boneName : outInfo->boneNames) {
                FbxNode* boneNode = scene->FindNodeByName(boneName.c_str());
                if (!boneNode) {
                    framePoses.push_back(DirectX::XMMatrixIdentity());
                    continue;
                }
                FbxAMatrix mat = boneNode->EvaluateGlobalTransform(t);
                DirectX::XMMATRIX dxMat = DirectX::XMMatrixIdentity();
                for (int r = 0; r < 4; ++r)
                    for (int c = 0; c < 4; ++c)
                        dxMat.r[r].m128_f32[c] = static_cast<float>(mat.Get(r, c));
                framePoses.push_back(dxMat);
            }
            Animator::Keyframe kf;
            kf.time = sec;
            kf.pose = framePoses;
            keyframes.push_back(kf);
        }
        FbxModelLoader::SkinningVertexInfo::Animation anim;
        anim.name = animName;
        anim.length = endSec - startSec;
        anim.keyframes = keyframes;
        outInfo->animations.push_back(anim);
    }

    OutputDebugStringA("[FBX] === FBXロード初期段階完了 ===\n");

    scene->Destroy();
    manager->Destroy();
    return true;
}

//------------------------------------------------------------
// アニメーションだけ抽出
//------------------------------------------------------------
bool FbxModelLoader::LoadAnimationOnly(
    const std::string& fbxPath,
    std::vector<Animator::Keyframe>& outKeyframes,
    double& outLength
) {
    FbxManager* manager = FbxManager::Create();
    FbxImporter* importer = FbxImporter::Create(manager, "");
    if (!importer->Initialize(fbxPath.c_str(), -1, manager->GetIOSettings())) {
        OutputDebugStringA("[FBX] LoadAnimationOnly: 初期化失敗\n");
        return false;
    }
    FbxScene* scene = FbxScene::Create(manager, "");
    if (!importer->Import(scene)) {
        OutputDebugStringA("[FBX] LoadAnimationOnly: インポート失敗\n");
        importer->Destroy(); manager->Destroy(); return false;
    }
    importer->Destroy();

    // ボーン名抽出
    std::vector<std::string> boneNames;
    std::function<void(FbxNode*)> FindBoneNodes = [&](FbxNode* node) {
        std::string name = node->GetName();
        if (name.find("mixamorig:") != std::string::npos)
            boneNames.push_back(name);
        for (int i = 0; i < node->GetChildCount(); ++i)
            FindBoneNodes(node->GetChild(i));
        };
    FindBoneNodes(scene->GetRootNode());

    int animStackCount = scene->GetSrcObjectCount<FbxAnimStack>();
    if (animStackCount == 0) {
        OutputDebugStringA("[FBX] アニメーションなし\n");
        scene->Destroy(); manager->Destroy(); return false;
    }
    FbxAnimStack* animStack = scene->GetSrcObject<FbxAnimStack>(0);
    FbxAnimLayer* animLayer = animStack->GetMember<FbxAnimLayer>(0);
    if (!animLayer) {
        OutputDebugStringA("[FBX] アニメーションレイヤ取得失敗\n");
        scene->Destroy(); manager->Destroy(); return false;
    }
    FbxTimeSpan timeSpan = animStack->GetLocalTimeSpan();
    FbxTime start = timeSpan.GetStart();
    FbxTime end = timeSpan.GetStop();
    double startSec = start.GetSecondDouble();
    double endSec = end.GetSecondDouble();
    double frameRate = 30.0;
    int numFrames = static_cast<int>((endSec - startSec) * frameRate) + 1;
    outLength = endSec - startSec;

    outKeyframes.clear();
    for (int f = 0; f < numFrames; ++f) {
        double sec = startSec + f / frameRate;
        FbxTime t;
        t.SetSecondDouble(sec);
        std::vector<DirectX::XMMATRIX> pose;
        for (const std::string& boneName : boneNames) {
            FbxNode* boneNode = scene->FindNodeByName(boneName.c_str());
            if (!boneNode) {
                pose.push_back(DirectX::XMMatrixIdentity());
                continue;
            }
            FbxAMatrix mat = boneNode->EvaluateGlobalTransform(t);
            DirectX::XMMATRIX dxMat = DirectX::XMMatrixIdentity();
            for (int r = 0; r < 4; ++r)
                for (int c = 0; c < 4; ++c)
                    dxMat.r[r].m128_f32[c] = static_cast<float>(mat.Get(r, c));
            pose.push_back(dxMat);
        }
        Animator::Keyframe kf;
        kf.time = sec;
        kf.pose = pose;
        outKeyframes.push_back(kf);
    }

    scene->Destroy();
    manager->Destroy();
    return true;
}


//【全体の流れ】
//
//このファイルは
//** 「FBXファイルからモデル・スキニング・アニメ情報をDirectX用データに変換するローダの実装」** です。
//
//1. コンストラクタ
//FbxModelLoader::FbxModelLoader() {}
//
//
//クラス初期化用。特に初期化処理はありません。
//
//2. Load（静的メッシュ読み込み）
//目的
//
//FBXファイルから「動かない（スキニングしない）静的メッシュ」を読み込む
//
//頂点座標、法線、UVなどを抽出してDirectX用Vertex配列とIndex配列を作成
//
//やっていること
//
//FBXマネージャ・インポータを作成してFBXファイルを読み込む
//
//全てのメッシュを「三角形化」する
//
//どんな多角形モデルも必ず三角形ポリゴンへ変換（DirectXで扱いやすくするため）
//
//各メッシュ内の全頂点（コントロールポイント）の座標(x, y, z)をfloat型でvectorに格納
//
//各ポリゴン（面）ごとに、法線・UV座標情報を付与
//
//頂点が持つ法線・UVが違う場合は複製して新しい頂点として扱う
//
//DirectX用の「Vertex型配列」と「Index配列」を構築
//
//バッファに詰めて返す
//
//ポイント
//
//double型 → float型への型変換が頻出（FBX→DirectXで重要）
//
//DirectXで効率よく描画できる「三角形＋頂点配列＋インデックス配列」形式に変換している
//
//3. IsExistNormalUVInfo
//bool FbxModelLoader::IsExistNormalUVInfo(const std::vector<float>&vertexInfo)
//
//
//頂点情報に法線とUV情報が既に含まれているかどうかを判定する
//
//8要素（x, y, z, nx, ny, nz, u, v）があればtrue
//
//4. CreateVertexInfo
//std::vector<float> FbxModelLoader::CreateVertexInfo(...)
//
//
//頂点の元座標データに法線・UV情報を合成して返す
//
//FBXメッシュの頂点を「x, y, z, nx, ny, nz, u, v」の形にする
//
//5. CreateNewVertexIndex
//int FbxModelLoader::CreateNewVertexIndex(...)
//
//
//同じコントロールポイントでも、法線・UVの組み合わせが違う場合は新しい頂点として追加
//
//これにより、モデルの各三角形の各頂点ごとに「最適な頂点情報」を確保
//
//6. IsSetNormalUV
//bool FbxModelLoader::IsSetNormalUV(...)
//
//
//既存頂点が、指定した法線・UVと同じかどうか（float同士の差が非常に小さければ同じと判定）
//
//7. LoadSkinningModel（スキニング対応モデル読み込み）
//目的
//
//「FBXファイルからスキニングモデル（ボーンで動くキャラ）」＋「アニメーション」まで一気に抽出する
//
//やっていること
//
//FBXマネージャ・インポータ作成＆FBX読み込み
//
//三角形化
//
//ボーン（Mixamoなら"mixamorig:"）の名前を全てリストアップ
//
//バインドポーズ行列（初期姿勢のボーン行列）を抽出
//
//全頂点（コントロールポイント）について、どのボーンに何 % 支配されているか（ボーンインデックス＋ウェイト）を抽出
//
//各面ごとに「位置」「法線」「UV」「ボーン4つ＋ウェイト4つ」情報を持ったSkinningVertex配列を作る
//
//Index配列も構築
//
//アニメーション情報をすべてキーフレームとして抜き出す
//
//すべてのアニメクリップごとに、各フレーム・各ボーンの「グローバル変換行列」を保存
//
//すべてSkinningVertexInfo構造体にまとめて返却
//
//ポイント
//
//スキニング情報やアニメ情報の抽出にはボーン名リストの順番が重要
//
//「ボーンごとの4つまでのインデックス・ウェイト」「各キーフレーム時のボーン姿勢行列」が完全に取得できる
//
//8. LoadAnimationOnly（アニメーションのみ抽出）
//目的
//
//FBXファイルから * *「モデル情報は不要、アニメーションだけを抽出したい」場合に使う * *
//
//やっていること
//
//FBXファイルを読み込む（マネージャ / インポータ / シーン）
//
//ボーン名リストを全て抽出（"mixamorig:"で判定）
//
//最初のアニメーションスタック（クリップ）を走査
//
//全フレーム（frameRate = 30として割り算）について：
//
//各ボーンのグローバル行列を取得してKeyframeに詰める
//
//全フレーム分をKeyframe配列として返す
//
//【全体まとめ】
//
//このクラスは「FBX形式で保存された3Dモデルデータ（メッシュ・ボーン・アニメ）」を「DirectXや独自エンジンで使える形式（頂点配列、ボーン行列、アニメフレーム配列など）」に変換してくれるローダである
//
//静的メッシュ用、スキニング付きキャラ用、アニメーションのみの3通りの抽出が可能
//
//DirectXやゲームエンジンで3Dキャラや背景を正しく表示・動かすために必須の「頂点・ボーン・アニメ」情報を安全かつ正確に変換・格納している
//
//MixamoやBlenderなどで作成・書き出したFBX資産をプログラムで自前で扱えるようにする基盤の役割
//
//面接で聞かれたら「FBXモデル・アニメ・ボーン情報を正しく抽出してDirectXで扱える頂点・アニメ・ボーンデータに変換するためのローダです」と一言で説明すればOKです。