#include "meshhighlight.h"

VertexHighLight::VertexHighLight(OpenGLContext *context):
    Drawable(context),
    representedVertex(nullptr)
{}

void VertexHighLight::create() {
    if(representedVertex != nullptr) {
        std::vector<Point4f> vertPosList;
        Point4f vertPos = representedVertex->mPos;
        vertPosList.push_back(vertPos);

        std::vector<Color4f> vertColList;
        Color4f vertCol = Color4f(1.f);
        vertColList.push_back(vertCol);

        // The normal of representors is not useful
        // No shading applied
        std::vector<Normal4f> vertNorList;
        Normal4f vertNor = Normal4f(1.f);
        vertNorList.push_back(vertNor);

        std::vector<GLuint> vertIndex;
        // There is only one vertex index for drawing an vertex
        vertIndex.push_back(0);

        count = vertIndex.size();

        generateIdx();
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
        mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertIndex.size() * sizeof(GLuint), vertIndex.data(), GL_STATIC_DRAW);

        generatePos();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
        mp_context->glBufferData(GL_ARRAY_BUFFER, vertPosList.size() * sizeof(glm::vec4), vertPosList.data(), GL_STATIC_DRAW);

        generateNor();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufNor);
        mp_context->glBufferData(GL_ARRAY_BUFFER, vertColList.size() * sizeof(glm::vec4), vertNorList.data(), GL_STATIC_DRAW);

        generateCol();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
        mp_context->glBufferData(GL_ARRAY_BUFFER, vertNorList.size() * sizeof(glm::vec4), vertColList.data(), GL_STATIC_DRAW);
    }
}

void VertexHighLight::updateVertex(Vertex* newVert) {
    representedVertex = newVert;
}

GLenum VertexHighLight::drawMode() {
    return GL_POINTS;
}

FaceHighLight::FaceHighLight(OpenGLContext *context):
    Drawable(context),
    representedFace(nullptr)
{}

void FaceHighLight::create() {
    if(representedFace != nullptr) {
        std::vector<Point4f> facePosList;
        std::vector<Color4f> faceColList;
        // The normal of representors is not useful
        // No shading applied
        std::vector<Normal4f> faceNorList;
        Normal4f faceNor = Normal4f(1.f);
        for (int i = 0; i < 4; ++i){
            faceNorList.push_back(faceNor);
        }

        // The color of the ring is the opposite of the original
        Color4f faceCol = Color4f(1.f, 1.f, 1.f, 2.f) - representedFace->mColor;
        for (int i = 0; i < 4; ++i){
            faceColList.push_back(faceCol);
        }

        // put in the vertices of the ring
        Point4f faceVertPos1 = representedFace->mEdgePtr->mVertexPtr->mPos;
        facePosList.push_back(faceVertPos1);

        HalfEdge *nextEdge = representedFace->mEdgePtr->mNextEdgePtr;
        while (nextEdge != representedFace->mEdgePtr) {
            facePosList.push_back(nextEdge->mVertexPtr->mPos);
            nextEdge = nextEdge->mNextEdgePtr;
        }

        std::vector<GLuint> faceIndex;
        for (int i = 0; i < facePosList.size(); ++i) {
            faceIndex.push_back(i);
            faceIndex.push_back((i + 1) % facePosList.size());
        }

        count = faceIndex.size();

        generateIdx();
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
        mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceIndex.size() * sizeof(GLuint), faceIndex.data(), GL_STATIC_DRAW);

        generatePos();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
        mp_context->glBufferData(GL_ARRAY_BUFFER, facePosList.size() * sizeof(glm::vec4), facePosList.data(), GL_STATIC_DRAW);

        generateNor();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufNor);
        mp_context->glBufferData(GL_ARRAY_BUFFER, faceColList.size() * sizeof(glm::vec4), faceNorList.data(), GL_STATIC_DRAW);

        generateCol();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
        mp_context->glBufferData(GL_ARRAY_BUFFER, faceNorList.size() * sizeof(glm::vec4), faceColList.data(), GL_STATIC_DRAW);
    }
}

void FaceHighLight::updateFace(Face* newFace){
    representedFace = newFace;
}

GLenum FaceHighLight::drawMode() {
    return GL_LINES;
}

HalfEdgeHighLight::HalfEdgeHighLight(OpenGLContext *context):
    Drawable(context),
    representedHalfEdge(nullptr)
{}

void HalfEdgeHighLight::create() {
    if(representedHalfEdge != nullptr) {
        std::vector<Point4f> EdgePosList;
        std::vector<Color4f> EdgeColList;
        // The normal of representors is not useful
        // No shading applied
        std::vector<Normal4f> EdgeNorList;

        Point4f edgeVertPos1 = representedHalfEdge->mVertexPtr->mPos;
        EdgePosList.push_back(edgeVertPos1);
        // Yellow end represents the vertex the half edge points to
        Color4f edgeVertCol1 = Color4f(1.f, 1.f, 0.f, 1.f);
        EdgeColList.push_back(edgeVertCol1);

        // Use a while loop to find the other end of the Half Edge
        HalfEdge *nextEdge = representedHalfEdge->mNextEdgePtr;
        while (nextEdge->mNextEdgePtr != representedHalfEdge) {
            nextEdge = nextEdge->mNextEdgePtr;
        }

        // Now nextEdge points to the end point of the start edge
        Point4f edgeVertPos2 = nextEdge->mVertexPtr->mPos;
        EdgePosList.push_back(edgeVertPos2);
        // Red end represents the vertex the "before" half edge points to
        Color4f edgeVertCol2 = Color4f(1.f, 0.f, 0.f, 1.f);
        EdgeColList.push_back(edgeVertCol2);

        Normal4f n = Normal4f(1.f);
        EdgeNorList.push_back(n);
        EdgeNorList.push_back(n);

        std::vector<GLuint> EdgeIndex;
        // There are only two Edge index for drawing an Edge
        EdgeIndex.push_back(0);
        EdgeIndex.push_back(1);

        count = EdgeIndex.size();

        generateIdx();
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
        mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, EdgeIndex.size() * sizeof(GLuint), EdgeIndex.data(), GL_STATIC_DRAW);

        generatePos();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
        mp_context->glBufferData(GL_ARRAY_BUFFER, EdgePosList.size() * sizeof(glm::vec4), EdgePosList.data(), GL_STATIC_DRAW);

        generateNor();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufNor);
        mp_context->glBufferData(GL_ARRAY_BUFFER, EdgeColList.size() * sizeof(glm::vec4), EdgeNorList.data(), GL_STATIC_DRAW);

        generateCol();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
        mp_context->glBufferData(GL_ARRAY_BUFFER, EdgeNorList.size() * sizeof(glm::vec4), EdgeColList.data(), GL_STATIC_DRAW);
    }
}

void HalfEdgeHighLight::updateHalfEdge(HalfEdge* newEdge) {
    representedHalfEdge = newEdge;
}

GLenum HalfEdgeHighLight::drawMode() {
    return GL_LINES;
}
