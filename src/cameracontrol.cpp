#include "mygl.h"
#include <QKeyEvent>
#include <QApplication>
#include <iostream>

void MyGL::mousePressEvent(QMouseEvent *e) {
    if((e->buttons() & (Qt::LeftButton | Qt::RightButton)) && m_SelectionMode == 1) {
        m_mousePosPrev = glm::vec2(e->pos().x(), e->pos().y());
        // Cast a ray to find which point to intersect
        Ray r = m_glCamera.rayCast(m_mousePosPrev);

        // Iterate over vertices to see of there is a nearest intersection
        float minDistance = FLT_MAX;
        QListWidgetItem *minComponent = nullptr;
        std::vector<uPtr<Vertex>>::iterator vertexIt;
        for (vertexIt = m_geoCube.mVerList.begin(); vertexIt != m_geoCube.mVerList.end(); ++vertexIt) {
            float vertSDF = vertexIt->get()->sphereVert->sdf(r.origin);
            if (vertSDF < minDistance) {

                minComponent = dynamic_cast<QListWidgetItem*>(vertexIt->get());
                minDistance = vertSDF;
            }
        }
        emit FoundVertex(minComponent);
    } else if((e->buttons() & (Qt::LeftButton | Qt::RightButton)) && m_SelectionMode == 2) {
        m_mousePosPrev = glm::vec2(e->pos().x(), e->pos().y());
        // Cast a ray to find which point to intersect
        Ray r = m_glCamera.rayCast(m_mousePosPrev);
        // Iterate over vertices to see of there is a nearest intersection
        float minDistance = FLT_MAX;
        QListWidgetItem *minComponent = nullptr;
        std::vector<uPtr<HalfEdge>>::iterator edgeIt;
        for (edgeIt = m_geoCube.mEdgeList.begin(); edgeIt != m_geoCube.mEdgeList.end(); ++edgeIt) {
            float edgeSDF = edgeIt->get()->capsuleEdge->sdf(r.origin);
            if(edgeSDF < minDistance) {
                minComponent = dynamic_cast<QListWidgetItem*>(edgeIt->get());
                minDistance = edgeSDF;
            }
        }
        emit FoundEdge(minComponent);
    } else if((e->buttons() & (Qt::LeftButton | Qt::RightButton)) && m_SelectionMode == 3) {
        m_mousePosPrev = glm::vec2(e->pos().x(), e->pos().y());
        // Cast a ray to find which point to intersect
        Ray r = m_glCamera.rayCast(m_mousePosPrev);
        // Iterate over vertices to see of there is a nearest intersection
        float minDistance = FLT_MAX;
        QListWidgetItem *minComponent = nullptr;
        std::vector<uPtr<Face>>::iterator faceIt;
        for (faceIt = m_geoCube.mFaceList.begin(); faceIt != m_geoCube.mFaceList.end(); ++faceIt) {
            float faceSDF = faceIt->get()->triangleFace->sdf(r.origin);
            //std::cout << faceSDF << std::endl;
            if (faceSDF < minDistance) {
                minComponent = dynamic_cast<QListWidgetItem*>(faceIt->get());
                minDistance = faceSDF;
            }
        }
        emit FoundFace(minComponent);
    } else if (e->buttons() & (Qt::LeftButton | Qt::RightButton)) {
        m_mousePosPrev = glm::vec2(e->pos().x(), e->pos().y());
    }
}


void MyGL::mouseMoveEvent(QMouseEvent *e)
{
    glm::vec2 pos(e->pos().x(), e->pos().y());
    if(e->buttons() & Qt::LeftButton)
    {
        // Rotation
        glm::vec2 diff = 0.5f * (pos - m_mousePosPrev);
        m_mousePosPrev = pos;
        m_glCamera.RotateAboutUp(-diff.x);
        m_glCamera.RotateAboutRight(-diff.y);
        update();
    }
    else if(e->buttons() & Qt::RightButton)
    {
        // Panning
        glm::vec2 diff = 0.05f * (pos - m_mousePosPrev);
        m_mousePosPrev = pos;
        m_glCamera.TranslateAlongRight(-diff.x);
        m_glCamera.TranslateAlongUp(diff.y);
        update();
    }
}


void MyGL::wheelEvent(QWheelEvent *e)
{
    m_glCamera.Zoom(e->delta() * 0.02f);
    update();
}
