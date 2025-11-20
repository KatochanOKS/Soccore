#include "pch.h"
#include "StaticMeshRenderer.h"

StaticMeshRenderer::~StaticMeshRenderer() {
    // FBXモデルなど個別newしたときだけdelete
    if (m_VertexInfo) delete m_VertexInfo;
    // modelBufferもFBXモデル等のnew BufferManager時だけdelete
    // Cube/Sphere/QuadなどEngineManagerのものはdeleteしない
    if (m_ModelBuffer && m_VertexInfo) delete m_ModelBuffer;
}

void StaticMeshRenderer::Draw() {
    // 描画はRendererクラスに委譲するのでここでは何もしない
}
