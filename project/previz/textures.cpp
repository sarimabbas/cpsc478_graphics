#include "textures.hpp"

using namespace std;

VEC3 TexturePerlin::getColor(VEC3 point) {
    Real noise = pn.noise(point[0] * grainFactor, point[1] * grainFactor,
                          point[2] * grainFactor);
    return VEC3(noise, noise, noise);
}

TexturePerlin::TexturePerlin(Real grainFactor) : grainFactor(grainFactor) {}

TexturePerlin::~TexturePerlin() {}