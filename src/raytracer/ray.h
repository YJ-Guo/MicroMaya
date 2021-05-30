#ifndef RAY_H
#define RAY_H
#include "utils.h"

class Ray{
public:
   Point3f origin;
   Vector3f direction;

   Ray(const Point3f &o, const Vector3f &d);

   // The GetTransformation method returns a copy of the
   // transformed new Ray by transformation matrix TM
   Ray GetTransformation(const glm::mat4 &TM) const;

};


#endif // RAY_H
