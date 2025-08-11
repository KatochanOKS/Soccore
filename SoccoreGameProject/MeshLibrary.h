// MeshLibrary.h
#pragma once
#include <vector>
#include "BufferManager.h"

namespace MeshLibrary {
    void GetCubeMesh(std::vector<Vertex>& vertices, std::vector<uint16_t>& indices);
	void GetQuadMesh2D(std::vector<Vertex>& vertices, std::vector<uint16_t>& indices);
	// ★追加：スカイドーム用
	void GetSphereMesh(std::vector<Vertex>& vertices, std::vector<uint16_t>& indices, int latDiv = 32, int lonDiv = 64);
}
