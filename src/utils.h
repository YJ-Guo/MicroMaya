#ifndef UTILS_H
#define UTILS_H
#pragma once

#include <string>
#include <cmath>
#include <vector>
#include <optional>
#include "smartpointerhelp.h"
#include "la.h"

static const float PI = 3.14159265358979323846f;

// Some name alias
typedef glm::vec3 Color3f;
typedef glm::vec3 Point3f;
typedef glm::vec3 Normal3f;
typedef glm::vec2 Point2f;
typedef glm::ivec2 Point2i;
typedef glm::ivec3 Point3i;
typedef glm::vec3 Vector3f;
typedef glm::vec4 Vector4f;
typedef glm::vec4 Point4f;
typedef glm::vec4 Color4f;
typedef glm::vec4 Normal4f;
typedef glm::vec2 Vector2f;
typedef glm::ivec2 Vector2i;
typedef glm::mat4 Matrix4x4;
typedef glm::mat3 Matrix3x3;

/// Float approximate-equality comparison
template<typename T>
inline bool fequal(T a, T b, T epsilon = 0.0001){
    if (a == b) {
        // Shortcut
        return true;
    }

    const T diff = std::abs(a - b);
    if (a * b == 0) {
        // a or b or both are zero; relative error is not meaningful here
        return diff < (epsilon * epsilon);
    }

    return diff / (std::abs(a) + std::abs(b)) < epsilon;
}


#endif // UTILS_H
