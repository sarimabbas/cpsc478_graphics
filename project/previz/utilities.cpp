#include "utilities.hpp"

#include <iostream>

using namespace std;

Real CUSTOM_EPSILON = generateEpsilon();

Real generateEpsilon(Real scale) {
    return scale * std::numeric_limits<Real>::epsilon();
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
        values[i] = 1;
        values[i + 1] = 1;
        values[i + 2] = 1;
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

VEC3 truncate(const VEC4& v) { return VEC3(v[0], v[1], v[2]); }

VEC4 extend(const VEC3& v) { return VEC4(v[0], v[1], v[2], 1.0); }