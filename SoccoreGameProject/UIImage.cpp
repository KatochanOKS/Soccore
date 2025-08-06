// UIImage.cpp
#include "UIImage.h"
#include "Renderer.h"

void UIImage::Draw(Renderer* renderer, size_t idx) {
    renderer->DrawUIImage(this, idx);
}
