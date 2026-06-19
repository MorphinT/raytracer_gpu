//
// Created by nolan on 01/03/2026.
//

#ifndef TIPE_RAYTRACING_GPU_MATERIAL_HPP
#define TIPE_RAYTRACING_GPU_MATERIAL_HPP

#include "Vec3.hpp"
#include "Random.hpp"
#include "Ray.hpp"
#include "OBN.hpp"

struct HitRecord;

// Types de matériaux
enum MATERIAL_TYPE {
    LAMBERTIAN,
    METAL,
    DIELECTRIC,
    DIFFUSE_LIGHT
};

struct Lambertian {
    Color albedo; // Couleur du Lambertien

    Lambertian() = default;

    Lambertian(Color color) : albedo(color) {}

    // Calcule le rayon réfléchi et la couleur renvoyée
    bool scatter(const HitRecord &rec, Color &attenuation, Ray &scattered, RNG &rng) const;
};

struct Metal {
    Color albedo; // Couleur du métal
    float fuzz; // Effet de flou du métal (de 0 à 1)

    Metal() = default;

    Metal(Color color, float fuzz) : albedo(color), fuzz(fuzz) {}

    // Indique s'il y a un rayon réfléchi et calcule celui-ci le cas échéant
    bool scatter(const Ray &r_in, const HitRecord &rec, Color &attenuation, Ray &scattered, RNG &rng) const;

};

struct Dielectric {
    float n; // Indice de réfraction du milieu (relatif au milieu extérieur)

    Dielectric() = default;

    Dielectric(float n) : n(n) {}

    // Calcul le rayon réfracté et la couleur émise
    bool scatter(const Ray &r_in, const HitRecord &rec, Color &attenuation, Ray &scattered) const;
};

struct Diffuse_light {
    Color color; // Couleur de la source lumineuse

    Diffuse_light() = default;

    explicit Diffuse_light(Color color) : color(color) {}

    // Retourne la couleur émise
    [[nodiscard]] Color emitted() const;
};

#pragma acc routine seq
inline Color Diffuse_light::emitted() const {
    return color;
}

// Un matériau quelconque
struct Material {
    MATERIAL_TYPE type; // Type du matériau

    union {
        Lambertian lambertian;
        Metal metal;
        Dielectric dielectric;
        Diffuse_light diffuse_light;
    };

    Material &operator=(const Material &mat);

    bool scatter(const Ray &r_in, const HitRecord &rec, Color &attenuation, Ray &scattered, RNG &rng) const;

    [[nodiscard]] Color emitted() const;
};

#pragma acc routine seq
inline Material &Material::operator=(const Material &mat) {
    switch (mat.type) {
        case LAMBERTIAN:
            type = LAMBERTIAN;
            lambertian = mat.lambertian;
            break;
        case METAL:
            type = METAL;
            metal = mat.metal;
            break;
        case DIFFUSE_LIGHT:
            type = DIFFUSE_LIGHT;
            diffuse_light = mat.diffuse_light;
            break;
        case DIELECTRIC:
            type = DIELECTRIC;
            dielectric = mat.dielectric;
            break;
    }

    return *this;
}

#pragma acc routine seq
inline bool Material::scatter(const Ray &r_in, const HitRecord &rec, Color &attenuation, Ray &scattered, RNG &rng) const {
    switch (type) {
        case LAMBERTIAN:
            return lambertian.scatter(rec, attenuation, scattered, rng);
        case METAL:
            return metal.scatter(r_in, rec, attenuation, scattered, rng);
        case DIELECTRIC:
            return dielectric.scatter(r_in, rec, attenuation, scattered);
        default:
			attenuation = Color(1, 1, 1);
            return false;
    }
}

#pragma acc routine seq
inline Color Material::emitted() const {
    switch (type) {
        case DIFFUSE_LIGHT:
            return diffuse_light.emitted();
        default:
            return {0, 0, 0};
    }
}

// Création d'une texture de lambertien
#pragma acc routine seq
inline Material create_lambertian(Color albedo) {
    Lambertian lambertian(albedo);
    Material mat{};
    mat.type = LAMBERTIAN;
    mat.lambertian = lambertian;
    return mat;
}

// Création d'une texture métallique
#pragma acc routine seq
inline Material create_metal(Color albedo, float fuzz) {
    Metal metal(albedo, fuzz);
    Material mat{};
    mat.type = METAL;
    mat.metal = metal;
    return mat;
}

// Création d'un milieu transparent
#pragma acc routine seq
inline Material create_dielectric(float n) {
    Dielectric dielectric(n);
    Material mat{};
    mat.type = DIELECTRIC;
    mat.dielectric = dielectric;
    return mat;
}

// Création d'une texture lumineuse
#pragma acc routine seq
inline Material create_diffuse_light(Color color) {
    Diffuse_light diffuse_light(color);
    Material mat{};
    mat.type = DIFFUSE_LIGHT;
    mat.diffuse_light = diffuse_light;
    return mat;
}

// Conteneur enregistrant les informations d'intersection
struct HitRecord {
    Point3 p; // Point d'intersection rayon objet
    Vec3 normal; // Normale à la surface d'intersection
    float t; // Paramètre du point d'intersection sur le rayon incident
    bool front_face; // Indique si l'intersection a lieu sur la face avant / extérieure ou sur la face arrière / intérieure de l'objet
    Material mat; // Matériau de l'objet intersecté

    // Met le rayon normal dans le sens correspondant au côté de l'intersection, et enregistre si le rayon intersecte la face avant ou arrière de l'objet
    void set_sens_normal(const Ray &ray, const Vec3 &normal_sortant);
};

#pragma acc routine seq
inline void HitRecord::set_sens_normal(const Ray &ray, const Vec3 &normal_sortant) {
    front_face = p_scal(ray.direction, normal_sortant) < 0.;
    normal = front_face ? normal_sortant : -normal_sortant;
}

#pragma acc routine seq
inline bool Lambertian::scatter(const HitRecord &rec, Color &attenuation, Ray &scattered, RNG &rng) const {
    // Création d'une base locale
    OBN obn;
    obn.build_from_v(rec.normal);

    // Calcul le rayon réfléchi suivant la loi des lambertiens
    Vec3 scatter_direction = normalised(obn.local(random_cosine_direction(rng)));
    scattered = Ray(rec.p, scatter_direction);

    // Indique la couleur de l'objet
    attenuation = albedo;

    return true;
}

#pragma acc routine seq
inline bool Metal::scatter(const Ray &r_in, const HitRecord &rec, Color &attenuation, Ray &scattered, RNG &rng) const {
    // Calcul de la direction du rayon réfléchi si la réflexion était parfaite
    Vec3 reflected = reflect(r_in.direction, rec.normal);

    // Déviation du rayon pondérée par fuzz
    reflected = normalised(reflected) + fuzz * random_in_unit_sphere(rng);

    // Création du rayon réfléchi
    scattered = Ray(rec.p, reflected);

    // Indique la couleur de l'objet
    attenuation = albedo;

    return p_scal(rec.normal, reflected) > 0; // Rayon réfléchi que si la direction obtenue est dans la demi-sphère centrée autour de normal
}

#pragma acc routine seq
inline bool Dielectric::scatter(const Ray &r_in, const HitRecord &rec, Color &attenuation, Ray &scattered) const {
    attenuation = Color(1.0f, 1.0f, 1.0f);

    // Détermine l'indice de réflexion à utiliser en fonction de si le rayon entre ou sort du milieu transparent
    const float effective_refractive_index = rec.front_face ? 1.f/n : n;

    // Détermine si le rayon est réfléchi ou réfracté
    const Vec3 unit_direction = normalised(r_in.direction);
    const float cos_teta = fminf(p_scal(unit_direction, rec.normal), 1.f);
    const float sin_teta = sqrtf(1.f - cos_teta * cos_teta);
    const bool cannot_refract = effective_refractive_index * sin_teta > 1.;

    // Détermine la direction du rayon après le rebond
    Vec3 direction{};
    if (cannot_refract) direction = reflect(unit_direction, rec.normal);
    else direction = refract(unit_direction, rec.normal, effective_refractive_index);

    // Créé le rayon qui a rebondi
    scattered = Ray(rec.p, direction);

    return true;
}

#endif //TIPE_RAYTRACING_GPU_MATERIAL_HPP