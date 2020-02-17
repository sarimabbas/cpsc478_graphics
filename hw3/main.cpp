#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "SETTINGS.h"

using namespace std;

// PROTOTYPES

// pipeline routines
vector<VEC3> viewportMatrix(vector<VEC3> vertices, int xRes, int yRes);
// debugging routines
template <typename T, typename A>
void printVector(vector<T, A> const& vec);
void testViewportMatrix(vector<VEC3> vertices);
// ppm routines
int indexIntoPPM(int x, int y, int xRes, int yRes,
                 bool originBottomLeft = false);
float* allocatePPM(int xRes, int yRes);
void initPPM(float* values, int xRes, int yRes);

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
VEC3 truncate(const VEC4& v) { return VEC3(v[0], v[1], v[2]); }
VEC4 extend(const VEC3& v) { return VEC4(v[0], v[1], v[2], 1.0); }

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
void readPPM(const string& filename, int& xRes, int& yRes, float*& values) {
    // try to open the file
    FILE* fp;
    fp = fopen(filename.c_str(), "rb");
    if (fp == NULL) {
        cout << " Could not open file \"" << filename.c_str()
             << "\" for reading." << endl;
        cout << " Make sure you're not trying to read from a weird location or "
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
        cout << " Make sure you're not trying to write from a weird location "
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

//////////////////////////////////////////////////////////////////////////////////
// build out a single square
//////////////////////////////////////////////////////////////////////////////////
void buildBigSquare(vector<VEC3>& vertices, vector<VEC3I>& indices,
                    vector<VEC3>& colors) {
    vertices.push_back(VEC3(1.0, 1.0, 1.5));
    vertices.push_back(VEC3(11.0, 1.0, 1.5));
    vertices.push_back(VEC3(1.0, 11.0, 1.5));
    vertices.push_back(VEC3(11.0, 11.0, 1.5));

    // front face
    indices.push_back(VEC3I(0, 1, 2));
    indices.push_back(VEC3I(2, 1, 3));
    colors.push_back(VEC3(1.0, 0.0, 0.0));
    colors.push_back(VEC3(0.0, 0.0, 1.0));
}

//////////////////////////////////////////////////////////////////////////////////
// build out a single square
//////////////////////////////////////////////////////////////////////////////////
void buildSquare(vector<VEC3>& vertices, vector<VEC3I>& indices,
                 vector<VEC3>& colors) {
    //  the four points of the square
    vertices.push_back(VEC3(-0.5, -0.5, 0.5));
    vertices.push_back(VEC3(0.5, -0.5, 0.5));
    vertices.push_back(VEC3(-0.5, 0.5, 0.5));
    vertices.push_back(VEC3(0.5, 0.5, 0.5));

    // front face
    indices.push_back(VEC3I(0, 1, 2));      // the first triangle
    indices.push_back(VEC3I(2, 1, 3));      // the second triangle
    colors.push_back(VEC3(1.0, 0.0, 0.0));  // the first triangle color
    colors.push_back(VEC3(0.0, 1.0, 0.0));  // the second triangle color
}

//////////////////////////////////////////////////////////////////////////////////
// build out a cube
//////////////////////////////////////////////////////////////////////////////////
void buildCube(vector<VEC3>& vertices, vector<VEC3I>& indices,
               vector<VEC3>& colors) {
    vertices.push_back(VEC3(-0.5, -0.5, 0.5));
    vertices.push_back(VEC3(0.5, -0.5, 0.5));
    vertices.push_back(VEC3(-0.5, 0.5, 0.5));
    vertices.push_back(VEC3(0.5, 0.5, 0.5));
    vertices.push_back(VEC3(-0.5, -0.5, -0.5));
    vertices.push_back(VEC3(0.5, -0.5, -0.5));
    vertices.push_back(VEC3(-0.5, 0.5, -0.5));
    vertices.push_back(VEC3(0.5, 0.5, -0.5));

    // front face
    indices.push_back(VEC3I(0, 1, 2));
    indices.push_back(VEC3I(2, 1, 3));
    colors.push_back(VEC3(1.0, 0.0, 0.0));
    colors.push_back(VEC3(1.0, 0.0, 0.0));

    // back face
    indices.push_back(VEC3I(5, 4, 7));
    indices.push_back(VEC3I(7, 4, 6));
    colors.push_back(VEC3(0.0, 1.0, 0.0));
    colors.push_back(VEC3(0.0, 1.0, 0.0));

    // left face
    indices.push_back(VEC3I(4, 0, 6));
    indices.push_back(VEC3I(6, 0, 2));
    colors.push_back(VEC3(0.0, 0.0, 1.0));
    colors.push_back(VEC3(0.0, 0.0, 1.0));

    // right face
    indices.push_back(VEC3I(1, 5, 3));
    indices.push_back(VEC3I(3, 5, 7));
    colors.push_back(VEC3(0.0, 1.0, 1.0));
    colors.push_back(VEC3(0.0, 1.0, 1.0));

    // top face
    indices.push_back(VEC3I(2, 3, 6));
    indices.push_back(VEC3I(6, 3, 7));
    colors.push_back(VEC3(1.0, 1.0, 0.0));
    colors.push_back(VEC3(1.0, 1.0, 0.0));

    // bottom face
    indices.push_back(VEC3I(4, 5, 0));
    indices.push_back(VEC3I(0, 5, 1));
    colors.push_back(VEC3(1.0, 0.0, 1.0));
    colors.push_back(VEC3(1.0, 0.0, 1.0));
}

//////////////////////////////////////////////////////////////////////////////////
// build out a cube
//////////////////////////////////////////////////////////////////////////////////
void buildCubePerVertexColors(vector<VEC3>& vertices, vector<VEC3I>& indices,
                              vector<VEC3>& colors) {
    vertices.push_back(VEC3(-0.5, -0.5, 0.5));
    vertices.push_back(VEC3(0.5, -0.5, 0.5));
    vertices.push_back(VEC3(-0.5, 0.5, 0.5));
    vertices.push_back(VEC3(0.5, 0.5, 0.5));
    vertices.push_back(VEC3(-0.5, -0.5, -0.5));
    vertices.push_back(VEC3(0.5, -0.5, -0.5));
    vertices.push_back(VEC3(-0.5, 0.5, -0.5));
    vertices.push_back(VEC3(0.5, 0.5, -0.5));
    colors.push_back(VEC3(1.0, 0.0, 0.0));
    colors.push_back(VEC3(1.0, 0.0, 0.0));
    colors.push_back(VEC3(0.0, 1.0, 0.0));
    colors.push_back(VEC3(0.0, 1.0, 0.0));
    colors.push_back(VEC3(0.0, 0.0, 1.0));
    colors.push_back(VEC3(0.0, 0.0, 1.0));
    colors.push_back(VEC3(1.0, 1.0, 0.0));
    colors.push_back(VEC3(1.0, 1.0, 0.0));

    // front face
    indices.push_back(VEC3I(0, 1, 2));
    indices.push_back(VEC3I(2, 1, 3));

    // back face
    indices.push_back(VEC3I(5, 4, 7));
    indices.push_back(VEC3I(7, 4, 6));

    // left face
    indices.push_back(VEC3I(4, 0, 6));
    indices.push_back(VEC3I(6, 0, 2));

    // right face
    indices.push_back(VEC3I(1, 5, 3));
    indices.push_back(VEC3I(3, 5, 7));

    // top face
    indices.push_back(VEC3I(2, 3, 6));
    indices.push_back(VEC3I(6, 3, 7));

    // bottom face
    indices.push_back(VEC3I(4, 5, 0));
    indices.push_back(VEC3I(0, 5, 1));
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
    // void buildSquare(vector<VEC3> & vertices, vector<VEC3I> & indices,
    //                  vector<VEC3> & colors)

    cout << "Starting build square" << endl;
    vector<VEC3> vertices;
    vector<VEC3I> indices;
    vector<VEC3> colors;
    // the pre-provided geometries are in the canonical view volume
    // the canonical view volumne is a cube centered at origin, with edges
    // that go from -1 to +1.
    buildSquare(vertices, indices, colors);
    // printVector(vertices);

    // we do a viewport matrix transform, modifying each coordinate
    vector<VEC3> transformedVertices = viewportMatrix(vertices, 800, 600);
    testViewportMatrix(transformedVertices);

    return 0;
}

// pipeline routines

vector<VEC3> viewportMatrix(vector<VEC3> vertices, int xRes, int yRes) {
    // the viewport transform is basically a scaling operation
    // it takes the canonical volume and stretches it to xRes * yRes * 2 box
    // (the z-axis is unchanged)

    MATRIX4 mvp;
    mvp.setZero();

    mvp(0, 0) = xRes / 2;
    mvp(0, 3) = xRes / 2;
    mvp(1, 1) = yRes / 2;
    mvp(1, 3) = yRes / 2;
    mvp(2, 2) = 1;
    mvp(3, 3) = 1;

    vector<VEC3> transformedVertices;
    for (int i = 0; i < vertices.size(); i++) {
        // first, convert the (3,1) vector to a (4,1)
        VEC4 extended = extend(vertices[i]);
        // multiply with the mvp
        VEC4 multiplied = mvp * extended;
        // truncate back to (3,1) vector
        VEC3 truncated = truncate(multiplied);
        // set that as the new vector
        transformedVertices.push_back(truncated);
    }

    return transformedVertices;
}

// debugging routines

template <typename T, typename A>
void printVector(vector<T, A> const& vec) {
    cout << "[";
    for (auto i = vec.cbegin(); i != vec.cend(); ++i) {
        cout << "{" << *i << "}" << endl;
    };
    cout << "]" << endl;
}

void testViewportMatrix(vector<VEC3> vertices) {
    // write out the vertices to a file
    int xRes = 800;
    int yRes = 600;
    float* ppm = allocatePPM(xRes, yRes);
    for (int i = 0; i < vertices.size(); i++) {
        int index = indexIntoPPM(vertices[i][0], vertices[i][1], xRes, yRes);
        ppm[index] = 255;
        ppm[index + 1] = 255;
        ppm[index + 2] = 255;
    }
    writePPM("testViewportMatrix.ppm", xRes, yRes, ppm);
    delete[] ppm;
}

// PPM routines

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