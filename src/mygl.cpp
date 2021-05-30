#include "mygl.h"
#include <la.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <QFileDialog>
#include <QDebug>


MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_geomSquare(this),
      m_progLambert(this), m_progFlat(this), m_progSkeleton(this),
      m_glCamera(), m_mousePosPrev(), m_geoCube(this), m_vertexDisplay(this),
      m_edgeDisplay(this), m_faceDisplay(this), m_SelectionMode(0), joint(nullptr),currentID(0)
{
    setFocusPolicy(Qt::StrongFocus);
}

MyGL::~MyGL()
{
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
    //m_geomSquare.destroy();
    m_geoCube.destroy();
    m_vertexDisplay.destroy();
    m_edgeDisplay.destroy();
    m_faceDisplay.destroy();
    joint->destroy();
}

void MyGL::SceneLoadDialog() {
    QString filepath = QFileDialog::getOpenFileName(0, QString("Load Scene"), QString(),QString("*.obj"));
    if(filepath.length() == 0)
    {
        std::cout << "Invalid File Name!" << std::endl;
        return;
    }

    m_geoCube.SetUpOBJMesh(filepath);
}

Joint* MyGL::initWithJSON(QJsonObject jointObj, Joint *parent) {
    QJsonArray jointPosition = jointObj["pos"].toArray();
    QJsonArray jointRotation = jointObj["rot"].toArray();
    QJsonArray jointChildren = jointObj["children"].toArray();
    QString name = jointObj["name"].toString();

    // Get the joint position
    Vector3f position = Vector3f(jointPosition[0].toDouble(),
                                 jointPosition[1].toDouble(),
                                 jointPosition[2].toDouble());

    // Get the rotation information to build quaternion
    double angle = jointRotation[0].toDouble();
    Vector3f axis = Vector3f(jointRotation[1].toDouble(),
                             jointRotation[2].toDouble(),
                             jointRotation[3].toDouble());
    glm::quat rotation = glm::quat(glm::angleAxis(float(angle), axis));

//    uPtr<Joint> jointUPtr = mkU<Joint>(Joint(this));
//    Joint* tempJoint = jointUPtr.get();
    Joint* tempJoint = new Joint(this);
    tempJoint->name = name;
    tempJoint->position = position;
    tempJoint->rotation = rotation;
    tempJoint->ID = currentID++;
    tempJoint->bindMat = glm::inverse(tempJoint->getOverallTransformation());
    tempJoint->setText(0, QString(QString::number(tempJoint->ID) + ". " + name));
    tempJoint->create();
    mJointList.push_back(tempJoint);

    for (int i = 0; i < jointChildren.size(); ++i) {
        Joint* child = initWithJSON(jointChildren[i].toObject(), tempJoint);
    }

    // Use to add child to the QTreeWidget
    if (parent != nullptr) {
        parent->addChild(tempJoint);
    }

    return tempJoint;
}

QString MyGL::SkeletonLoadDialog() {
    QString filepath = QFileDialog::getOpenFileName(0, QString("Load Skeleton"),QString(),QString("*.json"));
    if(filepath.length() == 0)
    {
        std::cout << "Invalid File Name!" << std::endl;
        return "";
    }
    return filepath;
}

Joint* MyGL::LoadSkeleton(const QString filepath) {
    // Open JSON and read all the information
    QFile file;
    file.setFileName(filepath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString allLines = file.readAll();
    file.close();
    // Use Json reader to translate information from text
    QJsonDocument doc = QJsonDocument::fromJson(allLines.toUtf8());
    QJsonObject obj = doc.object();
    QJsonObject jointObj = obj["root"].toObject();

    Joint* root = initWithJSON(jointObj, nullptr);

    return root;
}


void MyGL::SplitHalfEdge(HalfEdge *edge, Mesh &cubeMesh) {
    // Find the two end points of the half edge
    // With the edge itself and its SYM half edge
    HalfEdge *edge1 = edge;
    HalfEdge *edge1s = edge->mSYMEdgePtr;

    // V1 as the edge points to,
    // V2 as the SYM edge points to,
    // Vm as the middle point of V1 and V2
    Vertex *V1 = edge1->mVertexPtr;
    Vertex *V2 = edge1s->mVertexPtr;
    uPtr<Vertex> Vm = mkU<Vertex>(Vertex());
    // Set Vm as the mid point of V1 and V2
    // There will be an unproper shading effect when not add a little vector
    Vm->mPos = Point4f(Point3f((V1->mPos + V2->mPos) / 2.f), 1.f);
    if (!fequal(Vm->mPos.x, 0.f)) {
        Vm->mPos.x += 0.0001f;
    }
    if (!fequal(Vm->mPos.y, 0.f)) {
        Vm->mPos.y += 0.0001f;
    }
    if (!fequal(Vm->mPos.z, 0.f)) {
        Vm->mPos.z += 0.0001f;
    }

    // Creat two new half edges edge2 and edge2s an link them properly
    uPtr<HalfEdge> edge2 = mkU<HalfEdge>(HalfEdge());
    uPtr<HalfEdge> edge2s = mkU<HalfEdge>(HalfEdge());
    // Link the new edges with faces
    edge2->mFacePtr = edge1->mFacePtr;
    edge2s->mFacePtr = edge1s->mFacePtr;
    // Link the new edges with sym edge
    edge2->mSYMEdgePtr = edge2s.get();
    edge2s->mSYMEdgePtr = edge2.get();
    // Link the new edges with vertex pointing to
    edge2->mVertexPtr = Vm.get();
    edge2s->mVertexPtr = V2;
    edge1s->mVertexPtr = Vm.get();
    Vm->mEdgePtr = edge2.get();
    // Link the new edges with its next half edge
    edge2->mNextEdgePtr = edge1;
    edge2s->mNextEdgePtr = edge1s->mNextEdgePtr;
    edge1s->mNextEdgePtr = edge2s.get();
    // Change the edge before edge1 to point to edge2
    HalfEdge *nextEdge = edge1->mNextEdgePtr;
    while (nextEdge->mNextEdgePtr != edge1) {
        nextEdge = nextEdge->mNextEdgePtr;
    }
    nextEdge->mNextEdgePtr = edge2.get();

    // Add the new edges and vertex into the mesh
    cubeMesh.mVerList.push_back(std::move(Vm));
    cubeMesh.mEdgeList.push_back(std::move(edge2));
    cubeMesh.mEdgeList.push_back(std::move(edge2s));
}

int CountEdge(Face* face) {
    int edgeCount = 1;
    HalfEdge *startEdge = face->mEdgePtr;
    HalfEdge *nextEdge = startEdge->mNextEdgePtr;
    ++edgeCount;
    // find the previous edge from the starting edge
    while (nextEdge->mNextEdgePtr != startEdge) {
        ++edgeCount;
        nextEdge = nextEdge->mNextEdgePtr;
    }
    return edgeCount;
}

void MyGL::TriangulateFace(Face *face, Mesh &cubeMesh) {
    // First count the number of edges in the face,
    // if <= 3 then stop triangulation
    int edgeCount = 1;
    HalfEdge *startEdge = face->mEdgePtr;
    HalfEdge *nextEdge = startEdge->mNextEdgePtr;
    ++edgeCount;
    // find the previous edge from the starting edge
    while (nextEdge->mNextEdgePtr != startEdge) {
        ++edgeCount;
        nextEdge = nextEdge->mNextEdgePtr;
    }
    // Only begin triangulation when number of edges > 3
    if (edgeCount > 3) {
        // Make the original face as a triangle
        // and face2 as the residue of once face triangulation
        uPtr<Face> face2 = mkU<Face>(Face());
        uPtr<HalfEdge> neoEdge1 = mkU<HalfEdge>(HalfEdge());
        uPtr<HalfEdge> neoEdge2 = mkU<HalfEdge>(HalfEdge());
        // Change the linking property of 2 faces, 2 neo edges
        // Link the new edge in first triangle
        // Link the new edge in the other half of the face
        neoEdge2->mNextEdgePtr = startEdge->mNextEdgePtr->mNextEdgePtr;
        neoEdge2->mVertexPtr = startEdge->mNextEdgePtr->mVertexPtr;

        startEdge->mNextEdgePtr->mNextEdgePtr = neoEdge1.get();
        neoEdge1->mNextEdgePtr = startEdge;
        neoEdge1->mFacePtr = face;
        neoEdge1->mVertexPtr = nextEdge->mVertexPtr;
        neoEdge1->mSYMEdgePtr = neoEdge2.get();

        nextEdge->mNextEdgePtr = neoEdge2.get();
        neoEdge2->mFacePtr = face2.get();
        neoEdge2->mSYMEdgePtr = neoEdge1.get();

        // Change the face pointer of each edges in the new face
        HalfEdge* neoNextEdge = neoEdge2->mNextEdgePtr;
        while(neoNextEdge != neoEdge2.get()) {
            neoNextEdge->mFacePtr = face2.get();
            neoNextEdge = neoNextEdge->mNextEdgePtr;
        }

        // Link the new face with mesh components
        face2->mColor = face->mColor;
        face2->mEdgePtr = neoEdge2.get();

        // Add the new face and 2 new edges to the list
        cubeMesh.mFaceList.push_back(std::move(face2));
        cubeMesh.mEdgeList.push_back(std::move(neoEdge1));
        cubeMesh.mEdgeList.push_back(std::move(neoEdge2));

        // Recursively call the triangulate function with new face
        // Note that face2 has been moved
        TriangulateFace(cubeMesh.mFaceList.back().get(), cubeMesh);
    }
}


bool MyGL::TestPlanarity(Face *testFace, float tolerance) {
    // For every 4 vertex in a face, get 3 vector edges
    // Calculate there cross product of the 2 vectors
    // Then dot with the other vector to see if result < tolerance
    // If the number of edges <= 3 then must be plane
    // Traverse all the vertices in the face
    int edgeCount = CountEdge(testFace);
    bool flag = false;

    HalfEdge *startEdge = testFace->mEdgePtr;
    HalfEdge *nextEdge = startEdge->mNextEdgePtr;
    if (edgeCount <= 3) {
        return true;
    } else {
        // use a do-while loop to traverse all the vertices
        do {
            Point4f p0 = startEdge->mVertexPtr->mPos;
            Point4f p1 = nextEdge->mVertexPtr->mPos;
            Point4f p2 = nextEdge->mNextEdgePtr->mVertexPtr->mPos;
            Point4f p3 = nextEdge->mNextEdgePtr->mNextEdgePtr->mVertexPtr->mPos;

            Vector3f e1 = Vector3f(p1 - p0);
            Vector3f e2 = Vector3f(p2 - p0);
            Vector3f e3 = Vector3f(p3 - p0);

            float dotRes = glm::dot(e3, glm::cross(e1, e2));
            flag = fequal(dotRes, 0.f, tolerance);
            nextEdge = nextEdge->mNextEdgePtr;

        } while (nextEdge->mNextEdgePtr->mNextEdgePtr != startEdge);
        return flag;
    }
}

void MyGL::MakePlane(Mesh &cubeMesh) {
    // iterate over all faces to chek if need to be triangulated
    // Need to use goto to avoid vector resizing caused memory relocate
START:std::vector<uPtr<Face>>::iterator faceIt;
    for (faceIt = cubeMesh.mFaceList.begin(); faceIt != cubeMesh.mFaceList.end(); ++faceIt) {
        // If not a plane then triangulate it
        if (!TestPlanarity(faceIt->get(),0.00001f)) {
            TriangulateFace(faceIt->get(), cubeMesh);
            goto START;
        }
    }
}

// Calculate the normal of the face by averaging the normals of each
// Vertex on the face
Normal4f GetFaceNormal(Face *face) {
    HalfEdge *nextEdge = face->mEdgePtr;
    Normal4f sumNor(0.f);
    int countNor = 0;

    do {
        Point4f p1 = nextEdge->mVertexPtr->mPos;
        Point4f p2 = nextEdge->mSYMEdgePtr->mVertexPtr->mPos;
        Point4f p3 = nextEdge->mNextEdgePtr->mVertexPtr->mPos;
        Normal4f faceNor = Normal4f(Normal3f(glm::cross(Vector3f(p3 - p1), Vector3f(p2 - p1))), 0.f);
        if (!IsBlack(faceNor)) {
            sumNor += faceNor;
            ++countNor;
        }
        nextEdge = nextEdge->mNextEdgePtr;
    } while(nextEdge != face->mEdgePtr);

    return sumNor / (float)countNor;
}

void MyGL::ExtrudeFace(Face *face, Mesh &cubeMesh, float distance) {
    // First translate the selected face along its normal for some distance
    // Also need to creat 4 new vertices for this face
    Normal4f faceNor = GetFaceNormal(face);

    HalfEdge *edge = face->mEdgePtr;
    // STEP 1
    // Calculate the new vertex position along normal
    do {
        Vector4f direction = faceNor * distance;
        uPtr<Vertex> vertUPtr = mkU<Vertex>(Vertex());
        vertUPtr->mPos = edge->mVertexPtr->mPos + direction;
        // Update the linking b/w vertex and edge
        edge->mVertexPtr = vertUPtr.get();
        vertUPtr.get()->mEdgePtr = edge;
        cubeMesh.mVerList.push_back(std::move(vertUPtr));
        // Go to next edge
        edge = edge->mNextEdgePtr;
    } while (edge != face->mEdgePtr);

    // STEP 2
    // Now edge is face->mEdgePtr again
    // Now extrude every edge
    do {
        uPtr<HalfEdge> HE1BUPtr = mkU<HalfEdge>(HalfEdge());
        uPtr<HalfEdge> HE2BUPtr = mkU<HalfEdge>(HalfEdge());
        uPtr<HalfEdge> HE3UPtr = mkU<HalfEdge>(HalfEdge());
        uPtr<HalfEdge> HE4UPtr = mkU<HalfEdge>(HalfEdge());
        uPtr<Face> faceUPtr = mkU<Face>(Face());

        HalfEdge *HE2 = edge->mSYMEdgePtr;
        HalfEdge *HE1Pre = GetPreviousEdge(edge);
        HalfEdge *HE2Pre = GetPreviousEdge(HE2);

        Vertex *v1 = HE2Pre->mVertexPtr;
        Vertex *v2 = HE2->mVertexPtr;
        Vertex *v3 = edge->mVertexPtr;
        Vertex *v4 = HE1Pre->mVertexPtr;

        // Then we link these components
        faceUPtr->mEdgePtr = HE1BUPtr.get();
        // Use original color as the new faces' color
        faceUPtr->mColor = edge->mFacePtr->mColor;

        // Set up the Vertices, Edges amd Faces as in PPT
        HE1Pre->mVertexPtr = v4;
        HE1BUPtr->mVertexPtr = v4;
        HE2BUPtr->mVertexPtr = v1;
        HE3UPtr->mVertexPtr = v3;
        HE4UPtr->mVertexPtr = v2;

        edge->mSYMEdgePtr = HE1BUPtr.get();
        HE1BUPtr->mSYMEdgePtr = edge;
        HE2->mSYMEdgePtr = HE2BUPtr.get();
        HE2BUPtr->mSYMEdgePtr = HE2;

        HE1BUPtr->mNextEdgePtr = HE4UPtr.get();
        HE4UPtr->mNextEdgePtr = HE2BUPtr.get();
        HE2BUPtr->mNextEdgePtr = HE3UPtr.get();
        HE3UPtr->mNextEdgePtr = HE1BUPtr.get();

        HE1BUPtr->mFacePtr = faceUPtr.get();
        HE2BUPtr->mFacePtr = faceUPtr.get();
        HE3UPtr->mFacePtr = faceUPtr.get();
        HE4UPtr->mFacePtr = faceUPtr.get();

        // Then add these new components to corresponding list
        cubeMesh.mFaceList.push_back(std::move(faceUPtr));
        cubeMesh.mEdgeList.push_back(std::move(HE1BUPtr));
        cubeMesh.mEdgeList.push_back(std::move(HE2BUPtr));
        cubeMesh.mEdgeList.push_back(std::move(HE3UPtr));
        cubeMesh.mEdgeList.push_back(std::move(HE4UPtr));

        edge = edge->mNextEdgePtr;
    } while(edge != face->mEdgePtr);

    // STEP 3
    // Now edge is face->mEdgePtr again
    // Link the SYM edges with old face after adding all the new edges
    do {
        GetPreviousEdge(edge->mSYMEdgePtr)->mSYMEdgePtr = edge->mNextEdgePtr->mSYMEdgePtr->mNextEdgePtr;
        edge->mNextEdgePtr->mSYMEdgePtr->mNextEdgePtr->mSYMEdgePtr = GetPreviousEdge(edge->mSYMEdgePtr);

        edge = edge->mNextEdgePtr;
    } while (edge != face->mEdgePtr);

}

// Helper function to find 2 nearest joints
void MyGL::getNearestJoints(Vertex* tarVertex, std::vector<Joint*> &nearestJoints) {
    // Traverse the joints list to find the nearest 2 joints
    for (int i = 0; i < mJointList.size(); ++i) {
        Joint* curJoint = mJointList.at(i);
        if(nearestJoints.size() < 2) {
            nearestJoints.push_back(curJoint);
        } else {
           // Check distance to decide whether to change joint component
           // Distance for current joint
           Point4f jointPos = curJoint->getOverallTransformation() * Point4f(0, 0, 0, 1);
           float distance = glm::length(jointPos - tarVertex->mPos);
           // Distance for joint in list at 0
           Point4f joint0Pos = nearestJoints.at(0)->getOverallTransformation() * Point4f(0, 0, 0, 1);
           float distance0 = glm::length(joint0Pos - tarVertex->mPos);
           // Distance for joint in list at 1
           Point4f joint1Pos = nearestJoints.at(1)->getOverallTransformation() * Point4f(0, 0, 0, 1);
           float distance1 = glm::length(joint1Pos - tarVertex->mPos);

           // Find the nearest 2 joints to a given vertex
           if ((distance1 > distance0) && (distance1 > distance)) {
               nearestJoints.at(1) = curJoint;
           } else if ((distance0 > distance1) && (distance0 > distance)) {
               nearestJoints.at(0) = curJoint;
           }
        }
    }
}

// This part is for HW07 Joints
void MyGL::Skinning(Mesh &cubeMesh, Joint *root) {
    for (int i = 0; i < cubeMesh.mVerList.size(); ++i) {
        Vertex* tarVertex = cubeMesh.mVerList.at(i).get();
        std::vector<Joint*> nearestJoints;
        getNearestJoints(tarVertex, nearestJoints);
        // Then set influence for this two nearest joints
        Joint* j0 = nearestJoints.at(0);
        Joint* j1 = nearestJoints.at(1);
        JointInfluence j0Inf, j1Inf;
        // Using simple distance weight function
        Point4f j0Pos = j0->getOverallTransformation() * Point4f(0, 0, 0, 1);
        Point4f j1Pos = j1->getOverallTransformation() * Point4f(0, 0, 0, 1);

        // Calculate distance in world space
        float j0Dis = glm::length(j0Pos - tarVertex->mPos);
        float j1Dis = glm::length(j1Pos - tarVertex->mPos);
        float disSum = j0Dis + j1Dis;

        // Set up the weight distribution
        j0Inf.weight = 1 - (j0Dis / disSum);
        j0Inf.targetJoint = j0;
        tarVertex->mJointInfluences.push_back(j0Inf);

        j1Inf.weight = 1 - (j1Dis / disSum);
        j1Inf.targetJoint = j1;
        tarVertex->mJointInfluences.push_back(j1Inf);
    }

    // Add all the bindMats after skinning to a list
    for (int i = 0; i < mJointList.size(); ++i) {
        bindMats[i] = glm::inverse(mJointList.at(i)->getOverallTransformation());
        mJointList.at(i)->bindMat = bindMats[i];
        //std::cout << i << " check " << mJointList.at(i)->ID << std::endl;
    }
    std::cout << "Skinning Success!" << std::endl;
}


void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    // Set the size with which points should be rendered
    glPointSize(5);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.5, 0.5, 0.5, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instances of Cylinder and Sphere.
    //m_geomSquare.create();

    // Create the joint display
    joint = new Joint(this);
    joint->create();

    // Create the instance of a cube
    m_geoCube.MakeCube();
    m_geoCube.create();

    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
    // Create and set up the skeleton shader
    m_progSkeleton.create(":/glsl/skeleton.vert.glsl",":/glsl/skeleton.frag.glsl");
//    std::cout << m_progSkeleton.unifBindMats << " ! " << m_progSkeleton.unifTransMats << std::endl;

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);

    emit DisplayMesh(m_geoCube);
}

void MyGL::resizeGL(int w, int h)
{
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_glCamera = Camera(w, h);
    glm::mat4 viewproj = m_glCamera.getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    m_progLambert.setViewProjMatrix(viewproj);
    m_progFlat.setViewProjMatrix(viewproj);
    m_progSkeleton.setViewProjMatrix(viewproj);

    printGLErrorLog();
}

//This function is called by Qt any time your GL window is supposed to update
//For example, when the function update() is called, paintGL is called implicitly.
void MyGL::paintGL()
{
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_progFlat.setViewProjMatrix(m_glCamera.getViewProj());
    m_progLambert.setViewProjMatrix(m_glCamera.getViewProj());
    m_progLambert.setCamPos(m_glCamera.eye);
    m_progFlat.setModelMatrix(glm::mat4(1.f));

    //Create a model matrix. This one rotates the square by PI/4 radians then translates it by <-2,0,0>.
    //Note that we have to transpose the model matrix before passing it to the shader
    //This is because OpenGL expects column-major matrices, but you've
    //implemented row-major matrices.
    //    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-2,0,0)) * glm::rotate(glm::mat4(), 0.25f * 3.14159f, glm::vec3(0,1,0));
    //    //Send the geometry's transformation matrix to the shader
    //    m_progLambert.setModelMatrix(model);
    //    //Draw the example sphere using our lambert shader
    //    m_progLambert.draw(m_geomSquare);
    //    //Now do the same to render the cylinder
    //    //We've rotated it -45 degrees on the Z axis, then translated it to the point <2,2,0>
    //    model = glm::translate(glm::mat4(1.0f), glm::vec3(2,2,0)) * glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0,0,1));
    //    m_progLambert.setModelMatrix(model);
    //    m_progLambert.draw(m_geomSquare);

    // Creat a Cube
    glm::mat4 model = glm::translate(glm::mat4(), Vector3f(0, 0, 0))
            * glm::rotate(glm::mat4(), glm::radians(0.f), Vector3f(1, 0, 0))
            * glm::rotate(glm::mat4(), glm::radians(0.f), Vector3f(0, 1, 0))
            * glm::rotate(glm::mat4(), glm::radians(0.f), Vector3f(0, 0, 1))
            * glm::scale(glm::mat4(), Vector3f(1,1,1));

    m_progLambert.setModelMatrix(model);
//    m_progLambert.draw(m_geoCube);

    // Update the overall transformation matrix
    for (int i = 0; i < mJointList.size(); ++i) {
        transMats[i] = mJointList.at(i)->getOverallTransformation();
    }

    m_progSkeleton.setViewProjMatrix(m_glCamera.getViewProj());
    m_progSkeleton.setCamPos(m_glCamera.eye);
    m_progSkeleton.setModelMatrix(model);

    m_progSkeleton.setBindMatrix(bindMats);
//    std::cout << "BindMat?" << m_progSkeleton.unifBindMats << std::endl;

    m_progSkeleton.setTransformationMatrix(transMats);
//    std::cout << "TransMat?" << m_progSkeleton.unifTransMats << std::endl;

    if (m_geoCube.mJointIDs.size() != 0) {
        m_progSkeleton.draw(m_geoCube);
    } else {
        // Before skinning still use lambert
        m_progLambert.draw(m_geoCube);
    }

    //Creat the vertex, face and HalfEdge display
    glDisable(GL_DEPTH_TEST);
    // Creat the Vertex display
    m_vertexDisplay.create();
    if (m_vertexDisplay.representedVertex != nullptr) {
        m_progFlat.setModelMatrix(model);
        m_progFlat.draw(m_vertexDisplay);
    }

    // Creat the Edge display
    m_edgeDisplay.create();
    if (m_edgeDisplay.representedHalfEdge != nullptr) {
        m_progFlat.setModelMatrix(model);
        m_progFlat.draw(m_edgeDisplay);
    }

    // Creat the Face display
    m_faceDisplay.create();
    if (m_faceDisplay.representedFace != nullptr) {
        m_progFlat.setModelMatrix(model);
        m_progFlat.draw(m_faceDisplay);
    }

    if (joint != nullptr) {
        joint->drawSkeleton(*this, m_progFlat);
    }

    glEnable(GL_DEPTH_TEST);
}

void MyGL::JointUpdate() {
    for(int i = 0; i < mJointList.size(); ++i) {
        mJointList.at(i)->destroy();
        mJointList.at(i)->create();
    }
}

void MyGL::keyPressEvent(QKeyEvent *e)
{
    float amount = 2.0f;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 10.0f;
    }
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used
    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else if (e->key() == Qt::Key_Right) {
        m_glCamera.RotateAboutUp(-amount);
    } else if (e->key() == Qt::Key_Left) {
        m_glCamera.RotateAboutUp(amount);
    } else if (e->key() == Qt::Key_Up) {
        m_glCamera.RotateAboutRight(-amount);
    } else if (e->key() == Qt::Key_Down) {
        m_glCamera.RotateAboutRight(amount);
    } else if (e->key() == Qt::Key_1) {
        m_glCamera.fovy += amount;
    } else if (e->key() == Qt::Key_2) {
        m_glCamera.fovy -= amount;
    } else if (e->key() == Qt::Key_W) {
        m_glCamera.TranslateAlongLook(amount);
    } else if (e->key() == Qt::Key_S) {
        m_glCamera.TranslateAlongLook(-amount);
    } else if (e->key() == Qt::Key_D) {
        m_glCamera.TranslateAlongRight(amount);
    } else if (e->key() == Qt::Key_A) {
        m_glCamera.TranslateAlongRight(-amount);
    } else if (e->key() == Qt::Key_Q) {
        m_glCamera.TranslateAlongUp(-amount);
    } else if (e->key() == Qt::Key_E) {
        m_glCamera.TranslateAlongUp(amount);
    } else if (e->key() == Qt::Key_R) {
        m_glCamera = Camera(this->width(), this->height());
    }
    // Add some key press cases for Visual Debug
    else if (e->key() == Qt::Key_N) {
        // NEXT half-edge of the currently selected half-edge
        if (m_edgeDisplay.representedHalfEdge != nullptr) {
            m_edgeDisplay.representedHalfEdge = m_edgeDisplay.representedHalfEdge->mNextEdgePtr;
        }
    } else if (e->key() == Qt::Key_M) {
        // SYM half-edge of the currently selected half-edge
        if (m_edgeDisplay.representedHalfEdge != nullptr) {
            m_edgeDisplay.representedHalfEdge = m_edgeDisplay.representedHalfEdge->mSYMEdgePtr;
        }
    } else if (e->key() == Qt::Key_F) {
        // FACE of the currently selected half-edge
        if (m_edgeDisplay.representedHalfEdge != nullptr) {
            m_faceDisplay.representedFace = m_edgeDisplay.representedHalfEdge->mFacePtr;
        }
    } else if (e->key() == Qt::Key_V) {
        // VERTEX of the currently selected half-edge
        if (m_edgeDisplay.representedHalfEdge != nullptr) {
            m_vertexDisplay.representedVertex = m_edgeDisplay.representedHalfEdge->mVertexPtr;
            m_edgeDisplay.representedHalfEdge = nullptr;
        }
    } else if ((e->modifiers() == Qt::ShiftModifier) && (e->key() == Qt::Key_H)) {
        // HALF-EDGE of the currently selected face
        if (m_faceDisplay.representedFace != nullptr) {
            m_edgeDisplay.representedHalfEdge = m_faceDisplay.representedFace->mEdgePtr;
            m_faceDisplay.representedFace = nullptr;
        }
    } else if (e->key() == Qt::Key_H) {
        // HALF-EDGE of the currently selected vertex
        if (m_vertexDisplay.representedVertex != nullptr) {
            m_edgeDisplay.representedHalfEdge = m_vertexDisplay.representedVertex->mEdgePtr;
            m_vertexDisplay.representedVertex = nullptr;
        }
    }

    m_glCamera.RecomputeAttributes();
    update();  // Calls paintGL, among other things
}
