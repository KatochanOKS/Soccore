#pragma once
#include <fbxsdk.h>
#include <vector>
#include <array>
#include <string>
#pragma comment(lib, "libfbxsdk-md.lib")
#pragma comment(lib, "libxml2-md.lib")
#pragma comment(lib, "zlib-md.lib")
#include "BufferManager.h" // BufferManager�̃w�b�_�[�t�@�C�����C���N���[�h
#include <DirectXMath.h>

// --- FBX�L���b�V���p�\���� ---
// ����́uFBX���f����1�񂾂����[�h���A�K�v�ȏ��������ƕێ����邽�߂̔��v�ł�
struct FbxModelInstance {
	FbxManager* manager = nullptr;            // FBX SDK �Ǘ��C���X�^���X
	FbxScene* scene = nullptr;                // FBX�V�[���f�[�^
	std::vector<std::string> boneNames;       // �{�[�������X�g
	std::vector<DirectX::XMMATRIX> bindPoses; // �{�[���̃o�C���h�|�[�Y�s��
	double animationLength = 0.0;             // �A�j���[�V�����̑��Đ����ԁi�b�j

	// --- �f�X�g���N�^��FBX���\�[�X�̉���i�K�{!!�j---
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
		std::vector<Vertex> vertices; // �� ������ BufferManager��Vertex�^���g��
		std::vector<unsigned short> indices;
	};

	struct SkinningVertexInfo {
		std::vector<SkinningVertex> vertices;
		std::vector<uint16_t> indices;
		std::vector<std::string> boneNames;
		std::vector<DirectX::XMMATRIX> bindPoses;
	};

	// FBX�̃A�j���[�V�����f�[�^���L���b�V������\����
	struct FbxModelInstance {
		FbxManager* manager = nullptr;            // FBX�Ǘ��N���X
		FbxScene* scene = nullptr;                // FBX�V�[��
		std::vector<std::string> boneNames;       // �{�[�������X�g
		std::vector<DirectX::XMMATRIX> bindPoses; // �{�[�����Ƃ̃o�C���h�|�[�Y
		double animationLength = 0.0;             // �A�j���[�V�����̒����i�b�j
	};

	static bool Load(const std::string& filePath, VertexInfo* vertexInfo);

	static bool LoadSkinningModel(
		const std::string& filePath,
		SkinningVertexInfo* outInfo
	);

	// ��FBX�����[�h���ăL���b�V���\���̂��쐬�i���񂾂��j
	static FbxModelInstance* LoadAndCache(const std::string& filePath);

	// ���L���b�V���ς݃C���X�^���X����A�j���[�V���������Ń{�[���s����v�Z
	static void CalcCurrentBoneMatrices(
		FbxModelInstance* instance,             // �L���b�V���C���X�^���X
		double currentTime,                     // �Đ������������i�b�j
		std::vector<DirectX::XMMATRIX>& outMatrices // �v�Z���ʊi�[��
	);

private:
	static bool IsExistNormalUVInfo(const std::vector<float>& vertexInfo);
	static std::vector<float> CreateVertexInfo(const std::vector<float>& vertex, const FbxVector4& normalVec4, const FbxVector2& uvVec2);
	static int CreateNewVertexIndex(const std::vector<float>& vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2,
		std::vector<std::vector<float>>& vertexInfoList, int oldIndex, std::vector<std::array<int, 2>>& oldNewIndexPairList);
	static bool IsSetNormalUV(const std::vector<float> vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2);
};
