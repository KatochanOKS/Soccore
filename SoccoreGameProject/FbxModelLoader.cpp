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
// �R���X�g���N�^
//------------------------------------------------------------
FbxModelLoader::FbxModelLoader() {}


//------------------------------------------------------------
// �ÓI���b�V���̓ǂݍ���
//------------------------------------------------------------
bool FbxModelLoader::Load(const std::string& filePath, VertexInfo* vertexInfo)
{
    // FBX�}�l�[�W���E�C���|�[�^�쐬
    auto manager = FbxManager::Create();
    auto importer = FbxImporter::Create(manager, "");
    if (!importer->Initialize(filePath.c_str(), -1, manager->GetIOSettings()))
        return false;
    auto scene = FbxScene::Create(manager, "");
    importer->Import(scene);
    importer->Destroy();

    // ���b�V���̎O�p�`��
    FbxGeometryConverter geometryConverter(manager);
    if (!geometryConverter.Triangulate(scene, true))
        return false;

    // �S���b�V������
    int meshCount = scene->GetSrcObjectCount<FbxMesh>();
    std::vector<Vertex> allVertices;
    std::vector<unsigned short> allIndices;
    unsigned short indexOffset = 0;

    for (int m = 0; m < meshCount; ++m) {
        auto mesh = scene->GetSrcObject<FbxMesh>(m);
        if (!mesh) continue;

        // UV�Z�b�g���擾
        FbxStringList uvSetNameList;
        mesh->GetUVSetNames(uvSetNameList);
        if (uvSetNameList.GetCount() == 0) continue;
        const char* uvSetName = uvSetNameList.GetStringAt(0);

        // ���_���W���X�g
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

        // ���_���Ƃɖ@���EUV���t�^
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

        // ���_�f�[�^�\�z
        std::vector<Vertex> vertices;
        for (int i = 0; i < vertexInfoList.size(); i++) {
            std::vector<float>& vi = vertexInfoList[i];
            vertices.push_back(Vertex{
                vi[0], vi[1], vi[2],
                vi[3], vi[4], vi[5],
                vi[6], 1.0f - vi[7] // v�͏㉺���]
                });
        }
        // �C���f�b�N�X�E���_�z��i�[
        for (auto idx : indices) {
            allIndices.push_back(idx + indexOffset);
        }
        allVertices.insert(allVertices.end(), vertices.begin(), vertices.end());
        indexOffset += static_cast<unsigned short>(vertices.size());
    }

    // ���������
    scene->Destroy();
    manager->Destroy();
    *vertexInfo = { allVertices, allIndices };
    return true;
}

//------------------------------------------------------------
// ���_���@���EUV�������Ă��邩
//------------------------------------------------------------
bool FbxModelLoader::IsExistNormalUVInfo(const std::vector<float>& vertexInfo)
{
    return vertexInfo.size() == 8;
}

//------------------------------------------------------------
// ���_���ɖ@���EUV�ǉ�
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
// �V���Ȗ@���EUV�������_��V�K�ǉ�
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
// �������_���w��@���EUV�ƈ�v���邩
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
// �X�L�j���O�Ή����f���̓ǂݍ���
//------------------------------------------------------------
bool FbxModelLoader::LoadSkinningModel(const std::string& filePath, SkinningVertexInfo* outInfo)
{
    // --- FBX�}�l�[�W�� ---
    OutputDebugStringA("[FBX] 1. FbxManager�쐬\n");
    FbxManager* manager = FbxManager::Create();
    if (!manager) { OutputDebugStringA("[FBX] FbxManager�쐬���s\n"); return false; }

    // --- �C���|�[�^ ---
    OutputDebugStringA("[FBX] 2. FbxImporter�쐬��������\n");
    FbxImporter* importer = FbxImporter::Create(manager, "");
    if (!importer->Initialize(filePath.c_str(), -1, manager->GetIOSettings())) {
        OutputDebugStringA("[FBX] FbxImporter���������s\n"); return false;
    }

    // --- �V�[�� ---
    OutputDebugStringA("[FBX] 3. �V�[���������C���|�[�g\n");
    FbxScene* scene = FbxScene::Create(manager, "");
    if (!importer->Import(scene)) {
        OutputDebugStringA("[FBX] �C���|�[�g���s\n");
        importer->Destroy(); manager->Destroy(); return false;
    }
    importer->Destroy();

    // --- �O�p�`�� ---
    OutputDebugStringA("[FBX] 4. ���b�V���̎O�p�`��\n");
    FbxGeometryConverter geometryConverter(manager);
    if (!geometryConverter.Triangulate(scene, true)) {
        OutputDebugStringA("[FBX] �O�p�`�����s\n"); scene->Destroy(); manager->Destroy(); return false;
    }

    // --- ���[�g�m�[�h ---
    FbxNode* rootNode = scene->GetRootNode();
    if (!rootNode) {
        OutputDebugStringA("[FBX] ���[�g�m�[�h������܂���\n");
        scene->Destroy(); manager->Destroy(); return false;
    }

    // --- �{�[�������o ---
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

    // --- �o�C���h�|�[�Y�擾 ---
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

    // --- �X�L�j���O���_�E�C���f�b�N�X���o ---
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

    // --- �A�j���[�V�������i�L�[�t���[���j���o ---
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

    OutputDebugStringA("[FBX] === FBX���[�h�����i�K���� ===\n");

    scene->Destroy();
    manager->Destroy();
    return true;
}

//------------------------------------------------------------
// �A�j���[�V�����������o
//------------------------------------------------------------
bool FbxModelLoader::LoadAnimationOnly(
    const std::string& fbxPath,
    std::vector<Animator::Keyframe>& outKeyframes,
    double& outLength
) {
    FbxManager* manager = FbxManager::Create();
    FbxImporter* importer = FbxImporter::Create(manager, "");
    if (!importer->Initialize(fbxPath.c_str(), -1, manager->GetIOSettings())) {
        OutputDebugStringA("[FBX] LoadAnimationOnly: ���������s\n");
        return false;
    }
    FbxScene* scene = FbxScene::Create(manager, "");
    if (!importer->Import(scene)) {
        OutputDebugStringA("[FBX] LoadAnimationOnly: �C���|�[�g���s\n");
        importer->Destroy(); manager->Destroy(); return false;
    }
    importer->Destroy();

    // �{�[�������o
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
        OutputDebugStringA("[FBX] �A�j���[�V�����Ȃ�\n");
        scene->Destroy(); manager->Destroy(); return false;
    }
    FbxAnimStack* animStack = scene->GetSrcObject<FbxAnimStack>(0);
    FbxAnimLayer* animLayer = animStack->GetMember<FbxAnimLayer>(0);
    if (!animLayer) {
        OutputDebugStringA("[FBX] �A�j���[�V�������C���擾���s\n");
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


//�y�S�̗̂���z
//
//���̃t�@�C����
//** �uFBX�t�@�C�����烂�f���E�X�L�j���O�E�A�j������DirectX�p�f�[�^�ɕϊ����郍�[�_�̎����v** �ł��B
//
//1. �R���X�g���N�^
//FbxModelLoader::FbxModelLoader() {}
//
//
//�N���X�������p�B���ɏ����������͂���܂���B
//
//2. Load�i�ÓI���b�V���ǂݍ��݁j
//�ړI
//
//FBX�t�@�C������u�����Ȃ��i�X�L�j���O���Ȃ��j�ÓI���b�V���v��ǂݍ���
//
//���_���W�A�@���AUV�Ȃǂ𒊏o����DirectX�pVertex�z���Index�z����쐬
//
//����Ă��邱��
//
//FBX�}�l�[�W���E�C���|�[�^���쐬����FBX�t�@�C����ǂݍ���
//
//�S�Ẵ��b�V�����u�O�p�`���v����
//
//�ǂ�ȑ��p�`���f�����K���O�p�`�|���S���֕ϊ��iDirectX�ň����₷�����邽�߁j
//
//�e���b�V�����̑S���_�i�R���g���[���|�C���g�j�̍��W(x, y, z)��float�^��vector�Ɋi�[
//
//�e�|���S���i�ʁj���ƂɁA�@���EUV���W����t�^
//
//���_�����@���EUV���Ⴄ�ꍇ�͕������ĐV�������_�Ƃ��Ĉ���
//
//DirectX�p�́uVertex�^�z��v�ƁuIndex�z��v���\�z
//
//�o�b�t�@�ɋl�߂ĕԂ�
//
//�|�C���g
//
//double�^ �� float�^�ւ̌^�ϊ����p�o�iFBX��DirectX�ŏd�v�j
//
//DirectX�Ō����悭�`��ł���u�O�p�`�{���_�z��{�C���f�b�N�X�z��v�`���ɕϊ����Ă���
//
//3. IsExistNormalUVInfo
//bool FbxModelLoader::IsExistNormalUVInfo(const std::vector<float>&vertexInfo)
//
//
//���_���ɖ@����UV��񂪊��Ɋ܂܂�Ă��邩�ǂ����𔻒肷��
//
//8�v�f�ix, y, z, nx, ny, nz, u, v�j�������true
//
//4. CreateVertexInfo
//std::vector<float> FbxModelLoader::CreateVertexInfo(...)
//
//
//���_�̌����W�f�[�^�ɖ@���EUV�����������ĕԂ�
//
//FBX���b�V���̒��_���ux, y, z, nx, ny, nz, u, v�v�̌`�ɂ���
//
//5. CreateNewVertexIndex
//int FbxModelLoader::CreateNewVertexIndex(...)
//
//
//�����R���g���[���|�C���g�ł��A�@���EUV�̑g�ݍ��킹���Ⴄ�ꍇ�͐V�������_�Ƃ��Ēǉ�
//
//����ɂ��A���f���̊e�O�p�`�̊e���_���ƂɁu�œK�Ȓ��_���v���m��
//
//6. IsSetNormalUV
//bool FbxModelLoader::IsSetNormalUV(...)
//
//
//�������_���A�w�肵���@���EUV�Ɠ������ǂ����ifloat���m�̍������ɏ�������Γ����Ɣ���j
//
//7. LoadSkinningModel�i�X�L�j���O�Ή����f���ǂݍ��݁j
//�ړI
//
//�uFBX�t�@�C������X�L�j���O���f���i�{�[���œ����L�����j�v�{�u�A�j���[�V�����v�܂ň�C�ɒ��o����
//
//����Ă��邱��
//
//FBX�}�l�[�W���E�C���|�[�^�쐬��FBX�ǂݍ���
//
//�O�p�`��
//
//�{�[���iMixamo�Ȃ�"mixamorig:"�j�̖��O��S�ă��X�g�A�b�v
//
//�o�C���h�|�[�Y�s��i�����p���̃{�[���s��j�𒊏o
//
//�S���_�i�R���g���[���|�C���g�j�ɂ��āA�ǂ̃{�[���ɉ� % �x�z����Ă��邩�i�{�[���C���f�b�N�X�{�E�F�C�g�j�𒊏o
//
//�e�ʂ��ƂɁu�ʒu�v�u�@���v�uUV�v�u�{�[��4�{�E�F�C�g4�v����������SkinningVertex�z������
//
//Index�z����\�z
//
//�A�j���[�V�����������ׂăL�[�t���[���Ƃ��Ĕ����o��
//
//���ׂẴA�j���N���b�v���ƂɁA�e�t���[���E�e�{�[���́u�O���[�o���ϊ��s��v��ۑ�
//
//���ׂ�SkinningVertexInfo�\���̂ɂ܂Ƃ߂ĕԋp
//
//�|�C���g
//
//�X�L�j���O����A�j�����̒��o�ɂ̓{�[�������X�g�̏��Ԃ��d�v
//
//�u�{�[�����Ƃ�4�܂ł̃C���f�b�N�X�E�E�F�C�g�v�u�e�L�[�t���[�����̃{�[���p���s��v�����S�Ɏ擾�ł���
//
//8. LoadAnimationOnly�i�A�j���[�V�����̂ݒ��o�j
//�ړI
//
//FBX�t�@�C������ * *�u���f�����͕s�v�A�A�j���[�V���������𒊏o�������v�ꍇ�Ɏg�� * *
//
//����Ă��邱��
//
//FBX�t�@�C����ǂݍ��ށi�}�l�[�W�� / �C���|�[�^ / �V�[���j
//
//�{�[�������X�g��S�Ē��o�i"mixamorig:"�Ŕ���j
//
//�ŏ��̃A�j���[�V�����X�^�b�N�i�N���b�v�j�𑖍�
//
//�S�t���[���iframeRate = 30�Ƃ��Ċ���Z�j�ɂ��āF
//
//�e�{�[���̃O���[�o���s����擾����Keyframe�ɋl�߂�
//
//�S�t���[������Keyframe�z��Ƃ��ĕԂ�
//
//�y�S�̂܂Ƃ߁z
//
//���̃N���X�́uFBX�`���ŕۑ����ꂽ3D���f���f�[�^�i���b�V���E�{�[���E�A�j���j�v���uDirectX��Ǝ��G���W���Ŏg����`���i���_�z��A�{�[���s��A�A�j���t���[���z��Ȃǁj�v�ɕϊ����Ă���郍�[�_�ł���
//
//�ÓI���b�V���p�A�X�L�j���O�t���L�����p�A�A�j���[�V�����݂̂�3�ʂ�̒��o���\
//
//DirectX��Q�[���G���W����3D�L������w�i�𐳂����\���E���������߂ɕK�{�́u���_�E�{�[���E�A�j���v�������S�����m�ɕϊ��E�i�[���Ă���
//
//Mixamo��Blender�Ȃǂō쐬�E�����o����FBX���Y���v���O�����Ŏ��O�ň�����悤�ɂ����Ղ̖���
//
//�ʐڂŕ����ꂽ��uFBX���f���E�A�j���E�{�[�����𐳂������o����DirectX�ň����钸�_�E�A�j���E�{�[���f�[�^�ɕϊ����邽�߂̃��[�_�ł��v�ƈꌾ�Ő��������OK�ł��B