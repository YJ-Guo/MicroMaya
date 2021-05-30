#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mygl.h"


namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    std::vector<uPtr<QListWidgetItem>> SharpItems;
    Joint* mCurJoint;
    Vertex* mCurVertex;
    void EnableJointModifyBtn();

public slots:
    // slot for displaying mesh results
    void DisplayMeshResult(Mesh &cubeMesh);
    void ChooseVertex(QListWidgetItem *item);
    void ChooseEdge(QListWidgetItem *item);
    void ChooseFace(QListWidgetItem *item);
    // slots for setup selection mode
    void SelectVertex();
    void SelectHalfEdge();
    void SelectFace();
    void CancelSelection();

    // Slots for spinbox initialization and availability change
    void ActiveVertexPositionSpinBox(QListWidgetItem *curItem);
    void ActiveFaceColorSpinBox(QListWidgetItem *curItem);
    void DisableAllBox(QListWidgetItem *curItem);

    // slots for changing mesh vertex positions
    void ChangeVertexPositionX(double newX);
    void ChangeVertexPositionY(double newY);
    void ChangeVertexPositionZ(double newZ);

    // slots for changing face colors
    void ChangeFaceColorR(double newR);
    void ChangeFaceColorG(double newG);
    void ChangeFaceColorB(double newB);

    // slots for highlighting mesh components
    void HighLightVertex(QListWidgetItem *curVertex);
    void HighLightHalfEdge(QListWidgetItem *curEdge);
    void HighLightFace(QListWidgetItem *curFace);

    // slots for two functional pushbtns
    void AddMiddleVertex();
    void TriangulateFace();

    // slots for new functions by HW06
    void slot_CatmullClarkSubdivision();
    void slot_LoadOBJ();
    void slot_ExtrudeFace();
    void slot_SetSharp();

    // slots for new functions by HW07
    void slot_LoadJSON();
    void slot_Skinning();
    void slot_HDSkinning();
    void slot_hightlightJoint(QTreeWidgetItem*);
    void slot_UpdateJoints();

    // slots for joint transformation
    void slot_TranslateX();
    void slot_TranslateY();
    void slot_TranslateZ();
    void slot_TranslateX_();
    void slot_TranslateY_();
    void slot_TranslateZ_();
    void slot_RotationX();
    void slot_RotationY();
    void slot_RotationZ();

    // Slots for joint weight control
    void slot_Joint0WeightChange(double value);
    void slot_Joint1WeightChange(double value);
    void slot_Joint0Replace();
    void slot_Joint1Replace();

private slots:
    void on_actionQuit_triggered();

    void on_actionCamera_Controls_triggered();

private:
    Ui::MainWindow *ui;
};


#endif // MAINWINDOW_H
