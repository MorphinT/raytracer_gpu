//
// Created by nolan on 01/03/2026.
//

#ifndef TIPE_RAYTRACING_GPU_CAMERA_HPP
#define TIPE_RAYTRACING_GPU_CAMERA_HPP

#include "Ray.hpp"
#include "Vec3.hpp"
#include "Random.hpp"
#include "Interval.hpp"

inline constexpr float infinite = std::numeric_limits<float>::infinity();

// Convertie un angle de degrés en radians
inline float degrees_to_radians(const float degree) {
    return degree*pi/180.0f;
}

enum BackgroundType {
    BG_WHITE,
    BG_BLACK,
	BG_GREY,
    BG_STRONG_GREY,
    BG_GRADIENT,
	BG_NIGHT
};

struct Camera {
    // Paramètres réglables pour une image
    Point3 center; // Centre de la caméra
    Vec3 look_direction; // Direction d'orientation de la caméra
    Vec3 up; // Le haut pour la caméra (et donc l'image générée)
    float v_fov; // Angle de vue (en degrés)

    float ratio; // Résolution de l'image rendue
    int im_width; // La largeur de l'image rendue
    int samples_per_pixel; // Nombre de rayons par pixel
    int max_depth; // Nombre maximal de collisions pour les rayons

    float defocus_angle; // Variation maximale de l'angle de chaque rayon à travers les pixels
    float focus_dist; // La distance de la caméra à laquelle la mise au point est faite

    BackgroundType bg_type; // Type d'arrière-plan

    // Variables internes : toute modification manuelle risque d'être écrasée (ou de générer des images incohérentes)
    int im_height{}; // Hauteur de l'image
    Vec3 u{}, v{}, w{}; // Base locale
    Point3 pix00{}; // Le pixel en haut à gauche du viewport
    Vec3 du_viewport{}; // Vecteur de déplacement d'un pixel vers la droite
    Vec3 dv_viewport{}; // Vecteur de déplacement d'un pixel vers le bas
    Vec3 u_defocus_disk{}; // Vecteur largeur du disque de défocus
    Vec3 v_defocus_disk{}; // Vecteur hauteur du disque de défocus

    Camera(float ratio, const int im_width)
        : center(Point3(0, 0, 0)),
          look_direction(Vec3(0, 0, 1)),
          up(0,1,0),
          v_fov(90),
          ratio(ratio),
          im_width(im_width),
          samples_per_pixel(100),
          max_depth(50),
          defocus_angle(0),
          focus_dist(10),
          bg_type(BG_WHITE)
    {}

    // Initialisation de la caméra avant la génération
    void initialize() {
        // Création de la base locale
        w = -normalised(look_direction);
        u = normalised(p_vect(up, w));
        v = p_vect(w, u);

        // Détermination de la hauteur (en pixels) de l'image et détermination des dimensions du viewport
        im_height = static_cast<int>((float) im_width / ratio);
        const auto theta = degrees_to_radians(v_fov);
        const auto h = tanf(theta / 2);
        const auto viewport_height = 2 * h * focus_dist;
        const auto viewport_width = viewport_height / (im_height/static_cast<float>(im_width));

        // Création du viewport
        const auto u_viewport = (float) viewport_width * u;
        const auto v_viewport = (float) viewport_height * -v;
        const auto origin_viewport = center - u_viewport/2 - v_viewport/2 - focus_dist*w;

        // Création des variables permettant la navigation sur le viewport
        du_viewport = u_viewport / (float)im_width;
        dv_viewport = v_viewport / (float)im_height;
        pix00 = origin_viewport + 0.5f*du_viewport + 0.5f*dv_viewport;

        // Initialisation du disque de défocus
        const auto defocus_radius = focus_dist * tanf(degrees_to_radians(defocus_angle / 2));
        u_defocus_disk = defocus_radius * u;
        v_defocus_disk = defocus_radius * v;
    }

    // Carré de la taille d'un pixel de l'image dans la scène
    static Vec3 sample_square(RNG &rng);

    // Point aléatoire dans le disque de défocus
    [[nodiscard]] Point3 random_in_defocus_disk(RNG rng) const;

    // Retourne un rayon lancé par la caméra pour calculer la couleur du pixel de coordonnées (x, y) sur l'image
    [[nodiscard]] Ray getRay(int x, int y, RNG &rng) const;
};

#pragma acc routine seq
inline Vec3 Camera::sample_square(RNG &rng) {
    return {rng.next_uniform(0, 1) - 0.5f, rng.next_uniform(0, 1) - 0.5f, 0};
}

#pragma acc routine seq
[[nodiscard]] inline Point3 Camera::random_in_defocus_disk(RNG rng) const {
    const auto p = random_in_unit_disk(rng);
    return center + p.x * u_defocus_disk + p.y * v_defocus_disk;
}

#pragma acc routine seq
[[nodiscard]] inline Ray Camera::getRay(const int x, const int y, RNG &rng) const {
    const auto offset = sample_square(rng);
    const auto pixel_targeted = pix00 + ((float) x + offset.x) * du_viewport + ((float) y + offset.y) * dv_viewport;
    const auto ray_origin = (defocus_angle <= 0) ? center : random_in_defocus_disk(rng);
    return {ray_origin,  pixel_targeted - ray_origin};
}

// Création des étoiles dans le ciel
#pragma acc declare create(sky)
int sky[64000000];
inline void init_sky(float lim) {
    RNG sky_rng;
    sky_rng.state = 151514168;
    for (int &portion : sky) {
        if (sky_rng.next_uniform() < lim) portion = 1;
        else portion = 0;
    }
}

// Détermine la couleur des rayons qui se perdent à  l'infini
#pragma acc routine seq
inline Color background_color(const Camera& cam, const Vec3& dir)
{
    switch (cam.bg_type)
    {
        case BG_WHITE:
            return Color(1.f, 1.f, 1.f);

		case BG_GREY:
			return Color(0.3f, 0.3f, 0.3f);

        case BG_STRONG_GREY:
            return Color(0.01f, 0.01f, 0.01f);

        case BG_GRADIENT:
        {
            const float t = 0.5f * (normalised(dir).y + 1.0f);
            return (1.0f - t) * Color(1.f,1.f,1.f)
                 + t * Color(0.5f,0.7f,1.0f);
        }

        case BG_NIGHT:
        {
            Vec3 pos_in_sky = 199 * normalised(dir) + Vec3(100.f, 100.f, 100.f);
            if (sky[static_cast<int>(1000000*floor(abs(pos_in_sky.x)) + 1000*floor(abs(pos_in_sky.y)) + floor(abs(pos_in_sky.z))) % 64000000] == 1) return {1.f, 1.f, 1.f};
            return {0.f, 0.f, 0.f};
        }

        default:
            return {0.f,0.f,0.f};
    }
}


#endif //TIPE_RAYTRACING_GPU_CAMERA_HPP