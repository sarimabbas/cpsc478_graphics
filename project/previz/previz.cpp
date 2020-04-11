#include <float.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "SETTINGS.h"
#include "displaySkeleton.h"
#include "motion.h"
#include "shapes.hpp"
#include "skeleton.h"
#include "tracer.hpp"
#include "utilities.hpp"

using namespace std;

// Stick-man classes
DisplaySkeleton displayer;
Skeleton* skeleton;
Motion* motion;

int windowWidth = 640;
int windowHeight = 480;

VEC3 eye(-6, 0.5, 1);
VEC3 lookingAt(5, 0.5, 1);
VEC3 up(0, 1, 0);
Real distanceToNearPlane = 1.0;
Real fovy = 65;

// scene geometry
vector<Shape*> scene;
// vector<VEC3> sphereCenters;=
// vector<float> sphereRadii;
// vector<VEC3> sphereColors;

void destroyScene();
void buildFloor();
void buildPlatform();

VEC3 RED = VEC3(1, 0, 0);
VEC3 GREEN = VEC3(0, 1, 0);
VEC3 BLUE = VEC3(0, 0, 1);
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
// bool raySphereIntersect(const VEC3& center, const float radius,
//                         const VEC3& rayPos, const VEC3& rayDir, float& t)
//                         {
//     const VEC3 op = center - rayPos;
//     const float eps = 1e-8;
//     const float b = op.dot(rayDir);
//     float det = b * b - op.dot(op) + radius * radius;

//     // determinant check
//     if (det < 0) return false;

//     det = sqrt(det);
//     t = b - det;
//     if (t <= eps) {
//         t = b + det;
//         if (t <= eps) t = -1;
//     }

//     if (t < 0) return false;
//     return true;
// }

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
// void rayColor(const VEC3& rayPos, const VEC3& rayDir, VEC3& pixelColor) {
//     pixelColor = VEC3(1, 1, 1);

//     // look for intersections
//     int hitID = -1;
//     float tMinFound = FLT_MAX;
//     for (int y = 0; y < sphereCenters.size(); y++) {
//         float tMin = FLT_MAX;
//         if (raySphereIntersect(sphereCenters[y], sphereRadii[y], rayPos,
//         rayDir,
//                                tMin)) {
//             // is the closest so far?
//             if (tMin < tMinFound) {
//                 tMinFound = tMin;
//                 hitID = y;
//             }
//         }
//     }

//     // No intersection, return white
//     if (hitID == -1) return;

//     // set to the sphere color
//     pixelColor = sphereColors[hitID];
// }

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void renderImage(int& xRes, int& yRes, const string& filename, Camera cam,
                 vector<Light*> lights) {
    // TODO: you can keep this as is
    // allocate the final image
    // const int totalCells = xRes * yRes;
    // float* ppmOut = new float[3 * totalCells];

    float* ppmOut = allocatePPM(cam.xRes, cam.yRes);

    // TODO: this seems to be ray generation stuff using camera
    // compute image plane
    // const float halfY = (lookingAt - eye).norm() * tan(45.0f / 360.0f *
    // M_PI); const float halfX = halfY * 4.0f / 3.0f; const VEC3 cameraZ =
    // (lookingAt - eye).normalized(); const VEC3 cameraX =
    // up.cross(cameraZ).normalized(); const VEC3 cameraY =
    // cameraZ.cross(cameraX).normalized();

    for (int x = 0; x < xRes; x++)
        for (int y = 0; y < yRes; y++) {
            // generate the ray, making x-axis go left to right
            // const float ratioX = 1.0f - ((xRes - 1) - x) / float(xRes)
            // * 2.0f; const float ratioY = 1.0f - y / float(yRes) * 2.0f; const
            // VEC3 rayHitImage =
            //     lookingAt + ratioX * halfX * cameraX + ratioY * halfY *
            //     cameraY;
            // const VEC3 rayDir = (rayHitImage - eye).normalized();

            // generate ray
            Ray ray = rayGenerationAlt(x, y, cam);
            // get the ray color
            // VEC3 color = rayColor(scene, )

            // get the color
            VEC3 color = rayColor(scene, ray, lights, 10.0, true, true, false,
                                  true, true, 0, true, true);
            // rayColor(eye, rayDir, color);

            // set, in final image
            int index = indexIntoPPM(x, y, cam.xRes, cam.yRes, false);
            ppmOut[index] = color[0] * 255.0;
            ppmOut[index + 1] = color[1] * 255.0;
            ppmOut[index + 2] = color[2] * 255.0;

            // ppmOut[3 * (y * xRes + x)] = clamp(color[0]) * 255.0f;
            // ppmOut[3 * (y * xRes + x) + 1] = clamp(color[1]) * 255.0f;
            // ppmOut[3 * (y * xRes + x) + 2] = clamp(color[2]) * 255.0f;
        }
    writePPM(filename, xRes, yRes, ppmOut);

    delete[] ppmOut;
}

//////////////////////////////////////////////////////////////////////////////////
// Load up a new motion captured frame
//////////////////////////////////////////////////////////////////////////////////
void setSkeletonsToSpecifiedFrame(int frameIndex) {
    if (frameIndex < 0) {
        printf(
            "Error in SetSkeletonsToSpecifiedFrame: frameIndex %d is "
            "illegal.\n",
            frameIndex);
        exit(0);
    }
    if (displayer.GetSkeletonMotion(0) != NULL) {
        int postureID;
        if (frameIndex >= displayer.GetSkeletonMotion(0)->GetNumFrames()) {
            cout << " We hit the last frame! You might want to pick a "
                    "different sequence. "
                 << endl;
            postureID = displayer.GetSkeletonMotion(0)->GetNumFrames() - 1;
        } else
            postureID = frameIndex;
        displayer.GetSkeleton(0)->setPosture(
            *(displayer.GetSkeletonMotion(0)->GetPosture(postureID)));
    }
}

//////////////////////////////////////////////////////////////////////////////////
// Build a list of spheres in the scene
//////////////////////////////////////////////////////////////////////////////////
void buildScene() {
    // sphereCenters.clear();
    // sphereRadii.clear();
    // sphereColors.clear();

    buildFloor();
    buildPlatform();

    displayer.ComputeBonePositions(DisplaySkeleton::BONES_AND_LOCAL_FRAMES);

    // retrieve all the bones of the skeleton
    vector<MATRIX4>& rotations = displayer.rotations();
    vector<MATRIX4>& scalings = displayer.scalings();
    vector<VEC4>& translations = displayer.translations();
    vector<float>& lengths = displayer.lengths();

    // build a sphere list, but skip the first bone,
    // it's just the origin
    int totalBones = rotations.size();
    for (int x = 1; x < totalBones; x++) {
        MATRIX4& rotation = rotations[x];
        MATRIX4& scaling = scalings[x];
        VEC4& translation = translations[x];

        // get the endpoints of the cylinder
        VEC4 leftVertex(0, 0, 0, 1);
        VEC4 rightVertex(0, 0, lengths[x], 1);

        leftVertex = rotation * scaling * leftVertex + translation;
        rightVertex = rotation * scaling * rightVertex + translation;

        // get the direction vector
        VEC3 direction = (rightVertex - leftVertex).head<3>();
        const float magnitude = direction.norm();
        direction *= 1.0 / magnitude;

        // how many spheres?
        const float sphereRadius = 0.05;
        const int totalSpheres = magnitude / (2.0 * sphereRadius);
        const float rayIncrement = magnitude / (float)totalSpheres;

        // store the spheres
        Sphere* sphereLeft =
            new Sphere(0.05, leftVertex.head<3>(), VEC3(1, 0, 0), OPAQUE, 0.0);
        scene.push_back(sphereLeft);
        // sphereCenters.push_back(leftVertex.head<3>());
        // sphereRadii.push_back(0.05);
        // sphereColors.push_back(VEC3(1, 0, 0));

        Sphere* sphereRight =
            new Sphere(0.05, rightVertex.head<3>(), VEC3(1, 0, 0), OPAQUE, 0.0);
        scene.push_back(sphereRight);

        // sphereCenters.push_back(rightVertex.head<3>());
        // sphereRadii.push_back(0.05);
        // sphereColors.push_back(VEC3(1, 0, 0));

        for (int y = 0; y < totalSpheres; y++) {
            VEC3 center = ((float)y + 0.5) * rayIncrement * direction +
                          leftVertex.head<3>();
            Sphere* sphereCenter =
                new Sphere(0.05, center, VEC3(1, 0, 0), OPAQUE, 0.0);
            scene.push_back(sphereCenter);

            // sphereCenters.push_back(center);
            // sphereRadii.push_back(0.05);
            // sphereColors.push_back(VEC3(1, 0, 0));
        }
    }
}

void destroyScene() {
    for (int i = 0; i < scene.size(); i++) {
        delete scene[i];
    }
    scene.clear();
}

void buildFloor() {
    // 0th index is the z-axis
    // 2nd index is the x-axis
    VEC3 points[4] = {// left, closer
                      VEC3(-2, 0, 2),
                      // right, further
                      VEC3(5, 0, -2),
                      // left, further
                      VEC3(5, 0, 2),
                      // right, closer
                      VEC3(-2, 0, -1)};
    Triangle* one =
        new Triangle(points[0], points[1], points[2], GREEN, OPAQUE, 0.0);
    Triangle* two =
        new Triangle(points[0], points[3], points[1], GREEN, OPAQUE, 0.0);
    scene.push_back(one);
    scene.push_back(two);

    Sphere* a = new Sphere(0.05, points[0], RED, OPAQUE, 1.0);
    scene.push_back(a);
    Sphere* b = new Sphere(0.05, points[1], BLUE, OPAQUE, 1.0);
    scene.push_back(b);
    Sphere* c = new Sphere(0.05, points[2], VEC3(1, 0.4, 0.78), OPAQUE, 1.0);
    scene.push_back(c);
}

void buildPlatform() {
    VEC3 points[4] = {// left, closer
                      VEC3(-2, 0.6, 2.5),
                      // right, further
                      VEC3(5, 0.6, 0.5),
                      // left, further
                      VEC3(5, 0.6, 2.5),
                      // right, closer
                      VEC3(-2, 0.6, 0.5)};
    Triangle* one =
        new Triangle(points[0], points[1], points[2], GREEN, OPAQUE, 0.0);
    Triangle* two =
        new Triangle(points[0], points[3], points[1], GREEN, OPAQUE, 0.0);
    scene.push_back(one);
    scene.push_back(two);

    Sphere* a = new Sphere(0.05, points[0], RED, OPAQUE, 1.0);
    scene.push_back(a);
    Sphere* b = new Sphere(0.05, points[1], BLUE, OPAQUE, 1.0);
    scene.push_back(b);
    Sphere* c = new Sphere(0.05, points[2], VEC3(1, 0.4, 0.78), OPAQUE, 1.0);
    scene.push_back(c);
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
    string skeletonFilename("88.asf");
    string motionFilename("88_02.amc");

    // load up skeleton stuff
    skeleton = new Skeleton(skeletonFilename.c_str(), MOCAP_SCALE);
    skeleton->setBasePosture();
    displayer.LoadSkeleton(skeleton);

    // load up the motion
    motion = new Motion(motionFilename.c_str(), MOCAP_SCALE, skeleton);
    displayer.LoadMotion(motion);
    skeleton->setPosture(*(displayer.GetSkeletonMotion(0)->GetPosture(0)));

    // create lights
    Light one = Light(VEC3(10.0, 10.0, 5.0), VEC3(1.0, 1.0, 1.0));
    Light two = Light(VEC3(-10.0, 3.0, 7.5), VEC3(0.5, 0.0, 0.0));
    Light three = Light(VEC3(0.0, 5.0, 5.0), VEC3(1.0, 1.0, 1.0));
    vector<Light*> lights;
    lights.push_back(&one);
    lights.push_back(&two);
    lights.push_back(&three);

    // Note we're going 8 frames at a time, otherwise the animation
    // is really slow.
    for (int x = 0; x < 2400; x += 8) {
        setSkeletonsToSpecifiedFrame(x);
        destroyScene();
        buildScene();

        vector<VEC4>& translations = displayer.translations();
        VEC4 pelvisTranslation = translations[1];
        VEC3 cameraPos = VEC3(pelvisTranslation[0], pelvisTranslation[1],
                              pelvisTranslation[2]);

        // create a fixed camera for now
        Camera cam = Camera(eye, cameraPos, up, windowWidth, windowHeight,
                            distanceToNearPlane, fovy);

        char buffer[256];
        sprintf(buffer, "./frames/frame.%04i.ppm", x / 8);
        renderImage(windowWidth, windowHeight, buffer, cam, lights);
        cout << "Rendered " + to_string(x / 8) + " frames" << endl;
    }

    return 0;
}
