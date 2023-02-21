#include "commonConstants.h"
#include "glWidget.h"
#include "mainwindow.h"
#include "viewer3D.h"
#include "qlayout.h"

#include <QMenu>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow()
{
    m_viewer3D = nullptr;

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Terrain 3D");
    setFixedSize(250, 150);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    // menu
    QMenuBar* menuBar = new QMenuBar();
    QMenu *fileMenu = new QMenu("File");
    menuBar->addMenu(fileMenu);
    layout()->setMenuBar(menuBar);

    QAction* openDtm = new QAction(tr("&Open Digital Terrain Model..."), this);
    fileMenu->addAction(openDtm);
    connect(openDtm, &QAction::triggered, this, &MainWindow::on_actionOpenDTM);
}


void MainWindow::on_actionOpenDTM()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load Digital Elevation Model"), "", tr("ESRI grid files (*.flt)"));
    if (fileName == "") return;

    fileName = fileName.left(fileName.length()-4);

    std::string error;
    if (! gis::readEsriGrid(fileName.toStdString(), &m_dtm, &error))
    {
        QMessageBox::critical(this, "Error in load DTM", QString::fromStdString(error));
        return;
    }

    if (!gis::computeSlopeAspectMaps(m_dtm, &m_slopeMap, &m_aspectMap))
    {
        QMessageBox::critical(this, "Error in compute slope & aspect", QString::fromStdString(error));
        return;
    }

    setDefaultDTMScale(m_dtm.colorScale);

    if (m_viewer3D != nullptr)
    {
        m_viewer3D->glWidget->clear();
        m_viewer3D->close();
    }

    initializeGeometry();
    m_viewer3D = new Viewer3D(&m_geometry);
    m_viewer3D->show();
}


void MainWindow::shadowColor(const Crit3DColor &colorIn, Crit3DColor &colorOut, int row, int col)
{
    colorOut.red = colorIn.red;
    colorOut.green = colorIn.green;
    colorOut.blue = colorIn.blue;

    float aspect = m_aspectMap.getValueFromRowCol(row, col);
    if (! isEqual(aspect, m_aspectMap.header->flag))
    {
        float slope = m_slopeMap.getValueFromRowCol(row, col);
        if (! isEqual(slope, m_slopeMap.header->flag))
        {
            float slopeAmplification = 120.f / std::max(m_slopeMap.maximum, 1.f);
            float shadow = -cos(aspect * float(DEG_TO_RAD)) * std::max(5.f, slope * slopeAmplification);
            colorOut.red = std::min(255, std::max(0, int(colorOut.red + shadow)));
            colorOut.green = std::min(255, std::max(0, int(colorOut.green + shadow)));
            colorOut.blue = std::min(255, std::max(0, int(colorOut.blue + shadow)));
            if (slope > m_geometry.artifactSlope())
            {
                colorOut.red = std::min(255, std::max(0, int((colorOut.red + 256) / 2)));
                colorOut.green = std::min(255, std::max(0, int((colorOut.green + 256) / 2)));
                colorOut.blue = std::min(255, std::max(0, int((colorOut.blue + 256) / 2)));
            }
        }
    }
}


bool MainWindow::initializeGeometry()
{
    if (! m_dtm.isLoaded)
        return false;

    m_geometry.clear();

    // set center
    double xCenter, yCenter;
    gis::getUtmXYFromRowCol(m_dtm, m_dtm.header->nrRows / 2, m_dtm.header->nrCols / 2, &xCenter, &yCenter);
    gis::updateMinMaxRasterGrid(&m_dtm);
    float zCenter = (m_dtm.maximum + m_dtm.minimum) * 0.5f;
    m_geometry.setCenter(float(xCenter), float(yCenter), zCenter);

    // set dimension
    float dx = float(m_dtm.header->nrCols * m_dtm.header->cellSize);
    float dy = float(m_dtm.header->nrRows * m_dtm.header->cellSize);
    float dz = m_dtm.maximum + m_dtm.minimum;
    m_geometry.setDimension(dx, dy);

    // set magnify
    float magnify = ((dx + dy) * 0.5f) / (dz * 10.f);
    m_geometry.setMagnify(std::min(5.f, std::max(1.f, magnify)));

    // set triangles
    double x, y;
    float z1, z2, z3;
    gis::Crit3DPoint p1, p2, p3;
    Crit3DColor *c1, *c2, *c3;
    Crit3DColor sc1, sc2, sc3;
    for (long row = 0; row < m_dtm.header->nrRows; row++)
    {
        for (long col = 0; col < m_dtm.header->nrCols; col++)
        {
            z1 = m_dtm.getValueFromRowCol(row, col);
            if (! isEqual(z1, m_dtm.header->flag))
            {
                gis::getUtmXYFromRowCol(m_dtm, row, col, &x, &y);
                p1 = gis::Crit3DPoint(x, y, z1);
                c1 = m_dtm.colorScale->getColor(z1);
                shadowColor(*c1, sc1, row, col);

                z3 = m_dtm.getValueFromRowCol(row+1, col+1);
                if (! isEqual(z3, m_dtm.header->flag))
                {
                    gis::getUtmXYFromRowCol(m_dtm, row+1, col+1, &x, &y);
                    p3 = gis::Crit3DPoint(x, y, z3);
                    c3 = m_dtm.colorScale->getColor(z3);
                    shadowColor(*c3, sc3, row+1, col+1);

                    z2 = m_dtm.getValueFromRowCol(row+1, col);
                    if (! isEqual(z2, m_dtm.header->flag))
                    {
                        gis::getUtmXYFromRowCol(m_dtm, row+1, col, &x, &y);
                        p2 = gis::Crit3DPoint(x, y, z2);
                        c2 = m_dtm.colorScale->getColor(z2);
                        shadowColor(*c2, sc2, row+1, col);
                        m_geometry.addTriangle(p1, p2, p3, sc1, sc2, sc3);
                    }

                    z2 = m_dtm.getValueFromRowCol(row, col+1);
                    if (! isEqual(z2, m_dtm.header->flag))
                    {
                        gis::getUtmXYFromRowCol(m_dtm, row, col+1, &x, &y);
                        p2 = gis::Crit3DPoint(x, y, z2);
                        c2 = m_dtm.colorScale->getColor(z2);
                        shadowColor(*c2, sc2, row, col+1);
                        m_geometry.addTriangle(p3, p2, p1, sc3, sc2, sc1);
                    }
                }
            }
        }
    }

    return true;
}
