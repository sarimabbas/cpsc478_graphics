#include <assert.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "SETTINGS.h"

using namespace std;

// forward declaration
class Camera;
class Ray;
class Shape;
class Sphere;
class IntersectResult;
class Light;
Real degreesToRadians(Real degrees);
Ray rayGeneration(int pixel_i, int pixel_j, Camera cam);
Ray rayGenerationAlt(int pixel_i, int pixel_j, Camera cam);
float* allocatePPM(int xRes, int yRes);
void initPPM(float* values, int xRes, int yRes);
int indexIntoPPM(int x, int y, int xRes, int yRes,
                 bool originBottomLeft = false);
void writeColorToPPM(VEC3 color, float* ppm, int startIndex);
IntersectResult intersectScene(vector<Shape*> scene, Ray ray, Real tLow);
VEC3 rayColor(vector<Shape*> scene, Ray ray, vector<Light*> lights,
              Real phongExponent, bool useLights, bool useMultipleLights,
              bool useSpecular, bool useShadows, bool useMirror,
              int reflectionRecursionCounter, bool useRefraction,
              bool useFresnel);
VEC3 lightingEquation(Light* light, IntersectResult intersection,
                      Real phongExponent, Ray ray, bool useSpecular);
Real clamp(Real x, Real lower, Real upper);
VEC3 clampVec3(VEC3 vec, Real low, Real high);
bool isPointInShadow(vector<Shape*> scene, Light* light,
                     IntersectResult intersection);
Real CUSTOM_EPSILON = 10000.0 * std::numeric_limits<Real>::epsilon();
enum Material { OPAQUE, MIRROR, DIELECTRIC };
int MAX_RECURSION_DEPTH = 5;
Ray createReflectionRay(IntersectResult intersection, Ray ray);
Ray createRefractionRay(IntersectResult intersection, Ray ray);
Real REFRACT_GLASS = 1.5;
Real REFRACT_AIR = 1.0;
Real fresnel(IntersectResult intersection, Ray ray);

void part_1(Camera cam, vector<Shape*> scene);
void part_2(Camera cam, vector<Shape*> scene);
void part_3(Camera cam, vector<Shape*> scene);
void part_4(Camera cam, vector<Shape*> scene);
void part_5(Camera cam, vector<Shape*> scene);
void part_6(Camera cam, vector<Shape*> scene);
void part_7(Camera cam, vector<Shape*> scene);
void part_8(Camera cam, vector<Shape*> scene);
void part_9(Camera cam, vector<Shape*> scene);

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
           Real distanceToPlane, Real fovy)
        : eye(eye),
          lookAt(lookAt),
          up(up),
          xRes(xRes),
          yRes(yRes),
          distanceToPlane(distanceToPlane),
          fovy(fovy),
          aspect((Real)xRes / (Real)yRes) {}
};

class Ray {
   public:
    VEC3 origin;
    VEC3 direction;

    Ray(VEC3 origin, VEC3 direction) : origin(origin), direction(direction) {}
};

class IntersectResult {
   public:
    Real t;
    bool doesIntersect;
    VEC3 normal;
    VEC3 intersectionPoint;
    Shape* intersectingShape;

    // default constructor
    IntersectResult()
        : t(0.0),
          doesIntersect(false),
          normal(VEC3(0.0, 0.0, 0.0)),
          intersectionPoint(VEC3(0.0, 0.0, 0.0)),
          intersectingShape(NULL) {}

    // named constructor
    IntersectResult(Real t, bool doesIntersect, VEC3 normal,
                    VEC3 intersectionPoint, Shape* intersectingShape)
        : t(t),
          doesIntersect(doesIntersect),
          normal(normal),
          intersectionPoint(intersectionPoint),
          intersectingShape(intersectingShape) {}
};

ostream& operator<<(ostream& out, const Ray& r) {
    out << "[Origin: " << r.origin << ", Direction: " << r.direction << "]";
}

// shape class
class Shape {
   public:
    virtual IntersectResult intersect(Ray ray) = 0;
    VEC3 color;
    Material type;
    Real refractiveIndex;

    // base constructor
    Shape(VEC3 color, Material type, Real refractiveIndex)
        : color(color), type(type), refractiveIndex(refractiveIndex) {}
};

class Sphere : public Shape {
   public:
    Real radius;
    VEC3 center;

    // call the sphere constructor and the base constructor
    Sphere(Real radius, VEC3 center, VEC3 color, Material type,
           Real refractiveIndex)
        : radius(radius), center(center), Shape(color, type, refractiveIndex) {}

    IntersectResult intersect(Ray ray) {
        Real A = ray.direction.dot(ray.direction);
        Real B = (2.0 * ray.direction).dot(ray.origin - center);
        Real C =
            (ray.origin - center).dot(ray.origin - center) - (radius * radius);

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
            VEC3 normal = (intersectionPoint - center) /
                          (intersectionPoint - center).norm();

            return IntersectResult(closestT, true, normal, intersectionPoint,
                                   this);
        }
    }
};

class Light {
   public:
    VEC3 position;
    VEC3 color;

    Light(VEC3 position, VEC3 color) : position(position), color(color) {}
};

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
void readPPM(const string& filename, int& xRes, int& yRes, float*& values) {
    // try to open the file
    FILE* fp;
    fp = fopen(filename.c_str(), "rb");
    if (fp == NULL) {
        cout << " Could not open file \"" << filename.c_str()
             << "\" for reading." << endl;
        cout << " Make sure you're not trying to read from a weird "
                "location or "
                "with a "
             << endl;
        cout << " strange filename. Bailing ... " << endl;
        exit(0);
    }

    // get the dimensions
    fscanf(fp, "P6\n%d %d\n255\n", &xRes, &yRes);
    int totalCells = xRes * yRes;

    // grab the pixel values
    unsigned char* pixels = new unsigned char[3 * totalCells];
    fread(pixels, 1, totalCells * 3, fp);

    // copy to a nicer data type
    values = new float[3 * totalCells];
    for (int i = 0; i < 3 * totalCells; i++) values[i] = pixels[i];

    // clean up
    delete[] pixels;
    fclose(fp);
    cout << " Read in file " << filename.c_str() << endl;
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
void writePPM(const string& filename, int& xRes, int& yRes,
              const float* values) {
    int totalCells = xRes * yRes;
    unsigned char* pixels = new unsigned char[3 * totalCells];
    for (int i = 0; i < 3 * totalCells; i++) pixels[i] = values[i];

    FILE* fp;
    fp = fopen(filename.c_str(), "wb");
    if (fp == NULL) {
        cout << " Could not open file \"" << filename.c_str()
             << "\" for writing." << endl;
        cout << " Make sure you're not trying to write from a weird "
                "location "
                "or with a "
             << endl;
        cout << " strange filename. Bailing ... " << endl;
        exit(0);
    }

    fprintf(fp, "P6\n%d %d\n255\n", xRes, yRes);
    fwrite(pixels, 1, totalCells * 3, fp);
    fclose(fp);
    delete[] pixels;
}

// * ray generation maps
void part_1(Camera cam, vector<Shape*> scene) {
    // create a ray map
    float* ppm = allocatePPM(cam.xRes, cam.yRes);
    for (int i = 0; i < cam.xRes; i++) {
        for (int j = 0; j < cam.yRes; j++) {
            // generate the ray
            Ray ray = rayGeneration(i, j, cam);
            int index = indexIntoPPM(i, j, cam.xRes, cam.yRes, true);
            writeColorToPPM(VEC3(ray.direction[0], 0.0, 0.0), ppm, index);
        }
    }
    // write out to image
    writePPM("1x.ppm", cam.xRes, cam.yRes, ppm);
    delete[] ppm;

    // create a ray map
    ppm = allocatePPM(cam.xRes, cam.yRes);
    for (int i = 0; i < cam.xRes; i++) {
        for (int j = 0; j < cam.yRes; j++) {
            // generate the ray
            Ray ray = rayGeneration(i, j, cam);
            int index = indexIntoPPM(i, j, cam.xRes, cam.yRes, true);
            writeColorToPPM(VEC3(abs(ray.direction[0]), 0.0, 0.0), ppm, index);
        }
    }
    // write out to image
    writePPM("1xabs.ppm", cam.xRes, cam.yRes, ppm);
    delete[] ppm;

    // create a ray map
    ppm = allocatePPM(cam.xRes, cam.yRes);
    for (int i = 0; i < cam.xRes; i++) {
        for (int j = 0; j < cam.yRes; j++) {
            // generate the ray
            Ray ray = rayGeneration(i, j, cam);
            int index = indexIntoPPM(i, j, cam.xRes, cam.yRes, true);
            writeColorToPPM(VEC3(0.0, ray.direction[1], 0.0), ppm, index);
        }
    }
    // write out to image
    writePPM("1y.ppm", cam.xRes, cam.yRes, ppm);
    delete[] ppm;

    // create a ray map
    ppm = allocatePPM(cam.xRes, cam.yRes);
    for (int i = 0; i < cam.xRes; i++) {
        for (int j = 0; j < cam.yRes; j++) {
            // generate the ray
            Ray ray = rayGeneration(i, j, cam);
            int index = indexIntoPPM(i, j, cam.xRes, cam.yRes, true);
            writeColorToPPM(VEC3(0.0, abs(ray.direction[1]), 0.0), ppm, index);
        }
    }
    // write out to image
    writePPM("1yabs.ppm", cam.xRes, cam.yRes, ppm);
    delete[] ppm;
}

// * intersection of scene
void part_2(Camera cam, vector<Shape*> scene) {
    // no lights
    vector<Light*> lights;
    // create a ray map
    float* ppm = allocatePPM(cam.xRes, cam.yRes);
    for (int i = 0; i < cam.xRes; i++) {
        for (int j = 0; j < cam.yRes; j++) {
            // generate the ray
            Ray ray = rayGeneration(i, j, cam);
            // do a scene intersection
            VEC3 color = rayColor(scene, ray, lights, 10.0, false, false, false,
                                  false, false, 0, false, false);
            // color the pixel
            int index = indexIntoPPM(i, j, cam.xRes, cam.yRes, true);
            ppm[index] = color[0] * 255.0;
            ppm[index + 1] = color[1] * 255.0;
            ppm[index + 2] = color[2] * 255.0;
        }
    }
    // write out to image
    writePPM("2.ppm", cam.xRes, cam.yRes, ppm);
    delete[] ppm;
}

// * diffuse shading
void part_3(Camera cam, vector<Shape*> scene) {
    // add lights
    Light one = Light(VEC3(10.0, 3.0, 5.0), VEC3(1.0, 1.0, 1.0));
    Light two = Light(VEC3(-10.0, 3.0, 7.5), VEC3(0.5, 0.0, 0.0));
    vector<Light*> lights;
    lights.push_back(&one);
    lights.push_back(&two);
    Real phongExponent = 10.0;
    bool useLights = true;
    bool useMultipleLights = false;
    bool useSpecular = false;
    bool useShadows = false;
    bool useMirror = false;
    int reflectionRecursionCounter = 0;
    bool useRefraction = false;
    bool useFresnel = false;

    // create a ray map
    float* ppm = allocatePPM(cam.xRes, cam.yRes);
    for (int i = 0; i < cam.xRes; i++) {
        for (int j = 0; j < cam.yRes; j++) {
            // generate the ray
            Ray ray = rayGeneration(i, j, cam);
            // do a scene intersection
            VEC3 color =
                rayColor(scene, ray, lights, phongExponent, useLights,
                         useMultipleLights, useSpecular, useShadows, useMirror,
                         reflectionRecursionCounter, useRefraction, useFresnel);
            // color the pixel
            int index = indexIntoPPM(i, j, cam.xRes, cam.yRes, true);
            ppm[index] = color[0] * 255.0;
            ppm[index + 1] = color[1] * 255.0;
            ppm[index + 2] = color[2] * 255.0;
        }
    }
    // write out to image
    writePPM("3.ppm", cam.xRes, cam.yRes, ppm);
    delete[] ppm;
}

// * multiple lights
void part_4(Camera cam, vector<Shape*> scene) {
    // add lights
    Light one = Light(VEC3(10.0, 3.0, 5.0), VEC3(1.0, 1.0, 1.0));
    Light two = Light(VEC3(-10.0, 3.0, 7.5), VEC3(0.5, 0.0, 0.0));
    vector<Light*> lights;
    lights.push_back(&one);
    lights.push_back(&two);
    Real phongExponent = 10.0;
    bool useLights = true;
    bool useMultipleLights = true;
    bool useSpecular = false;
    bool useShadows = false;
    bool useMirror = false;
    int reflectionRecursionCounter = 0;
    bool useRefraction = false;
    bool useFresnel = false;

    // create a ray map
    float* ppm = allocatePPM(cam.xRes, cam.yRes);
    for (int i = 0; i < cam.xRes; i++) {
        for (int j = 0; j < cam.yRes; j++) {
            // generate the ray
            Ray ray = rayGeneration(i, j, cam);
            // do a scene intersection
            VEC3 color =
                rayColor(scene, ray, lights, phongExponent, useLights,
                         useMultipleLights, useSpecular, useShadows, useMirror,
                         reflectionRecursionCounter, useRefraction, useFresnel);
            // color the pixel
            int index = indexIntoPPM(i, j, cam.xRes, cam.yRes, true);
            ppm[index] = color[0] * 255.0;
            ppm[index + 1] = color[1] * 255.0;
            ppm[index + 2] = color[2] * 255.0;
        }
    }
    // write out to image
    writePPM("4.ppm", cam.xRes, cam.yRes, ppm);
    delete[] ppm;
}

// * specular reflections
void part_5(Camera cam, vector<Shape*> scene) {
    // add lights
    Light one = Light(VEC3(10.0, 3.0, 5.0), VEC3(1.0, 1.0, 1.0));
    Light two = Light(VEC3(-10.0, 3.0, 7.5), VEC3(0.5, 0.0, 0.0));
    vector<Light*> lights;
    lights.push_back(&one);
    lights.push_back(&two);
    Real phongExponent = 10.0;
    bool useLights = true;
    bool useMultipleLights = true;
    bool useSpecular = true;
    bool useShadows = false;
    bool useMirror = false;
    int reflectionRecursionCounter = 0;
    bool useRefraction = false;
    bool useFresnel = false;

    // create a ray map
    float* ppm = allocatePPM(cam.xRes, cam.yRes);
    for (int i = 0; i < cam.xRes; i++) {
        for (int j = 0; j < cam.yRes; j++) {
            // generate the ray
            Ray ray = rayGeneration(i, j, cam);
            // do a scene intersection
            VEC3 color =
                rayColor(scene, ray, lights, phongExponent, useLights,
                         useMultipleLights, useSpecular, useShadows, useMirror,
                         reflectionRecursionCounter, useRefraction, useFresnel);
            // color the pixel
            int index = indexIntoPPM(i, j, cam.xRes, cam.yRes, true);
            ppm[index] = color[0] * 255.0;
            ppm[index + 1] = color[1] * 255.0;
            ppm[index + 2] = color[2] * 255.0;
        }
    }
    // write out to image
    writePPM("5.ppm", cam.xRes, cam.yRes, ppm);
    delete[] ppm;
}

// * shadows
void part_6(Camera cam, vector<Shape*> scene) {
    // add lights
    Light one = Light(VEC3(10.0, 3.0, 5.0), VEC3(1.0, 1.0, 1.0));
    Light two = Light(VEC3(-10.0, 3.0, 7.5), VEC3(0.5, 0.0, 0.0));
    vector<Light*> lights;
    lights.push_back(&one);
    lights.push_back(&two);
    Real phongExponent = 10.0;
    bool useLights = true;
    bool useMultipleLights = true;
    bool useSpecular = true;
    bool useShadows = true;
    bool useMirror = false;
    int reflectionRecursionCounter = 0;
    bool useRefraction = false;
    bool useFresnel = false;

    // create a ray map
    float* ppm = allocatePPM(cam.xRes, cam.yRes);
    for (int i = 0; i < cam.xRes; i++) {
        for (int j = 0; j < cam.yRes; j++) {
            // generate the ray
            Ray ray = rayGeneration(i, j, cam);
            // do a scene intersection
            VEC3 color =
                rayColor(scene, ray, lights, phongExponent, useLights,
                         useMultipleLights, useSpecular, useShadows, useMirror,
                         reflectionRecursionCounter, useRefraction, useFresnel);
            // color the pixel
            int index = indexIntoPPM(i, j, cam.xRes, cam.yRes, true);
            ppm[index] = color[0] * 255.0;
            ppm[index + 1] = color[1] * 255.0;
            ppm[index + 2] = color[2] * 255.0;
        }
    }
    // write out to image
    writePPM("6.ppm", cam.xRes, cam.yRes, ppm);
    delete[] ppm;
}

// * mirror reflections
void part_7(Camera cam, vector<Shape*> scene) {
    // modify the scene and add the wall of spheres
    vector<Shape*> sceneCopy = scene;
    vector<Sphere> wallOfSpheres;

    for (int i = -20; i < 20; i += 2) {
        for (int j = -2; j < 18; j += 2) {
            wallOfSpheres.push_back(Sphere(1.0, VEC3((Real)i, (Real)j, 20.0),
                                           VEC3(1.0, 1.0, 1.0), OPAQUE, 0.0));
        }
    }

    // for (int i = -12; i < 0; i += 2) {
    //     for (int j = -2; j < 4; j += 2) {
    //         wallOfSpheres.push_back(Sphere(1.0, VEC3((Real)i, (Real)j, 20.0),
    //                                        VEC3(1.0, 1.0, 1.0), OPAQUE,
    //                                        0.0));
    //     }
    // }

    for (int i = 0; i < wallOfSpheres.size(); i++) {
        sceneCopy.push_back(&(wallOfSpheres[i]));
    }

    // make the first sphere black and a mirror
    sceneCopy[0]->type = MIRROR;
    sceneCopy[0]->color = VEC3(0.0, 0.0, 0.0);

    // add lights
    Light one = Light(VEC3(10.0, 10.0, 5.0), VEC3(1.0, 1.0, 1.0));
    Light two = Light(VEC3(-10.0, 10.0, 7.5), VEC3(0.5, 0.25, 0.25));
    vector<Light*> lights;
    lights.push_back(&one);
    lights.push_back(&two);
    Real phongExponent = 10.0;
    bool useLights = true;
    bool useMultipleLights = true;
    bool useSpecular = true;
    bool useShadows = true;
    bool useMirror = true;
    int reflectionRecursionCounter = 0;
    bool useRefraction = false;
    bool useFresnel = false;

    // create a ray map
    float* ppm = allocatePPM(cam.xRes, cam.yRes);
    for (int i = 0; i < cam.xRes; i++) {
        for (int j = 0; j < cam.yRes; j++) {
            // if (i == 318 && j == 214) {
            //     cout << "hello" << endl;
            // }

            // generate the ray
            Ray ray = rayGeneration(i, j, cam);
            // do a scene intersection
            VEC3 color =
                rayColor(sceneCopy, ray, lights, phongExponent, useLights,
                         useMultipleLights, useSpecular, useShadows, useMirror,
                         reflectionRecursionCounter, useRefraction, useFresnel);

            // if (i == 318 && j == 214) {
            //     cout << color[0] << " " << color[1] << " " << color[2] <<
            //     endl;
            // }

            // color the pixel
            int index = indexIntoPPM(i, j, cam.xRes, cam.yRes, true);
            ppm[index] = color[0] * 255.0;
            ppm[index + 1] = color[1] * 255.0;
            ppm[index + 2] = color[2] * 255.0;
        }
    }
    // write out to image
    writePPM("7.ppm", cam.xRes, cam.yRes, ppm);
    delete[] ppm;
}

// * refractions
void part_8(Camera cam, vector<Shape*> scene) {
    // modify the scene and add the wall of spheres
    vector<Shape*> sceneCopy = scene;
    vector<Sphere> wallOfSpheres;

    // for (int i = -12; i < 0; i += 2) {
    //     for (int j = -2; j < 4; j += 2) {
    //         wallOfSpheres.push_back(Sphere(1.0, VEC3((Real)i, (Real)j, 20.0),
    //                                        VEC3(1.0, 1.0, 1.0), OPAQUE,
    //                                        0.0));
    //     }
    // }

    for (int i = -20; i < 20; i += 2) {
        for (int j = -2; j < 18; j += 2) {
            wallOfSpheres.push_back(Sphere(1.0, VEC3((Real)i, (Real)j, 20.0),
                                           VEC3(1.0, 1.0, 1.0), OPAQUE, 0.0));
        }
    }

    for (int i = 0; i < wallOfSpheres.size(); i++) {
        sceneCopy.push_back(&(wallOfSpheres[i]));
    }

    // make the first sphere black, and glass
    sceneCopy[0]->color = VEC3(0.0, 0.0, 0.0);
    sceneCopy[0]->type = DIELECTRIC;
    sceneCopy[0]->refractiveIndex = REFRACT_GLASS;

    // add lights
    Light one = Light(VEC3(10.0, 10.0, 5.0), VEC3(1.0, 1.0, 1.0));
    Light two = Light(VEC3(-10.0, 10.0, 7.5), VEC3(0.5, 0.25, 0.25));
    vector<Light*> lights;
    lights.push_back(&one);
    lights.push_back(&two);
    Real phongExponent = 10.0;
    bool useLights = true;
    bool useMultipleLights = true;
    bool useSpecular = true;
    bool useShadows = true;
    bool useMirror = true;
    int reflectionRecursionCounter = 0;
    bool useRefraction = true;
    bool useFresnel = false;

    // create a ray map
    float* ppm = allocatePPM(cam.xRes, cam.yRes);
    for (int i = 0; i < cam.xRes; i++) {
        for (int j = 0; j < cam.yRes; j++) {
            // generate the ray
            Ray ray = rayGeneration(i, j, cam);
            // do a scene intersection
            VEC3 color =
                rayColor(sceneCopy, ray, lights, phongExponent, useLights,
                         useMultipleLights, useSpecular, useShadows, useMirror,
                         reflectionRecursionCounter, useRefraction, useFresnel);
            // color the pixel
            int index = indexIntoPPM(i, j, cam.xRes, cam.yRes, true);
            ppm[index] = color[0] * 255.0;
            ppm[index + 1] = color[1] * 255.0;
            ppm[index + 2] = color[2] * 255.0;
        }
    }
    // write out to image
    writePPM("8.ppm", cam.xRes, cam.yRes, ppm);
    delete[] ppm;
}

// * fresnel effect
void part_9(Camera cam, vector<Shape*> scene) {
    // modify the scene and add the wall of spheres
    vector<Shape*> sceneCopy = scene;
    vector<Sphere> wallOfSpheres;

    // for (int i = -12; i < 0; i += 2) {
    //     for (int j = -2; j < 4; j += 2) {
    //         wallOfSpheres.push_back(Sphere(1.0, VEC3((Real)i, (Real)j, 20.0),
    //                                        VEC3(1.0, 1.0, 1.0), OPAQUE,
    //                                        0.0));
    //     }
    // }

    for (int i = -20; i < 20; i += 2) {
        for (int j = -2; j < 18; j += 2) {
            wallOfSpheres.push_back(Sphere(1.0, VEC3((Real)i, (Real)j, 20.0),
                                           VEC3(1.0, 1.0, 1.0), OPAQUE, 0.0));
        }
    }

    for (int i = 0; i < wallOfSpheres.size(); i++) {
        sceneCopy.push_back(&(wallOfSpheres[i]));
    }

    // make the first sphere black, and glass
    sceneCopy[0]->color = VEC3(0.0, 0.0, 0.0);
    sceneCopy[0]->type = DIELECTRIC;
    sceneCopy[0]->refractiveIndex = REFRACT_GLASS;

    // add lights
    Light one = Light(VEC3(10.0, 10.0, 5.0), VEC3(1.0, 1.0, 1.0));
    Light two = Light(VEC3(-10.0, 10.0, 7.5), VEC3(0.5, 0.25, 0.25));
    vector<Light*> lights;
    lights.push_back(&one);
    lights.push_back(&two);
    Real phongExponent = 10.0;
    bool useLights = true;
    bool useMultipleLights = true;
    bool useSpecular = true;
    bool useShadows = true;
    bool useMirror = true;
    int reflectionRecursionCounter = 0;
    bool useRefraction = true;
    bool useFresnel = true;

    // create a ray map
    float* ppm = allocatePPM(cam.xRes, cam.yRes);
    for (int i = 0; i < cam.xRes; i++) {
        for (int j = 0; j < cam.yRes; j++) {
            // generate the ray
            Ray ray = rayGeneration(i, j, cam);
            // do a scene intersection
            VEC3 color =
                rayColor(sceneCopy, ray, lights, phongExponent, useLights,
                         useMultipleLights, useSpecular, useShadows, useMirror,
                         reflectionRecursionCounter, useRefraction, useFresnel);
            // color the pixel
            int index = indexIntoPPM(i, j, cam.xRes, cam.yRes, true);
            ppm[index] = color[0] * 255.0;
            ppm[index + 1] = color[1] * 255.0;
            ppm[index + 2] = color[2] * 255.0;
        }
    }
    // write out to image
    writePPM("9.ppm", cam.xRes, cam.yRes, ppm);
    delete[] ppm;
}

int main(int argc, char** argv) {
    int xRes = 800;
    int yRes = 600;
    VEC3 eye = VEC3(0.0, 0.0, 0.0);
    VEC3 lookAt = VEC3(0.0, 0.0, 1.0);
    VEC3 up = VEC3(0.0, 1.0, 0.0);
    Real distanceToNearPlane = 1.0;
    Real fovy = 65;
    Camera cam = Camera(eye, lookAt, up, xRes, yRes, distanceToNearPlane, fovy);

    // scene geometry
    vector<Shape*> scene;
    Sphere one =
        Sphere(3.0, VEC3(-3.5, 0.0, 10.0), VEC3(1.0, 0.25, 0.25), OPAQUE, 0.0);
    Sphere two =
        Sphere(3.0, VEC3(3.5, 0.0, 10.0), VEC3(0.25, 0.25, 1.0), OPAQUE, 0.0);
    Sphere three = Sphere(997.0, VEC3(0.0, -1000.0, 10.0), VEC3(0.5, 0.5, 0.5),
                          OPAQUE, 0.0);
    scene.push_back(&one);
    scene.push_back(&two);
    scene.push_back(&three);

    // part_1(cam, scene);
    // part_2(cam, scene);
    // part_3(cam, scene);
    // part_4(cam, scene);
    // part_5(cam, scene);
    // part_6(cam, scene);
    // part_7(cam, scene);
    // part_8(cam, scene);
    part_9(cam, scene);

    return 0;
}

VEC3 rayColor(vector<Shape*> scene, Ray ray, vector<Light*> lights,
              Real phongExponent, bool useLights, bool useMultipleLights,
              bool useSpecular, bool useShadows, bool useMirror,
              int reflectionRecursionCounter, bool useRefraction,
              bool useFresnel) {
    // do an intersection with the scene
    IntersectResult intersection = intersectScene(scene, ray, 0.0);

    // no intersection (return black)
    if (intersection.intersectingShape == NULL) {
        return VEC3(0.0, 0.0, 0.0);  // black background
    }

    // initialize with ambient
    VEC3 color = VEC3(0.0, 0.0, 0.0);

    // else if there is an intersection, do shading with lights
    if (useLights) {
        // for each light
        for (int i = 0; i < lights.size(); i++) {
            // shoot shadow ray
            if (useShadows) {
                // if not in shadow, add diffuse and specular, else skip
                if (!isPointInShadow(scene, lights[i], intersection)) {
                    color += lightingEquation(lights[i], intersection,
                                              phongExponent, ray, useSpecular);
                }
            }
            // only use the first light if flag not set
            if (!useMultipleLights) {
                break;
            }
        }
    } else {
        color += intersection.intersectingShape->color;
    }

    if (useMirror) {
        if (intersection.intersectingShape->type == MIRROR &&
            reflectionRecursionCounter != MAX_RECURSION_DEPTH) {
            Ray reflectionRay = createReflectionRay(intersection, ray);
            // call the reflection recursively
            VEC3 reflectionColor = rayColor(
                scene, reflectionRay, lights, phongExponent, useLights,
                useMultipleLights, useSpecular, useShadows, useMirror,
                reflectionRecursionCounter + 1, useRefraction, useFresnel);
            color += reflectionColor;
        }
    }

    if (useRefraction) {
        if (intersection.intersectingShape->type == DIELECTRIC &&
            reflectionRecursionCounter != MAX_RECURSION_DEPTH) {
            if (useFresnel) {
                // get fresnel coefficients
                Real kReflectance = fresnel(intersection, ray);
                Real kRefraction = 1.0 - kReflectance;
                Ray refractionRay = createRefractionRay(intersection, ray);
                Ray reflectionRay = createReflectionRay(intersection, ray);
                VEC3 reflectionColor = rayColor(
                    scene, reflectionRay, lights, phongExponent, useLights,
                    useMultipleLights, useSpecular, useShadows, useMirror,
                    reflectionRecursionCounter + 1, useRefraction, useFresnel);
                VEC3 refractionColor = rayColor(
                    scene, refractionRay, lights, phongExponent, useLights,
                    useMultipleLights, useSpecular, useShadows, useMirror,
                    reflectionRecursionCounter + 1, useRefraction, useFresnel);
                color += (kReflectance * reflectionColor) +
                         (kRefraction * refractionColor);

            } else {
                // if entering dielectric
                Ray refractionRay = createRefractionRay(intersection, ray);
                VEC3 refractionColor = rayColor(
                    scene, refractionRay, lights, phongExponent, useLights,
                    useMultipleLights, useSpecular, useShadows, useMirror,
                    reflectionRecursionCounter + 1, useRefraction, useFresnel);
                color += refractionColor;
            }
        }
    }

    // TODO: is this required?
    return clampVec3(color, 0.0, 1.0);
}

bool isPointInShadow(vector<Shape*> scene, Light* light,
                     IntersectResult intersection) {
    // adjust to avoid shadow acne problem
    VEC3 adjustedIntersectionPoint =
        intersection.intersectionPoint + (CUSTOM_EPSILON * intersection.normal);
    // generate shadow ray towards the light
    VEC3 shadowDirection = (light->position - adjustedIntersectionPoint);
    shadowDirection /= shadowDirection.norm();
    Ray shadowRay(adjustedIntersectionPoint, shadowDirection);
    // see if it intersects anything
    IntersectResult shadowIntersect = intersectScene(scene, shadowRay, 0.0);
    if (shadowIntersect.doesIntersect) {
        return true;
    }
}

Ray createReflectionRay(IntersectResult intersection, Ray ray) {
    // adjust to avoid shadow acne problem
    VEC3 adjustedIntersectionPoint =
        intersection.intersectionPoint + (CUSTOM_EPSILON * intersection.normal);
    // find the vector from intersection towards eye
    VEC3 V = (ray.origin - adjustedIntersectionPoint);
    V /= V.norm();
    // get the reflection direction
    VEC3 R = -V + (2.0 * V.dot(intersection.normal) * intersection.normal);
    R /= R.norm();
    // generate ray
    Ray reflectionRay(adjustedIntersectionPoint, R);
    return reflectionRay;
}

Real fresnel(IntersectResult intersection, Ray ray) {
    Real kReflectance;

    VEC3 N = intersection.normal;

    // incidence
    VEC3 I = (intersection.intersectionPoint - ray.origin);
    I /= I.norm();

    Real cosi = clamp(-1.0, 1.0, I.dot(N));
    Real etai = 1.0;
    Real etat = intersection.intersectingShape->refractiveIndex;
    if (cosi > 0.0) {
        std::swap(etai, etat);
    }
    // Compute sini using Snell's law
    Real sint = etai / etat * sqrt(std::max(0.0, 1.0 - cosi * cosi));
    // Total internal reflection
    if (sint >= 1.0) {
        kReflectance = 1.0;
    } else {
        Real cost = sqrt(std::max(0.0, 1.0 - sint * sint));
        cosi = fabs(cosi);
        Real Rs =
            ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
        Real Rp =
            ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
        kReflectance = (Rs * Rs + Rp * Rp) / 2.0;
    }

    return kReflectance;
    // As a consequence of the conservation of energy, transmittance is given
    // by: kt = 1 - kr;
}

Ray createRefractionRay(IntersectResult intersection, Ray ray) {
    // normal
    VEC3 N = intersection.normal;

    // incidence
    VEC3 I = (intersection.intersectionPoint - ray.origin);
    I /= I.norm();

    // reflection
    VEC3 R = -I + (2.0 * I.dot(N) * N);
    R /= R.norm();

    Real cosi = clamp(-1.0, 1.0, I.dot(N));
    Real etai = REFRACT_AIR;
    Real etat = intersection.intersectingShape->refractiveIndex;
    VEC3 n = N;
    if (cosi < 0) {
        cosi = -cosi;
    } else {
        std::swap(etai, etat);
        n = -N;
    }
    Real eta = etai / etat;
    Real k = 1.0 - eta * eta * (1.0 - cosi * cosi);

    VEC3 adjustedPoint = intersection.intersectionPoint - (n * CUSTOM_EPSILON);
    if (k < 0.0) {
        Ray reflectedRay = Ray(adjustedPoint, R);
        return reflectedRay;
    } else {
        VEC3 T = eta * I + (eta * cosi - sqrt(k)) * n;
        Ray refractedRay = Ray(adjustedPoint, T);
        return refractedRay;
    }
}

VEC3 lightingEquation(Light* light, IntersectResult intersection,
                      Real phongExponent, Ray ray, bool useSpecular) {
    // * calculate the three vectors
    // calculate L vec from the intersectionPoint and the light position
    VEC3 L = (light->position - intersection.intersectionPoint);
    L /= L.norm();
    // calculate V from the intersectionPoint and eye position
    VEC3 V = (ray.origin - intersection.intersectionPoint);
    V /= V.norm();
    // calculate the r vector from the dot product of L and normal
    VEC3 R = -L + (2 * L.dot(intersection.normal) * intersection.normal);
    R /= R.norm();

    // * calculate the three components
    // ambient
    VEC3 ambientColor = VEC3(0.0, 0.0, 0.0);  // TODO: what's this?
    Real ambientIntensity = 1.0;              // TODO: what's this?
    VEC3 ambientComponent = ambientColor * ambientIntensity;
    // diffuse
    VEC3 diffuseComponent = intersection.intersectingShape->color.cwiseProduct(
        light->color * std::max(0.0, intersection.normal.dot(L)));
    // diffuseComponent = clampVec3(diffuseComponent, 0.0, 1.0);

    // specular // TODO: apply phong exponent, but where?
    VEC3 specularComponent = intersection.intersectingShape->color.cwiseProduct(
        light->color * pow(std::max(0.0, R.dot(V)), phongExponent));
    // specularComponent = clampVec3(specularComponent, 0.0, 1.0);

    // * assemble the final color
    VEC3 finalColor;
    if (useSpecular) {
        // full shading for part 5.png
        finalColor = ambientComponent + diffuseComponent + specularComponent;
    } else {
        // diffuse shading for part 3.png
        finalColor = ambientComponent + diffuseComponent;
    }
    return clampVec3(finalColor, 0.0, 1.0);
}

IntersectResult intersectScene(vector<Shape*> scene, Ray ray, Real tLow) {
    // for each primitive in the scene
    // keep track of the closest hit
    Real closestT = INFINITY;
    // Shape* closestShape = NULL;
    IntersectResult closestValidIntersection = IntersectResult();
    for (int i = 0; i < scene.size(); i++) {
        // get intersection result
        IntersectResult result = scene[i]->intersect(ray);
        // if there was an intersection
        if (result.doesIntersect == true) {
            // and the intersection had a positive t intersect
            if (result.t >= 0.0 && result.t < closestT) {
                // update the closest intersection
                closestT = result.t;
                closestValidIntersection = result;
            }
        }
    }
    return closestValidIntersection;
}

Ray rayGeneration(int pixel_i, int pixel_j, Camera cam) {
    // construct an eye coordinate frame
    // TODO: felicia says she used squaredNorm, but it doesn't make a
    // difference
    VEC3 gaze = cam.lookAt - cam.eye;
    VEC3 W = -gaze / gaze.norm();
    VEC3 upCrossW = cam.up.cross(W);
    VEC3 U = upCrossW / upCrossW.norm();
    VEC3 V = W.cross(U);

    // convert fovy and aspect to top, right, bottom, left
    Real rasterHeight =
        2.0 * cam.distanceToPlane * tan(degreesToRadians(cam.fovy) / 2.0);
    Real rasterWidth = rasterHeight * cam.aspect;
    VEC3 center = cam.eye - (cam.distanceToPlane * W);
    VEC3 bottomLeft =
        center - (U * (rasterWidth / 2.0)) - (V * (rasterHeight / 2.0));
    VEC3 bottomRight =
        center + (U * (rasterWidth / 2.0)) - (V * (rasterHeight / 2.0));
    VEC3 topRight =
        center + (U * (rasterWidth / 2.0)) + (V * (rasterHeight / 2.0));

    // assert that the z-coordinates of everything are the same (they should
    // be)
    assert((bottomLeft[2] == bottomRight[2]) &&
           (bottomLeft[2] == topRight[2]) && (bottomRight[2] == topRight[2]));
    // assert that the plane is, in fact, a plane (paranoia)
    assert((bottomRight[0] == topRight[0]) &&
           (bottomLeft[1] == bottomRight[1]));

    Real top = topRight[1];
    Real right = topRight[0];
    Real bottom = bottomRight[1];
    Real left = bottomLeft[0];

    // cout << "T, R, B, L: " << top << ", " << right << ", " << bottom <<
    // ", "
    //      << left << "]" << endl;

    // assert that the bounds found this way match up with another way
    Real rightMinusLeft = (bottomRight - bottomLeft).norm();
    Real topMinusBottom = (topRight - bottomRight).norm();

    // cout << top - bottom << ":" << topMinusBottom << endl;
    // cout << right - left << ":" << rightMinusLeft << endl;

    // assert(((top - bottom) == topMinusBottom) &&
    //        ((right - left) == rightMinusLeft));

    // go from pixel coordinates to orthographic raster
    // coordinates. we offset the ray by half to make it
    // pass through the pixel center on the raster image
    Real u_raster =
        left + (((right - left) * ((Real)pixel_i + 0.5)) / (Real)cam.xRes);
    Real v_raster =
        bottom + (((top - bottom) * ((Real)pixel_j + 0.5)) / (Real)cam.yRes);

    // go from orthographic raster coordinates to perspective raster
    // coordinates
    Ray generate = Ray(
        cam.eye, (-cam.distanceToPlane * W) + (u_raster * U) + (v_raster * V));

    // cout << "[Origin: "
    //      << "(" << generate.origin[0] << ", " << generate.origin[1] << ",
    //      "
    //      << generate.origin[2] << "), ";

    // cout << "Direction: "
    //      << "(" << generate.direction[0] << ", " << generate.direction[1]
    //      << ", " << generate.direction[2] << "]" << endl;

    return generate;
}

Real degreesToRadians(Real degrees) { return (degrees)*M_PI / 180.0; }

VEC3 clampVec3(VEC3 vec, Real low, Real high) {
    VEC3 copy = vec;
    for (int i = 0; i < 3; i++) {
        copy[i] = clamp(copy[i], low, high);
    }
    return copy;
}

Real clamp(Real x, Real lower, Real upper) {
    return std::min(upper, std::max(x, lower));
}

float* allocatePPM(int xRes, int yRes) {
    float* values = new float[3 * xRes * yRes];
    if (!values) {
        cout << "Could not allocate values" << endl;
        return NULL;
    }
    initPPM(values, xRes, yRes);
    return values;
}

void initPPM(float* values, int xRes, int yRes) {
    for (int i = 0; i < xRes * yRes * 3; i += 3) {
        values[i] = 0;
        values[i + 1] = 0;
        values[i + 2] = 0;
    }
}

int indexIntoPPM(int x, int y, int xRes, int yRes, bool originBottomLeft) {
    int index = -1;

    if (originBottomLeft) {
        // bottom left origin
        int newY = yRes - 1 - y;
        index = (newY * xRes * 3) + (x * 3);

    } else {
        // top left origin (default)
        index = (y * xRes * 3) + (x * 3);
    }

    return index;
}

void writeColorToPPM(VEC3 color, float* ppm, int startIndex) {
    int index = startIndex;
    for (int i = 0; i < 3; i++) {
        if (color[i] < 0.0) {
            ppm[index] = 0.0;
        } else {
            ppm[index] = color[i] * 255;
        }
        index++;
    }
}