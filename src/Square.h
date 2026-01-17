#ifndef SQUARE_H
#define SQUARE_H

#include <cmath>
#include <vector>

#include "Mesh.h"
#include "Vec3.h"
#include "imageLoader.h"

using namespace ppmLoader;

struct RaySquareIntersection {
    bool intersectionExists;
    float t;
    float u, v;
    Vec3 intersection;
    Vec3 normal;
    int pixel_x = 0;
    int pixel_y = 0;
};

enum SquareTextureWrapMode {
    SquareTexture_Simple,
    SquareTexture_Cover,
    SquareTexture_Mirror,
    SquareTexture_Repeat,
    SquareTexture_Stretch,
    SquareTexture_Clamp,
};

struct TextureMap {
    ImageRGB image;
    RGB defaultColor = {255, 255, 255};
    SquareTextureWrapMode wrapMode = SquareTexture_Simple;
    ImageMirrorMode mirrorMode = ImageMirror_None;
};

class Square : public Mesh {
   public:
    Vec3 m_normal;
    Vec3 m_bottom_left;
    Vec3 m_right_vector;
    Vec3 m_up_vector;
    // Tiling/offset controls for texture mapping
    float tileU = 1.0f;  // number of repeats along right vector
    float tileV = 1.0f;  // number of repeats along up vector
    float offsetU = 0.0f;
    float offsetV = 0.0f;
    bool hasTexture = false;
    TextureMap textureMap;

    Square() : Mesh() {}
    Square(Vec3 const& bottomLeft, Vec3 const& rightVector, Vec3 const& upVector, float width = 1., float height = 1.,
           float uMin = 0.f, float uMax = 1.f, float vMin = 0.f, float vMax = 1.f) : Mesh() {
        setQuad(bottomLeft, rightVector, upVector, width, height, uMin, uMax, vMin, vMax);
    }

    void setQuad(Vec3 const& bottomLeft, Vec3 const& rightVector, Vec3 const& upVector, float width = 1., float height = 1.,
                 float uMin = 0.f, float uMax = 1.f, float vMin = 0.f, float vMax = 1.f) {
        m_right_vector = rightVector;
        m_up_vector = upVector;
        m_normal = Vec3::cross(rightVector, upVector);
        m_bottom_left = bottomLeft;

        m_normal.normalize();
        m_right_vector.normalize();
        m_up_vector.normalize();

        m_right_vector = m_right_vector * width;
        m_up_vector = m_up_vector * height;

        vertices.clear();
        vertices.resize(4);
        vertices[0].position = bottomLeft;
        vertices[0].u = uMin;
        vertices[0].v = vMin;
        vertices[1].position = bottomLeft + m_right_vector;
        vertices[1].u = uMax;
        vertices[1].v = vMin;
        vertices[2].position = bottomLeft + m_right_vector + m_up_vector;
        vertices[2].u = uMax;
        vertices[2].v = vMax;
        vertices[3].position = bottomLeft + m_up_vector;
        vertices[3].u = uMin;
        vertices[3].v = vMax;
        vertices[0].normal = vertices[1].normal = vertices[2].normal = vertices[3].normal = m_normal;
        triangles.clear();
        triangles.resize(2);
        triangles[0][0] = 0;
        triangles[0][1] = 1;
        triangles[0][2] = 2;
        triangles[1][0] = 0;
        triangles[1][1] = 2;
        triangles[1][2] = 3;
    }

    void updateQuad() {
        m_bottom_left = vertices[0].position;
        m_right_vector = vertices[1].position - vertices[0].position;
        m_up_vector = vertices[3].position - vertices[0].position;
        m_normal = Vec3::cross(m_right_vector, m_up_vector);
        m_normal.normalize();
    }

    void setTexture(TextureMap t) {
        this->textureMap = t;
        this->hasTexture = true;
    }

    void setTextureTransform(float tileU_, float tileV_, float offsetU_ = 0.0f, float offsetV_ = 0.0f) {
        tileU = tileU_;
        tileV = tileV_;
        offsetU = offsetU_;
        offsetV = offsetV_;
    }

    void removeTexture() {
        this->hasTexture = false;
    }

    inline Vec3 getPixelColor(float u, float v) const {
        static constexpr float INV_255 = 1.0f / 255.0f;
        RGB pixel = textureMap.defaultColor;
        int x = static_cast<int>(u * textureMap.image.w);
        int y = static_cast<int>(v * textureMap.image.h);
        switch (textureMap.wrapMode) {
            case SquareTexture_Simple: {
                pixel = get_color_simple(textureMap.image, x, y, textureMap.defaultColor, textureMap.mirrorMode);
                break;
            }
            case SquareTexture_Cover: {
                pixel = get_color_cover(textureMap.image, x, y, textureMap.mirrorMode);
                break;
            }
            case SquareTexture_Repeat: {
                pixel = get_color_cover_repeat(textureMap.image, x, y, textureMap.mirrorMode);
                break;
            }
            case SquareTexture_Mirror: {
                pixel = get_color_repeat(textureMap.image, x, y, ImageMirror_Both);
                break;
            }
            case SquareTexture_Clamp: {
                pixel = get_color_clamp(textureMap.image, x, y, textureMap.mirrorMode);
                break;
            }
            case SquareTexture_Stretch: {
                // Stretch behaves like simple sample but clamps UVs inside [0, w/h)
                pixel = get_color_clamp(textureMap.image, x, y, textureMap.mirrorMode);
                break;
            }
            default:
                break;
        }
        return Vec3(pixel.r * INV_255, pixel.g * INV_255, pixel.b * INV_255);
    }

    RaySquareIntersection intersect(const Ray& ray) {
        RaySquareIntersection intersection;

        Plane P = Plane(m_bottom_left, m_normal);
        intersection.intersection = P.getIntersectionPoint(ray);

        Vec3 inter_to_origin = intersection.intersection - m_bottom_left;

        // X = x * v1 + y * v2

        double x = Vec3::dot(inter_to_origin, m_right_vector) / m_right_vector.squareNorm();
        double y = Vec3::dot(inter_to_origin, m_up_vector) / m_up_vector.squareNorm();

        intersection.intersectionExists = x >= 0 && x <= 1 && y >= 0 && y <= 1 && Vec3::dot(ray.direction(), m_normal) < 0;

        intersection.normal = m_normal;
        float uTex = x * tileU + offsetU;
        float vTex = y * tileV + offsetV;
        intersection.u = uTex;
        intersection.v = vTex;
        if (hasTexture) {
            intersection.pixel_x = static_cast<int>(uTex * textureMap.image.w);
            intersection.pixel_y = static_cast<int>(vTex * textureMap.image.h);
        }
        intersection.t = (intersection.intersection - ray.origin()).norm() / ray.direction().norm();

        return intersection;
    }
};
#endif  // SQUARE_H
