//
// Created by nolan on 01/03/2026.
//

#ifndef TIPE_RAYTRACING_GPU_RAY_HPP
#define TIPE_RAYTRACING_GPU_RAY_HPP

#include "Vec3.hpp"

// Rayon de lumière
struct Ray {
    Point3 origin; // Origine du rayon
    Vec3 direction; // Vecteur directeur du rayon

    Ray();

    Ray(Point3 origin, Vec3 direction);

    // Retourne le point de paramètre t sur le rayon
    [[nodiscard]] Point3 at(float t) const;
};

#pragma acc routine seq
inline Ray::Ray(): origin(Vec3(0, 0, 0)), direction(Vec3(0, 0, -1)) {}

#pragma acc routine seq
inline Ray::Ray(Point3 origin, Vec3 direction): origin(origin), direction(direction) {}

#pragma acc routine seq
[[nodiscard]] inline Point3 Ray::at(float t) const {
    return origin + t * direction;
}

#endif //TIPE_RAYTRACING_GPU_RAY_HPP