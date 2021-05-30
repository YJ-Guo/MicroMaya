#ifndef SHAPE_H
#define SHAPE_H
#include "utils.h"
#include "raytracer/ray.h"
#include "raytracer/transform.h"

class Intersection;

class Shape{
public:
    Transform transform;

    Shape();
    Shape(Transform &transform);

    // a virtual destructor to make Shape class work as superclass
    virtual ~Shape(){}

    // find the signed distance of p from the surface
    virtual float sdf(Point3f) const = 0;
};


// A sphere class to mimic Vertices of the Mesh
class Sphere : public Shape {
public:
    Point3f origin;  // the origin of the sphere
    float radius;    // th radius of the sphere

    Sphere(Point3f origin);
    float sdf(Point3f) const;
    ~Sphere(){}
};

// A Capsule shape class to mimic HalfEdge of the Mesh
class Capsule : public Shape {
private:
    Point3f a;  // origin of semi-shpere at front
    Point3f b;  // origin of semi-shpere at rear
    float r;     // radius of the semi-sphere

public:
    Capsule(Point3f head, Point3f tail);
    float sdf(Point3f) const;
    ~Capsule(){}
};

class TriangleFace : public Shape {
private:
    // Three Vertex for a TriangleFace
    Point3f a;
    Point3f b;
    Point3f c;

public:
    TriangleFace(Point3f a, Point3f b, Point3f c);
    float sdf(Point3f) const;
    ~TriangleFace(){}
};

#endif // SHAPE_H
