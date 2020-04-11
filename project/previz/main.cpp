#include <assert.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "SETTINGS.h"
#include "animation.hpp"
#include "utilities.hpp"

using namespace std;

// Real CUSTOM_EPSILON = generateEpsilon();

// forward declaration

// int MAX_RECURSION_DEPTH = 10;

// Real REFRACT_GLASS = 1.5;
// Real REFRACT_AIR = 1.0;

// ostream& operator<<(ostream& out, const Ray& r) {
//     out << "[Origin: " << r.origin << ", Direction: " << r.direction << "]";
// }

// * triangles are mirrors
// void part_11(Camera cam, vector<Shape*> scene) {
// // modify the scene and add the wall of spheres
// vector<Shape*> sceneCopy = scene;

// // remove the second sphere
// sceneCopy.erase(sceneCopy.begin() + 1);

// vector<Sphere> wallOfSpheres;

// for (int i = -20; i < 20; i += 2) {
//     for (int j = -2; j < 18; j += 2) {
//         wallOfSpheres.push_back(Sphere(1.0, VEC3((Real)i, (Real)j, 20.0),
//                                        VEC3(1.0, 1.0, 1.0), OPAQUE,
//                                        0.0));
//     }
// }

// for (int i = 0; i < wallOfSpheres.size(); i++) {
//     sceneCopy.push_back(&(wallOfSpheres[i]));
// }

// // make the first sphere black, and glass
// sceneCopy[0]->color = VEC3(0.0, 0.0, 0.0);
// sceneCopy[0]->type = DIELECTRIC;
// sceneCopy[0]->refractiveIndex = REFRACT_GLASS;

// // transform
// MATRIX3 rotation;
// rotation.setZero();
// rotation(0, 0) = cos(degreesToRadians(45));
// rotation(0, 2) = sin(degreesToRadians(45));
// rotation(1, 1) = 1.0;
// rotation(2, 0) = -sin(degreesToRadians(45));
// rotation(2, 2) = cos(degreesToRadians(45));

// // prepare the vertices
// VEC3 triangleVertices[4] = {VEC3(0.5, -3.0, 10), VEC3(6.5, -3.0, 10.0),
//                             VEC3(6.5, 3.0, 10.0), VEC3(0.5, 3.0, 10.0)};

// for (int i = 0; i < 4; i++) {
//     triangleVertices[i] = triangleVertices[i] - VEC3(3.5, 0.0, 10.0);
//     triangleVertices[i] = rotation * triangleVertices[i];
//     triangleVertices[i] = triangleVertices[i] + VEC3(3.5, 0.0, 10.0);
// }

// Triangle triangleOne =
//     Triangle(triangleVertices[0], triangleVertices[2],
//     triangleVertices[3],
//              VEC3(0.0, 0.0, 0.0), MIRROR, 0.0);
// Triangle triangleTwo =
//     Triangle(triangleVertices[0], triangleVertices[1],
//     triangleVertices[2],
//              VEC3(0.0, 0.0, 0.0), MIRROR, 0.0);
// sceneCopy.push_back(&triangleOne);
// sceneCopy.push_back(&triangleTwo);

// // add lights
// Light one = Light(VEC3(10.0, 10.0, 5.0), VEC3(1.0, 1.0, 1.0));
// Light two = Light(VEC3(-10.0, 10.0, 7.5), VEC3(0.5, 0.25, 0.25));
// vector<Light*> lights;
// lights.push_back(&one);
// lights.push_back(&two);
// Real phongExponent = 10.0;
// bool useLights = true;
// bool useMultipleLights = true;
// bool useSpecular = true;
// bool useShadows = true;
// bool useMirror = true;
// int reflectionRecursionCounter = 0;
// bool useRefraction = true;
// bool useFresnel = true;

// // create a ray map
// float* ppm = allocatePPM(cam.xRes, cam.yRes);
// for (int i = 0; i < cam.xRes; i++) {
//     for (int j = 0; j < cam.yRes; j++) {
//         // generate the ray
//         Ray ray = rayGeneration(i, j, cam);
//         // do a scene intersection
//         VEC3 color =
//             rayColor(sceneCopy, ray, lights, phongExponent, useLights,
//                      useMultipleLights, useSpecular, useShadows,
//                      useMirror, reflectionRecursionCounter,
//                      useRefraction, useFresnel);
//         // color the pixel
//         int index = indexIntoPPM(i, j, cam.xRes, cam.yRes, true);
//         ppm[index] = color[0] * 255.0;
//         ppm[index + 1] = color[1] * 255.0;
//         ppm[index + 2] = color[2] * 255.0;
//     }
// }
// // write out to image
// writePPM("11.ppm", cam.xRes, cam.yRes, ppm);
// delete[] ppm;
// }

int main(int argc, char** argv) {
    // int xRes = 800;
    // int yRes = 600;
    // VEC3 eye = VEC3(0.0, 0.0, 0.0);
    // VEC3 lookAt = VEC3(0.0, 0.0, 1.0);
    // VEC3 up = VEC3(0.0, 1.0, 0.0);
    // Real distanceToNearPlane = 1.0;
    // Real fovy = 65;
    // Camera cam = Camera(eye, lookAt, up, xRes, yRes, distanceToNearPlane,
    // fovy);

    // // scene geometry
    // vector<Shape*> scene;
    // Sphere one =
    //     Sphere(3.0, VEC3(-3.5, 0.0, 10.0), VEC3(1.0, 0.25, 0.25), OPAQUE,
    //     0.0);
    // Sphere two =
    //     Sphere(3.0, VEC3(3.5, 0.0, 10.0), VEC3(0.25, 0.25, 1.0), OPAQUE,
    //     0.0);
    // Sphere three = Sphere(997.0, VEC3(0.0, -1000.0, 10.0), VEC3(0.5, 0.5,
    // 0.5),
    //                       OPAQUE, 0.0);
    // scene.push_back(&one);
    // scene.push_back(&two);
    // scene.push_back(&three);

    // part_11(cam, scene);

    return 0;
}
