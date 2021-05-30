#ifndef TRANSFORM_H
#define TRANSFORM_H
#include "utils.h"

class Transform{
private:
    Vector3f transVec,   // A 3D vector to store the translation transformation
             scaleVec,   // A 3D vector to store the scale transformation
             rotateVec;  // A 3D vector to store the rotation transformation

    glm::mat4 worldTransform,        // sequence: translate * (rotate along x, y, then z) * scale
              worldTransformInverse; // the inverse of worldTransform matrix

    glm::mat3 worldTransformInverseTranspose; // the transpose of the inverse of the world matrix

public:
    Transform();
    Transform(const Vector3f &t, const Vector3f &r, const Vector3f &s);

    // set the 3 transform matrices with input translation, rotation and scale
    void setMatrices();

    // getter functions
    const Vector3f &getScaleVec() const;
    const glm::mat4 &getWorldTrans() const;
    const glm::mat4 &getWorldTransInvs() const;
    const glm::mat3 &getWorldTransInvsTrp() const;

};

#endif // TRANSFORM_H
