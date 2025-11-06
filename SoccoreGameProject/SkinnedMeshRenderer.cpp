#include "SkinnedMeshRenderer.h"

SkinnedMeshRenderer::~SkinnedMeshRenderer() {
    if (m_ModelBuffer) delete m_ModelBuffer;
    if (m_SkinVertexInfo) delete m_SkinVertexInfo;
	if (m_BoneCB) delete m_BoneCB;
    // animator‚ÍGameObject‚Ìcomponents‚ÅˆêŠ‡delete‚³‚ê‚é‚Ì‚Å‚±‚±‚Å‚Ídelete‚µ‚È‚¢I
}

void SkinnedMeshRenderer::Draw() {
    // •`‰æ‚ÍRendererƒNƒ‰ƒX‚ÉˆÏ÷‚·‚é‚Ì‚Å‚±‚±‚Å‚Í‰½‚à‚µ‚È‚¢
}
