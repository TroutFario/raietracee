#ifndef SCENE_H
#define SCENE_H

#include <GL/glut.h>

#include <string>
#include <vector>

#include "Mesh.h"
#include "Sphere.h"
#include "Square.h"

#define DEFAULT_COLOR Vec3(1.f)
#define REMAXING_BOUNCES 5

enum ObjectType { SphereType, SquareType, MeshType };

enum LightType { LightType_Spherical, LightType_Quad };

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

        for (int i = 0; i < meshes.size(); i++) {
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
        for (int i = 0; i < spheres.size(); i++) {
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
        for (int i = 0; i < squares.size(); i++) {
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
                if (material.type == Material_Glass && NRemainingBounces > 0) {
                }
                break;
            case SquareType:
                N = raySceneIntersection.raySquareIntersection.normal;
                material = squares[raySceneIntersection.objectIndex].material;
                break;
            default:
                return Vec3(0.);
        }

        if (material.type == Material_Mirror && NRemainingBounces > 0) {
            Vec3 R = ray.direction() - 2.f * Vec3::dot(ray.direction(), N) * N;
            R.normalize();
            Ray reflectedRay(P + N * 0.001f, R);
            return rayTraceRecursive(reflectedRay, NRemainingBounces - 1);
        }

        if (material.type == Material_Glass && NRemainingBounces > 0) {
            return rayTraceRecursive(
                raySceneIntersection.raySphereIntersection.secondintersection,
                NRemainingBounces - 1);
        }

        N.normalize();
        Vec3 V = -ray.direction();
        V.normalize();

        Vec3 color = material.ambient_material * 0.1f;
        for (const Light& light : lights) {
            const Vec3 oldL = light.pos - P;
            const float theta = rand() / (float)RAND_MAX * 2.f * M_PI;
            const float phi = rand() / (float)RAND_MAX * 2.f * M_PI;
            const float rho =
                sqrt(rand() / (float)RAND_MAX) * light.radius * 0.5;
            Vec3 viveLaLumiere =
                Vec3(cos(theta) * cos(phi), sin(theta) * cos(phi), sin(phi))
                * rho;
                // Vec3(0.f);
            Vec3 L = oldL + viveLaLumiere;
            float distance_to_light = L.length();
            L.normalize();

            // Test d'ombre
            Ray shadowRay(P + N * 0.001f, L);
            RaySceneIntersection shadowIntersection =
                computeIntersection(shadowRay);
            if (shadowIntersection.intersectionExists &&
                shadowIntersection.t < distance_to_light)
                continue;
            // Composante diffuse
            float NdotL = std::max(0.0f, Vec3::dot(N, L));
            Vec3 diffuse = material.diffuse_material * light.material * NdotL;

            // Composante spéculaire
            Vec3 R = 2.0f * Vec3::dot(N, L) * N - L;
            R.normalize();
            float RdotV = std::max(0.0f, Vec3::dot(R, V));
            Vec3 specular = material.specular_material * light.material *
                            std::pow(RdotV, material.shininess);

            color = color + (diffuse + specular);
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
        return rayTraceRecursive(rayStart, REMAXING_BOUNCES);
    }

    void setup_single_sphere() {
        meshes.clear();
        spheres.clear();
        squares.clear();
        lights.clear();

        {
            lights.resize(lights.size() + 1);
            Light& light = lights[lights.size() - 1];
            light.pos = Vec3(-5, 5, 5);
            light.radius = 2.5f;
            light.powerCorrection = 2.f;
            light.type = LightType_Spherical;
            light.material = Vec3(1);
            light.isInCamSpace = false;
        }
        {
            // first sphere
            spheres.resize(spheres.size() + 1);
            Sphere& s = spheres[spheres.size() - 1];
            s.m_center = Vec3(1., 0., 0.);
            s.m_radius = 1.f;
            s.build_arrays();
            s.material.diffuse_material = Vec3(1., 0., 0.);
            s.material.specular_material = Vec3(0.2);
            s.material.shininess = 20;

            // second sphere
            spheres.resize(spheres.size() + 1);
            Sphere& s2 = spheres[spheres.size() - 1];
            s2.m_center = Vec3(-1., 0., 0.);
            s2.m_radius = 1.f;
            s2.build_arrays();
            s2.material.diffuse_material = Vec3(0., 1., 0.);
            s2.material.specular_material = Vec3(0.2);
            s2.material.shininess = 20;
        }
    }

    void setup_single_square() {
        meshes.clear();
        spheres.clear();
        squares.clear();
        lights.clear();

        {
            lights.resize(lights.size() + 1);
            Light& light = lights[lights.size() - 1];
            light.pos = Vec3(-5, 5, 5);
            light.radius = 2.5f;
            light.powerCorrection = 2.f;
            light.type = LightType_Spherical;
            light.material = Vec3(1);
            light.isInCamSpace = false;
        }

        {
            squares.resize(squares.size() + 1);
            Square& s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2.,
                      2.);
            s.build_arrays();
            s.material.diffuse_material = Vec3(1, 1, 0);
            s.material.specular_material = Vec3(0.8);
            s.material.shininess = 20;
        }
    }

    void setup_cornell_box() {
        meshes.clear();
        spheres.clear();
        squares.clear();
        lights.clear();

        {  // Light
            lights.resize(lights.size() + 1);
            Light& light = lights[lights.size() - 1];
            light.pos = Vec3(0.0, 1.5, 0.0);
            light.radius = 2.5f;
            light.powerCorrection = 2.f;
            light.type = LightType_Spherical;
            light.material = Vec3(1, 1, 1);
            light.isInCamSpace = false;
        }

        {  // Back Wall
            squares.resize(squares.size() + 1);
            Square& s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2.,
                      2.);
            s.scale(Vec3(2., 2., 1.));
            s.translate(Vec3(0., 0., -2.));
            s.build_arrays();
            s.material.ambient_material = Vec3(1., 0., 1.);
            s.material.diffuse_material = Vec3(1., 0., 1.);
            s.material.specular_material = Vec3(1., 1., 1.);
            s.material.shininess = 16;
        }

        {  // Left Wall

            squares.resize(squares.size() + 1);
            Square& s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2.,
                      2.);
            s.scale(Vec3(2., 2., 1.));
            s.translate(Vec3(0., 0., -2.));
            s.rotate_y(90);
            s.build_arrays();
            s.material.ambient_material = Vec3(1., 0., 0.);
            s.material.diffuse_material = Vec3(1., 0., 0.);
            s.material.specular_material = Vec3(1., 0., 0.);
            s.material.shininess = 16;
        }

        {  // Right Wall
            squares.resize(squares.size() + 1);
            Square& s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2.,
                      2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_y(-90);
            s.build_arrays();
            s.material.ambient_material = Vec3(0., 1.0, 0.);
            s.material.diffuse_material = Vec3(0.0, 1.0, 0.0);
            s.material.specular_material = Vec3(0.0, 1.0, 0.0);
            s.material.shininess = 16;
        }

        {  // Floor
            squares.resize(squares.size() + 1);
            Square& s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2.,
                      2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_x(-90);
            s.build_arrays();
            s.material.ambient_material = Vec3(1.0, 1.0, 1.0);
            s.material.diffuse_material = Vec3(1.0, 1.0, 1.0);
            s.material.specular_material = Vec3(1.0, 1.0, 1.0);
            s.material.shininess = 16;
        }

        {  // Ceiling
            squares.resize(squares.size() + 1);
            Square& s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2.,
                      2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_x(90);
            s.build_arrays();
            s.material.ambient_material = Vec3(1.0, 1.0, 0.);
            s.material.diffuse_material = Vec3(1.0, 1.0, 0.);
            s.material.specular_material = Vec3(1.0, 1.0, 1.0);
            s.material.shininess = 16;
        }

        {  // Front Wall
            squares.resize(squares.size() + 1);
            Square& s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2.,
                      2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_y(180);
            s.build_arrays();
            s.material.diffuse_material = Vec3(1.0, 1.0, 1.0);
            s.material.specular_material = Vec3(1.0, 1.0, 1.0);
            s.material.shininess = 16;
        }

        {  // MIRRORED Sphere

            spheres.resize(spheres.size() + 1);
            Sphere& s = spheres[spheres.size() - 1];
            s.m_center = Vec3(1.0, -1.25, 0.5);
            s.m_radius = 0.75f;
            s.build_arrays();
            s.material.type = Material_Mirror;
            s.material.diffuse_material = Vec3(1., 0., 0.);
            s.material.specular_material = Vec3(1., 0., 0.);
            s.material.shininess = 16;
            s.material.transparency = 0.;
            s.material.index_medium = 0.;
        }

        {  // GLASS Sphere
            spheres.resize(spheres.size() + 1);
            Sphere& s = spheres[spheres.size() - 1];
            s.m_center = Vec3(-1.0, -1.25, -0.5);
            s.m_radius = 0.75f;
            s.build_arrays();
            // s.material.type = Material_Mirror;
            // s.material.type = Material_Glass;
            s.material.diffuse_material = Vec3(0., 0., 1.);
            s.material.specular_material = Vec3(1., 1., 1.);
            s.material.shininess = 16;
            s.material.transparency = 1.0;
            s.material.index_medium = 1.5;
        }

        // {
        //     meshes.resize(meshes.size() + 1);
        //     Mesh& mesh = meshes[meshes.size() - 1];
        //     mesh.openOFF("assets/elephant_n.off", true, 2);
        //     // mesh.material.type = Material_Mirror;
        //     mesh.material.diffuse_material = Vec3(1., 0., 0.);
        //     mesh.material.specular_material = Vec3(1., 0., 0.);
        //     mesh.material.shininess = 16;
        // }
    }

    void setup_mesh_scene() {
        meshes.clear();
        spheres.clear();
        squares.clear();
        lights.clear();

        {
            lights.resize(lights.size() + 1);
            Light& light = lights[lights.size() - 1];
            light.pos = Vec3(-5, 5, 5);
            light.radius = 2.5f;
            light.powerCorrection = 2.f;
            light.type = LightType_Spherical;
            light.material = Vec3(1);
            light.isInCamSpace = false;
        }

        {
            meshes.resize(meshes.size() + 1);
            Mesh& mesh = meshes[meshes.size() - 1];
            mesh.openOFF("assets/elephant_n.off", true, 3);
            mesh.material.diffuse_material = Vec3(1., 0., 0.);
            mesh.material.specular_material = Vec3(1., 0., 0.);
            mesh.material.shininess = 16;
        }
    }
};

#endif
