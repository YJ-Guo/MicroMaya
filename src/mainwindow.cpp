#include "mainwindow.h"
#include <ui_mainwindow.h>
#include <QMessageBox>
#include <iostream>
#include "cameracontrolshelp.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mCurJoint(nullptr),
    mCurVertex(nullptr)
{
    ui->setupUi(this);
    ui->mygl->setFocus();

    // Set up the display of mesh result
    connect(ui->mygl, SIGNAL(DisplayMesh(Mesh&)), this, SLOT(DisplayMeshResult(Mesh&)));
    connect(ui->mygl, SIGNAL(FoundVertex(QListWidgetItem*)), this, SLOT(ChooseVertex(QListWidgetItem*)));
    connect(ui->mygl, SIGNAL(FoundEdge(QListWidgetItem*)), this, SLOT(ChooseEdge(QListWidgetItem*)));
    connect(ui->mygl, SIGNAL(FoundFace(QListWidgetItem*)), this, SLOT(ChooseFace(QListWidgetItem*)));
    // set up the selection mode
    connect(ui->VertexSelect, SIGNAL(clicked(bool)), this, SLOT(SelectVertex()));
    connect(ui->EdgeSelect, SIGNAL(clicked(bool)), this, SLOT(SelectHalfEdge()));
    connect(ui->FaceSelect, SIGNAL(clicked(bool)), this, SLOT(SelectFace()));
    connect(ui->cancelSelectBtn, SIGNAL(clicked(bool)), this, SLOT(CancelSelection()));

    // Display all spinbox until certain mesh component is selected
    ui->vertPosXSpinBox->setEnabled(false);
    ui->vertPosYSpinBox->setEnabled(false);
    ui->vertPosZSpinBox->setEnabled(false);
    ui->faceRedSpinBox->setEnabled(false);
    ui->faceGreenSpinBox->setEnabled(false);
    ui->faceBlueSpinBox->setEnabled(false);
    ui->AddVertexBtn->setEnabled(false);
    ui->TriangulateBtn->setEnabled(false);
    ui->Extrudebtn->setEnabled(false);
    ui->SharpList->setEnabled(false);

    if (mCurJoint == nullptr) {
        ui->RotateX->setEnabled(false);
        ui->RotateY->setEnabled(false);
        ui->RotateZ->setEnabled(false);
        ui->TranslateX->setEnabled(false);
        ui->TranslateY->setEnabled(false);
        ui->TranslateZ->setEnabled(false);
        ui->TranslateX_->setEnabled(false);
        ui->TranslateY_->setEnabled(false);
        ui->TranslateZ_->setEnabled(false);
    }

    // Wake up the spinbox when certain item is clicked
    connect(ui->vertsListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(ActiveVertexPositionSpinBox(QListWidgetItem*)));
    connect(ui->facesListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(ActiveFaceColorSpinBox(QListWidgetItem*)));
    connect(ui->halfEdgesListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(DisableAllBox(QListWidgetItem*)));

    // Take in the value change from spinbox
    connect(ui->vertPosXSpinBox, SIGNAL(valueChanged(double)), this, SLOT(ChangeVertexPositionX(double)));
    connect(ui->vertPosYSpinBox, SIGNAL(valueChanged(double)), this, SLOT(ChangeVertexPositionY(double)));
    connect(ui->vertPosZSpinBox, SIGNAL(valueChanged(double)), this, SLOT(ChangeVertexPositionZ(double)));
    connect(ui->faceRedSpinBox, SIGNAL(valueChanged(double)), this, SLOT(ChangeFaceColorR(double)));
    connect(ui->faceGreenSpinBox, SIGNAL(valueChanged(double)), this, SLOT(ChangeFaceColorG(double)));
    connect(ui->faceBlueSpinBox, SIGNAL(valueChanged(double)), this, SLOT(ChangeFaceColorB(double)));

    // HighLight the chosen mesh component
    connect(ui->vertsListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(HighLightVertex(QListWidgetItem*)));
    connect(ui->halfEdgesListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(HighLightHalfEdge(QListWidgetItem*)));
    connect(ui->facesListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(HighLightFace(QListWidgetItem*)));

    // The two functional buttons for Topology change of the mesh
    connect(ui->AddVertexBtn, SIGNAL(clicked(bool)), this, SLOT(AddMiddleVertex()));
    connect(ui->TriangulateBtn, SIGNAL(clicked(bool)), this, SLOT(TriangulateFace()));

    // New function features in HW06
    connect(ui->SubdivideBtn, SIGNAL(clicked(bool)), this, SLOT(slot_CatmullClarkSubdivision()));
    connect(ui->objLoader, SIGNAL(clicked(bool)), this, SLOT(slot_LoadOBJ()));
    connect(ui->Extrudebtn, SIGNAL(clicked(bool)), this, SLOT(slot_ExtrudeFace()));
    connect(ui->SharpBtn, SIGNAL(clicked(bool)), this, SLOT(slot_SetSharp()));

    // New function features in HW07
    connect(ui->jsonLoaderBtn, SIGNAL(clicked(bool)), this, SLOT(slot_LoadJSON()));
    connect(ui->SkinningBtn, SIGNAL(clicked(bool)), this, SLOT(slot_Skinning()));
    connect(ui->JointsTree, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(slot_hightlightJoint(QTreeWidgetItem*)));
    connect(ui->jointUpdateBtn, SIGNAL(clicked(bool)), this, SLOT(slot_UpdateJoints()));
    connect(ui->RotateX, SIGNAL(clicked(bool)), this, SLOT(slot_RotationX()));
    connect(ui->RotateY, SIGNAL(clicked(bool)), this, SLOT(slot_RotationY()));
    connect(ui->RotateZ, SIGNAL(clicked(bool)), this, SLOT(slot_RotationZ()));
    connect(ui->TranslateX, SIGNAL(clicked(bool)), this, SLOT(slot_TranslateX()));
    connect(ui->TranslateY, SIGNAL(clicked(bool)), this, SLOT(slot_TranslateY()));
    connect(ui->TranslateZ, SIGNAL(clicked(bool)), this, SLOT(slot_TranslateZ()));
    connect(ui->TranslateX_, SIGNAL(clicked(bool)), this, SLOT(slot_TranslateX_()));
    connect(ui->TranslateY_, SIGNAL(clicked(bool)), this, SLOT(slot_TranslateY_()));
    connect(ui->TranslateZ_, SIGNAL(clicked(bool)), this, SLOT(slot_TranslateZ_()));

    // Features for HW07 EC
    connect(ui->Joint0Weight, SIGNAL(valueChanged(double)), this, SLOT(slot_Joint0WeightChange(double)));
    connect(ui->Joint1Weight, SIGNAL(valueChanged(double)), this, SLOT(slot_Joint1WeightChange(double)));
    connect(ui->Joint0ReplaceBtn, SIGNAL(clicked(bool)), this, SLOT(slot_Joint0Replace()));
    connect(ui->Joint1ReplaceBtn, SIGNAL(clicked(bool)), this, SLOT(slot_Joint1Replace()));
    connect(ui->HDSkinningBtn, SIGNAL(clicked(bool)), this, SLOT(slot_HDSkinning()));
}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::EnableJointModifyBtn() {
    ui->RotateX->setEnabled(true);
    ui->RotateY->setEnabled(true);
    ui->RotateZ->setEnabled(true);
    ui->TranslateX->setEnabled(true);
    ui->TranslateY->setEnabled(true);
    ui->TranslateZ->setEnabled(true);
    ui->TranslateX_->setEnabled(true);
    ui->TranslateY_->setEnabled(true);
    ui->TranslateZ_->setEnabled(true);
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::exit();
}

void MainWindow::on_actionCamera_Controls_triggered()
{
    CameraControlsHelp* c = new CameraControlsHelp();
    c->show();
}

void MainWindow::DisplayMeshResult(Mesh &cubeMesh) {
    // Use iterators to get each component of the half edge mesh result
    for (std::vector<uPtr<Vertex>>::iterator vertIt = cubeMesh.mVerList.begin();
         vertIt != cubeMesh.mVerList.end(); ++vertIt) {
        // add every vertex item to the display list
        ui->vertsListWidget->addItem(vertIt->get());
    }

    for (std::vector<uPtr<Face>>::iterator faceIt = cubeMesh.mFaceList.begin();
         faceIt != cubeMesh.mFaceList.end(); ++faceIt) {
        // add every vertex item to the display list
        ui->facesListWidget->addItem(faceIt->get());
    }

    for (std::vector<uPtr<HalfEdge>>::iterator edgeIt = cubeMesh.mEdgeList.begin();
         edgeIt != cubeMesh.mEdgeList.end(); ++edgeIt) {
        // add every vertex item to the display list
        ui->halfEdgesListWidget->addItem(edgeIt->get());
    }

}

void MainWindow::CancelSelection() {
    ui->mygl->m_SelectionMode = 0;
    ui->SelectMode->setText(QString("NaN"));
}

void MainWindow::SelectVertex() {
    ui->mygl->m_SelectionMode = 1;
    ui->SelectMode->setText(QString("Vertex"));
}

void MainWindow::SelectHalfEdge() {
    ui->mygl->m_SelectionMode = 2;
    ui->SelectMode->setText(QString("HalfEdge"));
}

void MainWindow::SelectFace() {
    ui->mygl->m_SelectionMode = 3;
    ui->SelectMode->setText(QString("Face"));
}

void MainWindow::ChooseVertex(QListWidgetItem *item) {
    ui->vertsListWidget->setCurrentItem(item);
    HighLightVertex(item);
    ActiveVertexPositionSpinBox(item);
}

void MainWindow::ChooseEdge(QListWidgetItem *item) {
    ui->halfEdgesListWidget->setCurrentItem(item);
    HighLightHalfEdge(item);
    DisableAllBox(item);
}

void MainWindow::ChooseFace(QListWidgetItem *item) {
    ui->facesListWidget->setCurrentItem(item);
    HighLightFace(item);
    ActiveFaceColorSpinBox(item);
}

void MainWindow::ActiveVertexPositionSpinBox(QListWidgetItem* curItem) {
    // Change the availability of the spinbox
    ui->vertPosXSpinBox->setEnabled(true);
    ui->vertPosYSpinBox->setEnabled(true);
    ui->vertPosZSpinBox->setEnabled(true);
    ui->faceRedSpinBox->setEnabled(false);
    ui->faceGreenSpinBox->setEnabled(false);
    ui->faceBlueSpinBox->setEnabled(false);
    ui->AddVertexBtn->setEnabled(false);
    ui->TriangulateBtn->setEnabled(false);
    ui->Extrudebtn->setEnabled(false);

    // Initialize the spinbox value to vertex value
    Vertex* curVert = dynamic_cast<Vertex*>(curItem);
    mCurVertex = curVert;
    ui->vertPosXSpinBox->setValue(curVert->mPos.x);
    ui->vertPosYSpinBox->setValue(curVert->mPos.y);
    ui->vertPosZSpinBox->setValue(curVert->mPos.z);

    if (curVert->mJointInfluences.size() != 0) {
        ui->Joint0Dis->setText(QString(curVert->mJointInfluences.at(0).targetJoint->name));
        ui->Joint0Weight->setValue(curVert->mJointInfluences.at(0).weight);
        ui->Joint1Dis->setText(QString(curVert->mJointInfluences.at(1).targetJoint->name));
        ui->Joint1Weight->setValue(curVert->mJointInfluences.at(1).weight);
    }

}

void MainWindow::DisableAllBox(QListWidgetItem *curItem) {
    ui->vertPosXSpinBox->setEnabled(false);
    ui->vertPosYSpinBox->setEnabled(false);
    ui->vertPosZSpinBox->setEnabled(false);
    ui->faceRedSpinBox->setEnabled(false);
    ui->faceGreenSpinBox->setEnabled(false);
    ui->faceBlueSpinBox->setEnabled(false);
    ui->AddVertexBtn->setEnabled(true);
    ui->TriangulateBtn->setEnabled(false);
    ui->Extrudebtn->setEnabled(false);
}

void MainWindow::ActiveFaceColorSpinBox(QListWidgetItem* curItem) {
    // Change the availability of the spinbox
    ui->vertPosXSpinBox->setEnabled(false);
    ui->vertPosYSpinBox->setEnabled(false);
    ui->vertPosZSpinBox->setEnabled(false);
    ui->faceRedSpinBox->setEnabled(true);
    ui->faceGreenSpinBox->setEnabled(true);
    ui->faceBlueSpinBox->setEnabled(true);
    ui->AddVertexBtn->setEnabled(false);
    ui->TriangulateBtn->setEnabled(true);
    ui->Extrudebtn->setEnabled(true);

    // Initialize the spinbox value to vertex value
    Face* curFace = dynamic_cast<Face*>(curItem);
    ui->faceRedSpinBox->setValue(curFace->mColor.r);
    ui->faceGreenSpinBox->setValue(curFace->mColor.g);
    ui->faceBlueSpinBox->setValue(curFace->mColor.b);
}

void MainWindow::ChangeVertexPositionX(double newX) {
    // Get the current item in the list
    QListWidgetItem *curItem = ui->vertsListWidget->currentItem();
    // Cast this item to Vertex type
    Vertex *curVert = dynamic_cast<Vertex*>(curItem);
    curVert->mPos.x = newX;
    // Check planarity
    ui->mygl->MakePlane(ui->mygl->m_geoCube);
    // Update this mesh in mygl
    ui->mygl->setFocus();
    ui->mygl->m_geoCube.destroy();
    ui->mygl->m_geoCube.ClearOpenGLData();
    ui->mygl->m_geoCube.create();
    ui->mygl->update();
    emit ui->mygl->DisplayMesh(ui->mygl->m_geoCube);
}

void MainWindow::ChangeVertexPositionY(double newY) {
    // Get the current item in the list
    QListWidgetItem *curItem = ui->vertsListWidget->currentItem();
    // Cast this item to Vertex type
    Vertex *curVert = dynamic_cast<Vertex*>(curItem);
    curVert->mPos.y = newY;
    // Check planarity
    ui->mygl->MakePlane(ui->mygl->m_geoCube);
    // Update this mesh in mygl
    ui->mygl->setFocus();
    ui->mygl->m_geoCube.destroy();
    ui->mygl->m_geoCube.ClearOpenGLData();
    ui->mygl->m_geoCube.create();
    ui->mygl->update();
    emit ui->mygl->DisplayMesh(ui->mygl->m_geoCube);
}

void MainWindow::ChangeVertexPositionZ(double newZ) {
    // Get the current item in the list
    QListWidgetItem *curItem = ui->vertsListWidget->currentItem();
    // Cast this item to Vertex type
    Vertex *curVert = dynamic_cast<Vertex*>(curItem);
    curVert->mPos.z = newZ;
    // Check planarity
    ui->mygl->MakePlane(ui->mygl->m_geoCube);
    // Update this mesh in mygl
    ui->mygl->setFocus();
    ui->mygl->m_geoCube.destroy();
    ui->mygl->m_geoCube.ClearOpenGLData();
    ui->mygl->m_geoCube.create();
    ui->mygl->update();
    emit ui->mygl->DisplayMesh(ui->mygl->m_geoCube);
}

void MainWindow::ChangeFaceColorR(double newR){
    // Get the current item in the list
    QListWidgetItem *curItem = ui->facesListWidget->currentItem();
    // Cast this item to Face type
    Face *curFace = dynamic_cast<Face*>(curItem);
    curFace->mColor.r = newR;
    // Update this mesh in mygl
    ui->mygl->setFocus();
    ui->mygl->m_geoCube.destroy();
    ui->mygl->m_geoCube.ClearOpenGLData();
    ui->mygl->m_geoCube.create();
    ui->mygl->update();
    emit ui->mygl->DisplayMesh(ui->mygl->m_geoCube);
}

void MainWindow::ChangeFaceColorG(double newG){
    // Get the current item in the list
    QListWidgetItem *curItem = ui->facesListWidget->currentItem();
    // Cast this item to Face type
    Face *curFace = dynamic_cast<Face*>(curItem);
    curFace->mColor.g = newG;
    // Update this mesh in mygl
    ui->mygl->setFocus();
    ui->mygl->m_geoCube.destroy();
    ui->mygl->m_geoCube.ClearOpenGLData();
    ui->mygl->m_geoCube.create();
    ui->mygl->update();
    emit ui->mygl->DisplayMesh(ui->mygl->m_geoCube);
}

void MainWindow::ChangeFaceColorB(double newB){
    // Get the current item in the list
    QListWidgetItem *curItem = ui->facesListWidget->currentItem();
    // Cast this item to Face type
    Face *curFace = dynamic_cast<Face*>(curItem);
    curFace->mColor.b = newB;
    // Update this mesh in mygl
    ui->mygl->setFocus();
    ui->mygl->m_geoCube.destroy();
    ui->mygl->m_geoCube.ClearOpenGLData();
    ui->mygl->m_geoCube.create();
    ui->mygl->update();
    emit ui->mygl->DisplayMesh(ui->mygl->m_geoCube);
}


void MainWindow::HighLightVertex(QListWidgetItem *curVertex) {
    Vertex *v = dynamic_cast<Vertex*>(curVertex);
    mCurVertex = v;
    ui->mygl->setFocus();
    ui->mygl->m_faceDisplay.updateFace(nullptr);
    ui->mygl->m_vertexDisplay.destroy();
    ui->mygl->m_edgeDisplay.updateHalfEdge(nullptr);
    ui->mygl->m_vertexDisplay.updateVertex(v);
    ui->mygl->m_geoCube.ClearOpenGLData();
    ui->mygl->m_vertexDisplay.create();
    ui->mygl->update();
}

void MainWindow::HighLightHalfEdge(QListWidgetItem *curEdge) {
    HalfEdge *e = dynamic_cast<HalfEdge*>(curEdge);
    ui->mygl->setFocus();
    ui->mygl->m_faceDisplay.updateFace(nullptr);
    ui->mygl->m_vertexDisplay.updateVertex(nullptr);
    ui->mygl->m_edgeDisplay.destroy();
    ui->mygl->m_edgeDisplay.updateHalfEdge(e);
    ui->mygl->m_geoCube.ClearOpenGLData();
    ui->mygl->m_edgeDisplay.create();
    ui->mygl->update();
}

void MainWindow::HighLightFace(QListWidgetItem *curFace) {
    Face *f = dynamic_cast<Face*>(curFace);
    ui->mygl->setFocus();
    ui->mygl->m_faceDisplay.destroy();
    ui->mygl->m_vertexDisplay.updateVertex(nullptr);
    ui->mygl->m_edgeDisplay.updateHalfEdge(nullptr);
    ui->mygl->m_faceDisplay.updateFace(f);
    ui->mygl->m_geoCube.ClearOpenGLData();
    ui->mygl->m_faceDisplay.create();
    ui->mygl->update();
}

void MainWindow::AddMiddleVertex() {
    // Check for proper selection
    if (ui->mygl->m_edgeDisplay.representedHalfEdge != nullptr) {
        ui->mygl->SplitHalfEdge(ui->mygl->m_edgeDisplay.representedHalfEdge, ui->mygl->m_geoCube);
    }
    ui->mygl->setFocus();
    ui->mygl->m_geoCube.destroy();
    ui->mygl->m_geoCube.ClearOpenGLData();
    ui->mygl->m_geoCube.create();
    ui->mygl->update();
    emit ui->mygl->DisplayMesh(ui->mygl->m_geoCube);

}

void MainWindow::TriangulateFace() {
    // Check for proper selection
    if (ui->mygl->m_faceDisplay.representedFace != nullptr) {
        ui->mygl->TriangulateFace(ui->mygl->m_faceDisplay.representedFace, ui->mygl->m_geoCube);
    } else {
        QMessageBox::information(this, "Warning", " No face selected!");
    }
    ui->mygl->setFocus();
    ui->mygl->m_geoCube.destroy();
    ui->mygl->m_geoCube.ClearOpenGLData();
    ui->mygl->m_geoCube.create();
    ui->mygl->update();
    emit ui->mygl->DisplayMesh(ui->mygl->m_geoCube);
}

void MainWindow::slot_CatmullClarkSubdivision() {
    ui->mygl->m_geoCube.CatmullClarkSubdivision();
    ui->mygl->setFocus();
    ui->mygl->m_geoCube.destroy();
    ui->mygl->m_geoCube.ClearOpenGLData();
    ui->mygl->m_geoCube.create();
    ui->mygl->update();
    emit ui->mygl->DisplayMesh(ui->mygl->m_geoCube);
}

void MainWindow::slot_LoadOBJ() {
    std::cout << "Loading OBJ..." << std::endl;
    ui->mygl->m_geoCube.ClearAll();
    ui->mygl->SceneLoadDialog();
    std::cout << "Load Success!" << std::endl;
    ui->mygl->setFocus();
    ui->mygl->m_geoCube.destroy();
    ui->mygl->m_geoCube.create();
    ui->mygl->update();
    emit ui->mygl->DisplayMesh(ui->mygl->m_geoCube);
}

void MainWindow::slot_ExtrudeFace() {
    // Check for proper selection
    if (ui->mygl->m_faceDisplay.representedFace != nullptr) {
        ui->mygl->ExtrudeFace(ui->mygl->m_faceDisplay.representedFace, ui->mygl->m_geoCube);
    } else {
        QMessageBox::information(this, "Warning", " No face selected!");
    }
    ui->mygl->setFocus();
    ui->mygl->m_geoCube.destroy();
    ui->mygl->m_geoCube.ClearOpenGLData();
    ui->mygl->m_geoCube.create();
    ui->mygl->update();
    emit ui->mygl->DisplayMesh(ui->mygl->m_geoCube);
}

void MainWindow::slot_SetSharp() {
    if (ui->mygl->m_vertexDisplay.representedVertex != nullptr) {
        ui->mygl->m_vertexDisplay.representedVertex->IsSharp = true;
        int ID = ui->mygl->m_vertexDisplay.representedVertex->text().toInt();
        uPtr<QListWidgetItem> temp = mkU<QListWidgetItem>(QListWidgetItem());
        temp->setText("Vertex: " + QString::number(ID));
        std::vector<uPtr<QListWidgetItem>>::iterator listIt;
        bool flag = false;
        for (listIt = SharpItems.begin(); listIt != SharpItems.end(); ++listIt) {
            if (listIt->get()->text() == temp->text()) {
                flag = true;
            }
        }
        if (!flag) {
            ui->SharpList->addItem(temp.get());
            SharpItems.push_back(std::move(temp));
        }

    } else if (ui->mygl->m_edgeDisplay.representedHalfEdge != nullptr) {
        ui->mygl->m_edgeDisplay.representedHalfEdge->IsSharp = true;
        ui->mygl->m_edgeDisplay.representedHalfEdge->mSYMEdgePtr->IsSharp = true;
        int ID = ui->mygl->m_edgeDisplay.representedHalfEdge->text().toInt();
        int ID2 = ui->mygl->m_edgeDisplay.representedHalfEdge->mNextEdgePtr->text().toInt();
        uPtr<QListWidgetItem> temp = mkU<QListWidgetItem>(QListWidgetItem());
        temp->setText("HalfEdge: " + QString::number(ID));
        uPtr<QListWidgetItem> temp2 = mkU<QListWidgetItem>(QListWidgetItem());
        temp2->setText("HalfEdge: " + QString::number(ID2));

        std::vector<uPtr<QListWidgetItem>>::iterator listIt;
        bool flag = false;
        for (listIt = SharpItems.begin(); listIt != SharpItems.end(); ++listIt) {
            if (listIt->get()->text() == temp->text() ||
                    listIt->get()->text() == temp2->text()) {
                flag = true;
            }
        }
        if (!flag) {
            ui->SharpList->addItem(temp.get());
            SharpItems.push_back(std::move(temp));
            ui->SharpList->addItem(temp2.get());
            SharpItems.push_back(std::move(temp2));
        }


    } else if (ui->mygl->m_faceDisplay.representedFace != nullptr) {
        ui->mygl->m_faceDisplay.representedFace->IsSharp = true;
        int ID = ui->mygl->m_faceDisplay.representedFace->text().toInt();
        uPtr<QListWidgetItem> temp = mkU<QListWidgetItem>(QListWidgetItem());
        temp->setText("Face: " + QString::number(ID));

        std::vector<uPtr<QListWidgetItem>>::iterator listIt;
        bool flag = false;
        for (listIt = SharpItems.begin(); listIt != SharpItems.end(); ++listIt) {
            if (listIt->get()->text() == temp->text()) {
                flag = true;
            }
        }
        if (!flag) {
            ui->SharpList->addItem(temp.get());
            SharpItems.push_back(std::move(temp));
        }


        HalfEdge* nextEdge = ui->mygl->m_faceDisplay.representedFace->mEdgePtr;
        do {
            int IDE = nextEdge->text().toInt();
            int IDV = nextEdge->mVertexPtr->text().toInt();
            uPtr<QListWidgetItem> tempE = mkU<QListWidgetItem>(QListWidgetItem());
            uPtr<QListWidgetItem> tempV = mkU<QListWidgetItem>(QListWidgetItem());
            tempE->setText("HalfEdge: " + QString::number(IDE));
            tempV->setText("Vertex: " + QString::number(IDV));

            std::vector<uPtr<QListWidgetItem>>::iterator listIt;
            bool flag = false;
            for (listIt = SharpItems.begin(); listIt != SharpItems.end(); ++listIt) {
                if (listIt->get()->text() == tempE->text() ||
                        listIt->get()->text() == tempV->text()) {
                    flag = true;
                }
            }
            if (!flag) {
                ui->SharpList->addItem(tempE.get());
                ui->SharpList->addItem(tempV.get());
                SharpItems.push_back(std::move(tempE));
                SharpItems.push_back(std::move(tempV));
            }
            nextEdge = nextEdge->mNextEdgePtr;
        } while(nextEdge !=  ui->mygl->m_faceDisplay.representedFace->mEdgePtr);
    }
}

void MainWindow::slot_UpdateJoints() {
    ui->mygl->JointUpdate();
    ui->mygl->setFocus();
    ui->mygl->update();
}

void MainWindow::slot_LoadJSON() {
    std::cout << "Loading JSON..." << std::endl;
    ui->mygl->m_geoCube.ClearAll();
    ui->mygl->mJointList.clear();
    mCurJoint = nullptr;
    // Call the file dialog to get the json file info
    QString filepath = ui->mygl->SkeletonLoadDialog();
    // Check for file name
    if (filepath.length() != 0) {
        Joint* root = ui->mygl->LoadSkeleton(filepath);
        // clear the previous joint info
        ui->mygl->currentID = 0;
        ui->JointsTree->clear();
        ui->JointsTree->addTopLevelItem(root);
        ui->mygl->joint = root;
    } else {
       ui->mygl->mJointList.clear();
       ui->mygl->joint = nullptr;
       ui->mygl->currentID = 0;
       ui->JointsTree->clear();
       ui->mygl->setFocus();
       ui->mygl->update();
    }
    ui->mygl->setFocus();
    ui->mygl->update();
    //std::cout << ui->mygl->mJointList.size() << std::endl;
}

void MainWindow::slot_Skinning() {
    if(ui->mygl->mJointList.size() != 0) {
        ui->mygl->Skinning(ui->mygl->m_geoCube, ui->mygl->joint);
        ui->mygl->m_geoCube.destroy();
        ui->mygl->m_geoCube.ClearOpenGLData();
        ui->mygl->m_geoCube.create();
        ui->mygl->setFocus();
        ui->mygl->update();
    }
}

void MainWindow::slot_HDSkinning() {
    if(ui->mygl->mJointList.size() != 0) {
        ui->mygl->Skinning(ui->mygl->m_geoCube, ui->mygl->joint);
        ui->mygl->m_geoCube.destroy();
        ui->mygl->m_geoCube.ClearOpenGLData();
        ui->mygl->m_geoCube.create();
        ui->mygl->setFocus();
        ui->mygl->update();
    }
}

void MainWindow::slot_hightlightJoint(QTreeWidgetItem *curItem) {
    // Clear Previous highlight
    EnableJointModifyBtn();
    if (mCurJoint != nullptr) {
        mCurJoint->selected = false;
        mCurJoint->destroy();
        mCurJoint->create();
    }

    Joint* curJoint = dynamic_cast<Joint*>(curItem);
    mCurJoint = curJoint;
    curJoint->selected = true;
    curJoint->destroy();
    curJoint->create();
    ui->mygl->setFocus();
    ui->JointXR->setText(QString::number(mCurJoint->XRotate));
    ui->JointYR->setText(QString::number(mCurJoint->YRotate));
    ui->JointZR->setText(QString::number(mCurJoint->ZRotate));
    ui->JointX->setText(QString::number(mCurJoint->position.x));
    ui->JointY->setText(QString::number(mCurJoint->position.y));
    ui->JointZ->setText(QString::number(mCurJoint->position.z));
    ui->mygl->update();
    curJoint->selected = false;
}

void MainWindow::slot_RotationX() {
    if (mCurJoint != nullptr) {
        mCurJoint->selected = true;
        mCurJoint->XRotate += 5;
        ui->JointXR->setText(QString::number(mCurJoint->XRotate));
        mCurJoint->UpdateRotation();
        slot_UpdateJoints();
    }
}

void MainWindow::slot_RotationY() {
    if (mCurJoint != nullptr) {
        mCurJoint->selected = true;
        mCurJoint->YRotate += 5;
        ui->JointYR->setText(QString::number(mCurJoint->YRotate));
        mCurJoint->UpdateRotation();
        slot_UpdateJoints();
    }
}

void MainWindow::slot_RotationZ() {
    if (mCurJoint != nullptr) {
        mCurJoint->selected = true;
        mCurJoint->ZRotate += 5;
        ui->JointZR->setText(QString::number(mCurJoint->ZRotate));
        mCurJoint->UpdateRotation();
        slot_UpdateJoints();
    }
}

void MainWindow::slot_TranslateX() {
    if (mCurJoint != nullptr) {
        mCurJoint->selected = true;
        mCurJoint->position.x += 0.5;
        ui->JointX->setText(QString::number(mCurJoint->position.x));
        slot_UpdateJoints();
    }
}

void MainWindow::slot_TranslateY() {
    if (mCurJoint != nullptr) {
        mCurJoint->selected = true;
        mCurJoint->position.y += 0.5;
        ui->JointY->setText(QString::number(mCurJoint->position.y));
        slot_UpdateJoints();
    }
}

void MainWindow::slot_TranslateZ() {
    if (mCurJoint != nullptr) {
        mCurJoint->selected = true;
        mCurJoint->position.z += 0.5;
        ui->JointZ->setText(QString::number(mCurJoint->position.z));
        slot_UpdateJoints();
    }
}

void MainWindow::slot_TranslateX_() {
    if (mCurJoint != nullptr) {
        mCurJoint->selected = true;
        mCurJoint->position.x -= 0.5;
        ui->JointX->setText(QString::number(mCurJoint->position.x));
        slot_UpdateJoints();
    }
}

void MainWindow::slot_TranslateY_() {
    if (mCurJoint != nullptr) {
        mCurJoint->selected = true;
        mCurJoint->position.y -= 0.5;
        ui->JointY->setText(QString::number(mCurJoint->position.y));
        slot_UpdateJoints();
    }
}

void MainWindow::slot_TranslateZ_() {
    if (mCurJoint != nullptr) {
        mCurJoint->selected = true;
        mCurJoint->position.z -= 0.5;
        ui->JointZ->setText(QString::number(mCurJoint->position.z));
        slot_UpdateJoints();
    }
}

void MainWindow::slot_Joint0WeightChange(double value) {
    if (mCurVertex != nullptr && mCurVertex->mJointInfluences.size() != 0) {
        mCurVertex->mJointInfluences.at(0).weight = value;
        ui->Joint0Weight->setValue(value);
        mCurVertex->mJointInfluences.at(1).weight = 1 - value;
        ui->Joint1Weight->setValue(1 - value);
        ui->mygl->m_geoCube.destroy();
        ui->mygl->m_geoCube.ClearOpenGLData();
        ui->mygl->m_geoCube.create();
        ui->mygl->setFocus();
        ui->mygl->update();
    }
}

void MainWindow::slot_Joint1WeightChange(double value) {
    if (mCurVertex != nullptr && mCurVertex->mJointInfluences.size() != 0) {
        mCurVertex->mJointInfluences.at(1).weight = value;
        ui->Joint1Weight->setValue(value);
        mCurVertex->mJointInfluences.at(0).weight = 1 - value;
        ui->Joint0Weight->setValue(1 - value);
        ui->mygl->m_geoCube.destroy();
        ui->mygl->m_geoCube.ClearOpenGLData();
        ui->mygl->m_geoCube.create();
//        std::cout << "Weight1: " << mCurVertex->mJointInfluences.at(0).weight << std::endl;
//        std::cout << "Weight2: " << mCurVertex->mJointInfluences.at(1).weight << std::endl;
        ui->mygl->setFocus();
        ui->mygl->update();
    }
}

void MainWindow::slot_Joint0Replace() {
    if (mCurJoint != nullptr && mCurVertex != nullptr && mCurVertex->mJointInfluences.size() != 0) {
        mCurVertex->mJointInfluences.at(0).targetJoint = mCurJoint;
        ui->Joint0Dis->setText(QString(mCurJoint->name));
        ui->mygl->m_geoCube.destroy();
        ui->mygl->m_geoCube.ClearOpenGLData();
        ui->mygl->m_geoCube.create();
        ui->mygl->setFocus();
        ui->mygl->update();
    }
}

void MainWindow::slot_Joint1Replace() {
    if (mCurJoint != nullptr && mCurVertex != nullptr && mCurVertex->mJointInfluences.size() != 0) {
        mCurVertex->mJointInfluences.at(1).targetJoint = mCurJoint;
        ui->Joint1Dis->setText(QString(mCurJoint->name));
        ui->mygl->m_geoCube.destroy();
        ui->mygl->m_geoCube.ClearOpenGLData();
        ui->mygl->m_geoCube.create();
        ui->mygl->setFocus();
        ui->mygl->update();
    }
}
