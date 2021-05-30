#ifndef JOINT_H
#define JOINT_H
#include "utils.h"
#include "drawable.h"
#include "shaderprogram.h"
#include <QTreeWidgetItem>

class Joint : public QTreeWidgetItem, public Drawable {
public:
    int ID;                         // The ID of the joint.
    QString name;                   // The name of this joint.
    Joint *parent;                  // The parent of this joint
    std::vector<Joint*> children;   // The set of joints that have this joint as their parent.
    Vector3f position;              // The position of this joint relative to its parent joint.
    glm::quat rotation;             // The quaternion that represents this joint's current orientation.
    Matrix4x4 bindMat;              // The inverse of the joint's compound transformation matrix at the time
                                    // a mesh is bound to the joint's skeleton.
    bool selected;                  // If the joint is selected then selected = true;

    float XRotate;
    float YRotate;
    float ZRotate;

    // Returns a mat4 to represent the concatenation of a joint's position and rotation.
    Matrix4x4 getLocalTransformation();
    // Returns a mat4 that represents the concatentation of this joint's
    // local transformation with the transformations of its chain of parent joints.
    Matrix4x4 getOverallTransformation();

    // Use the GUI input data to update Rotation Quaternion
    void UpdateRotation();

    Joint(OpenGLContext *context);
    void addChild(Joint *_child);
    // Draw the joints as wireframe spheres and a single line
    void create() override;
    // Use this function to draw all the skeleton information after loading it
    void drawSkeleton(OpenGLContext &context, ShaderProgram &progFlat);
    virtual GLenum drawMode() override;

    void setBindMatrix();
};


#endif // JOINT_H
