#include "Mesh.h"

#include <fstream>
#include <iostream>

void Mesh::openOFF(const std::string& filename, bool load_normals,
                   float scale) {
    std::ifstream myfile(filename.c_str());
    if (!myfile.is_open()) {
        std::cout << filename << " cannot be opened" << std::endl;
        return;
    }

    std::string magic_s;
    myfile >> magic_s;

    if (magic_s != "OFF") {
        std::cout << magic_s << " != OFF : We handle ONLY *.off files."
                  << std::endl;
        myfile.close();
        return;
    }

    int n_vertices, n_faces, dummy_int;
    myfile >> n_vertices >> n_faces >> dummy_int;

    vertices.clear();
    triangles.clear();

    // --- Lecture des sommets ---
    for (int v = 0; v < n_vertices; ++v) {
        float x, y, z;
        Vec3 position, normal(0.f, 0.f, 0.f);

        myfile >> x >> y >> z;
        x *= scale;
        y *= scale;
        z *= scale;
        position = Vec3(x, y, z);

        if (load_normals) {
            myfile >> x >> y >> z;
            normal = Vec3(x, y, z);
        }

        vertices.push_back(MeshVertex(position, normal));
    }

    // --- Lecture des faces ---
    for (int f = 0; f < n_faces; ++f) {
        int n_vertices_on_face;
        myfile >> n_vertices_on_face;

        if (n_vertices_on_face == 3) {
            unsigned int v0, v1, v2;
            myfile >> v0 >> v1 >> v2;
            triangles.push_back(MeshTriangle(v0, v1, v2));
        } else if (n_vertices_on_face == 4) {
            unsigned int v0, v1, v2, v3;
            myfile >> v0 >> v1 >> v2 >> v3;

            // Quad → 2 triangles
            triangles.push_back(MeshTriangle(v0, v1, v2));
            triangles.push_back(MeshTriangle(v0, v2, v3));
        } else {
            std::cout
                << "We handle ONLY *.off files with 3 or 4 vertices per face"
                << std::endl;
            myfile.close();
            return;
        }
    }

    myfile.close();

    // Si les normales ne sont pas dans le fichier OFF
    if (!load_normals) {
        recomputeNormals();
    }

    build_arrays();
}

void Mesh::recomputeNormals() {
    for (unsigned int i = 0; i < vertices.size(); i++)
        vertices[i].normal = Vec3(0.0f, 0.0f, 0.0f);

    for (unsigned int i = 0; i < triangles.size(); i++) {
        const Vec3& p0 = vertices[triangles[i].v[0]].position;
        const Vec3& p1 = vertices[triangles[i].v[1]].position;
        const Vec3& p2 = vertices[triangles[i].v[2]].position;

        Vec3 e01 = p1 - p0;
        Vec3 e02 = p2 - p0;

        Vec3 faceNormal = Vec3::cross(e01, e02);

        vertices[triangles[i].v[0]].normal += faceNormal;
        vertices[triangles[i].v[1]].normal += faceNormal;
        vertices[triangles[i].v[2]].normal += faceNormal;
    }

    for (unsigned int i = 0; i < vertices.size(); i++)
        vertices[i].normal.normalize();
}

void Mesh::centerAndScaleToUnit() {
    Vec3 c(0, 0, 0);
    for (unsigned int i = 0; i < vertices.size(); i++)
        c += vertices[i].position;
    c /= vertices.size();
    float maxD = (vertices[0].position - c).length();
    for (unsigned int i = 0; i < vertices.size(); i++) {
        float m = (vertices[i].position - c).length();
        if (m > maxD) maxD = m;
    }
    for (unsigned int i = 0; i < vertices.size(); i++)
        vertices[i].position = (vertices[i].position - c) / maxD;
}
