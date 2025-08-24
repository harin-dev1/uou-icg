#pragma once

#include "Vector.h"

struct Light {
    Vec3f position;
    Vec3f intensity_ambient;
    Vec3f intensity_diffuse;
    Vec3f diffuse_color;
    Vec3f specular_color;
};