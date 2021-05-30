#include "ray.h"

Ray::Ray(const Point3f &o, const Vector3f &d):
    origin(o), direction(d)
{}

Ray Ray::GetTransformation(const glm::mat4 &TM) const {
    // temporarily convert to vec4 for calculation
    // introduce homogenous coordinates
    glm::vec4 o = glm::vec4(origin, 1.f);
    glm::vec4 d = glm::vec4(direction, 0.f);

    // transform then get the first three to rebuild
    o = TM * o;
    d = TM * d;

    Ray newRay = Ray(Point3f(o.x, o.y, o.z), Vector3f(d.x, d.y, d.z));
    return newRay;
}

