#ifndef MYGL_H
#define MYGL_H

#include <openglcontext.h>
#include <utils.h>
#include <shaderprogram.h>
#include <scene/squareplane.h>
#include "camera.h"

#include "meshhighlight.h"
#include "joint.h"

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class MyGL
    : public OpenGLContext
{
    Q_OBJECT
private:
    SquarePlane m_geomSquare;// The instance of a unit cylinder we can use to render any cylinder
    ShaderProgram m_progLambert;// A shader program that uses lambertian reflection
    ShaderProgram m_progFlat;// A shader program that uses "flat" reflection (no shadowing at all)
    ShaderProgram m_progSkeleton; // A shader that compiles the skeleton shader

    GLuint vao; // A handle for our vertex array object. This will store the VBOs created in our geometry classes.
                // Don't worry too much about this. Just know it is necessary in order to render geometry.

    Camera m_glCamera;

    // A variable used to track the mouse's previous position when
    // clicking and dragging on the GL viewport. Used to move the camera
    // in the scene.
    glm::vec2 m_mousePosPrev;


public:
    explicit MyGL(QWidget *parent = nullptr);
    ~MyGL();

    Mesh m_geoCube;
    VertexHighLight m_vertexDisplay;
    HalfEdgeHighLight m_edgeDisplay;
    FaceHighLight m_faceDisplay;
    int m_SelectionMode;
    Matrix4x4 bindMats[100];
    Matrix4x4 transMats[100];

    // Split a half edge into 2 then add a vertex at middle
    void SplitHalfEdge(HalfEdge *edge, Mesh &cubeMesh);
    // Triangulate the face
    void TriangulateFace(Face *face, Mesh &cubeMesh);
    // Extrude a selected face along its surface normal
    void ExtrudeFace(Face *face, Mesh &cubeMesh, float distance = 1.f);

    // Check if the in put face is a plane with default tolerance
    // return true if is plane
    bool TestPlanarity(Face *testFace, float tolerance = 0.00001f);
    void MakePlane(Mesh &cubeMesh);

    void SceneLoadDialog();

    // This part for HW07
    // Return the root node of the joint tree
    QString SkeletonLoadDialog();
    Joint* LoadSkeleton(const QString filepath);
    Joint* initWithJSON(QJsonObject jointObj, Joint* parent);
    // The root node of the skeleton joint
    Joint* joint;
    std::vector<Joint*> mJointList;
    int currentID;
    void getNearestJoints(Vertex* tarVertex, std::vector<Joint*> &nearestJoints);
    void Skinning(Mesh &cubeMesh, Joint *root);
    void JointUpdate();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

protected:
    void keyPressEvent(QKeyEvent *e);
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void wheelEvent(QWheelEvent* e);

signals:
    void DisplayMesh(Mesh &cubeMesh);
    void FoundVertex(QListWidgetItem* item);
    void FoundEdge(QListWidgetItem* item);
    void FoundFace(QListWidgetItem* item);
};


#endif // MYGL_H
