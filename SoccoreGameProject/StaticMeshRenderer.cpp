#include "StaticMeshRenderer.h"

StaticMeshRenderer::~StaticMeshRenderer() {
    if (modelBuffer) delete modelBuffer;
    if (vertexInfo) delete vertexInfo;
}

// 描画処理は後ほどRenderer側で呼ぶのでここは空でもOK（仮で何か書いても良い）
void StaticMeshRenderer::Draw() {
    // 必要なら「自分用の特殊な処理」をここに書く
    // 普通はRenderer.cppのほうで描画
}
