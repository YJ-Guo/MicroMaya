#include "joint.h"

Joint::Joint(OpenGLContext *context):
    Drawable(context)
{
    // Initialize all the member variables in a joint
    ID = -1;
    name = QString("InitJoint");
    parent = nullptr;
    position = Vector3f(0.f);
    rotation = glm::quat(1,0,0,0);
    bindMat = Matrix4x4(0.f);
    selected = false;
    setText(0, name);
    XRotate = 0;
    YRotate = 0;
    ZRotate = 0;
}

Matrix4x4 Joint::getLocalTransformation() {
    return glm::translate(Matrix4x4(1.f), position) *
            glm::mat4_cast(rotation) *
            glm::scale(Matrix4x4(1.f), Vector3f(1.f));
    // Here we default no scale as 1,1,1
}

Matrix4x4 Joint::getOverallTransformation() {
    // Multiply all the transformation from bottom up
    if (this->parent != nullptr) {
        return this->parent->getOverallTransformation() * getLocalTransformation();
    } else {
        return getLocalTransformation();
    }
}

void Joint::UpdateRotation()
{
    // Euler Angles to Rotation Matrix. X - Y - Z.
    glm::mat4 rMat = glm::rotate(glm::mat4(1.f), float(glm::radians(XRotate)), glm::vec3(1, 0, 0)) *
                     glm::rotate(glm::mat4(1.f), float(glm::radians(YRotate)), glm::vec3(0, 1, 0)) *
                     glm::rotate(glm::mat4(1.f), float(glm::radians(ZRotate)), glm::vec3(0, 0, 1));
    rotation = glm::quat_cast(rMat);
}

void Joint::addChild(Joint *_child) {
    // Add this child to the children list of current joint
    children.push_back(_child);
    // Connect this child properly
    _child->parent = this;
    // Add to GUI display
    QTreeWidgetItem::addChild(_child);
    _child->create();
}

// Hard code a 12-side polygon to mimic the wireframe shpere
void setSphereVertPos(std::vector<Point4f> &sphereVertPos) {
    // Sphere on the xy plane
    for (int i = 0; i < 12; ++i) {
        Point4f temp = glm::rotate(Matrix4x4(1.f), glm::radians(i * 30.f), Vector3f(0, 0, 1)) * Point4f(0, 0.5, 0, 1);
        sphereVertPos.at(i) = temp;
    }
    // Sphere on the yz plane
    for (int i = 12; i < 24; ++i) {
        Point4f temp = glm::rotate(Matrix4x4(1.f), glm::radians(i * 30.f), Vector3f(1, 0, 0)) * Point4f(0, 0.5, 0, 1);
        sphereVertPos.at(i) = temp;
    }
    // Sphere on the xz plane
    for (int i = 24; i < 36; ++i) {
        Point4f temp = glm::rotate(Matrix4x4(1.f), glm::radians(i * 30.f), Vector3f(0, 1, 0)) * Point4f(0.5, 0, 0, 1);
        sphereVertPos.at(i) = temp;
    }

    // If this joint is the root node with no more parent
    // for linking line between the parent and child
    sphereVertPos.at(36) = Point4f(0,0,0,1);
    sphereVertPos.at(37) = Point4f(0,0,0,1);
}

void setSphereVertPos(std::vector<Point4f> &sphereVertPos, Joint *curJoint) {
    // Sphere on the xy plane
    for (int i = 0; i < 12; ++i) {
        Point4f temp = glm::rotate(Matrix4x4(1.f), glm::radians(i * 30.f), Vector3f(0, 0, 1)) * Point4f(0, 0.5, 0, 1);
        sphereVertPos.at(i) = temp;
    }
    // Sphere on the yz plane
    for (int i = 12; i < 24; ++i) {
        Point4f temp = glm::rotate(Matrix4x4(1.f), glm::radians(i * 30.f), Vector3f(1, 0, 0)) * Point4f(0, 0.5, 0, 1);
        sphereVertPos.at(i) = temp;
    }
    // Sphere on the xz plane
    for (int i = 24; i < 36; ++i) {
        Point4f temp = glm::rotate(Matrix4x4(1.f), glm::radians(i * 30.f), Vector3f(0, 1, 0)) * Point4f(0.5, 0, 0, 1);
        sphereVertPos.at(i) = temp;
    }

    // This two more vertex indicate the position of a
    // joint and the parent of this joint
    sphereVertPos.at(36) = Point4f(0,0,0,1);
    sphereVertPos.at(37) = glm::inverse(curJoint->getLocalTransformation()) * Point4f(0,0,0,1);
}

// Hard code the linking indices of all vertices
void setVertLinking(std::vector<GLuint> &vertIndex) {
    // Link every two vertex with a line
    // For the three spheres use three loops
    for (int i = 0; i < 22; i += 2) {
        vertIndex.at(i) = i/2;
        vertIndex.at(i + 1) = 1 + i/2;
    }
    // connect the head and rear of the sphere1
    vertIndex.at(22) = 11;
    vertIndex.at(23) = 0;

    for (int i = 24; i < 46; i += 2) {
        vertIndex.at(i) = i/2;
        vertIndex.at(i + 1) = 1 + i/2;
    }
    // connect the head and rear of the sphere2
    vertIndex.at(46) = 23;
    vertIndex.at(47) = 12;

    for (int i = 48; i < 70; i += 2) {
        vertIndex.at(i) = i/2;
        vertIndex.at(i + 1) = 1 + i/2;
    }
    // connect the head and rear of the sphere3
    vertIndex.at(70) = 35;
    vertIndex.at(71) = 24;

    // Then the line between two joints
    vertIndex.at(72) = 36;
    vertIndex.at(73) = 37;
}

void setLineColor(std::vector<Color4f> &lineColor, bool selected) {
    if (!selected) {
        // Set Color for Sphere1
        for (int i = 0; i < 12; ++i) {
            lineColor.at(i) = Color4f(1, 0, 0, 0);
        }
        // Set Color for Sphere2
        for (int i = 12; i < 24; ++i) {
            lineColor.at(i) = Color4f(0, 1, 0, 0);
        }
        // Set Color for Sphere3
        for (int i = 24; i < 36; ++i) {
            lineColor.at(i) = Color4f(0, 0, 1, 0);
        }
        // Set Color for the linking line
        lineColor.at(36) = Color4f(1, 1, 0, 1);
        lineColor.at(37) = Color4f(1, 0, 1, 1);
    } else {
        // Show a different color behavior when selected
        for (int i = 0; i < lineColor.size(); ++i) {
            lineColor.at(i) = Color4f(1,1,1,0);
        }
    }
}

void Joint::create() {
    // Init proper space for joint display
    std::vector<Point4f> skeletonVertPos(38);
    std::vector<Color4f> skeletonColor(38);
    std::vector<GLuint> skeletonIndex(74);

    // Check for root node
    if (parent != nullptr) {
        setSphereVertPos(skeletonVertPos, this);
    } else {
        setSphereVertPos(skeletonVertPos);
    }

    setVertLinking(skeletonIndex);
    setLineColor(skeletonColor, selected);

    count = skeletonIndex.size();

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, skeletonIndex.size() * sizeof(GLuint), skeletonIndex.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, skeletonVertPos.size() * sizeof(glm::vec4), skeletonVertPos.data(), GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, skeletonColor.size() * sizeof(glm::vec4), skeletonColor.data(), GL_STATIC_DRAW);
}

void Joint::drawSkeleton(OpenGLContext &context, ShaderProgram &progFlat) {
    progFlat.setModelMatrix(getOverallTransformation());
    progFlat.draw(*this);

    // Draw all the other children joints recursively
    if (children.size() > 0) {
        for (int i = 0; i < children.size(); ++i) {
            children.at(i)->drawSkeleton(context, progFlat);
        }
    }
}

GLenum Joint::drawMode() {
    return GL_LINES;
}

void Joint::setBindMatrix() {
    bindMat = Matrix4x4(glm::inverse(getOverallTransformation()));
}
