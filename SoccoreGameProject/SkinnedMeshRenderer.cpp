#include "SkinnedMeshRenderer.h"

SkinnedMeshRenderer::~SkinnedMeshRenderer() {
    if (modelBuffer) delete modelBuffer;
    if (skinVertexInfo) delete skinVertexInfo;
	if (boneCB) delete boneCB;
    // animator��GameObject��components�ňꊇdelete�����̂ł����ł�delete���Ȃ��I
}

void SkinnedMeshRenderer::Draw() {
    // �K�v�Ȃ�u�����p�̓���ȏ����v�������ɏ���
}
