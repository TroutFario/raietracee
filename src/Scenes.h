#ifndef SCENES_H
#define SCENES_H

#include "Scene.h"

class Scenes {
   public:
    static void setup_single_sphere(Scene& scene) {
        scene.clearScene();

        {
            scene.addLight(Vec3(-5, 5, 5), 2.5f, 2.f, LightType_Spherical, Vec3(1), false);
        }
        {
            // first sphere
            Sphere s;
            s.m_center = Vec3(1., 0., 0.);
            s.m_radius = 1.f;
            s.build_arrays();
            s.material.diffuse_material = Vec3(1., 0., 0.);
            s.material.specular_material = Vec3(0.2);
            s.material.shininess = 20;
            scene.addSphere(s);

            // second sphere
            Sphere s2;
            s2.m_center = Vec3(-1., 0., 0.);
            s2.m_radius = 1.f;
            s2.build_arrays();
            s2.material.diffuse_material = Vec3(0., 1., 0.);
            s2.material.specular_material = Vec3(0.2);
            s2.material.shininess = 20;
            scene.addSphere(s2);
        }
    }

    static void setup_single_square(Scene& scene) {
        scene.clearScene();

        {
            scene.addLight(Vec3(-5, 5, 5), 2.5f, 2.f, LightType_Spherical, Vec3(1), false);
        }

        {
            Square s;
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.build_arrays();
            s.updateQuad();
            s.material.diffuse_material = Vec3(1, 1, 0);
            s.material.specular_material = Vec3(0.8);
            s.material.shininess = 20;
            ImageRGB img;
            load_ppm(img, "img/bernard.ppm");
            s.material.type = Material_Texture;
            TextureMap texture;
            texture.image = img;
            texture.wrapMode = SquareTexture_Clamp;
            texture.mirrorMode = ImageMirror_Vertical;
            s.setTexture(texture);
            scene.addSquare(s);
        }
    }

    static void setup_cornell_box(Scene& scene) {
        scene.clearScene();

        {  // Light
            scene.addLight(Vec3(0.0, 1.5, 0.0), 2.5f, 2.f, LightType_Spherical,
                           Vec3(1, 1, 1), false);
        }

        {  // Back Wall
            Square s;
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.scale(Vec3(2., 2., 1.));
            s.translate(Vec3(0., 0., -2.));
            s.updateQuad();
            s.build_arrays();
            s.material.ambient_material = Vec3(1., 0., 1.);
            s.material.diffuse_material = Vec3(1., 0., 1.);
            s.material.specular_material = Vec3(1., 1., 1.);
            s.material.shininess = 16;
            ImageRGB img;
            load_ppm(img, "img/bernard.ppm");
            s.material.type = Material_Texture;
            TextureMap texture;
            texture.image = img;
            texture.wrapMode = SquareTexture_Cover;
            texture.mirrorMode = ImageMirror_Vertical;
            s.setTexture(texture);
            scene.addSquare(s);
        }

        {  // Left Wall
            Square s;
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.scale(Vec3(2., 2., 1.));
            s.translate(Vec3(0., 0., -2.));
            s.rotate_y(90);
            s.updateQuad();
            s.build_arrays();
            s.material.ambient_material = Vec3(1., 0., 0.);
            s.material.diffuse_material = Vec3(1., 0., 0.);
            s.material.specular_material = Vec3(1., 0., 0.);
            s.material.shininess = 16;
            scene.addSquare(s);
        }

        {  // Right Wall
            Square s;
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_y(-90);
            s.updateQuad();
            s.build_arrays();
            s.material.ambient_material = Vec3(0., 1.0, 0.);
            s.material.diffuse_material = Vec3(0.0, 1.0, 0.0);
            s.material.specular_material = Vec3(0.0, 1.0, 0.0);
            s.material.shininess = 16;
            scene.addSquare(s);
        }

        {  // Floor
            Square s;
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_x(-90);
            s.updateQuad();
            s.build_arrays();
            s.material.ambient_material = Vec3(1.0, 1.0, 1.0);
            s.material.diffuse_material = Vec3(1.0, 1.0, 1.0);
            s.material.specular_material = Vec3(1.0, 1.0, 1.0);
            s.material.shininess = 16;
            scene.addSquare(s);
        }

        {  // Ceiling
            Square s;
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_x(90);
            s.updateQuad();
            s.build_arrays();
            s.material.ambient_material = Vec3(1.0, 1.0, 0.);
            s.material.diffuse_material = Vec3(1.0, 1.0, 0.);
            s.material.specular_material = Vec3(1.0, 1.0, 1.0);
            s.material.shininess = 16;
            scene.addSquare(s);
        }

        {  // Front Wall
            Square s;
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_y(180);
            s.updateQuad();
            s.build_arrays();
            s.material.diffuse_material = Vec3(.0, 1.0, 1.0);
            s.material.specular_material = Vec3(.0, 1.0, 1.0);
            s.material.shininess = 16;
            scene.addSquare(s);
        }

        {  // MIRRORED Sphere
            Sphere s;
            s.m_center = Vec3(1.0, -1.25, 0.5);
            s.m_radius = 0.75f;
            s.build_arrays();
            s.material.type = Material_Mirror;
            s.material.diffuse_material = Vec3(1., 0., 0.);
            s.material.specular_material = Vec3(1., 0., 0.);
            s.material.shininess = 16;
            s.material.transparency = 0.;
            s.material.index_medium = 0.;
            scene.addSphere(s);
        }

        {  // GLASS Sphere
            Sphere s;
            s.m_center = Vec3(-1.0, -1.25, -0.5);
            s.m_radius = 0.75f;
            s.build_arrays();
            s.material.type = Material_Glass;
            s.material.diffuse_material = Vec3(0., 0., 1.);
            s.material.specular_material = Vec3(1., 1., 1.);
            s.material.shininess = 16;
            s.material.transparency = 1.0;
            s.material.index_medium = 1.5;
            scene.addSphere(s);
        }

        {  // Mesh
            Mesh mesh;
            mesh.openOFF("assets/elephant_n.off", true, 2);
            mesh.material.type = Material_Glass;
            mesh.material.diffuse_material = Vec3(1., 0., 0.);
            mesh.material.specular_material = Vec3(1., 0., 0.);
            mesh.material.shininess = 16;
            mesh.material.transparency = 1.0;
            mesh.material.index_medium = 1.8;
            mesh.build_arrays();
            scene.addMesh(mesh);
        }
    }

    static void setup_mesh_scene(Scene& scene) {
        scene.clearScene();

        {
            scene.addLight(Vec3(-5, 5, 5), 2.5f, 2.f, LightType_Spherical, Vec3(1), false);
        }

        {
            Mesh mesh;
            mesh.openOFF("assets/elephant_n.off", true, 3);
            mesh.material.diffuse_material = Vec3(1., 0., 0.);
            mesh.material.specular_material = Vec3(1., 0., 0.);
            mesh.material.shininess = 16;
            mesh.build_arrays();
            scene.addMesh(mesh);
        }
    }
};

#endif
