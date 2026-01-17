#ifndef MATERIAL_H
#define MATERIAL_H

#include <GL/glut.h>

#include <cmath>

#include "Vec3.h"
#include "imageLoader.h"

enum MaterialType {
    Material_Diffuse_Blinn_Phong,
    Material_Glass,
    Material_Mirror,
    Material_Texture
};

struct Material {
    Vec3 ambient_material;
    Vec3 diffuse_material;
    Vec3 specular_material;
    double shininess;

    float index_medium;
    float transparency;

    MaterialType type;

    Material() {
        type = Material_Diffuse_Blinn_Phong;
        transparency = 0.0;
        index_medium = 1.0;
        ambient_material = Vec3(0.0);
    }
};

#endif  // MATERIAL_H
