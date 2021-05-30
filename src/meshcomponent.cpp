#include "meshcomponent.h"
#include <iostream>
#include <fstream>
#include <sstream>

// Initialize all the static ID variables for tracking
unsigned int Vertex::mID = 0;
unsigned int Face::mID = 0;
unsigned int HalfEdge::mID = 0;

Vertex::Vertex() {
    this->setText(QString::number(mID++));
    IsSharp = false;
}

Face::Face() {
    this->setText(QString::number(mID++));
    IsSharp = false;
}

HalfEdge::HalfEdge() {
    this->setText(QString::number(mID++));
    IsSharp = false;
    this->mVertBeforePtr = nullptr;
}

Mesh::Mesh(OpenGLContext* context):
    Drawable(context),
    rng(),
    IsOBJ(false)
{}

bool IsBlack(Vector4f v) {
    bool flag = false;
    if (fequal(v.x, 0.f) && fequal(v.y, 0.f) && fequal(v.z, 0.f)) {
        flag = true;
    }
    return flag;
}

// Use cross product to get the normalized surface for certain 3 vertices
Normal4f GetVertexNormal(Point4f v1, Point4f v2, Point4f v3) {
    Vector3f e1 = Vector3f(v2 - v1);
    Vector3f e2 = Vector3f(v3 - v1);
    Normal3f nor = glm::normalize(glm::cross(e1, e2));
    return Normal4f(nor, 1.f);
}

// Clear all the exsiting mesh components
void Mesh::ClearAll() {
    mVerList.clear();
    mFaceList.clear();
    mEdgeList.clear();
    ClearOpenGLData();
    Face::mID = 0;
    Vertex::mID = 0;
    HalfEdge::mID = 0;
}

// The function to read in obj file data and tranform into mesh components
void Mesh::SetUpOBJMesh(const QString filepath) {
    // Most of the code follows the style in the "tiny_obj_loader"
    std::ifstream file(filepath.toStdString());
    // Check if we can open the file
    if (!file) {
        std::cout << "Can't open selected obj file!" << std::endl;
    }

    IsOBJ = true;
    // We ignore the uv coordinates for now
    std::string meshType;
    // The index of the different meshTypes
    // i for index of vertices
    // j for index of normal
    // k for index of faces
    int i = 0, j = 0, k = 0;
    // The value of the Vertex information
    float x, y, z;
    // To count the number of positions in a face
    int faceSize;

    // A list to save the input mesh Normal results
    std::vector<Normal4f> fileNors;

    for (std::string eachLine; std::getline(file, eachLine);) {
        std::stringstream tempLine;
        tempLine << eachLine;
        tempLine >> meshType;

        // First load the vertices
        if (meshType == "v") {
            uPtr<Vertex> vertexUPtr = mkU<Vertex>(Vertex());
            tempLine >> x >> y >> z;
            Vertex* vertexptr = vertexUPtr.get();
            vertexptr->mPos = Point4f(x, y, z, 1.f);

            mVerList.push_back(std::move(vertexUPtr));
            // std::cout << "Vertex "<< i <<" loading success!" << std::endl;
            i += 1;

            // Then Set up the surface Normal
        } else if (meshType == "vn") {
            tempLine >> x >> y >> z;
            fileNors.push_back(Normal4f(x, y, z, 0.f));
            // std::cout << "Normal "<< j << " loading success!" << std::endl;
            j += 1;

            // Then Set up the face
        } else if (meshType == "f") {
            // For face reading we have to seperate the input data properly
            uPtr<Face> faceUPtr = mkU<Face>(Face());
            std::vector<int> position;
            std::vector<std::string> segment;
            std::string segmentType;

            while (tempLine >> segmentType) {
                segment.push_back(segmentType);
            }

            for (int w = 0; w < segment.size(); ++w) {
                int beginIndex = segment[w].find('/');
                std::string posStr = segment[w].substr(0, beginIndex);

                int nextIndex = segment[w].rfind('/');
                std::string norStr = segment[w].substr(nextIndex + 1, segment[w].length() - beginIndex - 1);

                position.push_back(std::stoi(posStr.c_str()));
                mNor.push_back(fileNors.at(std::stoi(norStr) -1));
            }

            faceSize = position.size();
            Face *facePtr = faceUPtr.get();
            facePtr->mColor = Color4f(rng.nextFloat(), rng.nextFloat(), rng.nextFloat() ,1.f);

            // Add these components to mesh lists
            mFaceList.push_back(std::move(faceUPtr));

            // Initialize some edges
            for (int w = 0; w < position.size(); ++w) {
                uPtr<HalfEdge> edgeUPtr = mkU<HalfEdge>(HalfEdge());
                mEdgeList.push_back(std::move(edgeUPtr));
            }

            // Link the edges' attributes
            for (int w = 0; w < position.size(); ++w) {
                mEdgeList.at(k * position.size() + w)->mFacePtr = mFaceList.at(k).get();
                mEdgeList.at(k * position.size() + w)->mVertexPtr = mVerList.at(position.at(w) - 1).get();
                mVerList.at(position.at(w) - 1)->mEdgePtr = mEdgeList.at(k * position.size() + w).get();
            }
            mFaceList.at(k)->mEdgePtr = mEdgeList.at(k * position.size() + 0).get();

            // Link the next edges
            for (int w = 0; w < position.size(); ++ w) {
                // Deal with the last edge
                if (w == position.size() - 1) {
                    mEdgeList.at(k * position.size() + w)->mNextEdgePtr = mEdgeList.at(k * position.size()).get();
                } else {
                    mEdgeList.at(k * position.size() + w)->mNextEdgePtr = mEdgeList.at(k * position.size() + w + 1).get();
                }
            }

            // By adding a ptr instead of calling find previous edge function
            for (int w = 0; w < position.size(); ++w) {
                if (w == 0) { // Deal with head
                    mEdgeList.at(k * position.size() + w)->mVertBeforePtr = mVerList.at(position.at(position.size() - 1) - 1).get();
                } else {
                    mEdgeList.at(k * position.size() + w)->mVertBeforePtr = mVerList.at(position.at(w - 1) - 1).get();
                }
            }
            k += 1;
        }
    }

    // Use a unordered_map to find the symedges
    std::unordered_map<std::pair<int, int>, HalfEdge*, hash_pair> edgeMap;
    for (std::vector<uPtr<HalfEdge>>::iterator edgeIt = mEdgeList.begin();
         edgeIt != mEdgeList.end(); ++edgeIt) {
        int a = edgeIt->get()->mVertexPtr->text().toInt();
        int b = edgeIt->get()->mVertBeforePtr->text().toInt();
        std::pair<int, int> edgeKey(b,a);
        auto e = edgeMap.find(edgeKey);
        if (e != edgeMap.end()) {
            edgeIt->get()->mSYMEdgePtr = e->second;
            e->second->mSYMEdgePtr = edgeIt->get();
        } else {
            edgeMap.emplace(std::make_pair(std::pair<int, int>(a, b), edgeIt->get()));
        }
    }
}

void Mesh::ClearOpenGLData() {
    // First clean them for Update
    mIndex.clear();
    mPos.clear();
    mNor.clear();
    mCol.clear();
    mJointIDs.clear();
    mJointWeights.clear();
}

void Mesh::SetOpenGLData() {
    // set up the pos, nor and col information
    // Add these vertics as spheres with radius = 0.01f
    Transform cubeTrans = Transform(Vector3f(0, 0, 0), Vector3f(0, 0, 0), Vector3f(1, 1, 1));
    for (int i = 0; i < mVerList.size(); ++i) {
        mVerList.at(i)->sphereVert = mkU<Sphere>(Sphere(Point3f(mVerList.at(i)->mPos)));
        mVerList.at(i)->sphereVert->transform = cubeTrans;
    }

    // Add these edges as capsules with radius = 0.05f
    for (int i = 0; i < mEdgeList.size(); ++i) {
        mEdgeList.at(i)->capsuleEdge = mkU<Capsule>(Capsule(Point3f(mEdgeList.at(i)->mVertexPtr->mPos) - Point3f(0.05f),
                                                            Point3f(mEdgeList.at(i)->mSYMEdgePtr->mVertexPtr->mPos)- Point3f(0.05f)));
        mEdgeList.at(i)->capsuleEdge->transform = cubeTrans;
    }

    // Add these faces as triangles
    for (int i = 0; i < mFaceList.size(); ++i) {
        mFaceList.at(i)->triangleFace = mkU<TriangleFace>(TriangleFace(Point3f(mFaceList.at(i)->mEdgePtr->mVertexPtr->mPos),
                                                                       Point3f(mFaceList.at(i)->mEdgePtr->mNextEdgePtr->mVertexPtr->mPos),
                                                                       Point3f(mFaceList.at(i)->mEdgePtr->mSYMEdgePtr->mVertexPtr->mPos)));
        mFaceList.at(i)->triangleFace->transform = cubeTrans;
    }


    // Use iterator to traverse the Face List
    for (std::vector<uPtr<Face>>::iterator faceIt = mFaceList.begin();
         faceIt != mFaceList.end(); ++faceIt) {

        Face *curFace = faceIt->get();

        // count records the number of HalfEdges in this face
        GLuint edgeCount = 0;
        // The index of first vertex of this face in the mPos List
        GLuint vertListIndex = mPos.size();

        HalfEdge *startEdge = curFace->mEdgePtr;
        // Push the vertex position, color and normal into corresponding list
        mPos.push_back(startEdge->mVertexPtr->mPos);
        // If the skinning is done
        if (startEdge->mVertexPtr->mJointInfluences.size() != 0) {
            mJointIDs.push_back(glm::ivec2(startEdge->mVertexPtr->mJointInfluences.at(0).targetJoint->ID,
                                           startEdge->mVertexPtr->mJointInfluences.at(1).targetJoint->ID));
            mJointWeights.push_back(glm::vec2(startEdge->mVertexPtr->mJointInfluences.at(0).weight,
                                              startEdge->mVertexPtr->mJointInfluences.at(1).weight));
        }

        mCol.push_back(curFace->mColor);
        // If it's obj file, we don't have to calculate normal
        if (!IsOBJ) {
            Normal4f vertNor = GetVertexNormal(startEdge->mVertexPtr->mPos,
                                               startEdge->mNextEdgePtr->mVertexPtr->mPos,
                                               startEdge->mNextEdgePtr->mNextEdgePtr->mVertexPtr->mPos);

            mNor.push_back(vertNor);
        }

        ++edgeCount;
        // Use a do-while loop to iterate over all the half edges
        HalfEdge *nextEdge = startEdge->mNextEdgePtr;
        do {
            // Push the vertex position, color and normal into corresponding list
            mPos.push_back(nextEdge->mVertexPtr->mPos);
            // If the skinning is done
            if (nextEdge->mVertexPtr->mJointInfluences.size() != 0) {
                mJointIDs.push_back(glm::ivec2(nextEdge->mVertexPtr->mJointInfluences.at(0).targetJoint->ID,
                                               nextEdge->mVertexPtr->mJointInfluences.at(1).targetJoint->ID));
                mJointWeights.push_back(glm::vec2(nextEdge->mVertexPtr->mJointInfluences.at(0).weight,
                                                  nextEdge->mVertexPtr->mJointInfluences.at(1).weight));
            }

            mCol.push_back(curFace->mColor);
            if (!IsOBJ) {
                Normal4f vertNor = GetVertexNormal(nextEdge->mVertexPtr->mPos,
                                                   nextEdge->mNextEdgePtr->mVertexPtr->mPos,
                                                   nextEdge->mNextEdgePtr->mNextEdgePtr->mVertexPtr->mPos);

                mNor.push_back(vertNor);
            }
            nextEdge = nextEdge->mNextEdgePtr;
            ++edgeCount;
        } while (nextEdge != startEdge);

        // Push back all the triangle indices in this face
        for (GLuint i = 0; i < edgeCount - 2; ++i) {
            mIndex.push_back(vertListIndex);
            mIndex.push_back(vertListIndex + i + 1);
            mIndex.push_back(vertListIndex + i + 2);
        }
    }
    IsOBJ = false;
}

void Mesh::create() {
    SetOpenGLData();

    // The number of indices for the triangle mesh
    count = mIndex.size();

//    std::cout << "IDs number: " <<mJointIDs.size() << std::endl;
//    std::cout << "Weights number: "<< mJointWeights.size() << std::endl;

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndex.size() * sizeof(GLuint), mIndex.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, mPos.size() * sizeof(glm::vec4), mPos.data(), GL_STATIC_DRAW);
//    std::cout <<"Position number:"<< mPos.size() << std::endl;

    generateNor();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufNor);
    mp_context->glBufferData(GL_ARRAY_BUFFER, mNor.size() * sizeof(glm::vec4), mNor.data(), GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, mCol.size() * sizeof(glm::vec4), mCol.data(), GL_STATIC_DRAW);

    generateJointIDs();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufJointIDs);
    mp_context->glBufferData(GL_ARRAY_BUFFER, mJointIDs.size() * sizeof(glm::ivec2), mJointIDs.data(), GL_STATIC_DRAW);

    generateJointWeights();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufJointWeights);
    mp_context->glBufferData(GL_ARRAY_BUFFER, mJointWeights.size() * sizeof(glm::vec2), mJointWeights.data(), GL_STATIC_DRAW);
}

void Mesh::MakeCube(){
    // For testing Cube, we have 8 Vertices, 6 Faces and 24 HalfEdges
    std::vector<uPtr<Vertex>> vertList;
    for (int i = 0; i < 8; ++i) {
        vertList.push_back(mkU<Vertex>(Vertex()));
    }

    std::vector<uPtr<Face>> faceList;
    for (int i = 0; i < 6; ++i) {
        faceList.push_back(mkU<Face>(Face()));
    }

    std::vector<uPtr<HalfEdge>> edgeList;
    for (int i = 0; i < 24; ++i) {
        edgeList.push_back(mkU<HalfEdge>(HalfEdge()));
    }

    // set up vertex components
    vertList.at(0)->mPos = Point4f(0.5, -0.5, 0.5, 1);  vertList.at(1)->mPos = Point4f(0.5, 0.5, 0.5, 1);
    vertList.at(2)->mPos = Point4f(-0.5, 0.5, 0.5, 1);  vertList.at(3)->mPos = Point4f(-0.5, -0.5, 0.5, 1);
    vertList.at(4)->mPos = Point4f(0.5, -0.5, -0.5, 1); vertList.at(5)->mPos = Point4f(0.5, 0.5, -0.5, 1);
    vertList.at(6)->mPos = Point4f(-0.5, 0.5, -0.5, 1); vertList.at(7)->mPos = Point4f(-0.5, -0.5, -0.5, 1);

    vertList.at(0)->mEdgePtr = edgeList.at(3).get(); vertList.at(1)->mEdgePtr = edgeList.at(0).get();
    vertList.at(2)->mEdgePtr = edgeList.at(1).get(); vertList.at(3)->mEdgePtr = edgeList.at(2).get();
    vertList.at(4)->mEdgePtr = edgeList.at(6).get(); vertList.at(5)->mEdgePtr = edgeList.at(5).get();
    vertList.at(6)->mEdgePtr = edgeList.at(4).get(); vertList.at(7)->mEdgePtr = edgeList.at(7).get();

    // Link the faces, vertices and half edges
    // front
    edgeList.at(0)->mNextEdgePtr = edgeList.at(1).get(); edgeList.at(1)->mNextEdgePtr = edgeList.at(2).get();
    edgeList.at(2)->mNextEdgePtr = edgeList.at(3).get(); edgeList.at(3)->mNextEdgePtr = edgeList.at(0).get();
    edgeList.at(0)->mFacePtr = faceList.at(0).get(); edgeList.at(1)->mFacePtr = faceList.at(0).get();
    edgeList.at(2)->mFacePtr = faceList.at(0).get(); edgeList.at(3)->mFacePtr = faceList.at(0).get();
    edgeList.at(0)->mSYMEdgePtr = edgeList.at(14).get(); edgeList.at(1)->mSYMEdgePtr = edgeList.at(19).get();
    edgeList.at(2)->mSYMEdgePtr = edgeList.at(8).get(); edgeList.at(3)->mSYMEdgePtr = edgeList.at(23).get();
    edgeList.at(0)->mVertexPtr = vertList.at(1).get(); edgeList.at(1)->mVertexPtr = vertList.at(2).get();
    edgeList.at(2)->mVertexPtr = vertList.at(3).get(); edgeList.at(3)->mVertexPtr = vertList.at(0).get();

    // back
    edgeList.at(4)->mNextEdgePtr = edgeList.at(5).get(); edgeList.at(5)->mNextEdgePtr = edgeList.at(6).get();
    edgeList.at(6)->mNextEdgePtr = edgeList.at(7).get(); edgeList.at(7)->mNextEdgePtr = edgeList.at(4).get();
    edgeList.at(4)->mFacePtr = faceList.at(1).get(); edgeList.at(5)->mFacePtr = faceList.at(1).get();
    edgeList.at(6)->mFacePtr = faceList.at(1).get(); edgeList.at(7)->mFacePtr = faceList.at(1).get();
    edgeList.at(4)->mSYMEdgePtr = edgeList.at(10).get(); edgeList.at(5)->mSYMEdgePtr = edgeList.at(17).get();
    edgeList.at(6)->mSYMEdgePtr = edgeList.at(12).get(); edgeList.at(7)->mSYMEdgePtr = edgeList.at(21).get();
    edgeList.at(4)->mVertexPtr = vertList.at(6).get(); edgeList.at(5)->mVertexPtr = vertList.at(5).get();
    edgeList.at(6)->mVertexPtr = vertList.at(4).get(); edgeList.at(7)->mVertexPtr = vertList.at(7).get();

    // left
    edgeList.at(8)->mNextEdgePtr = edgeList.at(9).get(); edgeList.at(9)->mNextEdgePtr = edgeList.at(10).get();
    edgeList.at(10)->mNextEdgePtr = edgeList.at(11).get(); edgeList.at(11)->mNextEdgePtr = edgeList.at(8).get();
    edgeList.at(8)->mFacePtr = faceList.at(2).get(); edgeList.at(9)->mFacePtr = faceList.at(2).get();
    edgeList.at(10)->mFacePtr = faceList.at(2).get(); edgeList.at(11)->mFacePtr = faceList.at(2).get();
    edgeList.at(8)->mSYMEdgePtr = edgeList.at(2).get(); edgeList.at(9)->mSYMEdgePtr = edgeList.at(18).get();
    edgeList.at(10)->mSYMEdgePtr = edgeList.at(4).get(); edgeList.at(11)->mSYMEdgePtr = edgeList.at(20).get();
    edgeList.at(8)->mVertexPtr = vertList.at(2).get(); edgeList.at(9)->mVertexPtr = vertList.at(6).get();
    edgeList.at(10)->mVertexPtr = vertList.at(7).get(); edgeList.at(11)->mVertexPtr = vertList.at(3).get();

    // right
    edgeList.at(12)->mNextEdgePtr = edgeList.at(13).get(); edgeList.at(13)->mNextEdgePtr = edgeList.at(14).get();
    edgeList.at(14)->mNextEdgePtr = edgeList.at(15).get(); edgeList.at(15)->mNextEdgePtr = edgeList.at(12).get();
    edgeList.at(12)->mFacePtr = faceList.at(3).get(); edgeList.at(13)->mFacePtr = faceList.at(3).get();
    edgeList.at(14)->mFacePtr = faceList.at(3).get(); edgeList.at(15)->mFacePtr = faceList.at(3).get();
    edgeList.at(12)->mSYMEdgePtr = edgeList.at(6).get(); edgeList.at(13)->mSYMEdgePtr = edgeList.at(16).get();
    edgeList.at(14)->mSYMEdgePtr = edgeList.at(0).get(); edgeList.at(15)->mSYMEdgePtr = edgeList.at(22).get();
    edgeList.at(12)->mVertexPtr = vertList.at(5).get(); edgeList.at(13)->mVertexPtr = vertList.at(1).get();
    edgeList.at(14)->mVertexPtr = vertList.at(0).get(); edgeList.at(15)->mVertexPtr = vertList.at(4).get();

    // top
    edgeList.at(16)->mNextEdgePtr = edgeList.at(17).get(); edgeList.at(17)->mNextEdgePtr = edgeList.at(18).get();
    edgeList.at(18)->mNextEdgePtr = edgeList.at(19).get(); edgeList.at(19)->mNextEdgePtr = edgeList.at(16).get();
    edgeList.at(16)->mFacePtr = faceList.at(4).get(); edgeList.at(17)->mFacePtr = faceList.at(4).get();
    edgeList.at(18)->mFacePtr = faceList.at(4).get(); edgeList.at(19)->mFacePtr = faceList.at(4).get();
    edgeList.at(16)->mSYMEdgePtr = edgeList.at(13).get(); edgeList.at(17)->mSYMEdgePtr = edgeList.at(5).get();
    edgeList.at(18)->mSYMEdgePtr = edgeList.at(9).get(); edgeList.at(19)->mSYMEdgePtr = edgeList.at(1).get();
    edgeList.at(16)->mVertexPtr = vertList.at(5).get(); edgeList.at(17)->mVertexPtr = vertList.at(6).get();
    edgeList.at(18)->mVertexPtr = vertList.at(2).get(); edgeList.at(19)->mVertexPtr = vertList.at(1).get();

    //bottom
    edgeList.at(20)->mNextEdgePtr = edgeList.at(21).get(); edgeList.at(21)->mNextEdgePtr = edgeList.at(22).get();
    edgeList.at(22)->mNextEdgePtr = edgeList.at(23).get(); edgeList.at(23)->mNextEdgePtr = edgeList.at(20).get();
    edgeList.at(20)->mFacePtr = faceList.at(5).get(); edgeList.at(21)->mFacePtr = faceList.at(5).get();
    edgeList.at(22)->mFacePtr = faceList.at(5).get(); edgeList.at(23)->mFacePtr = faceList.at(5).get();
    edgeList.at(20)->mSYMEdgePtr = edgeList.at(11).get(); edgeList.at(21)->mSYMEdgePtr = edgeList.at(7).get();
    edgeList.at(22)->mSYMEdgePtr = edgeList.at(15).get(); edgeList.at(23)->mSYMEdgePtr = edgeList.at(3).get();
    edgeList.at(20)->mVertexPtr = vertList.at(7).get(); edgeList.at(21)->mVertexPtr = vertList.at(4).get();
    edgeList.at(22)->mVertexPtr = vertList.at(0).get(); edgeList.at(23)->mVertexPtr = vertList.at(3).get();

    // set up Face components
    faceList.at(0)->mEdgePtr = edgeList.at(0).get();
    faceList.at(0)->mColor = Color4f(1, 0, 0, 0);

    faceList.at(1)->mEdgePtr = edgeList.at(4).get();
    faceList.at(1)->mColor = Color4f(0, 1, 0, 0);

    faceList.at(2)->mEdgePtr = edgeList.at(8).get();
    faceList.at(2)->mColor = Color4f(0, 0, 1, 0);

    faceList.at(3)->mEdgePtr = edgeList.at(12).get();
    faceList.at(3)->mColor = Color4f(1, 1, 0, 0);

    faceList.at(4)->mEdgePtr = edgeList.at(16).get();
    faceList.at(4)->mColor = Color4f(0, 1, 1, 0);

    faceList.at(5)->mEdgePtr = edgeList.at(20).get();
    faceList.at(5)->mColor = Color4f(1, 0, 1, 0);

    // move the temp lists to class field
    for(int i = 0; i < 8; ++i) {
        mVerList.push_back(std::move(vertList.at(i)));
    }

    for(int i = 0; i < 6; ++i) {
        mFaceList.push_back(std::move(faceList.at(i)));
    }

    for(int i = 0; i < 24; ++i) {
        mEdgeList.push_back(std::move(edgeList.at(i)));
    }

}

// Begin of Catmull Clark Subdivision
void Mesh::CopyOriginalVertexList(std::vector<Vertex*> &originalVertex) {
    for (int i = 0; i < mVerList.size(); ++i) {
        // Make a copy of original vertex list
        Vertex *temp = mVerList.at(i).get();
        originalVertex.push_back(temp);
    }
}

// 1.Find the centroid of a face
void Mesh::AddCentroidToFace(std::vector<Vertex *> &centroidVertex, Face *curFace) {
    // For each face we need to average over all the vertices in that face
    // Find the sum of all vertices
    int countVert = 0;
    Point4f sumVert(0.f);

    HalfEdge* nextEdge = curFace->mEdgePtr;
    do {
        sumVert += nextEdge->mVertexPtr->mPos;
        nextEdge = nextEdge->mNextEdgePtr;
        ++countVert;
    } while (nextEdge != curFace->mEdgePtr);

    // Get the averaged Centroid Point
    Point4f centerPos = Point4f(Point3f(sumVert / (float)countVert), 1.f);

    // Creat a centroid Point and give this position
    uPtr<Vertex> centroid = mkU<Vertex>(Vertex());
    centroid->mPos = centerPos;

    // Add this center point to two tracking List
    centroidVertex.push_back(centroid.get());
    mVerList.push_back(std::move(centroid));
}

void Mesh::AddCentroid(std::vector<Vertex*> &centroidVertex) {
    for (std::vector<uPtr<Face>>::iterator faceIt = mFaceList.begin();
         faceIt != mFaceList.end(); ++faceIt) {
        AddCentroidToFace(centroidVertex, faceIt->get());
    }
}

void Mesh::SplitEdgeWithMid(Point4f midpos, HalfEdge* originalEdge, std::vector<Vertex*> &midPoints) {
    //Similar to the function in HW6
    HalfEdge* edge1 = originalEdge;
    HalfEdge* edge1s = edge1->mSYMEdgePtr;

    // Get the two endpoints of this edge
    Vertex* v1 = edge1->mVertexPtr;
    Vertex* v2 = edge1s->mVertexPtr;
    // Prepare the mid Vertex
    uPtr<Vertex> midV = mkU<Vertex>(Vertex());

    // TODO: Do we need to set this point sharp?
    midV.get()->mPos = midpos;
    midPoints.push_back(midV.get());

    // Create the splited half edge and link them properly
    uPtr<HalfEdge> edge2Ptr = mkU<HalfEdge>(HalfEdge());
    uPtr<HalfEdge> edge2sPtr = mkU<HalfEdge>(HalfEdge());

    // The resulting sub-edges are also marked as "sharp" so that
    // further subdivisions retain a sharp appearance.
    if (originalEdge->IsSharp) {
        edge2Ptr->IsSharp = true;
        edge2sPtr->IsSharp = true;
    }
    HalfEdge* edge2 = edge2Ptr.get();
    HalfEdge* edge2s = edge2sPtr.get();

    edge2->mVertexPtr = v1;
    edge2s->mVertexPtr = v2;

    edge2->mFacePtr = edge1->mFacePtr;
    edge2s->mFacePtr = edge1s->mFacePtr;

    edge2->mNextEdgePtr = edge1->mNextEdgePtr;
    edge2s->mNextEdgePtr = edge1s->mNextEdgePtr;
    edge1->mNextEdgePtr = edge2;
    edge1s->mNextEdgePtr = edge2s;
    edge1->mVertexPtr = edge1s->mVertexPtr = midV.get();

    edge1->mSYMEdgePtr = edge2s;
    edge2s->mSYMEdgePtr = edge1;
    edge2->mSYMEdgePtr = edge1s;
    edge1s->mSYMEdgePtr = edge2;

    // Properly set the vertex v1 and v2
    v1->mEdgePtr = edge2;
    v2->mEdgePtr = edge2s;

    // Add these new connections back to the components list
    mVerList.push_back(std::move(midV));
    mEdgeList.push_back(std::move(edge2Ptr));
    mEdgeList.push_back(std::move(edge2sPtr));
}

// Helper function to decide where to split or not
bool DoSplit(std::vector<HalfEdge*> &splitedEdges, HalfEdge *checkEdge){
    for (std::vector<HalfEdge*>::iterator splitEdgeIt = splitedEdges.begin();
         splitEdgeIt != splitedEdges.end(); ++splitEdgeIt) {
        if (*splitEdgeIt == checkEdge) {
            return false;
        }
    }
    return true;
}

// Helper function to construct center vertex list
void ConstructCenterList(std::vector<Vector4f> &centerPos, std::vector<Vertex*> &centerVert, HalfEdge* edge) {
    Face* face1 = edge->mFacePtr;
    Face* face2 = edge->mSYMEdgePtr->mFacePtr;

    // If this face has no neighbor
    if (face2 == nullptr) {
        centerPos.push_back(centerVert.at(face1->text().toInt())->mPos);
    } else {
        centerPos.push_back(centerVert.at(face1->text().toInt())->mPos);
        centerPos.push_back(centerVert.at(face2->text().toInt())->mPos);
    }
}

// Helper function to find the mid point of each edge
Point4f GetMidpointPos(std::vector<Point4f> &originalPos, std::vector<Point4f> &centriodPos){
    int countPos = 0;
    Point4f sumPos(0.f);

    // Iterate over both point's position list
    for (std::vector<Point4f>::iterator oriIt = originalPos.begin(); oriIt != originalPos.end(); ++ oriIt) {
        sumPos += *oriIt;
        ++countPos;
    }

    for (std::vector<Point4f>::iterator cenIt = centriodPos.begin(); cenIt != centriodPos.end(); ++cenIt) {
        sumPos += *cenIt;
        ++countPos;
    }

    return Point4f(Point3f(sumPos / (float)countPos), 1.f);
}

// 2. Compute the smoothed midpoint of each edge in the mesh
// Deal with mid points of half edges, should only split original full edges
void Mesh::AddMidPoint(std::vector<Vertex*> &centroidVertex, std::vector<Vertex*> &midVertex) {
    // Use a list to store original full edges before split
    std::vector<HalfEdge*> fullEdges;
    for (std::vector<uPtr<HalfEdge>>::iterator edgeIt = mEdgeList.begin();
         edgeIt != mEdgeList.end(); ++edgeIt) {
        fullEdges.push_back(edgeIt->get());
    }

    // For each edge, only split it not in the splitEdge list
    std::vector<HalfEdge*> splitedEdges;
    for (std::vector<HalfEdge*>::iterator edgeIt = fullEdges.begin();
         edgeIt != fullEdges.end(); ++edgeIt) {
        if (DoSplit(splitedEdges, *edgeIt)) {
            // Put this edge and its symmetry in the splitededge
            // to avoid repeatition
            splitedEdges.push_back(*edgeIt);
            HalfEdge *temp = *edgeIt;
            splitedEdges.push_back(temp->mSYMEdgePtr);

            std::vector<Point4f> originalPos;
            std::vector<Point4f> centroidPos;
            ConstructCenterList(centroidPos, centroidVertex, *edgeIt);
            originalPos.push_back(temp->mVertexPtr->mPos);
            originalPos.push_back(temp->mSYMEdgePtr->mVertexPtr->mPos);
            // Find the expected mid point with a helper function
            Point4f midPos = GetMidpointPos(originalPos, centroidPos);
            // If this edge is sharp, then the mid point is just the average of two end points
            if (temp->IsSharp) {
                midPos = (temp->mVertexPtr->mPos + temp->mSYMEdgePtr->mVertexPtr->mPos) / 2.f;
            }
            // Split edge with given mid point position
            SplitEdgeWithMid(midPos, *edgeIt, midVertex);
        }
    }
}

// Helper function to get the sum over given vertex list
inline Point4f GetSumPosition(const std::vector<Vertex*> &vertList) {
    Point4f sumPos(0.f);
    for (int i = 0; i < vertList.size(); ++i) {
        sumPos += vertList.at(i)->mPos;
    }
    return sumPos;
}

// A helper function to find the previous edge for current edge
HalfEdge *GetPreviousEdge(const HalfEdge* curEdge) {
    // Since there are at least 3 edges in a face
    HalfEdge* nextEdge = curEdge->mNextEdgePtr;
    do {
        nextEdge = nextEdge->mNextEdgePtr;
    } while (nextEdge->mNextEdgePtr != curEdge);

    return nextEdge;
}

// Helper function to find adjacent Face
void FindAdjacentFace(const Vertex *targetVertex, std::vector<Face*> &adjacentFaces) {
    HalfEdge *edge = targetVertex->mEdgePtr;
    // use a do while loop to find all the adjacent faces for given vertex
    do{
        adjacentFaces.push_back(edge->mFacePtr);
        HalfEdge* symEdge = edge->mSYMEdgePtr;
        edge = GetPreviousEdge(symEdge);
    } while (edge != targetVertex->mEdgePtr);
}

// Helper function to find adjacent centroid point
void FindAdjacentCentroid(const Vertex *targetVertex, const std::vector<Vertex*> &centroids,
                          std::vector<Vertex*> &adjacentCentroids, std::vector<Face*> &adjacentFaces) {
    FindAdjacentFace(targetVertex, adjacentFaces);
    for (int i = 0; i < adjacentFaces.size(); ++i) {
        int FaceId = adjacentFaces.at(i)->text().toInt();
        adjacentCentroids.push_back(centroids.at(FaceId));
    }
}

// Helper function to find adjacent mid points
void FindAdjacentMidPoint(const Vertex *targetVertex, std::vector<Vertex*> &adjacentMidPoints,
                          const std::vector<Face*> &adjacentFaces) {
    for (int i = 0; i < adjacentFaces.size(); ++i) {
        Face* f = adjacentFaces.at(i);
        // Find which edge in this face points to this vertex
        HalfEdge* curEdge =f->mEdgePtr;
        while (curEdge->mVertexPtr != targetVertex) {
            curEdge = curEdge->mNextEdgePtr;
        }
        HalfEdge *preEdge = GetPreviousEdge(curEdge);
        adjacentMidPoints.push_back(preEdge->mVertexPtr);
    }
}

// 3. Smooth the original vertices
void Mesh::MoveOriginalVertex(std::vector<Vertex*> centroidVertex, std::vector<Vertex*>midVertex,
                              std::vector<Vertex*> originalVertex) {
    for (int i = 0; i < originalVertex.size(); ++i) {
        // Find the number of sharp edges this vertex connects to
        int numSharpEdge = 0;
        Point4f sumSharpEdgePos(0.f);
        HalfEdge* testEdge = originalVertex.at(i)->mEdgePtr;
        do {
            if (testEdge->IsSharp) {
                // Add the end point's position of a sharp edge
                sumSharpEdgePos += testEdge->mSYMEdgePtr->mVertexPtr->mPos;
                ++numSharpEdge;
            }
            // Go to Next edge points to this vertex
            testEdge = GetPreviousEdge(testEdge->mSYMEdgePtr);
        } while (testEdge != originalVertex.at(i)->mEdgePtr);

        // Three conditions
        // targetVertex is sharp: don't move
        // non-sharp targetVertex is connected to one sharp edge: orginal smooth
        // non-sharp targetVertex is connected to two sharp edge: new smooth function
        // non-sharp targetVertex is connecteg to three sharp edge: as sharp

        if (!originalVertex.at(i)->IsSharp && numSharpEdge < 2) {
            std::vector<Vertex*> adjacentMidPoints;
            std::vector<Vertex*> adjacentCentroids;
            std::vector<Face*> adjacentFaces;

            Vertex* targetVertex = originalVertex.at(i);
            FindAdjacentCentroid(targetVertex, centroidVertex, adjacentCentroids, adjacentFaces);
            FindAdjacentMidPoint(targetVertex, adjacentMidPoints, adjacentFaces);

            int numAdjacentMid = adjacentMidPoints.size();
            Point4f sumMidPoints = GetSumPosition(adjacentMidPoints);
            Point4f sumCentriods = GetSumPosition(adjacentCentroids);

            // Apply the smooth algorithm from ppt
            Point4f smoothPos = Point4f ((float)(numAdjacentMid - 2) * targetVertex->mPos / (float) numAdjacentMid +
                                         sumMidPoints / (float)(numAdjacentMid * numAdjacentMid) +
                                         sumCentriods / (float)(numAdjacentMid * numAdjacentMid));
            // Update the smoothed position
            targetVertex->mPos = smoothPos;

        } else if (!originalVertex.at(i)->IsSharp && numSharpEdge == 2 ) {
            Vertex* targetVertex = originalVertex.at(i);
            Point4f smoothPos = sumSharpEdgePos * 0.125f + 0.75f * targetVertex->mPos;
            targetVertex->mPos = smoothPos;
        }
    }
}

// Helper function to set up the symmetry relationship
void SetSYM(HalfEdge *tempEdge, HalfEdge *midEgde) {
    HalfEdge *tempEdgePre = GetPreviousEdge(tempEdge);
    HalfEdge *nextMidEgde = midEgde->mNextEdgePtr;
    nextMidEgde->mSYMEdgePtr = tempEdgePre;
    tempEdgePre->mSYMEdgePtr = nextMidEgde;
}

inline int GetVertexNumber(Face* f) {
    int countVert = 0;
    HalfEdge* curEdge = f->mEdgePtr;
    do {
        ++countVert;
        curEdge = curEdge->mNextEdgePtr;
    } while (curEdge != f->mEdgePtr);
    return countVert / 2;
}

// Helper function to construct all the subFaces' half edge and linking property
void Mesh::ConstructSubFace(Face *newFace, HalfEdge *midEdge, HalfEdge* midEdgePre, Vertex* centroid){
    uPtr<HalfEdge> HEAptr = mkU<HalfEdge>(HalfEdge());
    uPtr<HalfEdge> HEBptr = mkU<HalfEdge>(HalfEdge());

    HalfEdge* HEA = HEAptr.get();
    HalfEdge* HEB = HEBptr.get();
    Vertex* MidVertex = midEdgePre->mSYMEdgePtr->mVertexPtr;

    // Relink the Half Edges
    midEdge->mNextEdgePtr = HEA;
    HEA->mNextEdgePtr = HEB;
    HEB->mNextEdgePtr = midEdgePre;

    // Assign Vertex to edges
    HEA->mVertexPtr = centroid;
    centroid->mEdgePtr = HEA;
    HEB->mVertexPtr = MidVertex;

    // Assign Face to edges
    HEA->mFacePtr = newFace;
    HEB->mFacePtr = newFace;
    midEdge->mFacePtr = newFace;
    midEdgePre->mFacePtr = newFace;
    newFace->mEdgePtr = midEdge;

    mEdgeList.push_back(std::move(HEAptr));
    mEdgeList.push_back(std::move(HEBptr));
}

// Creat Faces and Halfedges and link them properly
void Mesh::QuadrangleFace(Face *f, Vertex *centroid, std::vector<HalfEdge*> midEdges) {
    // Make a copy of input midEdges
    std::vector<HalfEdge*> tempEdges;
    for(int i = 0; i < midEdges.size(); ++i) {
        tempEdges.push_back(midEdges.at(i)->mNextEdgePtr);
    }

    std::vector<Face*> newFaces;
    int numOrigin = GetVertexNumber(f);
    newFaces.push_back(f);
    for (int i = 0; i < numOrigin - 1; ++i) {
        uPtr<Face> facePtr = mkU<Face>(Face());
        facePtr->mColor = f->mColor;
        newFaces.push_back(facePtr.get());
        mFaceList.push_back(std::move(facePtr));
    }

    for (int i = 0; i < midEdges.size(); ++i) {
        HalfEdge *tempEdgePre;
        if (i == 0) {
            tempEdgePre = tempEdges.back();
        } else {
            tempEdgePre = tempEdges.at(i - 1);
        }
        ConstructSubFace(newFaces.at(i), midEdges.at(i), tempEdgePre, centroid);
    }

    for (int i = 0; i < midEdges.size(); ++i) {
        SetSYM(tempEdges.at(i), midEdges.at(i));
    }
}

void Mesh::QuadrangleMesh(std::vector<Vertex*> centroidVertex, std::vector<Vertex*> midVertex) {
    // Make a copy of original face list
    std::vector<Face*> oldFaceList;
    for (int i = 0; i < mFaceList.size(); ++i) {
        oldFaceList.push_back(mFaceList.at(i).get());
    }

    for (int i = 0; i< oldFaceList.size(); ++i){
        Face *f = oldFaceList.at(i);
        HalfEdge *curEdge = f->mEdgePtr;
        std::vector<HalfEdge*> midEdges;

        do {
            // Check if current vertex the edge points to is in the midPoints list
            bool flag = false;
            for (int i = 0; i < midVertex.size(); ++i) {
                if (curEdge->mVertexPtr == midVertex.at(i)) {
                    flag = true;
                }
            }
            // If in, then add to the midEdges list
            if (flag) {
                midEdges.push_back(curEdge);
            }
            // Goto next Edge
            curEdge = curEdge->mNextEdgePtr;
        } while (curEdge != f->mEdgePtr);

        Vertex *centroid = centroidVertex.at(f->text().toInt());
        QuadrangleFace(f, centroid, midEdges);
    }
}

void Mesh::CatmullClarkSubdivision() {
    std::vector<Vertex*> centroids;
    std::vector<Vertex*> midPoints;
    std::vector<Vertex*> originalVertiecs;

    // Before Subdivision, properly mark the sharp components
    std::vector<uPtr<Face>>::iterator faceIt;
    for (faceIt = mFaceList.begin(); faceIt != mFaceList.end(); ++faceIt) {
        // For a sharp face, every edge and vertex on the face is sharp
        if (faceIt->get()->IsSharp == true) {
            HalfEdge* nextEdge = faceIt->get()->mEdgePtr;
            do {
                nextEdge->IsSharp = true;
                nextEdge->mVertexPtr->IsSharp = true;
                nextEdge->mSYMEdgePtr->IsSharp = true;
                nextEdge = nextEdge->mNextEdgePtr;
            } while (nextEdge != faceIt->get()->mEdgePtr);
        }
    }

    // Follow the steps to call the Catmull clark subdivision process
    CopyOriginalVertexList(originalVertiecs);
    AddCentroid(centroids);
    AddMidPoint(centroids, midPoints);
    MoveOriginalVertex(centroids, midPoints, originalVertiecs);
    QuadrangleMesh(centroids, midPoints);
}
