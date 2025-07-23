#include "FbxModelLoader.h"
#include "BufferManager.h"
#include <DirectXMath.h>
#include <algorithm>
#include <map>
#include <tuple>
#include <iostream>   // std::cout, ��������OutputDebugStringA
#include <functional>  // ��������K���ǉ��I
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

    // ======= �S���b�V���擾 =======
    int meshCount = scene->GetSrcObjectCount<FbxMesh>();
    std::vector<Vertex> allVertices;
    std::vector<unsigned short> allIndices;
    unsigned short indexOffset = 0;

    for (int m = 0; m < meshCount; ++m) {
        auto mesh = scene->GetSrcObject<FbxMesh>(m);
        if (!mesh) continue;

        // UV�Z�b�g���̎擾
        FbxStringList uvSetNameList;
        mesh->GetUVSetNames(uvSetNameList);
        if (uvSetNameList.GetCount() == 0) continue;
        const char* uvSetName = uvSetNameList.GetStringAt(0);

        // ���_���W
        std::vector<std::vector<float>> vertexInfoList;
        for (int i = 0; i < mesh->GetControlPointsCount(); i++) {
            auto point = mesh->GetControlPointAt(i);
            std::vector<float> vertex;
            vertex.push_back(point[0]);
            vertex.push_back(point[1]);
            vertex.push_back(point[2]);
            vertexInfoList.push_back(vertex);
        }
        // ���_���̏��
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
        // ���_�f�[�^���\�z���ăI�t�Z�b�g���Z
        std::vector<Vertex> vertices;
        for (int i = 0; i < vertexInfoList.size(); i++) {
            std::vector<float>& vi = vertexInfoList[i];
            vertices.push_back(Vertex{
                vi[0], vi[1], vi[2],
                vi[3], vi[4], vi[5],
                vi[6], 1.0f - vi[7] // �� ������ύX FBX/Blender/Maya�� �� v�͉�����オ0��1 FBX/Blender/Maya�� �� v�͉�����オ0��1
                });


        }
        // �C���f�b�N�X�̓I�t�Z�b�g��t����allIndices��
        for (auto idx : indices) {
            allIndices.push_back(idx + indexOffset);
        }
        // ���_��allVertices��
        allVertices.insert(allVertices.end(), vertices.begin(), vertices.end());
        indexOffset += static_cast<unsigned short>(vertices.size());
    }

    // �}�l�[�W���[�A�V�[���̔j��
    scene->Destroy();
    manager->Destroy();
    *vertexInfo = { allVertices, allIndices };
    return true;
}

// --- �c��͂��̂܂� ---
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
// ...�i���̃C���N���[�h���K�v�ɉ����āj...

//----------------------------
// �X�L�j���O���A�j���t��FBX�̓ǂݍ���
//----------------------------
//bool FbxModelLoader::LoadSkinningModel(const std::string& filePath, SkinningVertexInfo* outInfo)
//{
//    //----------------------------------------
//    // 1. FBX SDK�}�l�[�W���A�C���|�[�^�A�V�[���̐���
//    //----------------------------------------
//    // FbxManager* manager = FbxManager::Create();
//    // FbxImporter* importer = FbxImporter::Create(manager, "");
//    // importer->Initialize(filePath.c_str(), -1, manager->GetIOSettings());
//    // FbxScene* scene = FbxScene::Create(manager, "");
//    // importer->Import(scene);
//    // importer->Destroy();
//
//    //----------------------------------------
//    // 2. ���b�V���̎O�p�`��
//    //----------------------------------------
//    // FbxGeometryConverter geometryConverter(manager);
//    // geometryConverter.Triangulate(scene, true);
//
//    //----------------------------------------
//    // 3. �{�[���K�w/���O/�o�C���h�|�[�Y�s��̒��o
//    //----------------------------------------
//    // - �V�[�����̑S���m�[�h�𑖍����AboneNames, bindPoses��outInfo�Ɋi�[
//
//    //----------------------------------------
//    // 4. �e���_���Ƃ�
//    //    �E���W, �@��, UV
//    //    �E�{�[���C���f�b�N�X, �{�[���E�F�C�g
//    // �𒊏o���ASkinningVertex�z��𐶐�
//    //----------------------------------------
//    // - �eFbxMesh�̃R���g���[���|�C���g����K�v�����擾
//
//    //----------------------------------------
//    // 5. �C���f�b�N�X�z�������
//    //----------------------------------------
//    // - �|���S����񂩂�
//
//    //----------------------------------------
//    // 6. �e�A�j���[�V�����iTake/Clip�j���Ƃ�
//    //    �E�S�L�[�t���[�����ƂɑS�{�[���̕ϊ��s��z��
//    //    �EAnimator::Keyframe�z��Ƃ���outInfo->animations�Ɋi�[
//    //----------------------------------------
//    // - �A�j���[�V�����X�^�b�N�⃌�C���𑖍�
//
//    //----------------------------------------
//    // 7. ���ׂ�outInfo�ɃZ�b�g������AFBX�I�u�W�F�N�g�J��
//    //----------------------------------------
//    // scene->Destroy();
//    // manager->Destroy();
//
//    //----------------------------------------
//    // 8. ������true�A���s����false
//    //----------------------------------------
//    return false; // ���܂��������B�ׂ��������͌�Œi�K�I��
//}

//----------------------------
// �X�L�j���O���A�j���t��FBX�̓ǂݍ���
//----------------------------
bool FbxModelLoader::LoadSkinningModel(const std::string& filePath, SkinningVertexInfo* outInfo)
{
    // --- 1. FBX SDK�}�l�[�W���쐬 ---
    OutputDebugStringA("[FBX] 1. FbxManager�쐬\n");
    FbxManager* manager = FbxManager::Create();
    if (!manager) {
        OutputDebugStringA("[FBX] FbxManager�쐬���s\n");
        return false;
    }

    // --- 2. �C���|�[�^�쐬�������� ---
    OutputDebugStringA("[FBX] 2. FbxImporter�쐬��������\n");
    FbxImporter* importer = FbxImporter::Create(manager, "");
    if (!importer->Initialize(filePath.c_str(), -1, manager->GetIOSettings())) {
        OutputDebugStringA("[FBX] FbxImporter���������s\n");
        return false;
    }

    // --- 3. �V�[���������C���|�[�g ---
    OutputDebugStringA("[FBX] 3. �V�[���������C���|�[�g\n");
    FbxScene* scene = FbxScene::Create(manager, "");
    if (!importer->Import(scene)) {
        OutputDebugStringA("[FBX] �C���|�[�g���s\n");
        importer->Destroy();
        manager->Destroy();
        return false;
    }
    importer->Destroy();

    // --- 4. �O�p�`�� ---
    OutputDebugStringA("[FBX] 4. ���b�V���̎O�p�`��\n");
    FbxGeometryConverter geometryConverter(manager);
    if (!geometryConverter.Triangulate(scene, true)) {
        OutputDebugStringA("[FBX] �O�p�`�����s\n");
        scene->Destroy();
        manager->Destroy();
        return false;
    }

    // --- 5. �ǂݍ��񂾃V�[���̃T�}�����o�́i�m�[�h���Ȃǁj ---
    FbxNode* rootNode = scene->GetRootNode();
    if (!rootNode) {
        OutputDebugStringA("[FBX] ���[�g�m�[�h������܂���\n");
        scene->Destroy();
        manager->Destroy();
        return false;
    }

    char msg[256];
    sprintf_s(msg, "[FBX] RootNode��: %s, �q�m�[�h��: %d\n",
        rootNode->GetName(), rootNode->GetChildCount());
    OutputDebugStringA(msg);

    // --- 6. �Ƃ肠�����S�m�[�h�����ċA�ŏo���Ă݂� ---
 // ��ɐ錾����
    std::function<void(FbxNode*, int)> PrintNodeNames;

    // ���̌�A������
    PrintNodeNames = [&](FbxNode* node, int depth) {
        std::string indent(depth * 2, ' ');
        char nodeMsg[256];
        sprintf_s(nodeMsg, "%s- %s\n", indent.c_str(), node->GetName());
        OutputDebugStringA(nodeMsg);
        for (int i = 0; i < node->GetChildCount(); ++i)
            PrintNodeNames(node->GetChild(i), depth + 1);
        };

    // --- 7. �{�[���m�[�h�̒��o�E���X�g�A�b�v ---
    OutputDebugStringA("[FBX] --- �{�[���m�[�h�ꗗ ---\n");

    // �{�[������̂��߂̊֐��i���Ƃ���Mixamo�Ȃ�m�[�h����"mixamorig:"���܂܂����̂��{�[���j
    auto IsBoneNode = [](FbxNode* node) -> bool {
        std::string name = node->GetName();
        // Mixamo�̃{�[������ "mixamorig:" ����n�܂�
        return name.find("mixamorig:") != std::string::npos;
        };

    // �ċA�I�Ƀ{�[���m�[�h�����o�́�boneNames�ɒǉ�
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
    sprintf_s(msg2, "[FBX] �{�[���� = %zu\n", outInfo->boneNames.size());
    OutputDebugStringA(msg2);


    // --- 8. �e�{�[���̃o�C���h�|�[�Y�s��擾 ---
    OutputDebugStringA("[FBX] --- �o�C���h�|�[�Y�s��i�����p���j�擾 ---\n");

    outInfo->bindPoses.clear();
    auto* pose = scene->GetPose(0);
    if (pose && pose->IsBindPose()) {
        OutputDebugStringA("[FBX] BindPose����s��擾\n");
        for (const std::string& boneName : outInfo->boneNames) {
            FbxNode* boneNode = scene->FindNodeByName(boneName.c_str());
            if (!boneNode) {
                char err[128];
                sprintf_s(err, "[FBX] �{�[���m�[�h��������Ȃ�: %s\n", boneName.c_str());
                OutputDebugStringA(err);
                outInfo->bindPoses.push_back(DirectX::XMMatrixIdentity());
                continue;
            }
            // Pose�z������[�v���Ĉ�v����m�[�h��T��
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
                sprintf_s(err, "[FBX] BindPose��������Ȃ�: %s\n", boneName.c_str());
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
        OutputDebugStringA("[FBX] BindPose��������Ȃ��IEvaluateGlobalTransform�ő�p\n");
        for (const std::string& boneName : outInfo->boneNames) {
            FbxNode* boneNode = scene->FindNodeByName(boneName.c_str());
            if (!boneNode) {
                char err[128];
                sprintf_s(err, "[FBX] �{�[���m�[�h��������Ȃ�: %s\n", boneName.c_str());
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


    // --- 9. �e���_���ƂɃ{�[���C���f�b�N�X���E�F�C�g���i�[ ---
    OutputDebugStringA("[FBX] --- �X�L�j���O���_��񒊏o ---\n");

    outInfo->vertices.clear();
    outInfo->indices.clear();

    // �S���b�V������
    int meshCount = scene->GetSrcObjectCount<FbxMesh>();
    unsigned short indexOffset = 0;
    for (int m = 0; m < meshCount; ++m) {
        auto mesh = scene->GetSrcObject<FbxMesh>(m);
        if (!mesh) continue;

        // --- �R���g���[���|�C���g�i���_�j���ƂɃ{�[���C���f�b�N�X�ƃE�F�C�g��z�� ---
        // �{�[���C���f�b�N�X: boneNames�z��̒��ň�v�����炻�̃C���f�b�N�X
        int controlPointCount = mesh->GetControlPointsCount();

        // �e�R���g���[���|�C���g�ɑ΂��čő�4�̃C���f�b�N�X�E�E�F�C�g�����蓖��
        struct BoneWeight {
            int indices[4] = { 0,0,0,0 };
            float weights[4] = { 0,0,0,0 };
        };
        std::vector<BoneWeight> boneWeights(controlPointCount);

        // �f�t�H�[�����i�X�L���j�𒊏o
        int skinCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
        for (int s = 0; s < skinCount; ++s) {
            FbxSkin* skin = (FbxSkin*)mesh->GetDeformer(s, FbxDeformer::eSkin);
            int clusterCount = skin->GetClusterCount();
            for (int c = 0; c < clusterCount; ++c) {
                FbxCluster* cluster = skin->GetCluster(c);
                std::string boneName = cluster->GetLink()->GetName();
                // boneNames���C���f�b�N�X�擾
                auto it = std::find(outInfo->boneNames.begin(), outInfo->boneNames.end(), boneName);
                int boneIdx = (it != outInfo->boneNames.end()) ? std::distance(outInfo->boneNames.begin(), it) : -1;
                if (boneIdx < 0) continue;

                int* indices = cluster->GetControlPointIndices();
                double* weights = cluster->GetControlPointWeights();
                int indexCount = cluster->GetControlPointIndicesCount();
                for (int i = 0; i < indexCount; ++i) {
                    int ctrlIdx = indices[i];
                    double weight = weights[i];
                    // �ő�4�܂ŋl�߂�i����4���܂��Ă����疳���j
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

        // ���_�z��𐶐�
        // FBX�̒��_�́u�R���g���[���|�C���g�{�@���EUV���Ƃɕ����v����邽�߁A�|���S���P�ʂŏ���
        for (int polIndex = 0; polIndex < mesh->GetPolygonCount(); polIndex++) {
            for (int polVertexIndex = 0; polVertexIndex < mesh->GetPolygonSize(polIndex); polVertexIndex++) {
                int ctrlIdx = mesh->GetPolygonVertex(polIndex, polVertexIndex);

                // --- �ʒu ---
                auto point = mesh->GetControlPointAt(ctrlIdx);

                // --- �@�� ---
                FbxVector4 normalVec4;
                mesh->GetPolygonVertexNormal(polIndex, polVertexIndex, normalVec4);

                // --- UV ---
                FbxStringList uvSetNameList;
                mesh->GetUVSetNames(uvSetNameList);
                const char* uvSetName = (uvSetNameList.GetCount() > 0) ? uvSetNameList.GetStringAt(0) : "";
                FbxVector2 uvVec2(0, 0);
                bool isUnMapped = false;
                mesh->GetPolygonVertexUV(polIndex, polVertexIndex, uvSetName, uvVec2, isUnMapped);

                // --- �X�L�j���O��� ---
                BoneWeight& bw = boneWeights[ctrlIdx];

                // --- ���_�f�[�^��SkinningVertex�^�ō\�z ---
                SkinningVertex v;
                v.x = (float)point[0];
                v.y = (float)point[1];
                v.z = (float)point[2];
                v.nx = (float)normalVec4[0];
                v.ny = (float)normalVec4[1];
                v.nz = (float)normalVec4[2];
                v.u = (float)uvVec2[0];
                v.v = 1.0f - (float)uvVec2[1]; // v���W�͏㉺���]
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


    // --- 10. �A�j���[�V�������i�L�[�t���[���j�̎擾 ---
    OutputDebugStringA("[FBX] --- �A�j���[�V������񒊏o ---\n");

    outInfo->animations.clear();

    int animStackCount = scene->GetSrcObjectCount<FbxAnimStack>();
    for (int stackIdx = 0; stackIdx < animStackCount; ++stackIdx) {
        FbxAnimStack* animStack = scene->GetSrcObject<FbxAnimStack>(stackIdx);
        std::string animName = animStack->GetName();
        OutputDebugStringA(("[FBX] �A�j����: " + animName + "\n").c_str());

        // �A�j���[�V�������C�����擾�i�ʏ�1��OK�j
        FbxAnimLayer* animLayer = animStack->GetMember<FbxAnimLayer>(0);
        if (!animLayer) continue;

        // �J�n�`�I�����Ԃ��擾
        FbxTimeSpan timeSpan = animStack->GetLocalTimeSpan();
        FbxTime start = timeSpan.GetStart();
        FbxTime end = timeSpan.GetStop();

        double startSec = start.GetSecondDouble();
        double endSec = end.GetSecondDouble();
        double frameRate = 30.0; // ����30FPS�iMixamo�͂قڂ��̒l�j
        int numFrames = int((endSec - startSec) * frameRate) + 1;

        OutputDebugStringA(("[FBX] �t���[����: " + std::to_string(numFrames) + "\n").c_str());

        // �L�[�t���[���z��
        std::vector<Animator::Keyframe> keyframes;

        // �S�t���[����
        for (int f = 0; f < numFrames; ++f) {
            double sec = startSec + f / frameRate;
            FbxTime t;
            t.SetSecondDouble(sec);

            // �e�{�[���̍s��
            std::vector<DirectX::XMMATRIX> framePoses;
            for (const std::string& boneName : outInfo->boneNames) {
                FbxNode* boneNode = scene->FindNodeByName(boneName.c_str());
                if (!boneNode) {
                    framePoses.push_back(DirectX::XMMatrixIdentity());
                    continue;
                }
                // �A�j�������ł̃O���[�o���ϊ����擾
                FbxAMatrix mat = boneNode->EvaluateGlobalTransform(t);
                DirectX::XMMATRIX dxMat = DirectX::XMMatrixIdentity();
                for (int r = 0; r < 4; ++r)
                    for (int c = 0; c < 4; ++c)
                        dxMat.r[r].m128_f32[c] = static_cast<float>(mat.Get(r, c));
                framePoses.push_back(dxMat);
            }
            // �L�[�t���[���o�^
            Animator::Keyframe kf;
            kf.time = sec;
            kf.pose = framePoses;
            keyframes.push_back(kf);
        }

        // Animation�\���̂ɂ܂Ƃ߂ēo�^
        FbxModelLoader::SkinningVertexInfo::Animation anim;
        anim.name = animName;
        anim.length = endSec - startSec;
        anim.keyframes = keyframes;
        outInfo->animations.push_back(anim);
    }



    // --- ����͂����܂� ---
    OutputDebugStringA("[FBX] === FBX���[�h�����i�K���� ===\n");

    // ���
    scene->Destroy();
    manager->Destroy();

    return true; // �܂��́u�t�@�C���ǂ߂ăm�[�h�ꗗ��print�ł���v���Ƃ�ڕW�ɁI
}


