#include "tracer.hpp"

using namespace std;

int MAX_RECURSION_DEPTH = 10;

Real frameCount = 0;

Camera::Camera(VEC3 eye, VEC3 lookAt, VEC3 up, int xRes, int yRes,
               Real distanceToPlane, Real fovy)
    : eye(eye),
      lookAt(lookAt),
      up(up),
      xRes(xRes),
      yRes(yRes),
      distanceToPlane(distanceToPlane),
      fovy(fovy),
      aspect((Real)xRes / (Real)yRes) {}

Light::Light(VEC3 position, VEC3 color) : position(position), color(color) {}

Ray::Ray(VEC3 origin, VEC3 direction) : origin(origin), direction(direction) {}

// default constructor
IntersectResult::IntersectResult()
    : t(0.0),
      doesIntersect(false),
      normal(VEC3(0.0, 0.0, 0.0)),
      intersectionPoint(VEC3(0.0, 0.0, 0.0)),
      intersectingShape(NULL) {}

// named constructor
IntersectResult::IntersectResult(Real t, bool doesIntersect, VEC3 normal,
                                 VEC3 intersectionPoint,
                                 Shape* intersectingShape)
    : t(t),
      doesIntersect(doesIntersect),
      normal(normal),
      intersectionPoint(intersectionPoint),
      intersectingShape(intersectingShape) {}

Ray rayGenerationAlt(int pixel_i, int pixel_j, Camera cam) {
    // compute image plane
    const float halfY =
        (cam.lookAt - cam.eye).norm() * tan(45.0f / 360.0f * M_PI);
    const float halfX = halfY * 4.0f / 3.0f;
    const VEC3 cameraZ = (cam.lookAt - cam.eye).normalized();
    const VEC3 cameraX = cam.up.cross(cameraZ).normalized();
    const VEC3 cameraY = cameraZ.cross(cameraX).normalized();
    // generate the ray, making x-axis go left to right
    const float ratioX =
        1.0f - ((cam.xRes - 1) - pixel_i) / float(cam.xRes) * 2.0f;
    const float ratioY = 1.0f - pixel_j / float(cam.yRes) * 2.0f;
    const VEC3 rayHitImage =
        cam.lookAt + ratioX * halfX * cameraX + ratioY * halfY * cameraY;
    const VEC3 rayDir = (rayHitImage - cam.eye).normalized();
    Ray generate = Ray(cam.eye, rayDir);
    return generate;
}

Ray rayGeneration(int pixel_i, int pixel_j, Camera cam) {
    // construct an eye coordinate frame
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

    // assert that the bounds found this way match up with another way
    // Real rightMinusLeft = (bottomRight - bottomLeft).norm();
    // Real topMinusBottom = (topRight - bottomRight).norm();

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

    return generate;
}

IntersectResult intersectScene(vector<Shape*> scene, Ray ray, Real tLow) {
    // for each primitive in the scene
    // keep track of the closest hit
    Real closestT = INFINITY;
    // Shape* closestShape = NULL;
    IntersectResult closestValidIntersection = IntersectResult();
    for (int i = 0; i < scene.size(); i++) {
        // get intersection result
        IntersectResult result = scene[i]->intersect(ray);
        // if there was an intersection
        if (result.doesIntersect == true) {
            // and the intersection had a positive t intersect
            if (result.t >= 0.0 && result.t < closestT) {
                // update the closest intersection
                closestT = result.t;
                closestValidIntersection = result;
            }
        }
    }
    return closestValidIntersection;
}

VEC3 rayColor(vector<Shape*> scene, Ray ray, vector<Light*> lights,
              Real phongExponent, bool useLights, bool useMultipleLights,
              bool useSpecular, bool useShadows, bool useMirror,
              int reflectionRecursionCounter, bool useRefraction,
              bool useFresnel, bool softShadows) {
    // do an intersection with the scene
    IntersectResult intersection = intersectScene(scene, ray, 0.0);

    // no intersection (return black)
    if (intersection.intersectingShape == NULL) {
        return VEC3(1.0, 1.0, 1.0);  // white background
    }

    // initialize with ambient
    VEC3 color = VEC3(0.0, 0.0, 0.0);

    // else if there is an intersection, do shading with lights
    if (useLights) {
        // for each light
        for (int i = 0; i < lights.size(); i++) {
            // shoot shadow ray
            if (useShadows) {
                Ray shadowRay =
                    createShadowRay(intersection, ray, lights[i], softShadows);
                IntersectResult shadowIntersect =
                    intersectScene(scene, shadowRay, 0.0);
                if (!shadowIntersect.doesIntersect) {
                    color += lightingEquation(lights[i], intersection,
                                              phongExponent, ray, useSpecular);
                }
            }
            // only use the first light if flag not set
            if (!useMultipleLights) {
                break;
            }
        }
    } else {
        color += intersection.intersectingShape->color;
    }

    if (useMirror) {
        if (intersection.intersectingShape->type == MIRROR &&
            reflectionRecursionCounter != MAX_RECURSION_DEPTH) {
            Ray reflectionRay = createReflectionRay(intersection, ray);
            // call the reflection recursively
            VEC3 reflectionColor =
                rayColor(scene, reflectionRay, lights, phongExponent, useLights,
                         useMultipleLights, useSpecular, useShadows, useMirror,
                         reflectionRecursionCounter + 1, useRefraction,
                         useFresnel, softShadows);
            color += reflectionColor;
        }
    }

    if (useRefraction) {
        if (intersection.intersectingShape->type == DIELECTRIC &&
            reflectionRecursionCounter != MAX_RECURSION_DEPTH) {
            if (useFresnel) {
                // get fresnel coefficients
                Real kReflectance = fresnel(intersection, ray);
                Real kRefraction = 1.0 - kReflectance;
                Ray refractionRay = createRefractionRay(intersection, ray);
                Ray reflectionRay = createReflectionRay(intersection, ray);
                VEC3 reflectionColor = rayColor(
                    scene, reflectionRay, lights, phongExponent, useLights,
                    useMultipleLights, useSpecular, useShadows, useMirror,
                    reflectionRecursionCounter + 1, useRefraction, useFresnel,
                    softShadows);
                VEC3 refractionColor = rayColor(
                    scene, refractionRay, lights, phongExponent, useLights,
                    useMultipleLights, useSpecular, useShadows, useMirror,
                    reflectionRecursionCounter + 1, useRefraction, useFresnel,
                    softShadows);
                color += (kReflectance * reflectionColor) +
                         (kRefraction * refractionColor);

            } else {
                // if entering dielectric
                Ray refractionRay = createRefractionRay(intersection, ray);
                VEC3 refractionColor = rayColor(
                    scene, refractionRay, lights, phongExponent, useLights,
                    useMultipleLights, useSpecular, useShadows, useMirror,
                    reflectionRecursionCounter + 1, useRefraction, useFresnel,
                    softShadows);
                color += refractionColor;
            }
        }
    }

    // do texturing
    if (intersection.intersectingShape->texture != NULL) {
        // texture lookup (fun: use time to make it animated)
        VEC3 lookup = VEC3(intersection.intersectionPoint[0],
                           intersection.intersectionPoint[1],
                           intersection.intersectionPoint[2]);
        color += intersection.intersectingShape->texture->getColor(lookup);
        frameCount += 1.0;
    }

    // prevent weird PPM problems by clamping color
    return clampVec3(color, 0.0, 1.0);
}

VEC3 lightingEquation(Light* light, IntersectResult intersection,
                      Real phongExponent, Ray ray, bool useSpecular) {
    // * calculate the three vectors
    // calculate L vec from the intersectionPoint and the light position
    VEC3 L = (light->position - intersection.intersectionPoint);
    L /= L.norm();
    // calculate V from the intersectionPoint and eye position
    VEC3 V = (ray.origin - intersection.intersectionPoint);
    V /= V.norm();
    // calculate the r vector from the dot product of L and normal
    VEC3 R = -L + (2 * L.dot(intersection.normal) * intersection.normal);
    R /= R.norm();

    // * calculate the three components
    // ambient
    VEC3 ambientColor = VEC3(0.0, 0.0, 0.0);
    Real ambientIntensity = 1.0;
    VEC3 ambientComponent = ambientColor * ambientIntensity;
    // diffuse
    VEC3 diffuseComponent = intersection.intersectingShape->color.cwiseProduct(
        light->color * std::max(0.0, intersection.normal.dot(L)));
    // diffuseComponent = clampVec3(diffuseComponent, 0.0, 1.0);

    // specular
    VEC3 specularComponent = intersection.intersectingShape->color.cwiseProduct(
        light->color * pow(std::max(0.0, R.dot(V)), phongExponent));
    // specularComponent = clampVec3(specularComponent, 0.0, 1.0);

    // * assemble the final color
    VEC3 finalColor;
    if (useSpecular) {
        // full shading for part 5.png
        finalColor = ambientComponent + diffuseComponent + specularComponent;
    } else {
        // diffuse shading for part 3.png
        finalColor = ambientComponent + diffuseComponent;
    }
    return clampVec3(finalColor, 0.0, 1.0);
}

Ray createShadowRay(IntersectResult intersection, Ray ray, Light* light,
                    bool softShadows) {
    // adjust to avoid shadow acne problem
    VEC3 adjustedIntersectionPoint =
        intersection.intersectionPoint + (CUSTOM_EPSILON * intersection.normal);
    // generate shadow ray towards the light
    VEC3 shadowDirection = (light->position - adjustedIntersectionPoint);
    // if softShadows, do Monte Carlo sampling and return a randomized ray
    if (softShadows) {
        // * calculate some vectors in the plane
        // normal is flipped because ur hitting the light
        VEC3 n = -intersection.normal;
        VEC3 p = light->position;  // p is the point light source
        // find two points
        VEC3 p1 =
            VEC3(0.0, 0.0, (((n[0] * p[0]) + (n[1] * p[1])) / n[2]) + p[2]);
        VEC3 p2 =
            VEC3((((n[1] * p[1]) + (n[2] * p[2])) / n[0]) + p[0], 0.0, 0.0);
        // create two vecs
        VEC3 u1 = p1 - p;
        u1 /= u1.norm();
        VEC3 u2 = p2 - p;
        u2 /= u2.norm();
        // generate combination
        u1 = (rand() % 1) * u1 * 0.5;
        u2 = (rand() % 1) * u2 * 0.5;
        // translate p point
        p = p + u1 + u2;
        // make direction
        shadowDirection = p - adjustedIntersectionPoint;
        shadowDirection /= shadowDirection.norm();
        Ray shadowRay(adjustedIntersectionPoint, shadowDirection);
        return shadowRay;
    }
    shadowDirection /= shadowDirection.norm();
    Ray shadowRay(adjustedIntersectionPoint, shadowDirection);
    return shadowRay;
}

Ray createReflectionRay(IntersectResult intersection, Ray ray) {
    // adjust to avoid shadow acne problem
    VEC3 adjustedIntersectionPoint =
        intersection.intersectionPoint + (CUSTOM_EPSILON * intersection.normal);
    // find the vector from intersection towards eye
    VEC3 V = (ray.origin - adjustedIntersectionPoint);
    V /= V.norm();
    // get the reflection direction
    VEC3 R = -V + (2.0 * V.dot(intersection.normal) * intersection.normal);
    R /= R.norm();
    // generate ray
    Ray reflectionRay(adjustedIntersectionPoint, R);
    return reflectionRay;
}

// Attribution
// https://www.scratchapixel.com/lessons/3d-basic-rendering/introduction-to-shading/reflection-refraction-fresnel
Ray createRefractionRay(IntersectResult intersection, Ray ray) {
    // normal
    VEC3 N = intersection.normal;

    // incidence
    VEC3 I = (intersection.intersectionPoint - ray.origin);
    I /= I.norm();

    // reflection
    VEC3 R = -I + (2.0 * I.dot(N) * N);
    R /= R.norm();

    // this gives direction of entry
    Real cosi = clamp(-1.0, 1.0, I.dot(N));

    // incoming and outgoing ior
    Real etai = REFRACT_AIR;
    Real etat = intersection.intersectingShape->refractiveIndex;

    // if going inside to outside, flip normal and swap ior
    VEC3 n = N;
    if (cosi < 0) {
        cosi = -cosi;
    } else {
        std::swap(etai, etat);
        n = -N;
    }

    // calculate ratio of ior
    Real eta = etai / etat;

    // test for total internal reflection
    Real k = 1.0 - eta * eta * (1.0 - cosi * cosi);

    // avoid refraction acne
    VEC3 adjustedPoint = intersection.intersectionPoint - (n * CUSTOM_EPSILON);

    if (k < 0.0) {
        Ray reflectedRay = Ray(adjustedPoint, R);
        return reflectedRay;
    } else {
        VEC3 T = eta * I + (eta * cosi - sqrt(k)) * n;
        Ray refractedRay = Ray(adjustedPoint, T);
        return refractedRay;
    }
}

// Attribution
// https://www.scratchapixel.com/lessons/3d-basic-rendering/introduction-to-shading/reflection-refraction-fresnel
Real fresnel(IntersectResult intersection, Ray ray) {
    Real kReflectance;

    VEC3 N = intersection.normal;

    // incidence
    VEC3 I = (intersection.intersectionPoint - ray.origin);
    I /= I.norm();

    // same side or separate
    Real cosi = clamp(-1.0, 1.0, I.dot(N));

    // the two indices of refraction
    Real etai = 1.0;
    Real etat = intersection.intersectingShape->refractiveIndex;

    // check direction
    if (cosi > 0.0) {
        std::swap(etai, etat);
    }
    Real iorRatio = etai / etat;

    // total internal reflection test
    Real sint = iorRatio * sqrt(std::max(0.0, 1.0 - cosi * cosi));

    // Total internal reflection
    if (sint >= 1.0) {
        kReflectance = 1.0;
    } else {
        Real cost = sqrt(std::max(0.0, 1.0 - sint * sint));
        cosi = fabs(cosi);
        Real Rs =
            ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
        Real Rp =
            ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
        kReflectance = (Rs * Rs + Rp * Rp) / 2.0;
    }

    return kReflectance;
}
