#include "shape.h"

Shape::Shape(){}

Shape::Shape(Transform &transform):
    transform(transform)
{}

Sphere::Sphere(Point3f origin):
    origin(origin),
    radius(0.04f)
{}

float Sphere::sdf(Point3f p) const {
    // first transform the input point p using homogeneous coordinates
    glm::vec4 localPoint = transform.getWorldTransInvs() * glm::vec4(p, 1.0f);
    p = Point3f(localPoint.x, localPoint.y, localPoint.z); // rebuild point p

    // using sfd formula for sphere from IQ's blog
    float sdfDistance = glm::length(p - origin) - radius;

    // scale the sdf distance by the minimum for return
    float minScale = glm::min(transform.getScaleVec().x, transform.getScaleVec().y);
    minScale = glm::min(minScale, transform.getScaleVec().y);
    return sdfDistance * minScale;
}

Capsule::Capsule(Point3f head, Point3f tail):
    a(head),
    b(tail),
    r(0.07f)
{}

float Capsule::sdf(Point3f p) const {
    // first transform the input point p using homogeneous coordinates
    glm::vec4 localPoint = transform.getWorldTransInvs() * glm::vec4(p, 1.0f);
    p = Point3f(localPoint.x, localPoint.y, localPoint.z); // rebuild point p

//    Vector3f pa = p - a;
//    Vector3f ba = b - a;
//    float h = glm::clamp(glm::dot(pa, ba)/glm::dot(ba, ba), 0.f, 1.f);
//    float sdfDistance = glm::length(pa - ba * h) - r;

    Vector3f  ba = b - a;
    Vector3f  pa = p - a;
    float baba = glm::dot(ba, ba);
    float paba = glm::dot(pa, ba);
    float x = glm::length(pa * baba - ba * paba) - r * baba;
    float y = glm::abs(paba - baba * 0.5) - baba * 0.5;
    float x2 = x * x;
    float y2 = y * y * baba;
    float d = (glm::max(x,y) < 0.0) ? -glm::min(x2, y2) : (((x > 0.0) ? x2 : 0.0) + ((y > 0.0) ? y2 : 0.0));
    float sdfDistance =  glm::sign(d) * glm::sqrt(glm::abs(d))/baba;

    // scale the sdf distance by the minimum for return
    float minScale = glm::min(transform.getScaleVec().x, transform.getScaleVec().y);
    minScale = glm::min(minScale, transform.getScaleVec().y);
    return sdfDistance * minScale;
}

TriangleFace::TriangleFace(Point3f a, Point3f b, Point3f c):
  a(a),
  b(b),
  c(c)
{}

float dot2(Vector3f a) {
    return glm::dot(a, a);
}

float TriangleFace::sdf(Point3f p) const {
    // first transform the input point p using homogeneous coordinates
    glm::vec4 localPoint = transform.getWorldTransInvs() * glm::vec4(p, 1.0f);
    p = Point3f(localPoint.x, localPoint.y, localPoint.z); // rebuild point p

    Vector3f ba = b - a; Vector3f pa = p - a;
    Vector3f cb = c - b; Vector3f pb = p - b;
    Vector3f ac = a - c; Vector3f pc = p - c;
    Vector3f nor = glm::cross( ba, ac );

    float sdfDistance = glm::sqrt(
                (glm::sign(glm::dot(glm::cross(ba,nor),pa)) +
                 glm::sign(glm::dot(glm::cross(cb,nor),pb)) +
                 glm::sign(glm::dot(glm::cross(ac,nor),pc)) < 2.0)
                ?
                 glm::min( glm::min(
                    dot2(ba*glm::clamp(glm::dot(ba,pa)/dot2(ba),0.f,1.f)-pa),
                    dot2(cb*glm::clamp(glm::dot(cb,pb)/dot2(cb),0.f,1.f)-pb)),
                    dot2(ac*glm::clamp(glm::dot(ac,pc)/dot2(ac),0.f,1.f)-pc))
                :
                 glm::dot(nor,pa)*glm::dot(nor,pa)/dot2(nor) );

    // scale the sdf distance by the minimum for return
    float minScale = glm::min(transform.getScaleVec().x, transform.getScaleVec().y);
    minScale = glm::min(minScale, transform.getScaleVec().y);
    return sdfDistance * minScale;
}
