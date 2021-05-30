#include "utils.h"
#include "transform.h"

Transform::Transform():
    Transform(Vector3f(0), Vector3f(0), Vector3f(1))
{}

Transform::Transform(const Vector3f &t, const Vector3f &r, const Vector3f &s):
    transVec(t),
    rotateVec(r),
    scaleVec(s)
{
    setMatrices();
}

void Transform::setMatrices() {
    worldTransform = glm::translate(glm::mat4(), transVec)
                   * glm::rotate(glm::mat4(), glm::radians(rotateVec.x), Vector3f(1, 0, 0))
                   * glm::rotate(glm::mat4(), glm::radians(rotateVec.y), Vector3f(0, 1, 0))
                   * glm::rotate(glm::mat4(), glm::radians(rotateVec.z), Vector3f(0, 0, 1))
                   * glm::scale(glm::mat4(), scaleVec);

    worldTransformInverse = glm::inverse(worldTransform);
    worldTransformInverseTranspose = glm::mat3(glm::transpose(worldTransformInverse));
}

// getter functions
const glm::mat4& Transform::getWorldTrans() const {
    return worldTransform;
}

const glm::mat4& Transform::getWorldTransInvs() const {
    return worldTransformInverse;
}

const glm::mat3& Transform::getWorldTransInvsTrp() const {
    return worldTransformInverseTranspose;
}

const Vector3f& Transform::getScaleVec() const {
    return scaleVec;
}
