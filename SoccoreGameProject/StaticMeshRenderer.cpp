#include "StaticMeshRenderer.h"

StaticMeshRenderer::~StaticMeshRenderer() {
    if (modelBuffer) delete modelBuffer;
    if (vertexInfo) delete vertexInfo;
}

// �`�揈���͌�ق�Renderer���ŌĂԂ̂ł����͋�ł�OK�i���ŉ��������Ă��ǂ��j
void StaticMeshRenderer::Draw() {
    // �K�v�Ȃ�u�����p�̓���ȏ����v�������ɏ���
    // ���ʂ�Renderer.cpp�̂ق��ŕ`��
}
