#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include "Camera.hpp"
#include "Objects.hpp"
#include "Scenes.hpp"
#include <cstdlib>

using namespace std;

int classic(int num_scene) {
    const auto start = chrono::high_resolution_clock::now();

    vector<Object> world;

    float im_ratio = 1.f; // Résolution de l'image
    int im_width = 1024; // Largeur de l'image
    Camera cam(im_ratio, im_width);

    string title;

	bool BVH = true;

    switch (num_scene) {
        case 0: lambertian_example(cam, world, title, BVH); break;
        case 1: metal_example(cam, world, title, BVH); break;
        case 2: dielectric_example(cam, world, title, BVH); break;
        case 3: light_example(cam, world, title, BVH); break;
        case 4: sphere_field_demo(cam, world, title, BVH); break;
        case 5: empty_cornell_box(cam, world, title, BVH); break;
        case 6: lambertianCube(cam, world, title, BVH); break;
        case 7: testMesh(cam, world, title, BVH); break;
        case 8: meshInLight(cam, world, title, BVH); break;
        default: demoNSI(cam, world, title, BVH); break;
    }


    vector<Color> frameBuffer = render(cam, world, BVH);

    ofstream fout(title);
    write_result(fout, cam, frameBuffer);

    const auto end = chrono::high_resolution_clock::now();
    const auto total_duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << "Execution time: " << total_duration.count() << " milliseconds" << endl;

    return 0;
}

int main() {
    return classic(5);
}