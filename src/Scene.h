#ifndef SCENE_H
#define SCENE_H

#include <GL/glut.h>

#include <string>
#include <vector>

#include "Mesh.h"
#include "Sphere.h"
#include "Square.h"

#define DEFAULT_COLOR Vec3(1.f)
#define REMAING_BOUNCES 10

enum ObjectType { SphereType,
                  SquareType,
                  MeshType };

enum LightType { LightType_Spherical,
                 LightType_Quad };

struct Light {
    Vec3 material;
    bool isInCamSpace;
    LightType type;

    Vec3 pos;
    float radius;

    Mesh quad;

    float powerCorrection;

    Light() : powerCorrection(1.0) {}
};

struct RaySceneIntersection {
    bool intersectionExists;
    ObjectType typeOfIntersectedObject;
    unsigned int objectIndex;
    float t;
    RayTriangleIntersection rayMeshIntersection;
    RaySphereIntersection raySphereIntersection;
    RaySquareIntersection raySquareIntersection;
    RaySceneIntersection() : intersectionExists(false), t(FLT_MAX) {}
};

class Scene {
    std::vector<Mesh> meshes;
    std::vector<Sphere> spheres;
    std::vector<Square> squares;
    std::vector<Light> lights;

   public:
    Scene() {}

    void clearScene() {
        meshes.clear();
        spheres.clear();
        squares.clear();
        lights.clear();
    }

    void addLight(const Vec3& pos, float radius, float powerCorrection,
                  LightType type, const Vec3& material, bool isInCamSpace) {
        lights.resize(lights.size() + 1);
        Light& light = lights[lights.size() - 1];
        light.pos = pos;
        light.radius = radius;
        light.powerCorrection = powerCorrection;
        light.type = type;
        light.material = material;
        light.isInCamSpace = isInCamSpace;
    }

    void addMesh(const Mesh& mesh) { meshes.push_back(mesh); }

    void addSphere(const Sphere& sphere) { spheres.push_back(sphere); }

    void addSquare(const Square& square) { squares.push_back(square); }

   private:
    // Calcule la direction réfractée selon la loi de Snell.
    bool refractDir(const Vec3& I, const Vec3& N_in, float ior, Vec3& T) const {
        Vec3 N = N_in;
        float n1 = 1.0f;
        float n2 = ior;
        float cosi = std::max(-1.0f, std::min(1.0f, Vec3::dot(I, N)));
        if (cosi > 0.0f) {
            // À l'intérieur: inverse la normale et échange les indices
            N = -N;
            std::swap(n1, n2);
            cosi = -cosi;
        }
        float eta = n1 / n2;
        float k = 1.0f - eta * eta * (1.0f - cosi * cosi);
        // Seuil plus permissif pour éviter réflexion totale trop facilement
        if (k < -0.01f) {
            return false;  // réflexion totale interne
        }
        // Si k est légèrement négatif, on le clamp à 0
        if (k < 0.0f) k = 0.0f;
        T = eta * I - (eta * cosi + sqrtf(k)) * N;
        T.normalize();
        return true;
    }

   public:
    void draw() {
        // iterer sur l'ensemble des objets, et faire leur rendu :
        for (unsigned int It = 0; It < meshes.size(); ++It) {
            Mesh const& mesh = meshes[It];
            mesh.draw();
        }
        for (unsigned int It = 0; It < spheres.size(); ++It) {
            Sphere const& sphere = spheres[It];
            sphere.draw();
        }
        for (unsigned int It = 0; It < squares.size(); ++It) {
            Square const& square = squares[It];
            square.draw();
        }
    }

    RaySceneIntersection computeIntersection(Ray const& ray) {
        RaySceneIntersection result;
        result.t = FLT_MAX;

        for (int i = 0; i < meshes.size(); ++i) {
            RayTriangleIntersection intersection = meshes[i].intersect(ray);
            if (intersection.intersectionExists && intersection.t < result.t &&
                intersection.t > 0.001) {
                result.intersectionExists = true;
                result.t = intersection.t;
                result.rayMeshIntersection = intersection;
                result.objectIndex = i;
                result.typeOfIntersectedObject = MeshType;
            }
        }
        for (int i = 0; i < spheres.size(); ++i) {
            RaySphereIntersection intersection = spheres[i].intersect(ray);
            if (intersection.intersectionExists && intersection.t < result.t &&
                intersection.t > 0.001) {
                result.intersectionExists = true;
                result.t = intersection.t;
                result.typeOfIntersectedObject = SphereType;
                result.raySphereIntersection = intersection;
                result.objectIndex = i;
            }
        }
        for (int i = 0; i < squares.size(); ++i) {
            RaySquareIntersection intersection = squares[i].intersect(ray);
            if (intersection.intersectionExists && intersection.t < result.t &&
                intersection.t > 0.001) {
                result.intersectionExists = true;
                result.t = intersection.t;
                result.raySquareIntersection = intersection;
                result.objectIndex = i;
                result.typeOfIntersectedObject = SquareType;
            }
        }
        return result;
    }

    Vec3 rayTraceRecursive(Ray ray, int NRemainingBounces) {
        RaySceneIntersection raySceneIntersection = computeIntersection(ray);
        if (!raySceneIntersection.intersectionExists) return DEFAULT_COLOR;

        Vec3 P = ray.origin() + raySceneIntersection.t * ray.direction();
        Vec3 N = Vec3(0.);
        Material material;

        switch (raySceneIntersection.typeOfIntersectedObject) {
            case MeshType:
                N = raySceneIntersection.rayMeshIntersection.normal;
                material = meshes[raySceneIntersection.objectIndex].material;
                break;
            case SphereType:
                N = raySceneIntersection.raySphereIntersection.normal;
                material = spheres[raySceneIntersection.objectIndex].material;

                break;
            case SquareType:
                N = raySceneIntersection.raySquareIntersection.normal;
                material = squares[raySceneIntersection.objectIndex].material;
                if (material.type == Material_Texture) {
                    float u = raySceneIntersection.raySquareIntersection.u;
                    float v = raySceneIntersection.raySquareIntersection.v;
                    material.diffuse_material = squares[raySceneIntersection.objectIndex].getPixelColor(u, v);
                }
                break;
            default:
                return Vec3(DEFAULT_COLOR);
        }

        N.normalize();
        Vec3 V = -ray.direction();
        V.normalize();

        Vec3 specular = Vec3(0.f);
        Vec3 color = material.ambient_material * 0.1f;
        for (const Light& light : lights) {
            const Vec3 oldL = light.pos - P;
            const float theta = rand() / (float)RAND_MAX * 2.f * M_PI;
            const float phi = rand() / (float)RAND_MAX * 2.f * M_PI;
            const float rho = sqrt(rand() / (float)RAND_MAX) * light.radius * 0.5;
            Vec3 viveLaLumiere = Vec3(cos(theta) * cos(phi), sin(theta) * cos(phi), sin(phi)) * rho;
            Vec3 L = oldL + viveLaLumiere;
            float distance_to_light = L.length();
            L.normalize();

            // Test d'ombre
            Ray shadowRay(P + N * 0.001f, L);
            RaySceneIntersection shadowIntersection = computeIntersection(shadowRay);
            if (shadowIntersection.intersectionExists && shadowIntersection.t < distance_to_light)
                continue;

            // Composante diffuse
            float NdotL = std::max(0.0f, Vec3::dot(N, L));
            Vec3 diffuse = material.diffuse_material * light.material * NdotL;

            // Composante spéculaire
            Vec3 R = 2.0f * Vec3::dot(N, L) * N - L;
            R.normalize();
            float RdotV = std::max(0.0f, Vec3::dot(R, V));
            specular += material.specular_material * light.material * std::pow(RdotV, material.shininess);

            color = color + (diffuse + specular);
        }

        if (material.type == Material_Mirror && NRemainingBounces > 0) {
            Vec3 R = ray.direction() - 2.f * Vec3::dot(ray.direction(), N) * N;
            R.normalize();
            Ray reflectedRay(P + N * 0.001f, R);
            return rayTraceRecursive(reflectedRay, NRemainingBounces - 1) + specular;
        }

        if (material.type == Material_Glass && NRemainingBounces > 0) {
            if (raySceneIntersection.typeOfIntersectedObject == SphereType) {
                return rayTraceRecursive(raySceneIntersection.raySphereIntersection.secondintersection, NRemainingBounces - 1) + specular;
            } else {
                Vec3 T;
                bool canRefract = refractDir(ray.direction(), N, material.index_medium, T);
                if (canRefract) {
                    Vec3 P = ray.origin() + raySceneIntersection.t * ray.direction();
                    Vec3 offset = (Vec3::dot(ray.direction(), N) < 0.0f) ? -N * 0.001f : N * 0.001f;
                    Ray refractedRay(P + offset, T);
                    return rayTraceRecursive(refractedRay, NRemainingBounces - 1) + specular;
                } else {
                    // Fallback: réflexion
                    Vec3 R = ray.direction() - 2.f * Vec3::dot(ray.direction(), N) * N;
                    R.normalize();
                    Ray reflectedRay((ray.origin() + raySceneIntersection.t * ray.direction()) + N * 0.001f, R);
                    return rayTraceRecursive(reflectedRay, NRemainingBounces - 1) + specular;
                }
            }
        }

        return color;
    }

    Vec3 rayTraceRecursive_phase1(Ray ray, int NRemainingBounces) {
        RaySceneIntersection raySceneIntersection = computeIntersection(ray);
        if (raySceneIntersection.intersectionExists)
            switch (raySceneIntersection.typeOfIntersectedObject) {
                case MeshType:
                    return meshes[raySceneIntersection.objectIndex]
                        .material.diffuse_material;
                case SphereType:
                    return spheres[raySceneIntersection.objectIndex]
                        .material.diffuse_material;
                case SquareType:
                    return squares[raySceneIntersection.objectIndex]
                        .material.diffuse_material;
                default:
                    return Vec3(0., 0., 0.);
            }
        return Vec3(0., 0., 0.);
    }

    Vec3 rayTrace(Ray const& rayStart) {
        return rayTraceRecursive(rayStart, REMAING_BOUNCES);
    }
};

#endif
