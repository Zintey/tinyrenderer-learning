#pragma once
#include "tgaimage.h"
#include "geometry.h"

struct Light {
    TGAColor color;
    double intensity;
    Transform transform;
    Light(TGAColor color_, double intensity_, Transform transform_)
        : color(color_), intensity(intensity_), transform(transform_) {}
};