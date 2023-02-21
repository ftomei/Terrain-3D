#ifndef MAINWINDOW_H
#define MAINWINDOW_H

    #include <QWidget>
    #include "geometry.h"
    #include "gis.h"
    #include "viewer3D.h"

    class MainWindow : public QWidget
    {
        Q_OBJECT

    public:
        MainWindow();

    private:
        Viewer3D* m_viewer3D;
        Crit3DGeometry m_geometry;
        gis::Crit3DRasterGrid m_slopeMap;
        gis::Crit3DRasterGrid m_aspectMap;
        gis::Crit3DRasterGrid m_dtm;

        bool initializeGeometry();
        void on_actionOpenDTM();
        void shadowColor(const Crit3DColor &colorIn, Crit3DColor &colorOut, int row, int col);
    };

#endif // MAINWINDOW_H
