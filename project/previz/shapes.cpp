#include "shapes.hpp"

using namespace std;

Real REFRACT_GLASS = 1.5;
Real REFRACT_AIR = 1.0;

// SHAPE

Shape::Shape(VEC3 color, Material type, Real refractiveIndex)
    : color(color), type(type), refractiveIndex(refractiveIndex) {}

// SPHERE

Sphere::Sphere(Real radius, VEC3 center, VEC3 color, Material type,
               Real refractiveIndex)
    : Shape(color, type, refractiveIndex), radius(radius), center(center) {}

IntersectResult Sphere::intersect(Ray ray) {
    Real A = ray.direction.dot(ray.direction);
    Real B = (2.0 * ray.direction).dot(ray.origin - center);
    Real C = (ray.origin - center).dot(ray.origin - center) - (radius * radius);

    Real discriminant = (B * B) - (4.0 * A * C);

    if (discriminant < 0.0) {
        return IntersectResult();
    } else {
        // compute intersection points
        Real t1 = (-B + sqrt(discriminant)) / (2.0 * A);
        Real t2 = (-B - sqrt(discriminant)) / (2.0 * A);
        Real closestT = t2;  // = std::min(t1, t2);
        if (t2 < 0.0) {
            closestT = t1;
        }

        // compute normal for lighting
        VEC3 intersectionPoint = ray.origin + (ray.direction * closestT);
        VEC3 normal =
            (intersectionPoint - center) / (intersectionPoint - center).norm();

        return IntersectResult(closestT, true, normal, intersectionPoint, this);
    }
}

Sphere::~Sphere() {}

// TRIANGLE

// call the sphere constructor and the base constructor
Triangle::Triangle(VEC3 a, VEC3 b, VEC3 c, VEC3 color, Material type,
                   Real refractiveIndex)
    : Shape(color, type, refractiveIndex), a(a), b(b), c(c) {}

// Attribution
// https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
IntersectResult Triangle::intersect(Ray ray) {
    // find the normal of the triangle
    // by taking cross product of two vectors in its plane
    VEC3 edge1 = b - a;
    VEC3 edge2 = c - a;
    VEC3 h = ray.direction.cross(edge2);
    VEC3 normal = edge2.cross(edge1);
    normal /= normal.norm();

    // parallel test
    Real v_a = edge1.dot(h);
    if (v_a > -CUSTOM_EPSILON && v_a < CUSTOM_EPSILON) {
        return IntersectResult();
    }

    // out of bounds tests
    Real v_f = 1.0 / v_a;
    VEC3 s = ray.origin - a;
    Real v_u = v_f * s.dot(h);
    if (v_u < 0.0 || v_u > 1.0) {
        return IntersectResult();
    }

    VEC3 q = s.cross(edge1);
    Real v_v = v_f * ray.direction.dot(q);
    if (v_v < 0.0 || v_u + v_v > 1.0) {
        return IntersectResult();
    }

    // compute ray intersection point
    Real t = v_f * edge2.dot(q);
    VEC3 intersectionPoint = ray.origin + ray.direction * t;
    return IntersectResult(t, true, normal, intersectionPoint, this);
}

Triangle::~Triangle() {}