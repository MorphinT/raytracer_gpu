//
// Created by nolan on 01/03/2026.
//

#ifndef TIPE_RAYTRACING_GPU_INTERVAL_HPP
#define TIPE_RAYTRACING_GPU_INTERVAL_HPP


// Un intervalle
struct Interval {
    float min; // Borne inférieure de l'intervalle
    float max; // Borne supérieure de l'intervalle

    Interval();

    // Initialise un intervalle à partir de ses bornes
    Interval(float min, float max);

    // Augmente la taille d'un intervalle de delta (symétriquement)
    [[nodiscard]] Interval expand(float delta) const;

    // Indique si value est dans l'intervalle fermé
    [[nodiscard]] bool contains(float value) const;

    // Indique si value est dans l'intervalle ouvert
    [[nodiscard]] bool surrounds(const float &value) const;

    // Retourne la valeur la plus proche de value contenue dans l'intervalle (fermé)
    [[nodiscard]] float clamp(const float value) const;

    // Retourne la taille de l'intervalle
    [[nodiscard]] float size() const;

    // Retourne la valeur au milieu de l'intervalle
    [[nodiscard]] float center() const;
};

#pragma routine seq
inline Interval::Interval(): min(0.0f), max(0.0f) {}

#pragma routine seq
inline Interval::Interval(const float min, const float max) : min(min), max(max) {}

#pragma routine seq
[[nodiscard]] inline Interval Interval::expand(const float delta) const {
    const float pas = delta / 2.0f;
    return {min + pas, max - pas};
}

#pragma routine seq
[[nodiscard]] inline bool Interval::contains(const float value) const {
    return value >= min && value <= max;
}

#pragma routine seq
[[nodiscard]] inline bool Interval::surrounds(const float &value) const {
    return value > min && value < max;
}

#pragma routine seq
[[nodiscard]] inline float Interval::clamp(const float value) const {
    if (value <= min) return min;
    if (value >= max) return max;
    return value;
}

#pragma routine seq
[[nodiscard]] inline float Interval::size() const {
    return max - min;
}

#pragma routine seq
[[nodiscard]] inline float Interval::center() const {
    return (min + max) / 2;
}

// Retourne la fusion de deux intervalles
#pragma routine seq
inline Interval merge(const Interval &a, const Interval &b) {
    return {a.min <= b.min ? a.min : b.min, a.max >= b.max ? a.max : b.max};
}

#endif //TIPE_RAYTRACING_GPU_INTERVAL_HPP