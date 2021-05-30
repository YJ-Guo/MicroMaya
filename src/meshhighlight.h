#ifndef MESHHIGHLIGHT_H
#define MESHHIGHLIGHT_H
#include "meshcomponent.h"

class VertexHighLight : public Drawable {
public:
    Vertex *representedVertex;

    // Creates VBO data to make a visual
    // representation of the currently selected Vertex
    void create() override;

    // Change which Vertex representedVertex points to
    void updateVertex(Vertex*);

    virtual GLenum drawMode() override;

    VertexHighLight(OpenGLContext *context);
};

class FaceHighLight : public Drawable {
public:
    Face *representedFace;

    // Creates VBO data to make a visual
    // representation of the currently selected Face
    void create() override;

    // Change which Face representedFace points to
    void updateFace(Face*);

    virtual GLenum drawMode() override;

    FaceHighLight(OpenGLContext *context);
};

class HalfEdgeHighLight : public Drawable {
public:
    HalfEdge *representedHalfEdge;

    // Creates VBO data to make a visual
    // representation of the currently selected HalfEdge
    void create() override;

    // Change which HalfEdge representedHalfEdge points to
    void updateHalfEdge(HalfEdge*);

    virtual GLenum drawMode() override;

    HalfEdgeHighLight(OpenGLContext *context);
};

#endif // MESHHIGHLIGHT_H
