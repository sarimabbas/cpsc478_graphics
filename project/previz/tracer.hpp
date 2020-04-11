#pragma once

#include <assert.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "SETTINGS.h"
#include "shapes.hpp"
#include "utilities.hpp"

using namespace std;

// forward declarations from shapes to prevent circular dependency
class Shape;

// primitives
class Camera {
   public:
    VEC3 eye;
    VEC3 lookAt;
    VEC3 up;
    int xRes;
    int yRes;
    Real distanceToPlane;
    Real fovy;
    Real aspect;

    Camera(VEC3 eye, VEC3 lookAt, VEC3 up, int xRes, int yRes,
           Real distanceToPlane, Real fovy);
};

class Light {
   public:
    VEC3 position;
    VEC3 color;

    Light(VEC3 position, VEC3 color);
};

class Ray {
   public:
    VEC3 origin;
    VEC3 direction;

    Ray(VEC3 origin, VEC3 direction);
};

class IntersectResult {
   public:
    Real t;
    bool doesIntersect;
    VEC3 normal;
    VEC3 intersectionPoint;
    Shape* intersectingShape;

    // default constructor
    IntersectResult();

    // named constructor
    IntersectResult(Real t, bool doesIntersect, VEC3 normal,
                    VEC3 intersectionPoint, Shape* intersectingShape);
};

// basic tracer code
Ray rayGeneration(int pixel_i, int pixel_j, Camera cam);
IntersectResult intersectScene(vector<Shape*> scene, Ray ray, Real tLow);
VEC3 rayColor(vector<Shape*> scene, Ray ray, vector<Light*> lights,
              Real phongExponent, bool useLights, bool useMultipleLights,
              bool useSpecular, bool useShadows, bool useMirror,
              int reflectionRecursionCounter, bool useRefraction,
              bool useFresnel);

// advanced tracer effects
VEC3 lightingEquation(Light* light, IntersectResult intersection,
                      Real phongExponent, Ray ray, bool useSpecular);
bool isPointInShadow(vector<Shape*> scene, Light* light,
                     IntersectResult intersection);
Ray createReflectionRay(IntersectResult intersection, Ray ray);
Ray createRefractionRay(IntersectResult intersection, Ray ray);
Real fresnel(IntersectResult intersection, Ray ray);
