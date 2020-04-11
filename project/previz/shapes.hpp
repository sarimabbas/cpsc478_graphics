#pragma once

#include <assert.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "SETTINGS.h"
#include "tracer.hpp"
#include "utilities.hpp"

using namespace std;

extern Real REFRACT_GLASS;
extern Real REFRACT_AIR;

enum Material { OPAQUE, MIRROR, DIELECTRIC };

// forward declarations from tracer to prevent circular dependency
class Ray;
class IntersectResult;

class Shape {
   public:
    virtual IntersectResult intersect(Ray ray) = 0;
    VEC3 color;
    Material type;
    Real refractiveIndex;

    // base constructor
    Shape(VEC3 color, Material type, Real refractiveIndex);
};

class Sphere : public Shape {
   public:
    Real radius;
    VEC3 center;

    // call the sphere constructor and the base constructor
    Sphere(Real radius, VEC3 center, VEC3 color, Material type,
           Real refractiveIndex);

    IntersectResult intersect(Ray ray);
};

class Triangle : public Shape {
   public:
    VEC3 a;
    VEC3 b;
    VEC3 c;

    // call the sphere constructor and the base constructor
    Triangle(VEC3 a, VEC3 b, VEC3 c, VEC3 color, Material type,
             Real refractiveIndex);

    // Attribution
    // https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
    IntersectResult intersect(Ray ray);
};
// class Cylinder;