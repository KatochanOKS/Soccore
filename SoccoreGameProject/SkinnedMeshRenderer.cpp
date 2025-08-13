#include "SkinnedMeshRenderer.h"

SkinnedMeshRenderer::~SkinnedMeshRenderer() {
    if (modelBuffer) delete modelBuffer;
    if (skinVertexInfo) delete skinVertexInfo;
    // animatorはGameObjectのcomponentsで一括deleteされるのでここではdeleteしない！
}

void SkinnedMeshRenderer::Draw() {
    // 必要なら「自分用の特殊な処理」をここに書く
}
