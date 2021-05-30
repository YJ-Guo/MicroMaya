#pragma once

#include <la.h>
#include "raytracer/ray.h"

//A perspective projection camera
//Receives its eye position and reference point from the scene XML file
class Camera
{
public:
    Camera();
    Camera(unsigned int w, unsigned int h);
    Camera(unsigned int w, unsigned int h, const glm::vec3 &e, const glm::vec3 &r, const glm::vec3 &worldUp);
    Camera(const Camera &c);

    float fovy;
    unsigned int width, height;  // Screen dimensions
    float near_clip;  // Near clip plane distance
    float far_clip;  // Far clip plane distance

    //Computed attributes
    float aspect;

    // Store the VPMat as a member variable
    glm::mat4 VPMatirx;

    glm::vec3 eye,      //The position of the camera in world space
              ref,      //The point in world space towards which the camera is pointing
              look,     //The normalized vector from eye to ref. Is also known as the camera's "forward" vector.
              up,       //The normalized vector pointing upwards IN CAMERA SPACE. This vector is perpendicular to LOOK and RIGHT.
              right,    //The normalized vector pointing rightwards IN CAMERA SPACE. It is perpendicular to UP and LOOK.
              world_up, //The normalized vector pointing upwards IN WORLD SPACE. This is primarily used for computing the camera's initial UP vector.
              V,        //Represents the vertical component of the plane of the viewing frustum that passes through the camera's reference point. Used in Camera::Raycast.
              H;        //Represents the horizontal component of the plane of the viewing frustum that passes through the camera's reference point. Used in Camera::Raycast.

    float theta;         // Represents the angle between current and x
    float phi;           // Represents the angele between current and z
    float zoom;          // Represents how much to zoom in

    glm::mat4 getViewProj();

    // Change the camera model from Orthogonal coordinate system to spherical system
    glm::mat4 orthToSpereCoord();

    void RecomputeAttributes();

    void RotateAboutUp(float deg);
    void RotateAboutRight(float deg);

    void RotateTheta(float deg);
    void RotatePhi(float deg);

    void TranslateAlongLook(float amt);
    void TranslateAlongRight(float amt);
    void TranslateAlongUp(float amt);

    void Zoom(float amt);

    // The raycast function takes in a 2D pixel coordinate
    // and return a Ray in world space
    Ray rayCast(const Point2f &pixelCord);
};
