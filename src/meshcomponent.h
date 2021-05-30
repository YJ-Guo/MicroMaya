#ifndef MESHCOMPONENT_H
#define MESHCOMPONENT_H
#include "utils.h"
#include <QListWidgetItem>
#include <unordered_map>
#include <pcg32.h>
#include "drawable.h"
#include "joint.h"
#include "raytracer/shape.h"

// Set up the rule to Hash a pair
struct hash_pair {
    template <class T1, class T2>
    size_t operator()(const std::pair<T1, T2>& p) const
    {
        auto hash1 = std::hash<T1>{}(p.first);
        auto hash2 = std::hash<T2>{}(p.second);
        return hash1 ^ hash2;
    }
};

// Set up the influence weight of joints on vertex
struct JointInfluence{
    Joint* targetJoint;
    float weight;
};

// Forward declearation for making pointers
class HalfEdge;
class Face;

class Vertex : public QListWidgetItem {
public:
    Point4f mPos;           // A vec3 for storing its position, made 4 for transformation
    HalfEdge *mEdgePtr;     // A pointer to one of the HalfEdges that points to this Vertex
    static unsigned int mID;// A unique integer to identify the Vertex in menus and while debugging

    uPtr<Shape> sphereVert; // Use a sphere shape to represent a vertex
    bool IsSharp;

    // Member for mesh binding
    std::vector<JointInfluence> mJointInfluences;

    Vertex();
};

class Face : public QListWidgetItem {
public:
    HalfEdge *mEdgePtr;     // A pointer to one of the HalfEdges that lies on this Face
    Color4f mColor;         // A vec3 to represent this Face's color as an RGB value, make 4 for consistancy
    static unsigned int mID;// A unique integer to identify the Face in menus and while debugging

    uPtr<TriangleFace> triangleFace; // // Use a triangle shape to represent a vertex
    bool IsSharp;

    Face();
};

class HalfEdge : public QListWidgetItem {
public:
    HalfEdge *mNextEdgePtr;  // A pointer to the next HalfEdge in the loop of HalfEdges that lie on this HalfEdge's Face
    HalfEdge *mSYMEdgePtr;   // This HalfEdge's symmetrical HalfEdge
    Face *mFacePtr;          // A pointer to the Face on which this HalfEdge lies
    Vertex *mVertexPtr;      // A pointer to the Vertex between this HalfEdge and its next HalfEdge
    // Created in OBJ file loading to find the symEdge
    Vertex *mVertBeforePtr;  // The other end of the edge
    static unsigned int mID; // A unique integer to identify the HalfEdge in menus and while debugging

    uPtr<Capsule> capsuleEdge; // Use a capsule to represent a vertex
    bool IsSharp;

    HalfEdge();
};

class Mesh : public Drawable {
public:
    // Lists of unique pointers to half edge elements
    std::vector<uPtr<Vertex>> mVerList;
    std::vector<uPtr<Face>> mFaceList;
    std::vector<uPtr<HalfEdge>> mEdgeList;

    // Convert the half edge elements to VBO's information
    std::vector<GLuint> mIndex;    // Store the triangle indices
    std::vector<Point4f> mPos;     // Store the Vertex position
    std::vector<Vector4f> mNor;    // Store the Vertex's normal
    std::vector<Vector4f> mCol;    // Store the Color of the face
    std::vector<glm::ivec2> mJointIDs;
    std::vector<glm::vec2> mJointWeights; // Store the influence of joints on vertices

    // Use a random number generator to set the face color
    // for obj file reading
    pcg32 rng;
    bool IsOBJ;

    Mesh(OpenGLContext *context);
    // This part is for setup the mesh
    void create();
    void SetOpenGLData();
    void MakeCube();

    // This Part is for Catmull-Clark subdivision
    // A copy list to backup the original vertices
    void CopyOriginalVertexList(std::vector<Vertex*> &originalVertex);

    // Find the the centroid of the face then add this vertex to vertices list
    void AddCentroidToFace(std::vector<Vertex*> &centriodVertex, Face* curFace);
    void AddCentroid(std::vector<Vertex*> &centroidVertex);

    // Compute the smoothed midpoint of each edge in the mesh
    void AddMidPoint(std::vector<Vertex*> &centroidVertex, std::vector<Vertex*> &midVertex);
    // Split the edge with the given midpoint position
    void SplitEdgeWithMid(Point4f midpos, HalfEdge* originalEdge, std::vector<Vertex*> &midPoints);

    //  Smooth the original vertices
    void MoveOriginalVertex(std::vector<Vertex*>centroidVertex,
                            std::vector<Vertex*>midVertex,
                            std::vector<Vertex*> originalVertex);

    // For each original face, split that face into N quadrangle faces
    void QuadrangleFace(Face* f, Vertex *centroid, std::vector<HalfEdge*> midEdges);
    void ConstructSubFace(Face *newFace, HalfEdge *midEdge, HalfEdge* midEdgePre, Vertex* centroid);
    void QuadrangleMesh(std::vector<Vertex*> centroidVertex,
                        std::vector<Vertex*> midVertex);
    // Final integration of subdivision
    void CatmullClarkSubdivision();

    // Clear all the mesh components for next obj
    void ClearAll();
    void ClearOpenGLData();

    // Convert the polygon information to HalfEdge Mesh information
    void SetUpOBJMesh(const QString filepath);
};

// A helper function to find the previous edge for current edge
HalfEdge *GetPreviousEdge(const HalfEdge* curEdge);
bool IsBlack(Vector4f v);

#endif // MESHCOMPONENT_H
