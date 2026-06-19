//
// Created by nolan on 01/03/2026.
//

#ifndef TIPE_RAYTRACING_GPU_RAYTRACER_HPP
#define TIPE_RAYTRACING_GPU_RAYTRACER_HPP

#include <random>
#include <fstream>
#include "Ray.hpp"
#include "Camera.hpp"
#include "Objects.hpp"

// Calcul la couleur du rayon ray avec BVH actif
#pragma acc routine seq
inline Color rayColorBVH(const Ray &ray, Object objects[], const BVHNode *nodes, RNG &rng, const Camera &camera, int root) {
	Ray current_ray = ray;
	Color attenuation(1, 1, 1);
	Color accumulation(0, 0, 0);

	// Déroulé sans récursivité de l'algorithme de raytracing
	for (int depth = 0; depth < camera.max_depth; depth++) {
		HitRecord rec{};

		// Retour de la couleur de l'arrière-plan si le rayon n'intersecte aucun objet
		if (!traverse_bvh(current_ray, nodes, objects, root, rec)) {
			return accumulation + attenuation * background_color(camera, current_ray.direction);
		}

		// Actualisation de la couleur en fonction du matériau de l'objet intersecté
		Color fac_attenuation(1, 1, 1);
		if (!rec.mat.scatter(current_ray, rec, fac_attenuation, current_ray, rng)) {
			return accumulation + rec.mat.emitted() * attenuation;
		}

		accumulation += rec.mat.emitted() * attenuation;
		attenuation *= fac_attenuation;
	}

	// On retourne la couleur accumulée
	return accumulation;
}

// Calcul la couleur du rayon ray sans BVH
#pragma acc routine seq
inline Color rayColorLin(const Ray &ray, Object objects[], RNG &rng, const Camera &camera, int nb_objects) {
	Ray current_ray = ray;
	Color attenuation(1, 1, 1);
	Color accumulation(0, 0, 0);

	// Déroulé sans récursivité de l'algorithme de raytracing
	for (int depth = 0; depth < camera.max_depth; depth++) {
		HitRecord rec{};

		// Retour de la couleur de l'arrière-plan si le rayon n'intersecte aucun objet
		if (!traverse(current_ray, objects, nb_objects, rec)) {
			return accumulation + attenuation * background_color(camera, current_ray.direction);
		}

		// Actualisation de la couleur en fonction du matériau de l'objet intersecté
		Color fac_attenuation(0, 0, 0);
		if (!rec.mat.scatter(current_ray, rec, fac_attenuation, current_ray, rng)) {
			return accumulation + rec.mat.emitted() * attenuation;
		}

		accumulation += rec.mat.emitted() * attenuation;
		attenuation *= fac_attenuation;
	}

	// On retourne la couleur accumulée
	return accumulation;
}

// Écrit l'image dans la sortie au format .ppm (version P3)
inline void write_result(std::ofstream &fout, const Camera &cam, const std::vector<Color> &framebuffer) {
    fout <<"P3\n" << cam.im_width << " " << cam.im_height << "\n255\n";
    for (int j = 0; j < cam.im_height; j++) {
        for (int i = 0; i < cam.im_width; i++) {
        	// Récupération de la couleur calculée pour le pixel de coordonnées (i, j) dans l'image
            auto color = framebuffer[j * cam.im_width + i];

        	// Conversion des couleurs d'une échelle linéaire à une échelle gamma
            static const Interval intensity(0., 0.999);
            const float x = linear_to_gamma(color.x);
            const float y = linear_to_gamma(color.y);
            const float z = linear_to_gamma(color.z);

        	// Calcul des composantes du code rgb de la couleur du pixel
            const int red_byte = static_cast<int>(256 * intensity.clamp(x));
            const int green_byte = static_cast<int>(256 * intensity.clamp(y));
            const int blue_byte = static_cast<int>(256 * intensity.clamp(z));

        	// Écriture de la couleur du pixel dans le fichier
            fout << red_byte << " " << green_byte << " " << blue_byte << std::endl;
        }
    }
}

// Lance le calcul de l'image et retourne le résultat sous forme d'un frameBuffer
inline std::vector<Color> render(Camera &cam, std::vector<Object> &world, bool BVH = true) {
    // Récupération de la liste d'objets sous forme d'un tableau
	Object *objects = world.data();
    int object_count = world.size();

	// Procédure d'initialisation avant rendu de la caméra
    cam.initialize();

	// Création du vecteur des couleurs des pixels et récupération du tableau de celles-ci
    std::vector<Color> frameBuffer(cam.im_width * cam.im_height);
    Color *fb = frameBuffer.data();

	// Choix de l'utilisation ou non de l'algorithme BVH
	if (BVH) {
		// Création du vecteur qui doit contenir les nœuds BVH
		std::vector<BVHNode> nodes;
    	nodes.reserve(2*world.size());

		// Initialisation de l'arbre BVH
    	std::cout << "Initialisation of the BVH structure" << std::endl;
    	int root = build_bvh(world, nodes, 0, world.size());

		// Récupération du tableau de node BVH
    	BVHNode *bvh_nodes = nodes.data();
    	int nodes_size = nodes.size();

		// Parcours parallélisé des pixels et calcul de leur couleur
		std::cout << "Calculating the image..." << std::endl;
    	#pragma acc data copyin(objects[0:object_count], bvh_nodes[0:nodes_size], cam) copyout(fb[0:cam.im_width * cam.im_height])
    	{
    	#pragma acc parallel loop collapse(2)
        for (int j = 0; j < cam.im_height; j++)
        	for (int i = 0; i < cam.im_width; i++)
            {
        		// Initialisation d'un générateur d'aléatoire avec une graine spécifique au pixel
            	RNG rng;
            	unsigned int seed = (j * 73856093) ^ (i * 19349663);
            	rng.state = wang_hash(seed);
            	rng.state = wang_hash(rng.state);

        		// Lancé des rayons pour le calcul de la couleur du pixel
            	Color pixel(0, 0, 0);
            	for (int s = 0; s < cam.samples_per_pixel; s++) {
            	    Ray ray = cam.getRay(i, j, rng);
            	    Color col = rayColorBVH(ray, objects, bvh_nodes, rng, cam, root);
            	    pixel += col;
            	}
        		// Enregistrement de la couleur calculée dans le framebuffer
            	fb[j * cam.im_width + i] = pixel * (1.0f/(float)cam.samples_per_pixel);
        	}
		}
	}
	else {
		// Parcours parallélisé des pixels et calcul de leur couleur
		std::cout << "Calculating the image..." << std::endl;
		#pragma acc data copyin(objects[0:object_count], cam) copyout(fb[0:cam.im_width * cam.im_height])
    	{
    	#pragma acc parallel loop collapse(2)
    	for (int j = 0; j < cam.im_height; j++)
        	for (int i = 0; i < cam.im_width; i++)
        	{
        		// Initialisation d'un générateur d'aléatoire avec une graine spécifique au pixel
            	RNG rng;
            	unsigned int seed = (j * 73856093) ^ (i * 19349663);
            	rng.state = wang_hash(seed);
            	rng.state = wang_hash(rng.state);

        		// Lancé des rayons pour le calcul de la couleur du pixel
            	Color pixel(0, 0, 0);
            	for (int s = 0; s < cam.samples_per_pixel; s++) {
            	    Ray ray = cam.getRay(i, j, rng);
            	    Color col = rayColorLin(ray, objects, rng, cam, object_count);
            	    pixel += col;
            	}
        		// Enregistrement de la couleur calculée dans le framebuffer
            	fb[j * cam.im_width + i] = pixel * (1.0f/(float)cam.samples_per_pixel);
        	}
		}
	}

    return frameBuffer;
}

#endif //TIPE_RAYTRACING_GPU_RAYTRACER_HPP