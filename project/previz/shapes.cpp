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

// Cylinder
Cylinder::Cylinder(VEC3 top, VEC3 bottom, Real radius, VEC4 translation,
                   MATRIX4 rotation, VEC3 color, Material type,
                   Real refractiveIndex)
    : Shape(color, type, refractiveIndex),
      top(top),
      bottom(bottom),
      radius(radius),
      translation(translation),
      rotation(rotation) {}

IntersectResult Cylinder::intersect(Ray ray) {
    // * construct the translation (i.e. the center of cylinder)
    VEC3 cylinderCenter = (top + bottom) / 2.0;

    // * construct rotation matrix
    // get cylinder z-axis (going from bottom to top)
    VEC3 cylinderAxisDirection = (top - bottom);
    cylinderAxisDirection /= cylinderAxisDirection.norm();
    // find the unit zAxis
    VEC3 zAxis = VEC3(1.0, 0.0, 0.0);
    zAxis /= zAxis.norm();
    // find the angle between them
    Real thetaBetween = acos(cylinderAxisDirection.dot(zAxis));
    // find an axis of rotation
    VEC3 axisOfRotation = cylinderAxisDirection.cross(zAxis);
    MATRIX3 rotationMatrix;
    rotationMatrix = AngleAxisd(thetaBetween, axisOfRotation);

    // * translate ray origin
    VEC3 trRayOrgn = ray.origin - cylinderCenter;

    // * rotate the ray origin
    trRayOrgn = rotationMatrix.transpose() * trRayOrgn;

    // * rotate the ray direction
    VEC3 trRayDir = rotationMatrix.transpose() * ray.direction;

    // do the canonical intersection test
    Real a = (trRayDir[0] * trRayDir[0]) + (trRayDir[2] * trRayDir[2]);
    Real b =
        (2.0 * trRayOrgn[0] * trRayDir[0]) + (2.0 * trRayOrgn[2] * trRayDir[2]);
    Real c =
        (trRayOrgn[0] * trRayOrgn[0]) + (trRayOrgn[2] * trRayOrgn[2]) - 1.0;

    Real discriminant = (b * b) - (4.0 * a * c);

    if (discriminant < 0.0) {
        return IntersectResult();
    }

    // find values of t
    Real t1 = (-b + sqrt(discriminant)) / (2.0 * a);
    Real t2 = (-b - sqrt(discriminant)) / (2.0 * a);
    Real closestT = t2;
    if (t2 < 0.0) {
        closestT = t1;
    }

    // calculate intersection point
    VEC3 intersectionPoint = trRayOrgn + (trRayDir * closestT);

    // check if intersection point between top and bottom, if not, the ray
    // missed the cylinder
    if (!(intersectionPoint[1] > bottom[1] && intersectionPoint[1] < top[1])) {
        return IntersectResult();
    }

    // calculate normal
    VEC3 axisPointAlignedWithIntersectionPoint =
        VEC3(cylinderCenter[0], cylinderCenter[1], intersectionPoint[2]);
    VEC3 normal = (intersectionPoint - axisPointAlignedWithIntersectionPoint);
    normal /= normal.norm();

    // convert back from local frame/object space to world space
    intersectionPoint = (rotationMatrix * intersectionPoint) + cylinderCenter;
    normal = rotationMatrix * normal;

    return IntersectResult(closestT, true, normal, intersectionPoint, this);
}

Cylinder::~Cylinder() {}