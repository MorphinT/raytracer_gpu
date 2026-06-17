//
// Created by nolan on 20/05/2026.
//

#ifndef TIPE_OBN_HPP
#define TIPE_OBN_HPP

#include "Vec3.hpp"

// Repère orthonormé local
struct OBN {
    Vec3 u, v, w; // Vecteurs de la base

    void build_from_v(const Vec3 &normal);
    Vec3 local(const Vec3 &v);
};

// Construit une base locale à partir du vecteur "vertical"
#pragma acc routine seq
void OBN::build_from_v(const Vec3 &normal) {
    v = normal;

    Vec3 a = (fabsf(v.x) > 0.9f) ? Vec3(0, 0, 1) : Vec3(1, 0, 0);

    u = normalised(p_vect(v,a));
    w = p_vect(v, u);
}

// Retourne le vecteur de la base locale a avec les coordonnées de la scène
#pragma acc routine seq
Vec3 OBN::local(const Vec3 &a) {
    return u*a.x + v*a.y + w*a.z;
}

#endif //TIPE_OBN_HPP