#pragma once

#include <assert.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "SETTINGS.h"

using namespace std;

class Texture {
   public:
    virtual VEC3 getColor(VEC3 point) = 0;
};
