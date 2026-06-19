//
// Created by nolan on 07/03/2026.
//

#ifndef TIPE_RAYTRACING_GPU_SCENES_HPP
#define TIPE_RAYTRACING_GPU_SCENES_HPP

#include "Raytracer.hpp"
#include "MeshReader.hpp"
#include <string>
#include <vector>

// Exemple de lambertien
inline void lambertian_example(Camera &cam, std::vector<Object> &world, std::string &title, bool &BVH) {
    const auto ground_material = create_lambertian(Color(0.39, 0.43, 0.78));
    world.push_back(create_sphere(Point3(0,-1000,0), 1000, ground_material));

    const auto c1 = Color(0.42, 0.24, 0.267);
    const auto c2 = Color(0.33, 0.6, 0.2);
    const auto material0 = create_lambertian(c1);
    const auto material1 = create_lambertian(0.5*c1+0.5*c2);
    const auto material2 = create_lambertian(c2);
    world.push_back(create_sphere(Point3(-1.3,0.65,0.5), 0.6, material0));
    world.push_back(create_sphere(Point3(0.,0.65,0.5), 0.6, material1));
    world.push_back(create_sphere(Point3(+1.3,0.65,0.5), 0.6, material2));

    cam.center = Point3(0., 0.5, -2.);
    cam.look_direction = Vec3(0., 0., 1.);

    cam.samples_per_pixel = 1000;
    cam.max_depth = 20;

    cam.v_fov = 60;
    cam.up = Vec3(0,1,0);

    cam.defocus_angle = 0.;
    cam.focus_dist = 10.;

    cam.bg_type = BG_GRADIENT;

	BVH = false;

    title = "images/3LambertiansExample.ppm";
}

// Exemple de métal
inline void metal_example(Camera &cam, std::vector<Object> &world, std::string &title, bool &BVH) {
    const auto ground_material = create_lambertian(Color(0.39, 0.43, 0.78));
    world.push_back(create_sphere(Point3(0,-1000,0), 1000, ground_material));

    const auto c1 = Color(0.8, 0.8, 0.8);
    const auto c2 = Color(0.2, 0.2, 0.2);
    const auto material0 = create_metal(c1, 0.);
    const auto material1 = create_metal(0.5 * c1 + 0.5 * c2, 0.4);
    const auto material2 = create_metal(c2, 0.8);
    world.push_back(create_sphere(Point3(-1.3,0.65,0.5), 0.6, material0));
    world.push_back(create_sphere(Point3(0.,0.65,0.5), 0.6, material1));
    world.push_back(create_sphere(Point3(+1.3,0.65,0.5), 0.6, material2));

    cam.center = Point3(0., 0.5, -2.);
    cam.look_direction = Vec3(0., 0., 1.);

    cam.samples_per_pixel = 10000;
    cam.max_depth = 10;

    cam.v_fov = 60;
    cam.up = Vec3(0,1,0);

    cam.defocus_angle = 0.;
    cam.focus_dist = 10.;

    cam.bg_type = BG_GRADIENT;

	BVH = true;

    title = "images/MetalExample.ppm";
}

// Exemple de dielectrique
inline void dielectric_example(Camera &cam, std::vector<Object> &world, std::string &title, bool &BVH) {
    const auto ground_material = create_lambertian(Color(0.39, 0.43, 0.78));
    world.push_back(create_sphere(Point3(0,-1000,0), 1000, ground_material));

    const auto glass = create_dielectric(1.5);
    const auto air_in_glass = create_dielectric(1./1.5);
    const auto air_in_water = create_dielectric(1./1.33);
    world.push_back(create_sphere(Point3(-0.85,0.85,0.5), 0.8, glass));
    world.push_back(create_sphere(Point3(-0.85,0.85,0.5), 0.6, air_in_glass));
    world.push_back(create_sphere(Point3(+0.85,0.85,0.5), 0.8, air_in_water));

    cam.center = Point3(0., 0.5, -2.);
    cam.look_direction = Vec3(0., 0., 1.);

    cam.samples_per_pixel = 10000;
    cam.max_depth = 10;

    cam.v_fov = 80;
    cam.up = Vec3(0,1,0);

    cam.defocus_angle = 0.;
    cam.focus_dist = 10.;

    cam.bg_type = BG_GRADIENT;

	BVH = true;

    title = "images/DielectricExample.ppm";
}

// Example de lumière
inline void light_example(Camera &cam, std::vector<Object> &world, std::string &title, bool &BVH) {
    const auto ground_material = create_lambertian(Color(0.39, 0.43, 0.78));
   	world.push_back(create_sphere(Point3(0,-100,0), 99.1, ground_material));

	const auto light_material = create_diffuse_light(Color(1.0, 1.0, 1.0));
	world.push_back(create_quad(Point3(1.1,-0.9,1), Vec3(0.,2,0), Vec3(0,0,-2), light_material));

    const auto grey = create_lambertian(Color(0.42, 0.24, 0.267));
    world.push_back(create_sphere(Point3(0.2,-0.2,-0.3), 0.7, grey));

    cam.center = Point3(0, 0, 4);
    Point3 look_point = Point3(0.45, 0, 0);
    cam.look_direction = look_point - cam.center;

    cam.samples_per_pixel = 10000;
    cam.max_depth = 50;

    cam.v_fov = 60;
    cam.up = Vec3(0, 1, 0);

    cam.defocus_angle = 0.;
    cam.focus_dist = 10.;

    cam.bg_type = BG_STRONG_GREY;
    init_sky(0.005f);
    #pragma acc update device(sky[0:64000000])

	BVH = true;

    title = "images/LightExample.ppm";
}

// Démonstration avec un champ de sphère
inline void sphere_field_demo(Camera &cam, std::vector<Object> &world, std::string &title, bool &BVH) {
    auto ground_material = create_lambertian(Color(0.5, 0.5, 0.5));
    world.push_back(create_sphere(Point3(0,-1000,0), 1000, ground_material));

    RNG rng;
    rng.state = 4534135;
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = rng.next_uniform();
            Point3 center((float)a + 0.9f*rng.next_uniform(), 0.2f, (float)b + 0.9f*rng.next_uniform());

            if ((center - Point3(4, 0.2f, 0)).length() > 0.9) {

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = Color::random(rng) * Color::random(rng);
                    Material sphere_material = create_lambertian(albedo);
                    world.push_back(create_sphere(center, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = Color::random(rng, 0.5, 1);
                    auto fuzz = rng.next_uniform(0, 0.5);
                    Material sphere_material = create_metal(albedo, fuzz);
                    world.push_back(create_sphere(center, 0.2, sphere_material));
                } else {
                    // glass
                    Material sphere_material = create_dielectric(1.5);
                    world.push_back(create_sphere(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = create_dielectric(1.5);
    world.push_back(create_sphere(Point3(0, 1, 0), 1.0, material1));

    auto material2 = create_lambertian(Color(0.4, 0.2, 0.1));
    world.push_back(create_sphere(Point3(-4, 1, 0), 1.0, material2));

    auto material3 = create_metal(Color(0.7, 0.6, 0.5), 0.0);
    world.push_back(create_sphere(Point3(4, 1, 0), 1.0, material3));

    cam.center = Point3(13, 2., 3.);
    cam.look_direction = Vec3(-13., -2., -3.);

    cam.samples_per_pixel = 100;
    cam.max_depth = 50;

    cam.v_fov = 20;
    cam.up = Vec3(0,1,0);

    cam.defocus_angle = 0.2;
    cam.focus_dist = 10.0;

    cam.bg_type = BG_WHITE;

	BVH = false;

    title = "images/Sphere_field_demo.ppm";
}

// Cornell-box vide
inline void empty_cornell_box(Camera &cam, std::vector<Object> &world, std::string &title, bool &BVH) {
    // Materials
    auto red = create_lambertian(Color(0.65, 0.05, 0.05));
    auto white = create_lambertian(Color(0.73, 0.73, 0.73));
    auto green = create_lambertian(Color(0.12, 0.45, 0.15));
    auto light = create_diffuse_light(Color(1.0, 1.0, 1.0));

    // Objects
    world.push_back(create_quad(Point3(555,0,0), Vec3(0,555,0), Vec3(0,0,555), green));
    world.push_back(create_quad(Point3(0,0,0), Vec3(0,555,0), Vec3(0,0,555), red));
    world.push_back(create_quad(Point3(178, 554, 300), Vec3(200,0,0), Vec3(0,0,140), light));
    world.push_back(create_quad(Point3(0,0,0), Vec3(555,0,0), Vec3(0,0,555), white));
    world.push_back(create_quad(Point3(555,555,555), Vec3(-555,0,0), Vec3(0,0,-555), white));
    world.push_back(create_quad(Point3(0,0,555), Vec3(555,0,0), Vec3(0,555,0), white));
    world.push_back(create_quad(Point3(0,0,0), Vec3(555,0,0), Vec3(0,555,0), white));

    cam.center = Point3(278,278,1);
    cam.look_direction = Vec3(0., 0., 800);

    cam.samples_per_pixel = 10000;
    cam.max_depth = 50;

    cam.v_fov = 90;
    cam.up = Vec3(0,1,0);

    cam.defocus_angle = 0.;
    cam.focus_dist = 10.;

    cam.bg_type = BG_BLACK;

	BVH = true;

    title = "images/empty_cornell_box.ppm";
}

// Cornell-box avec un cube de sphères de lambertiens
inline void lambertianCube(Camera &cam, std::vector<Object> &world, std::string &title, bool &BVH) {
    // Materials
    auto red = create_lambertian(Color(0.65, 0.05, 0.05));
    auto white = create_lambertian(Color(0.73, 0.73, 0.73));
    auto green = create_lambertian(Color(0.12, 0.45, 0.15));
    auto light = create_diffuse_light(Color(1.0, 1.0, 1.0));

    // Objects
    world.push_back(create_quad(Point3(555,0,0), Vec3(0,555,0), Vec3(0,0,555), green));
    world.push_back(create_quad(Point3(0,0,0), Vec3(0,555,0), Vec3(0,0,555), red));
    world.push_back(create_quad(Point3(400, 554, 409), Vec3(-235,0,0), Vec3(0,0,-250), light));
    world.push_back(create_quad(Point3(0,0,0), Vec3(555,0,0), Vec3(0,0,555), white));
    world.push_back(create_quad(Point3(555,555,555), Vec3(-555,0,0), Vec3(0,0,-555), white));
    world.push_back(create_quad(Point3(0,0,555), Vec3(555,0,0), Vec3(0,555,0), white));

    auto blue = create_lambertian(Color(0.17, 0.20, 0.57));
    RNG rng;
    rng.state = 764115099;
    int ns = 1000;
    world.reserve(ns);
    for (int i=0; i < ns; i++) {
        world.push_back(create_sphere(Point3::random(rng, 150, 400), 10, blue));
    }

    cam.center = Point3(278,278,-800);
    cam.look_direction = Vec3(0., 0., 800);

    cam.samples_per_pixel = 1000;
    cam.max_depth = 50;

    cam.v_fov = 35;
    cam.up = Vec3(0,1,0);

    cam.defocus_angle = 0.;
    cam.focus_dist = 10.;

    cam.bg_type = BG_GREY;

	BVH = true;

    title = "images/lambertianCube1000spheres.ppm";
}

// Cornell-box avec un cube de sphères de lambertiens
inline void lambertianCube(Camera &cam, std::vector<Object> &world, std::string &title, int nb_spheres) {
    // Materials
    auto red = create_lambertian(Color(0.65, 0.05, 0.05));
    auto white = create_lambertian(Color(0.73, 0.73, 0.73));
    auto green = create_lambertian(Color(0.12, 0.45, 0.15));
    auto light = create_diffuse_light(Color(1.0, 1.0, 1.0));

    // Objects
    world.push_back(create_quad(Point3(555,0,0), Vec3(0,555,0), Vec3(0,0,555), green));
    world.push_back(create_quad(Point3(0,0,0), Vec3(0,555,0), Vec3(0,0,555), red));
    world.push_back(create_quad(Point3(400, 554, 409), Vec3(-235,0,0), Vec3(0,0,-250), light));
    world.push_back(create_quad(Point3(0,0,0), Vec3(555,0,0), Vec3(0,0,555), white));
    world.push_back(create_quad(Point3(555,555,555), Vec3(-555,0,0), Vec3(0,0,-555), white));
    world.push_back(create_quad(Point3(0,0,555), Vec3(555,0,0), Vec3(0,555,0), white));

    auto blue = create_lambertian(Color(0.17, 0.20, 0.57));
    RNG rng;
    rng.state = 764183099;
    int ns = nb_spheres;
    world.reserve(ns);
    for (int i=0; i < ns; i++) {
        world.push_back(create_sphere(Point3::random(rng, 150, 400), 3, blue));
    }

    cam.center = Point3(278,278,-800);
    cam.look_direction = Vec3(0., 0., 800);

    cam.samples_per_pixel = 100;
    cam.max_depth = 50;

    cam.v_fov = 35;
    cam.up = Vec3(0,1,0);

    cam.defocus_angle = 0.;
    cam.focus_dist = 10.;

    cam.bg_type = BG_WHITE;

    title = "images/lambertianCube.ppm";
}

// Test mesh
inline void testMesh(Camera &cam, std::vector<Object> &world, std::string &title, bool &BVH) {
    const auto ground_material = create_lambertian(Color(225, 197, 123)/256);
   	world.push_back(create_sphere(Point3(-600,-10600, 0), 10000, ground_material)); // dragon 0 -100 0 99 lucy -600 -10600 0 10000 aspasia 0 -100 0 100.5

    const auto grey = create_lambertian(Color(0.42, 0.24, 0.267));

    const auto mesh = MeshReader("meshs/lucy.obj", grey, -90, 0, 180); // dragon 90 0 0 lucy -90 0 180 aspasia 0 90 0 autres rien
    std::cout << "Conversion of the mesh to a list of triangles" << std::endl;
    mesh.convert(world);

    cam.center = Point3(-600, 300, 1000); // lucy -600 300 1000 king 0 1 4.5 thinker 0 1 5 dragon 0 0.3 1 aspasia 0.5 10
    Point3 look_point = Point3(-600, 300, 0); // lucy -600 0 1000 king 0 1 0 thinker 0 0 0 dragon 0 0.1 0 aspasia 0 5.5 0
    cam.look_direction = look_point - cam.center;

    cam.samples_per_pixel = 1000;
    cam.max_depth = 50;

    cam.v_fov = 90; // lucy 90 king 30 thinker 20 dragon 20 aspasia 60
    cam.up = Vec3(0, 1, 0);

    cam.defocus_angle = 0.;
    cam.focus_dist = 10.;

    cam.bg_type = BG_GRADIENT;

	BVH = true;

    title = "images/test_lucy.ppm";
}

// Test mesh
inline int testDragon(Camera &cam, std::vector<Object> &world, std::string &title, int nb_triangles) {
    const auto ground_material = create_lambertian(Color(50, 106, 201)/256);
   	world.push_back(create_sphere(Point3(0,-100,0), 99, ground_material));

    const auto grey = create_lambertian(Color(0.42, 0.24, 0.267)); // dragon 0.42 0.24 0.267

    const auto mesh = MeshReader("meshs/dragon_" + std::to_string(nb_triangles) + ".obj", grey, 90, 0, 0);
    std::cout << "Conversion of the mesh to a list of triangles" << std::endl;
    mesh.convert(world);

    cam.center = Point3(0, 0.3, 1); // lucy 500 400 1000 king 0 1 4.5 thinker 0 1 5 dragon 0 0.3 1
    Point3 look_point = Point3(0, 0.1, 0); // lucy 500 400 1000 king 0 1 0 thinker 0 0 0 dragon 0 0.1 0
    cam.look_direction = look_point - cam.center;

    cam.samples_per_pixel = 100;
    cam.max_depth = 50;

    cam.v_fov = 20; // lucy 120 king 30 thinker 20
    cam.up = Vec3(0, 1, 0); // lucy 0 0 1 king 0 1 0 thinker 0 1 0

    cam.defocus_angle = 0.;
    cam.focus_dist = 10.;

    cam.bg_type = BG_GRADIENT;

    title = "images/test_mesh_dragon" + std::to_string(nb_triangles) + ".ppm";

    return mesh.nb_triangles();
}

inline void meshInLight(Camera &cam, std::vector<Object> &world, std::string &title, bool &BVH) {
    const auto ground_material = create_lambertian(Color(0.5, 0.5, 0.5));
   	world.push_back(create_sphere(Point3(0,-100,0), 99.1, ground_material));

	const auto light_material = create_diffuse_light(Color(1.0, 1.0, 1.0));
	world.push_back(create_quad(Point3(1.1,0,4), Vec3(0.,4,0), Vec3(0,0,-4), light_material));

    const auto grey = create_lambertian(Color(0.2, 0.2, 0.2));

    const auto mesh = MeshReader("meshs/king.obj", grey);
    std::cout << "Conversion of the mesh to a list of triangles" << std::endl;
    mesh.convert(world);

    cam.center = Point3(0, 1, 4.5); // lucy 500 400 1000 king 0 1 4.5 thinker 0 1 5 car
    Point3 look_point = Point3(0, 1, 0); // lucy 500 400 1000 king 0 1 0 thinker 0 0 0 car
    cam.look_direction = look_point - cam.center;

    cam.samples_per_pixel = 1000;
    cam.max_depth = 50;

    cam.v_fov = 30; // lucy 120 king 30 thinker 20 car
    cam.up = Vec3(0, 1, 0); // lucy 0 0 1 king 0 1 0 thinker 0 1 0 car

    cam.defocus_angle = 0.;
    cam.focus_dist = 10.;

    cam.bg_type = BG_NIGHT;
    init_sky(0.005f);
    #pragma acc update device(sky[0:64000000])

	BVH = true;

    title = "images/king_in_night.ppm";
}

inline void demoNSI(Camera &cam, std::vector<Object> &world, std::string &title, bool &BVH) {
    const auto ground_material = create_lambertian(Color(0.5, 0.5, 0.5));
    world.push_back(create_sphere(Point3(0,-1000,0), 1000, ground_material));

    const auto magenta = create_lambertian(Color(255, 0, 255));
    world.push_back(create_sphere(Point3(3,2,0), 2, magenta));

    const auto mir = create_metal(Color(0.2, 0.2, 0.2), 0.8);
    world.push_back(create_quad(Point3(0, 0, 3), Vec3(0, 0, -6), Vec3(0, 6, 0), mir));

    cam.center = Point3(10, 2, 0);
    cam.look_direction = Vec3(-6, 0, 0);

    cam.samples_per_pixel = 10000;
    cam.max_depth = 50;

    cam.v_fov = 60;
    cam.up = Vec3(0,1,0);

    cam.defocus_angle = 0.;
    cam.focus_dist = 10.;

    cam.bg_type = BG_GRADIENT;

	BVH = true;

    title = "images/demoNSI.ppm";
}

#endif //TIPE_RAYTRACING_GPU_SCENES_HPP