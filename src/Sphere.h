#ifndef Sphere_H
#define Sphere_H
#include <cmath>
#include <vector>

#include "Mesh.h"
#include "Ray.h"
#include "Vec3.h"

struct RaySphereIntersection {
    bool intersectionExists;
    float t;
    float theta, phi;
    Vec3 intersection;
    Vec3 normal;
    Ray secondintersection;
};

static Vec3 SphericalCoordinatesToEuclidean(Vec3 ThetaPhiR) {
    return ThetaPhiR[2] * Vec3(cos(ThetaPhiR[0]) * cos(ThetaPhiR[1]),
                               sin(ThetaPhiR[0]) * cos(ThetaPhiR[1]),
                               sin(ThetaPhiR[1]));
}
static Vec3 SphericalCoordinatesToEuclidean(float theta, float phi) {
    return Vec3(cos(theta) * cos(phi), sin(theta) * cos(phi), sin(phi));
}

static Vec3 EuclideanCoordinatesToSpherical(Vec3 xyz) {
    float R = xyz.length();
    float phi = asin(xyz[2] / R);
    float theta = atan2(xyz[1], xyz[0]);
    return Vec3(theta, phi, R);
}

class Sphere : public Mesh {
   public:
    Vec3 m_center;
    float m_radius;

    Sphere() : Mesh() {}
    Sphere(Vec3 c, float r) : Mesh(), m_center(c), m_radius(r) {}

    void build_arrays() {
        unsigned int nTheta = 20, nPhi = 20;
        positions_array.resize(3 * nTheta * nPhi);
        normalsArray.resize(3 * nTheta * nPhi);
        uvs_array.resize(2 * nTheta * nPhi);
        for (unsigned int thetaIt = 0; thetaIt < nTheta; ++thetaIt) {
            float u = (float)(thetaIt) / (float)(nTheta - 1);
            float theta = u * 2 * M_PI;
            for (unsigned int phiIt = 0; phiIt < nPhi; ++phiIt) {
                unsigned int vertexIndex = thetaIt + phiIt * nTheta;
                float v = (float)(phiIt) / (float)(nPhi - 1);
                float phi = -M_PI / 2.0 + v * M_PI;
                Vec3 xyz = SphericalCoordinatesToEuclidean(theta, phi);
                positions_array[3 * vertexIndex + 0] =
                    m_center[0] + m_radius * xyz[0];
                positions_array[3 * vertexIndex + 1] =
                    m_center[1] + m_radius * xyz[1];
                positions_array[3 * vertexIndex + 2] =
                    m_center[2] + m_radius * xyz[2];
                normalsArray[3 * vertexIndex + 0] = xyz[0];
                normalsArray[3 * vertexIndex + 1] = xyz[1];
                normalsArray[3 * vertexIndex + 2] = xyz[2];
                uvs_array[2 * vertexIndex + 0] = u;
                uvs_array[2 * vertexIndex + 1] = v;
            }
        }
        triangles_array.clear();
        for (unsigned int thetaIt = 0; thetaIt < nTheta - 1; ++thetaIt) {
            for (unsigned int phiIt = 0; phiIt < nPhi - 1; ++phiIt) {
                unsigned int vertexuv = thetaIt + phiIt * nTheta;
                unsigned int vertexUv = thetaIt + 1 + phiIt * nTheta;
                unsigned int vertexuV = thetaIt + (phiIt + 1) * nTheta;
                unsigned int vertexUV = thetaIt + 1 + (phiIt + 1) * nTheta;
                triangles_array.push_back(vertexuv);
                triangles_array.push_back(vertexUv);
                triangles_array.push_back(vertexUV);
                triangles_array.push_back(vertexuv);
                triangles_array.push_back(vertexUV);
                triangles_array.push_back(vertexuV);
            }
        }
    }

    RaySphereIntersection intersect(const Ray& ray) const {
        const float EPS = 1e-4f;
        RaySphereIntersection intersection;
        intersection.intersectionExists = false;

        Vec3 O = ray.origin();
        Vec3 D = ray.direction();
        D.normalize();

        Vec3 C = m_center;
        float R = m_radius;

        /* ===============================
           1. Intersection rayon / sphère
           =============================== */

        float a = Vec3::dot(D, D);
        float b = 2.f * Vec3::dot(O - C, D);
        float c = Vec3::dot(O - C, O - C) - R * R;

        float delta = b * b - 4.f * a * c;
        if (delta < 0.f) return intersection;

        float sqrtDelta = sqrt(delta);
        float t1 = (-b - sqrtDelta) / (2.f * a);
        float t2 = (-b + sqrtDelta) / (2.f * a);

        // Prendre le plus petit t positif (intersection avant)
        intersection.t = (t1 > 0.001f) ? t1 : t2;

        if (intersection.t < EPS) return intersection;

        intersection.intersectionExists = true;
        intersection.intersection = O + intersection.t * D;

        /* ===============================
           2. Normale à l’entrée
           =============================== */

        Vec3 N = intersection.intersection - C;
        N.normalize();

        if (Vec3::dot(N, D) > 0) N = -N;

        intersection.normal = N;

        Vec3 tpR = EuclideanCoordinatesToSpherical(N);
        intersection.theta = tpR[0];
        intersection.phi = tpR[1];

        /* ===============================
           3. Si pas du verre, on s’arrête
           =============================== */

        if (material.type != Material_Glass) return intersection;

        /* ===============================
           4. Réfraction entrée (air → verre)
           =============================== */

        float n1 = 1.f;
        float n2 = material.index_medium;
        float eta = n1 / n2;

        Vec3 I = D;
        float cos_i = -Vec3::dot(N, I);
        float k = 1.f - eta * eta * (1.f - cos_i * cos_i);

        // Réflexion totale (rare à l’entrée mais possible numériquement)
        if (k < 0.f) {
            Vec3 Rdir = I + 2.f * cos_i * N;
            intersection.secondintersection =
                Ray(intersection.intersection, Rdir);
            return intersection;
        }

        Vec3 T = eta * I + (eta * cos_i - sqrt(k)) * N;
        T.normalize();

        Ray insideRay(intersection.intersection + EPS * T, T);

        /* ===============================
           5. Intersection de sortie
           =============================== */

        Vec3 O2 = insideRay.origin();
        Vec3 D2 = insideRay.direction();

        float a2 = Vec3::dot(D2, D2);
        float b2 = 2.f * Vec3::dot(O2 - C, D2);
        float c2 = Vec3::dot(O2 - C, O2 - C) - R * R;

        float delta2 = b2 * b2 - 4.f * a2 * c2;
        if (delta2 < 0.f) return intersection;

        float sqrtDelta2 = sqrt(delta2);
        float s1 = (-b2 - sqrtDelta2) / (2.f * a2);
        float s2 = (-b2 + sqrtDelta2) / (2.f * a2);

        float s = (s1 > EPS) ? s1 : s2;
        Vec3 exitPoint = O2 + s * D2;

        /* ===============================
           6. Normale à la sortie
           =============================== */

        Vec3 N2 = exitPoint - C;
        N2.normalize();

        if (Vec3::dot(N2, D2) > 0) N2 = -N2;

        /* ===============================
           7. Réfraction sortie (verre → air)
           =============================== */

        float eta2 = n2 / n1;
        float cos_i2 = -Vec3::dot(N2, D2);
        float k2 = 1.f - eta2 * eta2 * (1.f - cos_i2 * cos_i2);

        // Réflexion totale interne
        if (k2 < 0.f) {
            Vec3 R2 = D2 + 2.f * cos_i2 * N2;
            intersection.secondintersection = Ray(exitPoint, R2);
            return intersection;
        }

        Vec3 T2 = eta2 * D2 + (eta2 * cos_i2 - sqrt(k2)) * N2;
        T2.normalize();

        intersection.secondintersection = Ray(exitPoint + EPS * T2, T2);

        return intersection;
    }
};
#endif
