#include "camera.h"

#include <la.h>
#include <iostream>


Camera::Camera():
    Camera(400, 400)
{
    look = glm::vec3(0,0,-1);
    up = glm::vec3(0,1,0);
    right = glm::vec3(1,0,0);
}

Camera::Camera(unsigned int w, unsigned int h):
    Camera(w, h, glm::vec3(0,0,10), glm::vec3(0,0,0), glm::vec3(0,1,0))
{}

Camera::Camera(unsigned int w, unsigned int h, const glm::vec3 &e, const glm::vec3 &r, const glm::vec3 &worldUp):
    fovy(45),
    width(w),
    height(h),
    near_clip(0.1f),
    far_clip(1000),
    eye(e),
    ref(r),
    world_up(worldUp),
    up(Vector3f(0, 1,0)),
    zoom(-5.f)
{
    RecomputeAttributes();
}

Camera::Camera(const Camera &c):
    fovy(c.fovy),
    width(c.width),
    height(c.height),
    near_clip(c.near_clip),
    far_clip(c.far_clip),
    aspect(c.aspect),
    eye(c.eye),
    ref(c.ref),
    look(c.look),
    up(c.up),
    right(c.right),
    world_up(c.world_up),
    V(c.V),
    H(c.H),
    theta(0.f),
    phi(0.f),
    zoom(-5.f)
{}

glm::mat4 Camera::orthToSpereCoord() {
    // Use a standard camera and apply transformation to it
    glm::mat4 ry = glm::rotate(glm::mat4(1.f), glm::radians(theta), glm::vec3(0, 1, 0));
    glm::mat4 rx = glm::rotate(glm::mat4(1.f), glm::radians(phi), glm::vec3(1, 0, 0));
    glm::mat4 t = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, zoom));
    return rx * ry *t;
}

void Camera::RecomputeAttributes()
{
    look = glm::normalize(ref - eye);
    right = glm::normalize(glm::cross(look, world_up));
    up = glm::cross(right, look);


    float tan_fovy = tan(glm::radians(fovy/2));
    //float len = glm::length(ref - eye);

    glm::mat4 sphereToOrth = glm::translate(glm::mat4(1.f), ref);
    Vector4f eyeWorld = sphereToOrth * orthToSpereCoord() * Vector4f(Vector3f(eye), 1.f);

    float len = glm::length(ref - Vector3f(eyeWorld));
    aspect = width / static_cast<float>(height);
    V = up*len*tan_fovy;
    H = right*len*aspect*tan_fovy;
}

glm::mat4 Camera::getViewProj()
{
    // Initialize the spherical transformation matrix with reference off set
    glm::mat4 sphereToOrth = glm::translate(glm::mat4(1.f), ref);
    Vector4f eyeWorld = sphereToOrth * orthToSpereCoord() * Vector4f(Vector3f(eye), 1.f);
    Vector4f upWorld = sphereToOrth * orthToSpereCoord() * Vector4f(Vector3f(up), 0.f);
    VPMatirx = glm::perspective(glm::radians(fovy), width / (float)height, near_clip, far_clip)
            * glm::lookAt(Vector3f(eyeWorld), ref, Vector3f(upWorld));
    return VPMatirx;
}

void Camera::RotateAboutUp(float deg)
{
//    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(deg), up);
//    ref = ref - eye;
//    ref = glm::vec3(rotation * glm::vec4(ref, 1));
//    ref = ref + eye;

    theta += deg;
    RecomputeAttributes();
}
void Camera::RotateAboutRight(float deg)
{
//    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(deg), right);
//    ref = ref - eye;
//    ref = glm::vec3(rotation * glm::vec4(ref, 1));
//    ref = ref + eye;
    phi += deg;
    RecomputeAttributes();
}

void Camera::Zoom(float amt)
{
    glm::vec3 translation = look * amt;
    eye += translation;
}


void Camera::TranslateAlongLook(float amt)
{
//    glm::vec3 translation = look * amt;
//    eye += translation;
//    ref += translation;
    zoom -= amt;

}

void Camera::TranslateAlongRight(float amt)
{
    glm::vec3 translation = right * amt;
//    eye += translation;
    ref += translation;
}

void Camera::TranslateAlongUp(float amt)
{
    glm::vec3 translation = up * amt;
//    eye += translation;
    ref += translation;
}

Ray Camera::rayCast(const Point2f &pixelCord) {
    // compute the coordinates in NDC
    float ndcX = (2.f * pixelCord.x / width - 1);
    float ndcY = (1 - 2.f * pixelCord.y / height);

    glm::mat4 sphereToOrth = glm::translate(glm::mat4(1.f), ref);
    Vector4f eyeWorld = sphereToOrth * orthToSpereCoord() * Vector4f(Vector3f(eye), 1.f);

    Vector3f p = ref + ndcX * H + ndcY * V;

    Ray resultRay(Vector3f(eyeWorld), glm::normalize(p - Vector3f(eyeWorld)));

    return resultRay;
}
