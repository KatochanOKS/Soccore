#include "StaticMeshRenderer.h"

StaticMeshRenderer::~StaticMeshRenderer() {
    // FBX���f���Ȃǌ�new�����Ƃ�����delete
    if (vertexInfo) delete vertexInfo;
    // modelBuffer��FBX���f������new BufferManager������delete
    // Cube/Sphere/Quad�Ȃ�EngineManager�̂��̂�delete���Ȃ�
    if (modelBuffer && vertexInfo) delete modelBuffer;
}


// �`�揈���͌�ق�Renderer���ŌĂԂ̂ł����͋�ł�OK�i���ŉ��������Ă��ǂ��j
void StaticMeshRenderer::Draw() {
    // �K�v�Ȃ�u�����p�̓���ȏ����v�������ɏ���
    // ���ʂ�Renderer.cpp�̂ق��ŕ`��
}
