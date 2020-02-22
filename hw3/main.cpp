#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "SETTINGS.h"

using namespace std;

// PROTOTYPES

// parts
void partOne();
void partTwo();
void partThree();
void partFour();
void partFive();

// pipeline routines
MATRIX4 viewportMatrix(int xRes, int yRes);
float* triangleRasterization(vector<VEC3> vertices, vector<VEC3I> indices,
                             vector<VEC3> colors, int xRes, int yRes,
                             bool interpolateColors = false);
MATRIX4 orthographicProjectionMatrix(Real left, Real right, Real bottom,
                                     Real top, Real near, Real far);
MATRIX4 perspectiveProjectionMatrix(Real fovy, Real aspect, Real near,
                                    Real far);
MATRIX4 cameraMatrix(VEC3 eye, VEC3 lookAt, VEC3 up);
vector<VEC3> transformVertices(vector<VEC3> vertices, MATRIX4 matrix);

// debugging routines
template <typename T, typename A>
void printVector(vector<T, A> const& vec);

// other routines
bool betweenZeroAndOneInclusive(Real num);
Real degreesToRadians(Real degrees);

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
    // partOne();
    // partTwo();
    // partThree();
    // partFour();
    partFive();

    return 0;
    vector<VEC3> vertices;
    vector<VEC3I> indices;
    vector<VEC3> colors;
    // the pre-provided geometries are in the canonical view volume
    // the canonical view volumne is a cube centered at origin, with edges
    // that go from -1 to +1.
    // buildBigSquare(vertices, indices, colors);
    buildCube(vertices, indices, colors);
    // printVector(vertices);

    int xRes = 800;
    int yRes = 600;

    // scale vertices by 0.5
    vector<VEC3> scaledVertices;
    for (int i = 0; i < vertices.size(); i++) {
        scaledVertices.push_back(0.5 * vertices[i]);
    }

    // do orthographic projection matrix transform, modifying each coordinate
    cout << "Starting camera transform" << endl;
    MATRIX4 mcam = cameraMatrix(VEC3(1.0, 1.0, 1.0), VEC3(0.0, 0.0, 0.0),
                                VEC3(0.0, 1.0, 0.0));

    // do orthographic projection matrix transform, modifying each coordinate
    cout << "Starting projection transform" << endl;
    MATRIX4 mortho =
        orthographicProjectionMatrix(0.0, 12.0, 0.0, 12.0, 12.0, 0.0);

    // get the perspective matrix
    cout << "Starting perspective projection transform" << endl;
    MATRIX mp = perspectiveProjectionMatrix(65.0, 4.0 / 3.0, 1.0, 100.0);

    // we do a viewport matrix transform, modifying each coordinate
    cout << "Starting viewport transform" << endl;
    MATRIX4 mvp = viewportMatrix(xRes, yRes);

    // printVector(viewportVertices);
    // testViewportMatrix(vertices, xRes, yRes);

    vector<VEC3> transformedVertices;
    MATRIX4 compose = mvp * mp * mcam;

    for (int i = 0; i < vertices.size(); i++) {
        // first, convert the (3,1) vector to a (4,1)
        VEC4 extended = extend(vertices[i]);
        // multiply with the mvp
        VEC4 multiplied = compose * extended;
        // truncate back to (3,1) vector
        VEC3 truncated = truncate(multiplied);
        // set that as the new vector
        transformedVertices.push_back(truncated);
    }

    // do the triangleRasterization
    cout << "Starting triangle rasterization" << endl;
    float* ppm =
        triangleRasterization(transformedVertices, indices, colors, xRes, yRes);
    if (!ppm) {
        cout << "Invalid ppm" << endl;
        return -1;
    }
    // write the PPM file
    cout << "Writing ppm file" << endl;
    writePPM("rasterization.ppm", xRes, yRes, ppm);

    delete[] ppm;

    return 0;
}

vector<VEC3> transformVertices(vector<VEC3> vertices, MATRIX4 matrix) {
    vector<VEC3> transformedVertices;
    for (int i = 0; i < vertices.size(); i++) {
        // first, convert the (3,1) vector to a (4,1)
        VEC4 extended = extend(vertices[i]);
        // multiply with the mvp
        VEC4 multiplied = matrix * extended;
        // truncate back to (3,1) vector
        VEC3 truncated = truncate(multiplied);
        // set that as the new vector
        transformedVertices.push_back(truncated);
    }
    return transformedVertices;
}

void partOne() {
    vector<VEC3> vertices;
    vector<VEC3I> indices;
    vector<VEC3> colors;
    buildSquare(vertices, indices, colors);

    int xRes = 800;
    int yRes = 600;

    MATRIX4 mvp = viewportMatrix(xRes, yRes);
    MATRIX4 compose = mvp;
    vector<VEC3> transformedVertices = transformVertices(vertices, compose);

    float* ppm = allocatePPM(xRes, yRes);
    for (int i = 0; i < transformedVertices.size(); i++) {
        int index = indexIntoPPM(transformedVertices[i][0],
                                 transformedVertices[i][1], xRes, yRes);
        ppm[index] = 255;
        ppm[index + 1] = 255;
        ppm[index + 2] = 255;
    }
    writePPM("1.ppm", xRes, yRes, ppm);
    delete[] ppm;
}

void partTwo() {
    vector<VEC3> vertices;
    vector<VEC3I> indices;
    vector<VEC3> colors;
    buildSquare(vertices, indices, colors);

    int xRes = 800;
    int yRes = 600;

    MATRIX4 mvp = viewportMatrix(xRes, yRes);
    MATRIX4 compose = mvp;
    vector<VEC3> transformedVertices = transformVertices(vertices, compose);

    float* ppm =
        triangleRasterization(transformedVertices, indices, colors, xRes, yRes);

    writePPM("2.ppm", xRes, yRes, ppm);
    delete[] ppm;
}

void partThree() {
    vector<VEC3> vertices;
    vector<VEC3I> indices;
    vector<VEC3> colors;
    buildBigSquare(vertices, indices, colors);

    int xRes = 800;
    int yRes = 600;

    Real left = 0.0;
    Real right = 12.0;
    Real bottom = 0.0;
    Real top = 12.0;
    Real near = 12.0;
    Real far = 0.0;

    MATRIX4 mvp = viewportMatrix(xRes, yRes);
    MATRIX4 mortho =
        orthographicProjectionMatrix(left, right, bottom, top, near, far);
    MATRIX4 compose = mvp * mortho;
    vector<VEC3> transformedVertices = transformVertices(vertices, compose);

    float* ppm =
        triangleRasterization(transformedVertices, indices, colors, xRes, yRes);

    writePPM("3.ppm", xRes, yRes, ppm);
    delete[] ppm;
}

void partFour() {
    vector<VEC3> vertices;
    vector<VEC3I> indices;
    vector<VEC3> colors;
    buildBigSquare(vertices, indices, colors);

    int xRes = 800;
    int yRes = 600;

    Real left = 0.0;
    Real right = 12.0;
    Real bottom = 0.0;
    Real top = 12.0;
    Real near = 12.0;
    Real far = 0.0;

    VEC3 eye = VEC3(0.2, 0.2, 1);
    VEC3 lookAt = VEC3(0.0, 0.0, 0.0);
    VEC3 up = VEC3(0.0, 1.0, 0.0);

    MATRIX4 mvp = viewportMatrix(xRes, yRes);
    MATRIX4 mortho =
        orthographicProjectionMatrix(left, right, bottom, top, near, far);
    MATRIX4 mcam = cameraMatrix(eye, lookAt, up);
    MATRIX4 compose = mvp * mortho * mcam;
    vector<VEC3> transformedVertices = transformVertices(vertices, compose);

    float* ppm =
        triangleRasterization(transformedVertices, indices, colors, xRes, yRes);

    writePPM("4.ppm", xRes, yRes, ppm);
    delete[] ppm;
}

void partFive() {
    vector<VEC3> vertices;
    vector<VEC3I> indices;
    vector<VEC3> colors;
    buildCube(vertices, indices, colors);

    int xRes = 800;
    int yRes = 600;

    Real left = 0.0;
    Real right = 12.0;
    Real bottom = 0.0;
    Real top = 12.0;
    Real near = 1.0;
    Real far = 100.0;

    VEC3 eye = VEC3(1.0, 1.0, 1.0);
    VEC3 lookAt = VEC3(0.0, 0.0, 0.0);
    VEC3 up = VEC3(0.0, 1.0, 0.0);

    Real fovy = 65.0;
    Real aspect = 4.0 / 3.0;

    MATRIX4 mvp = viewportMatrix(xRes, yRes);
    MATRIX4 mortho =
        orthographicProjectionMatrix(left, right, bottom, top, near, far);
    MATRIX4 mcam = cameraMatrix(eye, lookAt, up);

    // scale vertices
    vector<VEC3> scaledVertices;
    for (int i = 0; i < vertices.size(); i++) {
        scaledVertices.push_back(0.5 * vertices[i]);
    }
    MATRIX4 mp = perspectiveProjectionMatrix(fovy, aspect, near, far);

    MATRIX4 compose = mp * mvp * mcam;
    vector<VEC3> transformedVertices =
        transformVertices(scaledVertices, compose);

    float* ppm =
        triangleRasterization(transformedVertices, indices, colors, xRes, yRes);

    writePPM("5.ppm", xRes, yRes, ppm);
    delete[] ppm;
}

Real degreesToRadians(Real degrees) { return (degrees)*M_PI / 180.0; }

// pipeline routines

MATRIX4 viewportMatrix(int xRes, int yRes) {
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

    return mvp;
}

MATRIX4 orthographicProjectionMatrix(Real left, Real right, Real bottom,
                                     Real top, Real near, Real far) {
    MATRIX4 mortho;
    mortho.setZero();

    mortho(0, 0) = 2.0 / (right - left);
    mortho(0, 3) = -(((2.0 * left) / (right - left)) + 1);
    mortho(1, 1) = 2.0 / (top - bottom);
    mortho(1, 3) = -(((2.0 * bottom) / (top - bottom)) + 1);
    mortho(2, 2) = 2.0 / (near - far);
    mortho(2, 3) = -(((2.0 * far) / (near - far)) + 1);
    mortho(3, 3) = 1.0;

    return mortho;
}

MATRIX4 cameraMatrix(VEC3 eye, VEC3 lookAt, VEC3 up) {
    // first get the gaze direction
    VEC3 gaze = lookAt - eye;
    VEC3 w = -gaze / gaze.norm();
    VEC3 upCrossW = up.cross(w);
    VEC3 u = upCrossW / upCrossW.norm();
    VEC3 v = w.cross(u);

    MATRIX4 left;
    left.setZero();
    left(0, 0) = u[0];
    left(0, 1) = u[1];
    left(0, 2) = u[2];
    left(1, 0) = v[0];
    left(1, 1) = v[1];
    left(1, 2) = v[2];
    left(2, 0) = w[0];
    left(2, 1) = w[1];
    left(2, 2) = w[2];
    left(3, 3) = 1.0;

    MATRIX4 right;
    right.setZero();
    right(0, 0) = 1.0;
    right(0, 3) = -eye[0];
    right(1, 1) = 1.0;
    right(1, 3) = -eye[1];
    right(2, 2) = 1.0;
    right(2, 3) = -eye[2];
    right(3, 3) = 1.0;

    MATRIX4 mcam;
    mcam.setZero();
    mcam = left * right;

    return mcam;
}

MATRIX4 perspectiveProjectionMatrix(Real fovy, Real aspect, Real near,
                                    Real far) {
    MATRIX4 mp;

    mp.setZero();

    Real f = 1.0 / tan(degreesToRadians(fovy) / 2.0);

    mp(0, 0) = f / aspect;
    mp(1, 1) = f;
    mp(2, 2) = (near + far) / (near - far);
    mp(2, 3) = (2.0 * near * far) / (near - far);
    mp(3, 2) = -1.0;

    return mp;
}

float* triangleRasterization(vector<VEC3> vertices, vector<VEC3I> indices,
                             vector<VEC3> colors, int xRes, int yRes,
                             bool interpolateColors) {
    // first, init an image
    float* ppm = allocatePPM(xRes, yRes);

    // calculate areas for each triangle in advance to avoid repeated
    // computation
    vector<Real> triangleAreas;
    vector<VEC3> triangleCrosses;
    for (int k = 0; k < indices.size(); k++) {
        VEC3I triangle = indices[k];
        VEC3 pointA = vertices[triangle[0]];
        VEC3 pointB = vertices[triangle[1]];
        VEC3 pointC = vertices[triangle[2]];

        // find the full area of the triangle
        VEC3 ab = pointB - pointA;
        VEC3 ac = pointC - pointA;
        VEC3 cross = ab.cross(ac);
        Real fullArea = 0.5 * (cross.norm());
        triangleCrosses.push_back(cross);
        triangleAreas.push_back(fullArea);
    }

    // for each pixel
    for (int i = 0; i < xRes; i++) {
        for (int j = 0; j < yRes; j++) {
            // for each primitive (i.e. triangle)
            for (int k = 0; k < indices.size(); k++) {
                VEC3I triangle = indices[k];
                VEC3 pointA = vertices[triangle[0]];
                VEC3 pointB = vertices[triangle[1]];
                VEC3 pointC = vertices[triangle[2]];

                // split the full triangle via the pixel coordinate
                // it does not appear that the z coordinate is important
                VEC3 pointP = VEC3(i, j, 0.5);
                VEC3 crossA = (pointC - pointB).cross(pointP - pointB);
                VEC3 crossB = (pointA - pointC).cross(pointP - pointC);
                VEC3 crossC = (pointB - pointA).cross(pointP - pointA);

                // Real areaA = 0.5 * (crossA.norm());
                // Real areaB = 0.5 * (crossB.norm());
                // Real areaC = 0.5 * (crossC.norm());

                Real alpha = triangleCrosses[k].dot(crossA) /
                             triangleCrosses[k].squaredNorm();
                Real beta = triangleCrosses[k].dot(crossB) /
                            triangleCrosses[k].squaredNorm();
                Real gamma = triangleCrosses[k].dot(crossC) /
                             triangleCrosses[k].squaredNorm();

                // color based on the decision
                if (betweenZeroAndOneInclusive(alpha) &&
                    betweenZeroAndOneInclusive(beta) &&
                    betweenZeroAndOneInclusive(gamma)) {
                    Real color = (alpha * colors[k][0]) +
                                 (beta * colors[k][1]) + (gamma * colors[k][2]);
                    int index = indexIntoPPM(i, j, xRes, yRes, true);
                    if (interpolateColors) {
                        ppm[index] = colors[k][0] * alpha * 255;
                        ppm[index + 1] = colors[k][1] * beta * 255;
                        ppm[index + 2] = colors[k][2] * gamma * 255;
                    } else {
                        ppm[index] = colors[k][0] * 255;
                        ppm[index + 1] = colors[k][1] * 255;
                        ppm[index + 2] = colors[k][2] * 255;
                    }
                }

                // writePPM("rasterization.ppm", xRes, yRes, ppm);
            }
        }
    }

    // TODO: remove
    // return NULL;
    return ppm;
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

// other routines

bool betweenZeroAndOneInclusive(Real num) {
    if (num >= 0.0 && num <= 1.0) {
        return true;
    }
    return false;
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