#ifndef TRIANGLE_H
#define TRIANGLE_H
#include "Plane.h"
#include "Ray.h"
#include "Vec3.h"

struct RayTriangleIntersection {
    bool intersectionExists;
    float t;
    float w0, w1, w2;
    unsigned int tIndex;
    Vec3 intersection;
    Vec3 normal;
};

class Triangle {
   private:
    Vec3 m_c[3], m_normal;
    float area;

   public:
    Triangle() {}
    Triangle(Vec3 const& c0, Vec3 const& c1, Vec3 const& c2) {
        m_c[0] = c0;
        m_c[1] = c1;
        m_c[2] = c2;
        updateAreaAndNormal();
    }

    void updateAreaAndNormal() {
        Vec3 nNotNormalized = Vec3::cross(m_c[1] - m_c[0], m_c[2] - m_c[0]);
        float norm = nNotNormalized.length();
        m_normal = nNotNormalized / norm;
        area = norm / 2.f;
    }

    void setC0(Vec3 const& c0) {
        m_c[0] = c0;
    }  // remember to update the area and normal afterwards!

    void setC1(Vec3 const& c1) {
        m_c[1] = c1;
    }  // remember to update the area and normal afterwards!

    void setC2(Vec3 const& c2) {
        m_c[2] = c2;
    }  // remember to update the area and normal afterwards!

    Vec3 const& normal() const { return m_normal; }
    Vec3 projectOnSupportPlane(Vec3 const& p) const {
        return Plane(m_c[0], m_normal).project(p);
    }

    float squareDistanceToSupportPlane(Vec3 const& p) const {
        return projectOnSupportPlane(p).squareLength();
    }

    float distanceToSupportPlane(Vec3 const& p) const {
        return sqrt(squareDistanceToSupportPlane(p));
    }

    bool isParallelTo(Line const& L) const {
        return Vec3::dot(L.direction(), m_normal) == 0;
    }

    Vec3 getIntersectionPointWithSupportPlane(Line const& L) const {
        Vec3 result;

        if (isParallelTo(L)) {
            return result;
        }

        Plane P = Plane(m_c[0], m_normal);
        return P.getIntersectionPoint(L);
    }

    void computeBarycentricCoordinates(Vec3 const& p, float& u0, float& u1,
                                       float& u2) const {
        // chaque u_i est le pourcentage de l'aire du triangle opposé au sommet
        // i sur l'aire totale du triangle
        Vec3 v0 = m_c[1] - m_c[0];
        Vec3 v1 = m_c[2] - m_c[0];
        Vec3 v2 = p - m_c[0];

        float d00 = Vec3::dot(v0, v0);
        float d01 = Vec3::dot(v0, v1);
        float d11 = Vec3::dot(v1, v1);
        float d20 = Vec3::dot(v2, v0);
        float d21 = Vec3::dot(v2, v1);
        float denom = d00 * d11 - d01 * d01;

        u1 = (d11 * d20 - d01 * d21) / denom;
        u2 = (d00 * d21 - d01 * d20) / denom;
        u0 = 1.0f - u1 - u2;
    }

    RayTriangleIntersection getIntersection(Ray const& ray) const {
        RayTriangleIntersection result;
        result.intersectionExists = false;
        const float epsilon = 1e-4f;
        // 1) check that the ray is not parallel to the triangle:
        if (isParallelTo(ray)) return result;
        // 2) check that the triangle is "in front of" the ray:
        const float ndotd = Vec3::dot(m_normal, ray.direction());
        if (ndotd >= -epsilon) return result;

        // 3) check that the intersection point is inside the triangle:
        // CONVENTION: compute u,v such that p = w0*c0 + w1*c1 + w2*c2, check
        // that 0 <= w0,w1,w2 <= 1
        Plane trianglePlane(m_c[0], m_normal);
        Vec3 P = trianglePlane.getIntersectionPoint(ray);
        float w0, w1, w2;
        computeBarycentricCoordinates(P, w0, w1, w2);
        if (w0 < -epsilon || w1 < -epsilon || w2 < -epsilon) return result;

        // 4) Finally, if all conditions were met, then there is an
        // intersection! :

        result.t = Vec3::dot(P - ray.origin(), ray.direction());
        if (result.t < epsilon) {
            return result;
        }
        result.intersectionExists = true;
        result.w0 = w0;
        result.w1 = w1;
        result.w2 = w2;
        result.tIndex = 0;  // Assuming single triangle, index 0
        result.intersection = P;
        result.normal = m_normal;
        return result;
    }
};
#endif
