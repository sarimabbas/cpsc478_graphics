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
Real degreesToRadians(Real degrees);
Ray rayGeneration(int pixel_i, int pixel_j, Camera cam);
Ray rayGenerationAlt(int pixel_i, int pixel_j, Camera cam);
float* allocatePPM(int xRes, int yRes);
void initPPM(float* values, int xRes, int yRes);
int indexIntoPPM(int x, int y, int xRes, int yRes,
                 bool originBottomLeft = false);
void writeColorToPPM(VEC3 color, float* ppm, int startIndex);
Shape* intersectScene(vector<Shape*> scene, Ray ray);
VEC3 rayColor(vector<Shape*> scene, Ray ray);
void part_1_1(Camera cam, vector<Shape*> scene);
void part_1_2(Camera cam, vector<Shape*> scene);

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

    IntersectResult(Real t, bool doesIntersect)
        : t(t), doesIntersect(doesIntersect) {}
};

ostream& operator<<(ostream& out, const Ray& r) {
    out << "[Origin: " << r.origin << ", Direction: " << r.direction << "]";
}

// shape class
class Shape {
   public:
    virtual IntersectResult intersect(Ray ray) = 0;
    VEC3 color;

    // base constructor
    Shape(VEC3 color) : color(color) {}
};

class Sphere : public Shape {
   public:
    Real radius;
    VEC3 center;

    // call the sphere constructor and the base constructor
    Sphere(Real radius, VEC3 center, VEC3 color)
        : radius(radius), center(center), Shape(color) {}

    IntersectResult intersect(Ray ray) {
        Real A = ray.direction.dot(ray.direction);
        Real B = (2.0 * ray.direction).dot(ray.origin - center);
        Real C =
            (ray.origin - center).dot(ray.origin - center) - (radius * radius);

        Real discriminant = (B * B) - (4.0 * A * C);

        if (discriminant < 0.0) {
            return IntersectResult(0.0, false);
        } else {
            Real t1 = (-B + sqrt(discriminant)) / (2.0 * A);
            Real t2 = (-B - sqrt(discriminant)) / (2.0 * A);
            return IntersectResult(std::min(t1, t2), true);
        }
    }
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

void part_1_1(Camera cam, vector<Shape*> scene) {
    // create a ray map
    float* ppm = allocatePPM(cam.xRes, cam.yRes);
    for (int i = 0; i < cam.xRes; i++) {
        for (int j = 0; j < cam.yRes; j++) {
            // generate the ray
            Ray ray = rayGenerationAlt(i, j, cam);
            int index = indexIntoPPM(i, j, cam.xRes, cam.yRes, false);
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
            Ray ray = rayGenerationAlt(i, j, cam);
            int index = indexIntoPPM(i, j, cam.xRes, cam.yRes, false);
            writeColorToPPM(VEC3(abs(ray.direction[0]), 0.0, 0.0), ppm, index);
        }
    }
    // write out to image
    writePPM("1xabs.ppm", cam.xRes, cam.yRes, ppm);
    delete[] ppm;
}

void part_1_2(Camera cam, vector<Shape*> scene) {
    // create a ray map
    float* ppm = allocatePPM(cam.xRes, cam.yRes);
    for (int i = 0; i < cam.xRes; i++) {
        for (int j = 0; j < cam.yRes; j++) {
            // generate the ray
            Ray ray = rayGeneration(i, j, cam);
            // do a scene intersection
            VEC3 color = rayColor(scene, ray);
            // color the pixel
            int index = indexIntoPPM(i, j, cam.xRes, cam.yRes, false);
            ppm[index] = color[0] * 255.0;
            ppm[index + 1] = color[1] * 255.0;
            ppm[index + 2] = color[2] * 255.0;
        }
    }
    // write out to image
    writePPM("1_2.ppm", cam.xRes, cam.yRes, ppm);
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
    Sphere one = Sphere(3.0, VEC3(-3.5, 0.0, 10.0), VEC3(1.0, 0.25, 0.25));
    Sphere two = Sphere(3.0, VEC3(3.5, 0.0, 10.0), VEC3(0.25, 0.25, 1.0));
    Sphere three = Sphere(997.0, VEC3(0.0, -1000.0, 10.0), VEC3(0.5, 0.5, 0.5));
    scene.push_back(&one);
    scene.push_back(&two);
    scene.push_back(&three);

    // part_1_1(cam, scene);
    part_1_2(cam, scene);

    return 0;
}

VEC3 rayColor(vector<Shape*> scene, Ray ray) {
    Shape* shape = intersectScene(scene, ray);
    if (shape == NULL) {
        return VEC3(0.0, 0.0, 0.0);  // black
    } else {
        return shape->color;  // red
    }
}

Shape* intersectScene(vector<Shape*> scene, Ray ray) {
    // for each primitive in the scene
    // keep track of the closest hit
    Real closestT = INFINITY;
    Shape* closestShape = NULL;
    for (int i = 0; i < scene.size(); i++) {
        // check intersection
        IntersectResult result = scene[i]->intersect(ray);
        if (result.doesIntersect) {
            if (result.t < closestT) {
                closestT = result.t;
                closestShape = scene[i];
            }
        }
    }
    return closestShape;
}

Ray rayGenerationAlt(int pixel_i, int pixel_j, Camera cam) {
    // known:
    // camera: eye, look at, up
    // image plane: distance to plane, fovy,
    // aspect screen: pixel resolution

    // construct coordinate frame
    VEC3 gaze = cam.lookAt - cam.eye;
    VEC3 W = gaze / gaze.norm();  // TODO: -gaze
    VEC3 upCrossW = cam.up.cross(W);
    VEC3 U = upCrossW / upCrossW.norm();
    VEC3 V = W.cross(U);

    // image plane setup
    Real height =
        tan(degreesToRadians(cam.fovy) / 2.0) * 2.0 * cam.distanceToPlane;
    Real width = height * cam.aspect;
    VEC3 C = cam.eye - (W * cam.distanceToPlane);
    VEC3 L = C - (U * width / 2.0) - (V * height / 2.0);
    VEC3 s = L + (U * ((Real)pixel_i) * width / ((Real)cam.xRes)) +
             (V * ((Real)pixel_j) * height / ((Real)cam.yRes));

    // return ray
    Ray ray = Ray(cam.eye, s - cam.eye);
    return ray;
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