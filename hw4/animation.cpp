#include "animation.hpp"
#include "utilities.hpp"

using namespace std;

vector<VEC3> rotateVertices(vector<VEC3> vertices, VEC3 anchor, Real xDeg,
                            Real yDeg, Real zDeg) {
    vector<VEC3> verticesCopy = vertices;
    MATRIX3 xMat;
    MATRIX3 yMat;
    MATRIX3 zMat;
    xMat.setIdentity();
    yMat.setIdentity();
    zMat.setIdentity();

    if (yDeg > 0.0 || yDeg < 0.0) {
        MATRIX3 yMat;
        yMat.setZero();
        yMat(0, 0) = cos(degreesToRadians(yDeg));
        yMat(0, 2) = sin(degreesToRadians(yDeg));
        yMat(1, 1) = 1.0;
        yMat(2, 0) = -sin(degreesToRadians(yDeg));
        yMat(2, 2) = cos(degreesToRadians(yDeg));
    }

    MATRIX3 compose = zMat * yMat * xMat;
    for (int i = 0; i < vertices.size(); i++) {
        verticesCopy[i] -= -anchor;
        verticesCopy[i] = compose * verticesCopy[i];
        verticesCopy[i] += anchor;
    }

    return verticesCopy;
}