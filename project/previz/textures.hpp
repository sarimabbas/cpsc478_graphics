#pragma once

#include <assert.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "PerlinNoise.h"
#include "SETTINGS.h"

using namespace std;

class Texture {
   public:
    virtual VEC3 getColor(VEC3 point) = 0;

    // virtual destructor
    virtual ~Texture(){};
};

class TexturePerlin : public Texture {
   public:
    PerlinNoise pn;
    Real grainFactor;
    VEC3 getColor(VEC3 point);

    TexturePerlin(Real grainFactor);

    ~TexturePerlin();
};
