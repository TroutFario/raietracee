#ifndef MESH_H
#define MESH_H

#include <GL/glut.h>

#include <algorithm>
#include <cfloat>
#include <string>
#include <vector>

#include "Material.h"
#include "Ray.h"
#include "Triangle.h"
#include "Vec3.h"

// -------------------------------------------
// AABB - Axis-Aligned Bounding Box
// -------------------------------------------

struct AABB {
    Vec3 min, max;

    AABB() : min(Vec3(FLT_MAX)), max(Vec3(-FLT_MAX)) {}
    AABB(const Vec3& min_, const Vec3& max_) : min(min_), max(max_) {}

    void expand(const Vec3& p) {
        min[0] = std::min(min[0], p[0]);
        min[1] = std::min(min[1], p[1]);
        min[2] = std::min(min[2], p[2]);
        max[0] = std::max(max[0], p[0]);
        max[1] = std::max(max[1], p[1]);
        max[2] = std::max(max[2], p[2]);
    }

    void expand(const AABB& box) {
        expand(box.min);
        expand(box.max);
    }

    bool intersect(const Ray& ray, float& tmin, float& tmax) const {
        for (int i = 0; i < 3; ++i) {
            float invD = 1.0f / ray.direction()[i];
            float t0 = (min[i] - ray.origin()[i]) * invD;
            float t1 = (max[i] - ray.origin()[i]) * invD;
            if (invD < 0.0f) std::swap(t0, t1);
            tmin = t0 > tmin ? t0 : tmin;
            tmax = t1 < tmax ? t1 : tmax;
            if (tmax <= tmin) return false;
        }
        return true;
    }

    Vec3 centroid() const { return (min + max) * 0.5f; }
};

// -------------------------------------------
// BVH Node
// -------------------------------------------

struct BVHNode {
    AABB box;
    int left, right;              // indices des fils (-1 si feuille)
    std::vector<int> triIndices;  // triangles contenus si feuille

    BVHNode() : left(-1), right(-1) {}
    bool isLeaf() const { return left == -1; }
};

// -------------------------------------------
// Mesh classes
// -------------------------------------------

struct MeshVertex {
    inline MeshVertex() {}
    inline MeshVertex(const Vec3& _p, const Vec3& _n)
        : position(_p), normal(_n), u(0), v(0) {}
    inline MeshVertex(const MeshVertex& vertex)
        : position(vertex.position),
          normal(vertex.normal),
          u(vertex.u),
          v(vertex.v) {}
    inline virtual ~MeshVertex() {}
    inline MeshVertex& operator=(const MeshVertex& vertex) {
        position = vertex.position;
        normal = vertex.normal;
        u = vertex.u;
        v = vertex.v;
        return (*this);
    }
    // membres :
    Vec3 position;  // une position
    Vec3 normal;    // une normale
    float u, v;     // coordonnees uv
};

struct MeshTriangle {
    inline MeshTriangle() { v[0] = v[1] = v[2] = 0; }
    inline MeshTriangle(const MeshTriangle& t) {
        v[0] = t.v[0];
        v[1] = t.v[1];
        v[2] = t.v[2];
    }
    inline MeshTriangle(unsigned int v0, unsigned int v1, unsigned int v2) {
        v[0] = v0;
        v[1] = v1;
        v[2] = v2;
    }
    unsigned int& operator[](unsigned int iv) { return v[iv]; }
    unsigned int operator[](unsigned int iv) const { return v[iv]; }
    inline virtual ~MeshTriangle() {}
    inline MeshTriangle& operator=(const MeshTriangle& t) {
        v[0] = t.v[0];
        v[1] = t.v[1];
        v[2] = t.v[2];
        return (*this);
    }
    // membres :
    unsigned int v[3];
};

class Mesh {
   protected:
    void build_positions_array() {
        positions_array.resize(3 * vertices.size());
        for (unsigned int v = 0; v < vertices.size(); ++v) {
            positions_array[3 * v + 0] = vertices[v].position[0];
            positions_array[3 * v + 1] = vertices[v].position[1];
            positions_array[3 * v + 2] = vertices[v].position[2];
        }
    }
    void build_normals_array() {
        normalsArray.resize(3 * vertices.size());
        for (unsigned int v = 0; v < vertices.size(); ++v) {
            normalsArray[3 * v + 0] = vertices[v].normal[0];
            normalsArray[3 * v + 1] = vertices[v].normal[1];
            normalsArray[3 * v + 2] = vertices[v].normal[2];
        }
    }
    void build_UVs_array() {
        uvs_array.resize(2 * vertices.size());
        for (unsigned int vert = 0; vert < vertices.size(); ++vert) {
            uvs_array[2 * vert + 0] = vertices[vert].u;
            uvs_array[2 * vert + 1] = vertices[vert].v;
        }
    }
    void build_triangles_array() {
        triangles_array.resize(3 * triangles.size());
        for (unsigned int t = 0; t < triangles.size(); ++t) {
            triangles_array[3 * t + 0] = triangles[t].v[0];
            triangles_array[3 * t + 1] = triangles[t].v[1];
            triangles_array[3 * t + 2] = triangles[t].v[2];
        }
    }

   public:
    // membres :
    std::vector<MeshVertex> vertices;
    std::vector<MeshTriangle> triangles;

    std::vector<float> positions_array;
    std::vector<float> normalsArray;
    std::vector<float> uvs_array;
    std::vector<unsigned int> triangles_array;

    // KD-tree ========================
    std::vector<BVHNode> bvhNodes;  // tableau de noeuds
    int rootNodeIndex = -1;         // index du noeud racine dans bvhNodes
    int maxTrianglesPerLeaf = 4;    // combien de triangles par feuille
    int maxDepth = 32;              // sécurité pour profondeur max

    AABB computeTriangleAABB(const MeshTriangle& tri) const {
        const Vec3& c0 = vertices[tri.v[0]].position;
        const Vec3& c1 = vertices[tri.v[1]].position;
        const Vec3& c2 = vertices[tri.v[2]].position;
        AABB box;
        box.expand(c0);
        box.expand(c1);
        box.expand(c2);
        return box;
    }

    int buildBVHRecursive(std::vector<int>& triangleIndices, int depth) {
        BVHNode node;

        // Calculer la bounding box de ce noeud
        for (int idx : triangleIndices) {
            node.box.expand(computeTriangleAABB(triangles[idx]));
        }

        // Condition d'arrêt => feuille
        if (triangleIndices.size() <= maxTrianglesPerLeaf ||
            depth >= maxDepth) {
            node.triIndices = triangleIndices;
            bvhNodes.push_back(node);
            return bvhNodes.size() - 1;
        }

        // Choisir l'axe de split
        Vec3 extent = node.box.max - node.box.min;
        int axis = 0;
        if (extent[1] > extent[0]) axis = 1;
        if (extent[2] > extent[axis]) axis = 2;

        // Plan au milieu
        float splitPos = 0.5f * (node.box.min[axis] + node.box.max[axis]);

        // Partitionner les triangles (sans duplication, sinon la taille ne
        // décroît jamais et le BVH explose en mémoire).
        std::vector<int> leftTris, rightTris;
        leftTris.reserve(triangleIndices.size());
        rightTris.reserve(triangleIndices.size());
        for (int idx : triangleIndices) {
            const MeshTriangle& tri = triangles[idx];
            const Vec3 c0 = vertices[tri.v[0]].position;
            const Vec3 c1 = vertices[tri.v[1]].position;
            const Vec3 c2 = vertices[tri.v[2]].position;

            float centroid = (c0[axis] + c1[axis] + c2[axis]) / 3.0f;
            if (centroid <= splitPos)
                leftTris.push_back(idx);
            else
                rightTris.push_back(idx);
        }

        // Si une des partitions est vide (tous les triangles du même côté),
        // on force un split équilibré pour éviter la récursion infinie.
        if (leftTris.empty() || rightTris.empty()) {
            leftTris.clear();
            rightTris.clear();
            size_t mid = triangleIndices.size() / 2;
            leftTris.insert(leftTris.end(), triangleIndices.begin(),
                            triangleIndices.begin() + mid);
            rightTris.insert(rightTris.end(), triangleIndices.begin() + mid,
                             triangleIndices.end());
        }

        int nodeIndex = bvhNodes.size();
        bvhNodes.push_back(node);  // noeud temporaire

        int leftChild = buildBVHRecursive(leftTris, depth + 1);
        int rightChild = buildBVHRecursive(rightTris, depth + 1);

        bvhNodes[nodeIndex].left = leftChild;
        bvhNodes[nodeIndex].right = rightChild;

        return nodeIndex;
    }

    void buildBVH() {
        bvhNodes.clear();
        std::vector<int> allTriangles(triangles.size());
        for (size_t i = 0; i < triangles.size(); ++i) allTriangles[i] = i;
        rootNodeIndex = buildBVHRecursive(allTriangles, 0);
    }

    // ================================

    Material material;

    void openOFF(const std::string& filename, bool load_normals = false,
                 float scale = 1.0);
    void recomputeNormals();
    void centerAndScaleToUnit();
    void scaleUnit();

    virtual void build_arrays() {
        recomputeNormals();
        buildBVH();
        build_positions_array();
        build_normals_array();
        build_UVs_array();
        build_triangles_array();
    }

   public:
    void translate(Vec3 const& translation) {
        for (unsigned int v = 0; v < vertices.size(); ++v) {
            vertices[v].position += translation;
        }
    }

    void apply_transformation_matrix(Mat3 transform) {
        for (unsigned int v = 0; v < vertices.size(); ++v) {
            vertices[v].position = transform * vertices[v].position;
        }

        //        recomputeNormals();
        //        build_positions_array();
        //        build_normals_array();
    }

    void scale(Vec3 const& scale) {
        Mat3 scale_matrix(
            scale[0], 0., 0., 0., scale[1], 0., 0., 0.,
            scale[2]);  // Matrice de transformation de mise à l'échelle
        apply_transformation_matrix(scale_matrix);
    }

    void rotate_x(float angle) {
        float x_angle = angle * M_PI / 180.;
        Mat3 x_rotation(1., 0., 0., 0., cos(x_angle), -sin(x_angle), 0.,
                        sin(x_angle), cos(x_angle));
        apply_transformation_matrix(x_rotation);
    }

    void rotate_y(float angle) {
        float y_angle = angle * M_PI / 180.;
        Mat3 y_rotation(cos(y_angle), 0., sin(y_angle), 0., 1., 0.,
                        -sin(y_angle), 0., cos(y_angle));
        apply_transformation_matrix(y_rotation);
    }

    void rotate_z(float angle) {
        float z_angle = angle * M_PI / 180.;
        Mat3 z_rotation(cos(z_angle), -sin(z_angle), 0., sin(z_angle),
                        cos(z_angle), 0., 0., 0., 1.);
        apply_transformation_matrix(z_rotation);
    }

    void draw() const {
        if (triangles_array.size() == 0) return;
        GLfloat material_color[4] = {material.diffuse_material[0],
                                     material.diffuse_material[1],
                                     material.diffuse_material[2], 1.0};

        GLfloat material_specular[4] = {material.specular_material[0],
                                        material.specular_material[1],
                                        material.specular_material[2], 1.0};

        GLfloat material_ambient[4] = {material.ambient_material[0],
                                       material.ambient_material[1],
                                       material.ambient_material[2], 1.0};

        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_specular);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_color);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material_ambient);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material.shininess);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, 3 * sizeof(float),
                        (GLvoid*)(normalsArray.data()));
        glVertexPointer(3, GL_FLOAT, 3 * sizeof(float),
                        (GLvoid*)(positions_array.data()));
        glDrawElements(GL_TRIANGLES, triangles_array.size(), GL_UNSIGNED_INT,
                       (GLvoid*)(triangles_array.data()));
    }

    RayTriangleIntersection intersectNaif(Ray const& ray) const {
        RayTriangleIntersection closestIntersection;
        closestIntersection.t = FLT_MAX;

        float triangleScaling = 1.000001;
        Triangle triangle;

        for (unsigned int t = 0; t < triangles.size(); ++t) {
            unsigned int v0Index = triangles[t].v[0];
            unsigned int v1Index = triangles[t].v[1];
            unsigned int v2Index = triangles[t].v[2];
            Vec3 c0 = vertices[v0Index].position * triangleScaling;
            Vec3 c1 = vertices[v1Index].position * triangleScaling;
            Vec3 c2 = vertices[v2Index].position * triangleScaling;
            triangle = Triangle(c0, c1, c2);
            RayTriangleIntersection intersection = triangle.getIntersection(ray);
            intersection.tIndex = t;
            if (intersection.intersectionExists &&
                intersection.t < closestIntersection.t) {
                closestIntersection = intersection;
            }
        }

        if (closestIntersection.intersectionExists) {
            const MeshTriangle& tri = triangles[closestIntersection.tIndex];
            const Vec3& n0 = vertices[tri.v[0]].normal;
            const Vec3& n1 = vertices[tri.v[1]].normal;
            const Vec3& n2 = vertices[tri.v[2]].normal;

            float w0 = closestIntersection.w0;
            float w1 = closestIntersection.w1;
            float w2 = closestIntersection.w2;

            Vec3 smoothNormal = w0 * n0 + w1 * n1 + w2 * n2;
            smoothNormal.normalize();

            if (Vec3::dot(smoothNormal, ray.direction()) > 0.0f)
                smoothNormal = -smoothNormal;

            closestIntersection.normal = smoothNormal;
            // closestIntersection.normal = triangle.normal();
        }

        return closestIntersection;
    }

    RayTriangleIntersection intersectBVH(const Ray& ray, int nodeIndex) const {
        RayTriangleIntersection closestHit;
        closestHit.t = FLT_MAX;

        if (nodeIndex < 0) return closestHit;
        const BVHNode& node = bvhNodes[nodeIndex];

        float tmin = 0.0f, tmax = FLT_MAX;
        if (!node.box.intersect(ray, tmin, tmax)) return closestHit;

        if (node.isLeaf()) {
            for (int triIndex : node.triIndices) {
                const MeshTriangle& tri = triangles[triIndex];
                Triangle triangle(vertices[tri.v[0]].position, vertices[tri.v[1]].position, vertices[tri.v[2]].position);
                RayTriangleIntersection hit = triangle.getIntersection(ray);
                hit.tIndex = triIndex;

                if (hit.intersectionExists && hit.t < closestHit.t)
                    closestHit = hit;
            }
            return closestHit;
        }

        // On teste les AABB des enfants avant la récursion
        float tminLeft = 0.0f, tmaxLeft = FLT_MAX;
        float tminRight = 0.0f, tmaxRight = FLT_MAX;
        bool hitLeft = (node.left >= 0) && bvhNodes[node.left].box.intersect(ray, tminLeft, tmaxLeft);
        bool hitRight = (node.right >= 0) && bvhNodes[node.right].box.intersect(ray, tminRight, tmaxRight);

        // Si aucune intersection, retourner
        if (!hitLeft && !hitRight) return closestHit;

        // Déterminer l'ordre de traversée (tester le plus proche en premier)
        int firstChild = -1, secondChild = -1;
        float secondTmin = FLT_MAX;
        
        if (hitLeft && hitRight) {
            if (tminLeft < tminRight) {
                firstChild = node.left;
                secondChild = node.right;
                secondTmin = tminRight;
            } else {
                firstChild = node.right;
                secondChild = node.left;
                secondTmin = tminLeft;
            }
        } else if (hitLeft) {
            firstChild = node.left;
        } else {
            firstChild = node.right;
        }

        // Tester le premier enfant
        if (firstChild >= 0) {
            closestHit = intersectBVH(ray, firstChild);
        }

        // Tester le second enfant seulement si son AABB est plus proche que l'intersection trouvée
        if (secondChild >= 0 && secondTmin < closestHit.t) {
            RayTriangleIntersection secondHit = intersectBVH(ray, secondChild);
            if (secondHit.intersectionExists && secondHit.t < closestHit.t) {
                closestHit = secondHit;
            }
        }

        return closestHit;
    }

    RayTriangleIntersection intersect(const Ray& ray) const {
        RayTriangleIntersection intersection = intersectBVH(ray, rootNodeIndex);

        if (intersection.intersectionExists) {
            const MeshTriangle& tri = triangles[intersection.tIndex];
            const Vec3& n0 = vertices[tri.v[0]].normal;
            const Vec3& n1 = vertices[tri.v[1]].normal;
            const Vec3& n2 = vertices[tri.v[2]].normal;

            float w0 = intersection.w0;
            float w1 = intersection.w1;
            float w2 = intersection.w2;

            Vec3 smoothNormal = w0 * n0 + w1 * n1 + w2 * n2;
            smoothNormal.normalize();

            if (Vec3::dot(smoothNormal, ray.direction()) > 0.0f)
                smoothNormal = -smoothNormal;

            intersection.normal = smoothNormal;
        }

        if (material.type != Material_Glass) return intersection;

        // Gestion du matériau verre avec une seconde intersection
        Plane plane(intersection.intersection, intersection.normal);
        Ray refractedRay = plane.getRefractedRay(
            material, intersection.intersection, ray.direction());
        intersection.secondIntersection = refractedRay;
        return intersection;
    }
};

#endif
