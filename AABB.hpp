//
// Created by nolan on 01/03/2026.
//

#ifndef TIPE_RAYTRACING_GPU_AABB_HPP
#define TIPE_RAYTRACING_GPU_AABB_HPP

#include "Interval.hpp"
#include "Ray.hpp"
#include "Vec3.hpp"

// Axis-Aligned Bounding Box
struct AABB {
    Interval ix{}, iy{}, iz{}; // Intervalles respectivement selon l'axe x, y, z

    AABB() = default;

    // AABB définie avec des intervalles pour chaque coordonnée
    AABB(const Interval &x, const Interval &y, const Interval &z): ix(x), iy(y), iz(z) {}

    // AABB définie à partir de deux points extrêmes
    AABB(const Point3 &a, const Point3 &b) {
        ix = a.x <= b.x ? Interval(a.x, b.x) : Interval(b.x, a.x);
        iy = a.y <= b.y ? Interval(a.y, b.y) : Interval(b.y, a.y);
        iz = a.z <= b.z ? Interval(a.z, b.z) : Interval(b.z, a.z);
    }

    // AABB définie comme fusion de deux AABB
    AABB(const AABB &b1, const AABB &b2) {
        ix = merge(b1.ix, b2.ix);
        iy = merge(b1.iy, b2.iy);
        iz = merge(b1.iz, b2.iz);
    }

    // Retourne ix si n = 0, iy si n = 1, iz sinon
    [[nodiscard]] const Interval &axis_interval(int n) const;

    // Vérifie s'il y a intersection entre l'AABB et le rayon
    [[nodiscard]] bool hit(const Ray &r, Interval ray_t) const;

    // Coin inférieur gauche arrière
    [[nodiscard]] Point3 min() const;

    // Coin supérieur droit avant
    [[nodiscard]] Point3 max() const;

    // Centre de l'AABB
    [[nodiscard]] Point3 center() const;
};

#pragma acc routine seq
[[nodiscard]] inline const Interval &AABB::axis_interval(int n) const {
    switch (n) {
        case 0: return ix;
        case 1: return iy;
        default: return iz;
    }
}

#pragma acc routine seq
[[nodiscard]] inline bool AABB::hit(const Ray &r, Interval ray_t) const {
    const Point3 &orig = r.origin;
    const Vec3 dir = r.direction;

    for (int i = 0; i < 3; i++) {
        const Interval &axis = axis_interval(i);
        // S'il n'y a pas de progression du rayon suivant l'axe, on vérifie si le plan de propagation coupe l'AABB
        if (fabsf(dir[i]) < 0.000001f) {
            if (orig[i] < axis.min or orig[i] > axis.max) return false;
            continue;
        }
        const float adinv = 1.f/dir[i];

        // Calcul des paramètres d'intersection du rayon avec les bords de la boite
        const auto t0 = (axis.min - orig[i]) * adinv;
        const auto t1 = (axis.max - orig[i]) * adinv;

        // Réduction de l'intervalle ray_t pour correspondre à l'intérieur de l'AABB (ray_t passé en copie, n'interfère pas avec les autres fonctions)
        if (t0 < t1) {
            if (t0 > ray_t.min) {ray_t.min = t0;}
            if (t1 < ray_t.max) {ray_t.max = t1;}
        }
        else {
            if (t1 > ray_t.min) {ray_t.min = t1;}
            if (t0 < ray_t.max) {ray_t.max = t0;}
        }

        // Si ray_t est vide, pas d'intersection
        if (ray_t.max <= ray_t.min) {return false;}
    }

    return true;
}

#pragma acc routine seq
[[nodiscard]] inline Point3 AABB::min() const {
    return {ix.min, iy.min, iz.min};
}

#pragma acc routine seq
[[nodiscard]] inline Point3 AABB::max() const {
    return {ix.max, iy.max, iz.min};
}

#pragma acc routine seq
[[nodiscard]] inline Point3 AABB::center() const {
    return {ix.center(), iy.center(), iz.center()};
}

#endif //TIPE_RAYTRACING_GPU_AABB_HPP