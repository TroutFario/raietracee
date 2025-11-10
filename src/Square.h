#ifndef SQUARE_H
#define SQUARE_H
#include "Vec3.h"
#include <vector>
#include "Mesh.h"
#include <cmath>

struct RaySquareIntersection
{
    bool intersectionExists;
    float t;
    float u, v;
    Vec3 intersection;
    Vec3 normal;
};

class Square : public Mesh
{
public:
    Vec3 m_normal;
    Vec3 m_bottom_left;
    Vec3 m_right_vector;
    Vec3 m_up_vector;

    Square() : Mesh() {}
    Square(Vec3 const &bottomLeft, Vec3 const &rightVector, Vec3 const &upVector, float width = 1., float height = 1.,
           float uMin = 0.f, float uMax = 1.f, float vMin = 0.f, float vMax = 1.f) : Mesh()
    {
        setQuad(bottomLeft, rightVector, upVector, width, height, uMin, uMax, vMin, vMax);
    }

    void setQuad(Vec3 const &bottomLeft, Vec3 const &rightVector, Vec3 const &upVector, float width = 1., float height = 1.,
                 float uMin = 0.f, float uMax = 1.f, float vMin = 0.f, float vMax = 1.f)
    {
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
        m_right_vector = vertices[1].position - vertices[0].position;
        m_up_vector = vertices[3].position - vertices[0].position;
        m_normal = Vec3::cross(m_right_vector, m_up_vector);
        m_normal.normalize();
        m_bottom_left = vertices[0].position;
        m_center = m_bottom_left + 0.5f * m_right_vector + 0.5f * m_up_vector;
    }

    RaySquareIntersection intersect(const Ray &ray)
    {
        this->updateQuad();
        RaySquareIntersection intersection;
        Plane P = Plane(m_bottom_left, m_normal);
        intersection.intersection = P.getIntersectionPoint(ray);

        intersection.t = (ray.origin() - intersection.intersection).length();
        intersection.normal = m_normal;
        // calcul
        Line ligneR = Line(m_bottom_left, m_right_vector);
        Vec3 projR = ligneR.project(intersection.intersection);
        intersection.u = (projR - m_bottom_left).length();
        Line ligneU = Line(m_bottom_left, m_up_vector);
        Vec3 projU = ligneU.project(intersection.intersection);
        intersection.v = (projU - m_bottom_left).length();
        intersection.intersectionExists = (intersection.u <= m_right_vector.length() && intersection.v <= m_up_vector.length() && intersection.u >= 0 && intersection.v >= 0);
        return intersection;
    }
};
#endif // SQUARE_H
