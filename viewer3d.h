#ifndef VIEWER3D_H
#define VIEWER3D_H

#include <QWidget>
#include <Qt3DExtras>
#include "gis.h"
#include "terrain.h"


#define DEFAULT_ZOOMLEVEL 8

class Viewer3D : public QWidget
{
    Q_OBJECT

    public:
        Viewer3D();

    protected:
        bool eventFilter(QObject *obj, QEvent *ev);
        void mouseMoveEvent(QMouseEvent *ev);
        void mousePressEvent(QMouseEvent *ev);
        void mouseReleaseEvent(QMouseEvent *ev);
        void wheelEvent(QWheelEvent *we);

    private:
        Terrain *m_terrain;

        QByteArray m_vertexPositionArray;
        QByteArray m_vertexColorArray;
        QByteArray m_triangleIndexArray;
        Qt3DRender::QGeometry *m_geometry;
        QPointer<Qt3DCore::QEntity> m_rootEntity;

        Qt3DExtras::Qt3DWindow *m_view;
        QPoint m_moveStartPoint;
        QMatrix4x4 m_cameraMatrix;
        QVector3D m_cameraPosition;
        QVector3D m_centerObject;

        double m_cosTable[3600];
        double m_sinTable[3600];

        bool isCameraChanging;

        double m_dz;
        float m_rotationZ;
        double m_x_angle;
        double m_magnify;
        double m_zoomLevel;
        double m_size;
        double m_ratio;
        int m_nrVertex;

        gis::Crit3DPoint m_dtmCenter;
        Qt::MouseButton m_button;

        void buildLookupTables();
        double getCosTable(double angle);
        void createScene();
        void cleanScene();

        void on_actionLoadDEM();
        void initialize3D();
};


#endif // VIEWER3D_H
