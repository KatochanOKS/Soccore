// MeshLibrary.h
#pragma once
#include <vector>
#include "BufferManager.h"

namespace MeshLibrary {
	void GetCubeMesh(std::vector<Vertex>& vertices, std::vector<uint16_t>& indices);
	void GetQuadMesh2D(std::vector<Vertex>& vertices, std::vector<uint16_t>& indices);
	// ★追加：スカイドーム用
	void GetSphereMesh(std::vector<Vertex>& vertices, std::vector<uint16_t>& indices, int latDiv = 32, int lonDiv = 64);

	void GetSphereMesh(std::vector<Vertex>& outVertices, std::vector<uint16_t>& outIndices, float radius = 1.0f, int slice = 20, int stack = 20);

	// ★追加：円柱メッシュ生成
	void GetCylinderMesh(std::vector<Vertex>& vertices, std::vector<uint16_t>& indices, int slice = 32);
}
