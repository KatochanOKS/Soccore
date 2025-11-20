#include "pch.h"
#include "SkinnedMeshRenderer.h"

SkinnedMeshRenderer::~SkinnedMeshRenderer() {
    if (m_ModelBuffer) delete m_ModelBuffer;
    if (m_SkinVertexInfo) delete m_SkinVertexInfo;
	if (m_BoneCB) delete m_BoneCB;
    // animatorはGameObjectのcomponentsで一括deleteされるのでここではdeleteしない！
}

void SkinnedMeshRenderer::Draw() {
    // 描画はRendererクラスに委譲するのでここでは何もしない
}
