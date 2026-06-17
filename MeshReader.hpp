//
// Created by nolan on 24/03/2026.
//

#ifndef TIPE_RAYTRACING_GPU_MESHREADER_HPP
#define TIPE_RAYTRACING_GPU_MESHREADER_HPP

#include <array>

#include "Vec3.hpp"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include "Material.hpp"
#include "Objects.hpp"

// Découpe une chaine de caractères avec des coupures au niveau des caractères delimiter
inline std::vector<std::string> splitString(const std::string& str, const char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Interface fichier .obj / scène pour les meshs
struct MeshReader {
    std::vector<Vec3> points; // Vertices du mesh
    std::vector<Vec3> normals; // Vecteurs normals du mesh
    std::vector<std::array<int, 4>> triangles; // Triangles du mesh
    Material material; // Matériau du mesh

    bool rotation = false; // Inidique s'il y a rotation du mesh
    float tetax = 0; // Angle de rotation autour de l'axe des abscisses
    float tetay = 0; // Angle de rotation autour de l'axe des ordonnées
    float tetaz = 0; // Angle de rotation autour de l'axe des côtes

    // Initialisation en lisant le fichier .obj sans rotation
    MeshReader(const std::string &source_file, Material mat): material(mat) {
        std::ifstream fin(source_file, std::ios::in);
        if (fin.is_open()) {
            std::string line;
            while (std::getline(fin, line)) {
                std::stringstream ss(line);
                std::string mode;
                ss >> mode;

                if (mode == "v") {
                    float x, y, z;
                    ss >> x >> y >> z;
                    points.emplace_back(x, y, z);
                }

                if (mode == "vn") {
                    float x, y, z;
                    ss >> x >> y >> z;
                    normals.emplace_back(x, y, z);
                }

                if (mode == "f") {
                    std::array<int, 4> tr{-1, -1, -1, -1};
                    for (int i = 0; i < 3; i++) {
                        std::string som;
                        ss >> som;
                        std::vector<std::string> vs = splitString(som, '/');
                        tr[i] = std::stoi(vs[0])-1;
                        if (vs.size() > 1) tr[3] = std::stoi(vs[2])-1;
                    }
                    triangles.emplace_back(tr);
                }
            }
        }
        else {
            std::cout << source_file << std::endl;
            std::cout << "Could not open the mesh file" << std::endl;
        }
        fin.close();
    }

    // Initialisation en lisant le fichier .obj avec rotation
    MeshReader(const std::string &source_file, Material mat, float tetax, float tetay, float tetaz): material(mat), rotation(true), tetax(tetax), tetay(tetay), tetaz(tetaz) {
        std::ifstream fin(source_file, std::ios::in);
        if (fin.is_open()) {
            std::string line;
            while (std::getline(fin, line)) {
                std::stringstream ss(line);
                std::string mode;
                ss >> mode;

                if (mode == "v") {
                    float x, y, z;
                    ss >> x >> y >> z;
                    points.emplace_back(x, y, z);
                }

                if (mode == "vn") {
                    float x, y, z;
                    ss >> x >> y >> z;
                    normals.emplace_back(x, y, z);
                }

                if (mode == "f") {
                    std::array<int, 4> tr{-1, -1, -1, -1};
                    for (int i = 0; i < 3; i++) {
                        std::string som;
                        ss >> som;
                        std::vector<std::string> vs = splitString(som, '/');
                        tr[i] = std::stoi(vs[0])-1;
                        if (vs.size() > 1) tr[3] = std::stoi(vs[2])-1;
                    }
                    triangles.emplace_back(tr);
                }
            }
        }
        else {
            std::cout << source_file << std::endl;
            std::cout << "Could not open the mesh file" << std::endl;
        }
        fin.close();
    }

    // Ajoute les triangles du mesh dans la scène
    void convert(std::vector<Object> &world) const {
        world.reserve(triangles.size());
        std::cout << "Number of vertices : " << points.size() << std::endl;
		std::cout << "Number of triangles : " << triangles.size() << std::endl;
        for (const auto tr_brack: triangles) {
            auto s1 = points[tr_brack[0]];
            auto s2 = points[tr_brack[1]];
            auto s3 = points[tr_brack[2]];
            if (tr_brack[3] != -1) {
                auto n = normals[tr_brack[3]];
                if (!rotation) world.emplace_back(create_triangle(s1, s2, s3, n, material));
                else world.emplace_back(create_triangle(s1, s2, s3, n, material, tetax, tetay, tetaz));
            }
            else if (!rotation) world.emplace_back(create_triangle(s1, s2, s3, p_vect(s3 - s1, s2 - s1), material));
            else world.emplace_back(create_triangle(s1, s2, s3, p_vect(s3 - s1, s2 - s1), material, tetax, tetay, tetaz));
        }
    }

    // Retourne le nombre de triangles qui composent le mesh
    int nb_triangles() const {return triangles.size();};

    // Retourne le nombre de vertices qui composent le mesh
    int nb_vertices() const {return points.size();};
};

#endif //TIPE_RAYTRACING_GPU_MESHREADER_HPP