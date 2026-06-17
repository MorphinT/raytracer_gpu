//
// Created by nolan on 01/03/2026.
//

#ifndef TIPE_RAYTRACING_GPU_RANDOM_HPP
#define TIPE_RAYTRACING_GPU_RANDOM_HPP
#include <cmath>
#include <cstdint>

// Un générateur de nombres aléatoires à base de xorshift32
// Attention, passez TOUJOURS un RNG en paramètre par référence (non constante), sous peine d'avoir des valeurs aléatoires identiques d'une fonction à l'autre
struct RNG {
    unsigned int state{}; // État du générateur
    bool has_spare = false; // Enregistre s'il y a un entier généré selon une loi gaussienne en mémoire
    float spare{}; // Entier enregistré s'il y a lieu

    // Génère un entier 32 bits aléatoire
    unsigned int next_uint();

    // Génère un flottant choisi uniformément entre 0 et 1
    float next_uniform();
    // Génère un flottant choisi uniformément entre min et max
    float next_uniform(float min, float max);

    // Génère un flottant choisi selon une loi gaussienne de paramètres 0 et 1
    float next_gaussian();
    // Génère un flottant choisi selon une loi gaussienne de paramètres mean et stddev
    float next_gaussian(float mean, float stddev);

    // Génère un entier 32 bits choisi uniformément entre min et max
    int next_int(int min, int max);
};

#pragma acc routine seq
inline unsigned int RNG::next_uint() {
    // Algorithme xorshift32
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

#pragma acc routine seq
inline float RNG::next_uniform() {
    return ((float)next_uint() + 1.0f) * (1.0f / 4294967296.0f);
}

#pragma acc routine seq
inline float RNG::next_gaussian() {
    // Retour du flottant en mémoir s'il y a lieu
    if (has_spare) {
        has_spare = false;
        return spare;
    }

    // Génération de deux flottants selon une loi gaussienne
    const float u1 = fmaxf(next_uniform(), 1e-7f);
    const float u2 = next_uniform();

    const float r = sqrtf(-2.0f * logf(u1));
    const float theta = 2.0f * 3.14159265359f * u2;

    spare = r * sinf(theta);
    has_spare = true;

    return r * cosf(theta);
}

#pragma acc routine seq
inline int RNG::next_int(const int min, const int max) {
    const auto range = (unsigned int)(max - min + 1);

    // Limite de la borne max de l'entier généré pour assurer une loi réellement uniforme
    const unsigned int limit = UINT32_MAX - (UINT32_MAX % range);

    unsigned int r;

    do {
        r = next_uint();
    } while (r >= limit);

    return min + (r % range);
}

#pragma acc routine seq
inline float RNG::next_uniform(float min, float max) {
    return min + (max - min) * next_uniform();
}


#pragma acc routine seq
inline float RNG::next_gaussian(const float mean, const float stddev) {
    return mean + stddev * next_gaussian();
}

// Hachage pour générer des graines indépendantes
#pragma acc routine seq
inline unsigned int wang_hash(unsigned int seed) {
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}
#endif //TIPE_RAYTRACING_GPU_RANDOM_HPP
