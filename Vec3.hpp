//
// Created by nolan on 01/03/2026.
//

#ifndef TIPE_RAYTRACING_GPU_VEC3_HPP
#define TIPE_RAYTRACING_GPU_VEC3_HPP


#include <cmath>
#include "Random.hpp"
inline constexpr float pi = 3.1415926535897932385;

// Vecteur 3D
struct Vec3 {
    float x, y, z; // Composantes du vecteur

    Vec3() = default;

    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 &operator+=(const Vec3 &other);

    Vec3 &operator-=(const Vec3 &other);

    Vec3 &operator*=(float lamb);

    Vec3 &operator*=(const Vec3 &other); // Multiplication coordonnée par coordonnée

    Vec3 &operator/=(float lamb);

    float &operator[](int i); // Retourne la i-ème coordonnée (0 : x ; 1 : y ; 2 : z)

    float operator[](int i) const; // Retourne la i-ème coordonnée (0 : x ; 1 : y ; 2 : z)

    bool operator==(const Vec3 &other) const;

    Vec3 &operator=(const Vec3 &other);

    Vec3 operator-() const; // Retourne l'opposé

    // Norme du vecteur
    [[nodiscard]] float length() const;

    // Norme au carré du vecteur
    [[nodiscard]] float squaredNorm() const;

    // Indique si le vecteur est presque le vecteur nul (utile par exemple avant un calcul de norme pour s'assurer que le résultat n'est pas 0)
    [[nodiscard]] bool near_zero() const;

    // Vecteur aléatoire avec des coordonnées tirées uniformément entre 0 et 1
    [[nodiscard]] static Vec3 random(RNG &rng);

    // Vecteur aléatoire avec des coordonnées tirées uniformément entre min et max
    [[nodiscard]] static Vec3 random(RNG &rng, float min, float max);
};

#pragma acc routine seq
inline Vec3 &Vec3::operator+=(const Vec3 &other) {
    // Addition composante par composante
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

#pragma acc routine seq
inline Vec3 &Vec3::operator-=(const Vec3 &other) {
    // Soustraction composante par composante
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}

#pragma acc routine seq
inline Vec3 &Vec3::operator*=(float lamb) {
    // Multiplication composante par composante
    x *= lamb;
    y *= lamb;
    z *= lamb;
    return *this;
}

#pragma acc routine seq
inline Vec3 &Vec3::operator*=(const Vec3 &other) {
    // Multiplication composante par composante
    x *= other.x;
    y *= other.y;
    z *= other.z;
    return *this;
}

#pragma acc routine seq
inline Vec3 &Vec3::operator/=(float lamb) {
    // Division composante par composante
    x /= lamb;
    y /= lamb;
    z /= lamb;
    return *this;
}

#pragma acc routine seq
inline float &Vec3::operator[](const int i) {
    switch (i) {
        case 0: return x;
        case 1: return y;
        default: return z;
    }
}

#pragma acc routine seq
inline float Vec3::operator[](const int i) const {
    switch (i) {
        case 0: return x;
        case 1: return y;
        default: return z;
    }
}

#pragma acc routine seq
inline bool Vec3::operator==(const Vec3 &other) const {
    return x == other.x && y == other.y && z == other.z; // Égalité ssi égalité de chaque composante
}

#pragma acc routine seq
inline Vec3 &Vec3::operator=(const Vec3 &other) = default;

#pragma acc routine seq
inline Vec3 Vec3::operator-() const {
    return Vec3{-x, -y, -z}; // Passage à l'opposé composante par composante
}

#pragma acc routine seq
[[nodiscard]] inline float Vec3::length() const {
    return sqrtf(x * x + y * y + z * z);
}

#pragma acc routine seq
[[nodiscard]] inline float Vec3::squaredNorm() const {
    return x * x + y * y + z * z;
}

#pragma acc routine seq
[[nodiscard]] inline bool Vec3::near_zero() const {
    constexpr auto s = 1e-8f;
    return std::fabs(x) < s && std::fabs(y) < s && std::fabs(z) < s;
}

inline Vec3 Vec3::random(RNG &rng) {
    return {rng.next_uniform(), rng.next_uniform(), rng.next_uniform()};
}

inline Vec3 Vec3::random(RNG &rng, float min, float max) {
    return {rng.next_uniform(min, max), rng.next_uniform(min, max), rng.next_uniform(min, max)};
}

// Addition usuelle des vecteurs
#pragma acc routine seq
inline Vec3 operator+(const Vec3 &e1, const Vec3 &e2) {
    return Vec3{e1.x + e2.x, e1.y + e2.y, e1.z + e2.z};
}

// Soustraction usuelle des vecteurs
#pragma acc routine seq
inline Vec3 operator-(const Vec3 &e1, const Vec3 &e2) {
    return Vec3{e1.x - e2.x, e1.y - e2.y, e1.z - e2.z};
}

// Multiplication usuelle par un flottant
#pragma acc routine seq
inline Vec3 operator*(const Vec3 &e, float lamb) {
    return Vec3{e.x * lamb, e.y * lamb, e.z * lamb};
}

// Multiplication usuelle par un flottant
#pragma acc routine seq
inline Vec3 operator*(float lamb, const Vec3 &e) {
    return e * lamb;
}

// Multiplication composante par composante
#pragma acc routine seq
inline Vec3 operator*(const Vec3 &e1, const Vec3 &e2) {
    return {e1.x * e2.x, e1.y * e2.y, e1.z * e2.z};
}

// Division usuelle par un flottant
#pragma acc routine seq
inline Vec3 operator/(const Vec3 &e, float lamb) {
    return 1.0f/lamb * e;
}

// Retourne le vecteur e unitaire
#pragma acc routine seq
inline Vec3 normalised(const Vec3 &e) {
    return e / e.length();
}

// Retourne le produit vectoriel usuel des vecteurs
#pragma acc routine seq
inline Vec3 p_vect(const Vec3 &e1, const Vec3 &e2) {
    return Vec3{e1.y * e2.z - e1.z * e2.y, e1.z * e2.x - e1.x * e2.z, e1.x * e2.y - e1.y * e2.x};
}

// Retourne le produit scalaire usuel de deux vecteurs
#pragma acc routine seq
inline float p_scal(const Vec3 &e1, const Vec3 &e2) {
    return e1.x * e2.x + e1.y * e2.y + e1.z * e2.z;
}

using Point3 = Vec3;
using Color = Vec3;

// Retourne un vecteur aléatoire de norme 1 selon une loi uniforme
#pragma acc routine seq
inline Vec3 random_in_unit_sphere(RNG &rng) {
    const auto x = rng.next_gaussian();
    const auto y = rng.next_gaussian();
    const auto z = rng.next_gaussian();
    return normalised(Vec3(x, y, z));
}

// Retourne un vecteur aléatoire de norme 1 dans la demi-sphère centrée autour de normal selon une loi uniforme
#pragma acc routine seq
inline Vec3 random_on_hemisphere(const Vec3 &normal, RNG &rng) {
    Vec3 vec_in_unit_sphere = random_in_unit_sphere(rng);
    if (p_scal(normal, vec_in_unit_sphere) > 0) return vec_in_unit_sphere;
    return -vec_in_unit_sphere;
}

// Retourne un vecteur aléatoire de norme 1 dans le disque unité selon une loi uniforme (la troisième composante est nulle)
#pragma acc routine seq
inline Vec3 random_in_unit_disk(RNG &rng) {
    const auto x = rng.next_gaussian(0, 1);
    const auto y = rng.next_gaussian(0, 1);
    return normalised(Vec3(x, y, 0));
}

// Transforme une composante linéaire en composante gamma (pour les couleurs)
inline float linear_to_gamma(const float linear_component) {
    return sqrtf(linear_component);
}

// Retourne le vecteur réfléchi sur la surface de normale n de v
#pragma acc routine seq
inline Vec3 reflect(const Vec3 &v, const Vec3 &n) {
    return v - 2*p_scal(v, n) * n;
}

// Retourne le vecteur réfracté à travers la surface de normal n avec un rapport d'indice de réfraction des deux milieux valant eta
#pragma acc routine seq
inline Vec3 refract(const Vec3 &v, const Vec3 &n, const float eta) {
    const auto cos_teta = std::fminf(p_scal(-v, n), 1.f);
    const Vec3 r_out_perp = eta * (v + cos_teta * n);
    const Vec3 r_out_parallel = -sqrtf(std::fabs(1.f - r_out_perp.squaredNorm()))*n;
    return r_out_perp + r_out_parallel;
}

// Un vecteur direction adapté aux lambertians
#pragma acc routine seq
inline Vec3 random_cosine_direction(RNG& rng) {
    float r1 = rng.next_uniform();
    float r2 = rng.next_uniform();

    float phi = 2.0f * pi * r1;

    float z = cosf(phi) * sqrtf(r2);
    float x = sinf(phi) * sqrtf(r2);
    float y = sqrtf(1.0f - r2);

    return Vec3(x, y, z);
}

// Rotation d'angle teta par rapport à l'axe des abscisses
#pragma acc routine seq
inline Vec3 rotatex(Vec3 vec3, float costeta, float sinteta) {
    return {vec3.x, vec3.y * costeta - vec3.z * sinteta, vec3.y * sinteta + vec3.z * costeta};
}

// Rotation d'angle teta par rapport à l'axe des ordonnées
#pragma acc routine seq
inline Vec3 rotatey(Vec3 vec3, float costeta, float sinteta) {
    return {vec3.x * costeta + vec3.z * sinteta, vec3.y, -vec3.x * sinteta + vec3.z * costeta};
}

// Rotation d'angle teta par rapport à l'axe des côtes
#pragma acc routine seq
inline Vec3 rotatez(Vec3 vec3, float costeta, float sinteta) {
    return {vec3.x * costeta - vec3.y * sinteta, vec3.x * sinteta + vec3.y * costeta, vec3.z};
}

#endif //TIPE_RAYTRACING_GPU_VEC3_HPP