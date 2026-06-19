//
// Created by nolan on 01/03/2026.
//

#ifndef TIPE_RAYTRACING_GPU_OBJECTS_HPP
#define TIPE_RAYTRACING_GPU_OBJECTS_HPP

#include "AABB.hpp"
#include "Vec3.hpp"
#include "Ray.hpp"
#include "Interval.hpp"
#include "Material.hpp"
#include <algorithm>
#include <vector>
#include <cstdlib>

// Types des objets pouvant être ajoutés à la scène
enum ObjectType {
    OBJ_SPHERE,
    OBJ_TRIANGLE,
    OBJ_QUAD
};

// Objet sphère
struct Sphere {
    Point3 center; // Centre de la sphère
    float radius; // Rayon de la sphère
    Material mat; // Matériau qui compose cette sphère
    AABB bbox; // Boite englobante

    Sphere(): center(), radius(0.f), mat() {}

    Sphere(const Point3 &center, const float radius, Material mat): center(center), radius(radius), mat(mat) {
        const auto r_vec = Vec3(radius, radius, radius);
        bbox = AABB(center - r_vec, center + r_vec);
    }

    // Indique si ray intersecte la sphere, et remplie rec le cas échéant
    bool hit(const Ray &ray, Interval int_valid, HitRecord &rec) const;
};

#pragma acc routine seq
inline bool Sphere::hit(const Ray &ray, const Interval int_valid, HitRecord &rec) const {
    const Vec3 oc = center - ray.origin;
    const float a = ray.direction.squaredNorm();
    const float h = p_scal(oc, ray.direction);
    const float c = oc.squaredNorm() - radius * radius;
    const float discr = h * h - a * c;
    if (discr < 0) {
        return false;
    }

    const float sqrtdiscr = sqrtf(discr);
    float racine = (h - sqrtdiscr) / a;
    if (not int_valid.surrounds(racine)) {
        racine = (h + sqrtdiscr) / a;
        if (not int_valid.surrounds(racine)) {
            return false;
        }
    }

    rec.t = racine;
    rec.p = ray.at(racine);
    rec.set_sens_normal(ray, (rec.p - center) / radius);
    rec.mat = mat;

    return true;
}

// Objet triangle
struct Triangle {
    Point3 origin; // Le coin de référence du triangle
    Vec3 u; // Premier vecteur représentant un des côtés issus d'origin
    Vec3 v; // Deuxième vecteur représentant l'autre côté issu d'origin
    Vec3 normal{}; // Vecteur normal à la surface (pour l'orientation)
    Material mat; // Matériau qui compose ce triangle
    AABB bbox{}; // Boite englobante

    // Construction à partir d'un point et de deux côtés
    Triangle(const Point3 &origin, const Vec3 &u, const Vec3 &v, Material mat): origin(origin), u(u), v(v), mat(mat) {
        normal = normalised(p_vect(u, v));
        Point3 p0 = origin;
        Point3 p1 = origin + u;
        Point3 p2 = origin + v;

        Vec3 min_v(
            std::min({p0.x, p1.x, p2.x}),
            std::min({p0.y, p1.y, p2.y}),
            std::min({p0.z, p1.z, p2.z})
        );

        Vec3 max_v(
            std::max({p0.x, p1.x, p2.x}),
            std::max({p0.y, p1.y, p2.y}),
            std::max({p0.z, p1.z, p2.z})
        );

        constexpr float eps = 1e-4f;

        min_v -= Vec3(eps, eps, eps);
        max_v += Vec3(eps, eps, eps);

        bbox = AABB(min_v, max_v);
    }

    // Construction à partir de 3 points
    Triangle(const Point3 &p0, const Point3 &p1, const Point3 &p2, const Vec3 &normal, Material mat): origin(p0), u(p1-p0), v(p2-p0), normal(normalised(normal)), mat(mat) {
        Vec3 min_v(
            std::min({p0.x, p1.x, p2.x}),
            std::min({p0.y, p1.y, p2.y}),
            std::min({p0.z, p1.z, p2.z})
        );

        Vec3 max_v(
            std::max({p0.x, p1.x, p2.x}),
            std::max({p0.y, p1.y, p2.y}),
            std::max({p0.z, p1.z, p2.z})
        );

        constexpr float eps = 1e-4f;

        min_v -= Vec3(eps, eps, eps);
        max_v += Vec3(eps, eps, eps);

        bbox = AABB(min_v, max_v);
    }

    // Indique si ray intersecte le triangle, et remplie rec le cas échéant
    bool hit(const Ray &ray, Interval int_valid, HitRecord &rec) const;
};

#pragma acc routine seq
inline bool Triangle::hit(const Ray &ray, const Interval int_valid, HitRecord &rec) const {
    const Vec3 h = p_vect(ray.direction, v);
    const float a = p_scal(u, h);
    if (a == 0) return false;

    const Vec3 s = ray.origin - origin;
    const float fu = p_scal(s, h)/a;
    if (fu < 0 or fu > 1) return false;

    const Vec3 q = p_vect(s, u);
    const float fv = p_scal(ray.direction, q) / a;
    if (fv < 0 or fv > 1 - fu) return false;

    rec.t = -p_scal(p_vect(s, v), u) / a;
    rec.p = origin + fu * u + fv * v;
    rec.set_sens_normal(ray, normal);
    rec.mat = mat;
    return int_valid.contains(rec.t);
}

// En réalité un parallélogramme
struct Quad {
    Point3 origin; // Le coin de référence du rectangle
    Vec3 u; // Premier vecteur représentant un des côtés issus d'origin
    Vec3 v; // Deuxième vecteur représentant l'autre côté issu d'origin
    Vec3 normal{}; // Vecteur normal à la surface (pour l'orientation)
    Material mat; // Matériau qui compose ce rectangle
    AABB bbox{}; // Boite englobante

    // Construction à partir d'un point et de deux côtés
    Quad(const Point3 &origin, const Vec3 &u, const Vec3 &v, const Material mat): origin(origin), u(u), v(v), mat(mat) {
        normal = normalised(p_vect(u, v));
        const Point3 p0 = origin;
        const Point3 p1 = origin + u;
        const Point3 p2 = origin + v;
        const Point3 p3 = origin + u + v;

        Vec3 min_v(
            std::min({p0.x, p1.x, p2.x, p3.x}),
            std::min({p0.y, p1.y, p2.y, p3.y}),
            std::min({p0.z, p1.z, p2.z, p3.z})
        );

        Vec3 max_v(
            std::max({p0.x, p1.x, p2.x, p3.x}),
            std::max({p0.y, p1.y, p2.y, p3.y}),
            std::max({p0.z, p1.z, p2.z, p3.z})
        );

        constexpr float eps = 1e-4f;

        min_v -= Vec3(eps, eps, eps);
        max_v += Vec3(eps, eps, eps);

        bbox = AABB(min_v, max_v);
    }

    // Indique si ray intersecte le parallélogramme, et remplie rec le cas échéant
    bool hit(const Ray &ray, Interval int_valid, HitRecord &rec) const;
};

#pragma acc routine seq
inline bool Quad::hit(const Ray &ray, Interval int_valid, HitRecord &rec) const {
    const Vec3 h = p_vect(ray.direction, v);
    const float a = p_scal(u, h);
    if (a == 0) return false;

    const Vec3 s = ray.origin - origin;
    const float fu = p_scal(s, h)/a;
    if (fu < 0 or fu > 1) return false;

    const Vec3 q = p_vect(s, u);
    const float fv = p_scal(ray.direction, q) / a;
    if (fv < 0 or fv > 1) return false;

    rec.t = -p_scal(p_vect(s, v), u) / a;
    rec.p = origin + fu * u + fv * v;
    rec.set_sens_normal(ray, normal);
    rec.mat = mat;
    return int_valid.contains(rec.t);
}

// Un objet quelconque
struct Object {
    ObjectType type{}; // Type de l'objet
    union {
        Sphere sphere;
        Triangle triangle;
        Quad quad;
    };
    bool rotation = false; // Indique si l'objet subit une rotation
    float x_rotation_angle = 0; // Angle de rotation autour de l'axe des abscisses
    float y_rotation_angle = 0; // Angle de rotation autour de l'axe des ordonnées
    float z_rotation_angle = 0; // Angle de rotation autour de l'axe des côtes
    // Valeur des pré-calculs des sinus et des cosinus des angles de rotation
    float cos_rotx = 1;
    float sin_rotx = 0;
    float cos_roty = 1;
    float sin_roty = 0;
    float cos_rotz = 1;
    float sin_rotz = 0;

    AABB bbox; // Boite englobante

    Object();

    Object(float tetax, float tetay, float tetaz);

    Object &operator=(const Object &obj);

    // Retourne l'aabb de l'objet
    [[nodiscard]] AABB bounding_box() const;

    // Appelle la bonne fonction hit
    bool hit_stat(const Ray &ray, Interval int_valid, HitRecord &hit_record) const;

    // Réalise les transformations et choisi le bon hit
    bool hit(const Ray &ray, Interval int_valid, HitRecord &hit_record) const;

    // Initialise la boite englobante (obligatoire à la création de l'objet)
    void init_aabb();
};

inline Object::Object() {}

inline Object::Object(float tetax, float tetay, float tetaz): rotation(true) {
    // Convertion des angles en radians
    tetax = degrees_to_radians(tetax);
    tetay = degrees_to_radians(tetay);
    tetaz = degrees_to_radians(tetaz);

    cos_rotx = cosf(tetax);
    sin_rotx = sinf(tetax);
    cos_roty = cosf(tetay);
    sin_roty = sinf(tetay);
    cos_rotz = cosf(tetaz);
    sin_rotz = sinf(tetaz);
}


#pragma acc routine seq
inline Object &Object::operator=(const Object &obj) {
    switch (obj.type) {
        case OBJ_SPHERE:
            type = OBJ_SPHERE;
            sphere = obj.sphere;
            break;
        case OBJ_TRIANGLE:
            type = OBJ_TRIANGLE;
            triangle = obj.triangle;
            break;
        case OBJ_QUAD:
            type = OBJ_QUAD;
            quad = obj.quad;
            break;
    }

	rotation = obj.rotation;
	cos_rotx = obj.cos_rotx;
	sin_rotx = obj.sin_rotx;
	cos_roty = obj.cos_roty;
	sin_roty = obj.sin_roty;
	cos_rotz = obj.cos_rotz;
	sin_rotz = obj.sin_rotz;

	bbox = obj.bbox;

    return *this;
}

#pragma acc routine seq
[[nodiscard]] inline AABB Object::bounding_box() const {
    return bbox;
}

#pragma acc routine seq
inline bool Object::hit_stat(const Ray &ray, Interval int_valid, HitRecord &hit_record) const {
    switch (type) {
        case OBJ_SPHERE:
            return sphere.hit(ray, int_valid, hit_record);
        case OBJ_TRIANGLE:
            return triangle.hit(ray, int_valid, hit_record);
        case OBJ_QUAD:
            return quad.hit(ray, int_valid, hit_record);
    }
}

#pragma acc routine seq
inline bool Object::hit(const Ray &ray, Interval int_valid, HitRecord &hit_record) const {
    if (!rotation) return hit_stat(ray, int_valid, hit_record);

    // Rotation du rayon incident
    Vec3 origin = rotatez(rotatey(rotatex(ray.origin, cos_rotx, -sin_rotx), cos_roty, -sin_roty), cos_rotz, -sin_rotz);
    Vec3 direction = rotatez(rotatey(rotatex(ray.direction, cos_rotx, -sin_rotx), cos_roty, -sin_roty), cos_rotz, -sin_rotz);
    Ray rotated_ray = Ray(origin, direction);

    if (!hit_stat(rotated_ray, int_valid, hit_record)) return false;

    // Renseignement de hit_record en prenant en compte la rotation
    hit_record.p = rotatex(rotatey(rotatez(hit_record.p, cos_rotz, sin_rotz), cos_roty, sin_roty), cos_rotx, sin_rotx);
    hit_record.normal = rotatex(rotatey(rotatez(hit_record.normal, cos_rotz, sin_rotz), cos_roty, sin_roty), cos_rotx, sin_rotx);

    return true;
}

inline void Object::init_aabb() {
    // Sélection de la boite englobante correspondant au type de l'objet
    switch (type) {
        case OBJ_SPHERE: bbox = sphere.bbox; break;
        case OBJ_TRIANGLE: bbox = triangle.bbox; break;
        case OBJ_QUAD: bbox = quad.bbox; break;
    }

    // Rotation de la boite englobante si nécessaire
    if (rotation) {
        Point3 min(infinite, infinite, infinite);
        Point3 max(-infinite, -infinite, -infinite);

        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                for (int k = 0; k < 2; k++) {
                    float x = i * bbox.ix.max + (1-i) * bbox.ix.min;
                    float y = j * bbox.iy.max + (1-j) * bbox.iy.min;
                    float z = k * bbox.iz.max + (1-k) * bbox.iz.min;

                    Vec3 new_vec = rotatex(rotatey(rotatez(Vec3(x, y, z), cos_rotz, sin_rotz), cos_roty, sin_roty), cos_rotx, sin_rotx);

                    for (int c = 0; c < 3; c++) {
                        min[c] = std::fminf(min[c], new_vec[c]);
                        max[c] = std::fmaxf(max[c], new_vec[c]);
                    }
                }
            }
        }

        bbox = AABB(min, max);
    }
}

// Création d'un objet de type sphère
inline Object create_sphere(Point3 center, float radius, Material mat) {
    Sphere sphere(center, radius, mat);
    Object object{};
    object.type = OBJ_SPHERE;
    object.sphere = sphere;
    object.init_aabb();
    return object;
}

// Création d'un objet de type triangle à partir d'un point et de deux côtés
inline Object create_triangle(const Point3 &origin, const Vec3 &u, const Vec3 &v, Material mat) {
    Triangle triangle{origin, u, v, mat};
    Object object{};
    object.type = OBJ_TRIANGLE;
    object.triangle = triangle;
    object.init_aabb();
    return object;
}

// Création d'un objet de type triangle à partir de trois points et d'une normale
inline Object create_triangle(const Point3 &x, const Point3 &y, const Point3 &z, const Vec3 &normal, Material mat) {
    Triangle triangle{x, y, z, normal, mat};
    Object object{};
    object.type = OBJ_TRIANGLE;
    object.triangle = triangle;
    object.init_aabb();
    return object;
}

// Création d'un objet de type parallélogramme à partir d'un point et de deux côtés
inline Object create_quad(const Point3 &origin, const Vec3 &u, const Vec3 &v, const Material mat) {
    Quad rectangle{origin, u, v, mat};
    Object object{};
    object.type = OBJ_QUAD;
    object.quad = rectangle;
    object.init_aabb();
    return object;
}

// Création d'un objet de type triangle à partir d'un point et de deux côtés rotationné
inline Object create_triangle(const Point3 &origin, const Vec3 &u, const Vec3 &v, Material mat, float tetax, float tetay, float tetaz) {
    Triangle triangle{origin, u, v, mat};
    Object object(tetax, tetay, tetaz);
    object.type = OBJ_TRIANGLE;
    object.triangle = triangle;
    object.init_aabb();
    return object;
}

// Création d'un objet de type triangle à partir de trois points et d'une normale rotationné
inline Object create_triangle(const Point3 &x, const Point3 &y, const Point3 &z, const Vec3 &normal, Material mat, float tetax, float tetay, float tetaz) {
    Triangle triangle{x, y, z, normal, mat};
    Object object(tetax, tetay, tetaz);
    object.type = OBJ_TRIANGLE;
    object.triangle = triangle;
    object.init_aabb();
    return object;
}

// Création d'un objet de type parallélogramme à partir d'un point et de deux côtés rotationné
inline Object create_quad(const Point3 &origin, const Vec3 &u, const Vec3 &v, const Material mat, float tetax, float tetay, float tetaz) {
    Quad rectangle{origin, u, v, mat};
    Object object(tetax, tetay, tetaz);
    object.type = OBJ_QUAD;
    object.quad = rectangle;
    object.init_aabb();
    return object;
}

// Noeud BVH
struct BVHNode {
    AABB bbox;

    int left{};      // index enfant gauche
    int right{};     // index enfant droit

    int start{};     // index première primitive (si feuille)
    int count{};     // nombre primitives (si feuille)
};

inline int build_bvh(std::vector<Object>& primitives,
              std::vector<BVHNode>& nodes,
              int start,
              int end)
{
    BVHNode node;

    // Bounding box globale
    AABB bbox = primitives[start].bounding_box();

    for (int i = start + 1; i < end; ++i)
        bbox = AABB(bbox, primitives[i].bounding_box());

    node.bbox = bbox;

    int object_count = end - start;

    // Condition feuille
    if (object_count <= 2)
    {
        node.start = start;
        node.count = object_count;
        // Aucun nœud enfant
        node.left  = -1;
        node.right = -1;

        int index = nodes.size();
        nodes.push_back(node);
        return index;
    }

    // Choix axe dominant
    const Vec3 extent = bbox.max() - bbox.min();

    int axis = 0;
    if (extent.y > extent.x) axis = 1;
    if (extent.z > extent[axis]) axis = 2;

    int mid = start + object_count / 2;

    // Partition rapide (nth_element)
    std::nth_element(
        primitives.begin() + start,
        primitives.begin() + mid,
        primitives.begin() + end,
        [axis](const Object& a, const Object& b)
        {
            return a.bounding_box().center()[axis] < b.bounding_box().center()[axis];
        });

    // Construction récursive
    int left_child  = build_bvh(primitives, nodes, start, mid);
    int right_child = build_bvh(primitives, nodes, mid, end);

    node.left  = left_child;
    node.right = right_child;
    // Pas d'objets (ce n'est pas une feuille)
    node.start = 0;
    node.count = 0;

    // Récupération de l'indice et ajout du nœud construit dans nodes
    const int index = nodes.size();
    nodes.push_back(node);

    return index;
}

// Test d'intersection d'un rayon avec la scène avec l'algorithme BVH
#pragma acc routine seq
inline bool traverse_bvh(const Ray& ray,
                  const BVHNode* nodes,
                  const Object* primitives,
                  int root,
                  HitRecord& rec)
{
    int stack[64]; // Pile des nœuds traités
    int stack_ptr = 0;

    stack[stack_ptr++] = root; // Initialisation de la pile avec le nœud racine

    bool hit_anything = false;
    float closest = 1e30f;

    // Parcours de l'arbre BVH
    while (stack_ptr > 0)
    {
        // Récupération de l'indice du nœud BVH au sommet de la pile
        int node_index = stack[--stack_ptr];
        const BVHNode& node = nodes[node_index];

        // On abandonne la branche si le rayon ne passe pas dans la zone englobée par le nœud BVH
        if (!node.bbox.hit(ray, Interval(0.001f, closest)))
            continue;

        if (node.count > 0)
        {
            // Traitement de la feuille
            for (int i = 0; i < node.count; ++i)
            {
                const int obj_index = node.start + i;

                // Test d'intersection avec l'objet
                HitRecord temp{};
                if (primitives[obj_index].hit(ray, Interval(0.001f, closest), temp))
                {
                    hit_anything = true;
                    closest = temp.t;
                    rec = temp;
                }
            }
        }
        else
        {
            // Ajout des nœuds BVH enfants à la pile
            stack[stack_ptr++] = node.left;
            stack[stack_ptr++] = node.right;
        }
    }

    return hit_anything;
}

// Test d'intersection d'un rayon avec la scène sans l'algorithme BVH
#pragma acc routine seq
inline bool traverse(const Ray& ray,
                  	 const Object* primitives,
					 int object_count,
                  	 HitRecord& rec) {
	HitRecord temp{};
	auto hit_anything = false;
	auto closest = 1e30f;
	for (int i = 0; i < object_count; ++i) {
		if (primitives[i].hit(ray, Interval(0.001f, closest), temp)) {
			hit_anything = true;
            closest = temp.t;
            rec = temp;
		}
	}

	return hit_anything;
}

#endif //TIPE_RAYTRACING_GPU_OBJECTS_HPP