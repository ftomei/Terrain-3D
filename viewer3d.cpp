#include "commonConstants.h"
#include "viewer3d.h"
#include "QHBoxLayout"

#include <QVector3D>
#include <Qt3DExtras/QPerVertexColorMaterial>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <QAttribute>
#include <QLabel>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>


#define DEFAULT_ZOOMLEVEL 8


void Viewer3D::buildLookupTables()
{
    for (int angle = 0; angle < 3600; angle++)
    {
        m_cosTable[angle] = cos(double(angle) / 10. * DEG_TO_RAD);
        m_sinTable[angle] = sin(double(angle) / 10. * DEG_TO_RAD);
    }
}


Viewer3D::Viewer3D()
{
    m_terrain = nullptr;
    m_geometry = nullptr;
    m_rootEntity = nullptr;
    m_magnify = NODATA;
    m_size = NODATA;
    m_ratio = NODATA;
    m_nrVertex = 0;
    isCameraChanging = false;
    m_zoomLevel = DEFAULT_ZOOMLEVEL;

    this->buildLookupTables();

    this->setWindowTitle(QStringLiteral("Terrain 3D"));

    m_view = new Qt3DExtras::Qt3DWindow();
    m_view->installEventFilter(this);
    m_view->defaultFrameGraph()->setClearColor(QColor::fromRgbF(1, 1, 1, 1.0));

    QWidget *container = QWidget::createWindowContainer(m_view);
    QSize screenSize = m_view->screen()->size();
    container->setMinimumSize(QSize(200, 200));
    container->setMaximumSize(screenSize);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(container, 1);
    this->setLayout(layout);

    this->resize(1024, 720);

    //menu
    QMenuBar* menuBar = new QMenuBar();
    QMenu *fileMenu = new QMenu("File");
    menuBar->addMenu(fileMenu);

    QAction* loadDTM = new QAction(tr("&Load DTM"), this);
    connect(loadDTM, &QAction::triggered, this, &Viewer3D::on_actionLoadDEM);
    fileMenu->addAction(loadDTM);
    fileMenu->addAction("Exit");

    this->layout()->setMenuBar(menuBar);
}



void Viewer3D::initialize3D()
{
    double dy = m_terrain->DTM.header->nrRows * m_terrain->DTM.header->cellSize;
    double dx = m_terrain->DTM.header->nrCols * m_terrain->DTM.header->cellSize;
    m_dz = maxValue(double(m_terrain->DTM.maximum - m_terrain->DTM.minimum), 10);

    m_dtmCenter.utm.x = m_terrain->DTM.header->llCorner->x + dx * 0.5;
    m_dtmCenter.utm.y = m_terrain->DTM.header->llCorner->y + dy * 0.5;
    m_dtmCenter.z = double(m_terrain->DTM.minimum) + m_dz * 0.5;

    m_size = sqrt(dx*dy);
    m_ratio = m_size / m_dz;
    m_magnify = maxValue(1, minValue(10, m_ratio / 5));

    m_zoomLevel = DEFAULT_ZOOMLEVEL;

    // Set root object of the scene
    createScene();

    m_centerObject = QVector3D(m_dtmCenter.utm.x, m_dtmCenter.utm.y, m_dtmCenter.z * m_magnify);

    // Camera
    m_view->camera()->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 1000000.0f);
    m_view->camera()->setPosition(QVector3D(m_centerObject.x(), m_centerObject.y(), m_centerObject.z() + (m_dz * m_zoomLevel) * m_magnify));
    m_view->camera()->setViewCenter(m_centerObject);
    //m_view->camera()->setUpVector(QVector3D(0, 0, 1));

    // Camera controls
    //Qt3DExtras::QOrbitCameraController *camController = new Qt3DExtras::QOrbitCameraController(m_rootEntity);
    //camController->setCamera(m_view->camera());

    m_view->setRootEntity(m_rootEntity);

    m_cameraMatrix = m_view->camera()->transform()->matrix();
    m_cameraPosition = m_view->camera()->transform()->translation();
    m_rotationZ = m_view->camera()->transform()->rotationZ();
}



void Viewer3D::on_actionLoadDEM()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open raster Grid"), "", tr("ESRI grid files (*.flt)"));

    if (fileName == "") return;

    qDebug() << "loading raster";
    m_terrain = new Terrain();
    if (!m_terrain->loadDTM(fileName)) return;

    this->initialize3D();
}


bool Viewer3D::eventFilter(QObject *obj, QEvent *ev)
{
    if (ev->type() == QEvent::Wheel)
    {
        wheelEvent(dynamic_cast<QWheelEvent*>(ev));
        return true;
    }
    else if (ev->type() == QEvent::MouseButtonPress)
    {
        mousePressEvent(dynamic_cast<QMouseEvent*>(ev));
        return true;
    }
    else if (ev->type() == QEvent::MouseMove)
    {
        mouseMoveEvent(dynamic_cast<QMouseEvent*>(ev));
        return true;
    }
    else if (ev->type() == QEvent::MouseButtonRelease)
    {
        mouseReleaseEvent(dynamic_cast<QMouseEvent*>(ev));
        return true;
    }

    return QObject::eventFilter(obj, ev);
}


void Viewer3D::wheelEvent(QWheelEvent *we)
{
    QPoint delta = we->angleDelta();

    if (delta.y() < 0)
        m_zoomLevel *= 1.25;
    else
        m_zoomLevel *= 0.8;

    QVector3D translation = QVector3D(m_cameraPosition.x(), m_cameraPosition.y(), (m_dtmCenter.z + m_dz * m_zoomLevel) * m_magnify);

    m_view->camera()->transform()->setTranslation(translation);
    m_cameraPosition = m_view->camera()->transform()->translation();
}


void Viewer3D::mousePressEvent(QMouseEvent *ev)
{
    if ((ev->button() == Qt::LeftButton) || (ev->button() == Qt::RightButton))
    {
        m_button = ev->button();
        m_moveStartPoint = ev->pos();
        m_cameraMatrix = m_view->camera()->transform()->matrix();
        m_cameraPosition = m_view->camera()->transform()->translation();
        m_rotationZ = m_view->camera()->transform()->rotationZ();
        isCameraChanging = true;
    }
}


void Viewer3D::mouseMoveEvent(QMouseEvent *ev)
{
    if (isCameraChanging)
    {
        QPoint delta = ev->pos() - m_moveStartPoint;
        if (m_button == Qt::RightButton)
        {  
            /*float angleX = delta.x() * (m_size/10000000);
            QMatrix4x4 m;
            m.translate(-m_centerObject);           // Move the object to the coordinate system's origin
            m.rotate(angleX, QVector3D(0, 0, 1));   // Rotate around the z-axis with your angle.
            m.translate(m_centerObject);            // Restore the object's original position
            m_view->camera()->transform()->setMatrix(m * m_cameraMatrix);*/

            float agleX = delta.x() * float(m_size/10000000.0);
            QVector3D axis = QVector3D(1, 0, 1);
            QMatrix4x4 m = Qt3DCore::QTransform::rotateAround(-m_centerObject, agleX, axis);
            m_view->camera()->transform()->setMatrix(m * m_cameraMatrix);

            /*float dz = maxValue(m_terrain->DTM.maximum - m_terrain->DTM.minimum, 10.f);
            float z = m_terrain->DTM.minimum + dz * 0.5f;
            float dy = delta.y() * m_zoomLevel;
            m_view->camera()->setViewCenter(QVector3D(float(m_center.x), float(m_center.y), z * m_magnify));*/

            /*float anglex = m_rotationZ - delta.x() * m_zoomLevel / 10;
            m_view->camera()->transform()->setRotationZ(anglex);*/

            /*float angley = delta.y() * m_zoomLevel / 360.f;
            m_view->camera()->panAboutViewCenter(angley);*/
        }
        else if (m_button == Qt::LeftButton)
        {
            QVector3D translation = QVector3D(m_cameraPosition.x() - delta.x() * m_zoomLevel * m_size / 3000,
                                              m_cameraPosition.y() + delta.y() * m_zoomLevel * m_size / 3000,
                                              m_cameraPosition.z());
            m_view->camera()->transform()->setTranslation(translation);
        }
    }
}


void Viewer3D::mouseReleaseEvent(QMouseEvent* ev)
{
    Q_UNUSED(ev);
    if (isCameraChanging)
    {
        m_cameraMatrix = m_view->camera()->transform()->matrix();
        m_cameraPosition = m_view->camera()->transform()->translation();
        m_rotationZ = m_view->camera()->transform()->rotationZ();
        isCameraChanging = false;
    }
}



double Viewer3D::getCosTable(double angle)
{
    if (angle < 0)
        angle += 360;
    else if (angle >= 360)
        angle -= 360;
    return m_cosTable[int(angle * 10)];
}


float getValueInRange(float value, float minimum, float maximum)
{
    if (value < minimum)
        return minimum;
    else if (value > maximum)
        return maximum;
    else
        return value;
}


void setVertexColor(float* vertexColor, int vertexIndex, Crit3DColor* myColor, float shadow)
{
    float red = getValueInRange(myColor->red + shadow, 0, 255);
    float green = getValueInRange(myColor->green + shadow, 0, 255);
    float blue = getValueInRange(myColor->blue + shadow, 0, 255);

    vertexColor[vertexIndex*3] = red / 256.f;
    vertexColor[vertexIndex*3+1] = green / 256.f;
    vertexColor[vertexIndex*3+2] = blue / 256.f;
}


void Viewer3D::cleanScene()
{
    if (m_rootEntity != nullptr)
    {
        m_vertexPositionArray.clear();
        m_vertexColorArray.clear();
        m_triangleIndexArray.clear();
        m_geometry->clearPropertyTracking(Qt3DRender::QAttribute::defaultPositionAttributeName());
        m_geometry->clearPropertyTracking(Qt3DRender::QAttribute::defaultColorAttributeName());
        m_geometry->clearPropertyTracking(Qt3DRender::QAttribute::defaultJointIndicesAttributeName());
        m_rootEntity.clear();
    }
}


void Viewer3D::createScene()
{
    m_nrVertex = int(m_terrain->indexMap.maximum) + 1;

    m_vertexPositionArray.resize(m_nrVertex * 3 * int(sizeof(float)));
    m_vertexColorArray.resize(m_nrVertex * 3 * int(sizeof(float)));
    m_triangleIndexArray.resize(m_nrVertex * 4 * 3 * int(sizeof(uint)));

    float *vertexPosition = reinterpret_cast<float *>(m_vertexPositionArray.data());
    float *vertexColor = reinterpret_cast<float *>(m_vertexColorArray.data());
    uint *indexData = reinterpret_cast<uint *>(m_triangleIndexArray.data());

    double SlopeAmplification = 128.0 / double(m_terrain->slopeMap.maximum);
    double myAspect, mySlope, shadow;

    // Vertices
    long index;
    float x, y, z;
    Crit3DColor *myColor;
    for (int row = 0; row < m_terrain->indexMap.header->nrRows; row++)
    {
        for (int col = 0; col < m_terrain->indexMap.header->nrCols; col++)
        {
            index = long(m_terrain->indexMap.value[row][col]);
            if (index != long(m_terrain->indexMap.header->flag))
            {
                z = m_terrain->DTM.value[row][col];
                if (int(z) != int(m_terrain->DTM.header->flag))
                {
                    gis::getUtmXYFromRowColSinglePrecision(*(m_terrain->DTM.header), row, col, &x, &y);

                    vertexPosition[index*3] = x;
                    vertexPosition[index*3+1] = y;
                    vertexPosition[index*3+2] = z  * float(m_magnify);

                    myColor = m_terrain->DTM.colorScale->getColor(z);

                    shadow = 0;
                    myAspect = m_terrain->aspectMap.value[row][col];
                    mySlope = m_terrain->slopeMap.value[row][col];
                    if ((myAspect != m_terrain->aspectMap.header->flag)
                            && (mySlope != m_terrain->slopeMap.header->flag))
                    {
                            // light from south-west
                            shadow = getCosTable(myAspect+140) * mySlope * SlopeAmplification;
                    }

                    setVertexColor(vertexColor, index, myColor, float(shadow));
                }
            }
        }
    }

    Qt3DRender::QBuffer *vertexPositionBuffer = new Qt3DRender::QBuffer();
    vertexPositionBuffer->setData(m_vertexPositionArray);
    Qt3DRender::QBuffer *vertexColorBuffer = new Qt3DRender::QBuffer();
    vertexColorBuffer->setData(m_vertexColorArray);

    // Attributes
    Qt3DRender::QAttribute *positionAttribute = new Qt3DRender::QAttribute();
    positionAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    positionAttribute->setVertexBaseType(Qt3DRender::QAttribute::VertexBaseType::Float);
    positionAttribute->setBuffer(vertexPositionBuffer);
    positionAttribute->setVertexSize(3);
    positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());

    Qt3DRender::QAttribute *colorAttribute = new Qt3DRender::QAttribute();
    colorAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    positionAttribute->setVertexBaseType(Qt3DRender::QAttribute::VertexBaseType::Float);
    colorAttribute->setBuffer(vertexColorBuffer);
    colorAttribute->setVertexSize(3);
    colorAttribute->setName(Qt3DRender::QAttribute::defaultColorAttributeName());

    // Indices
    long v0, v1, v2, v3;
    index = 0;
    for (int row = 0; row < m_terrain->indexMap.header->nrRows; row++)
    {
        for (int col = 0; col < m_terrain->indexMap.header->nrCols; col++)
        {
            // initialize
            v0 = long(m_terrain->indexMap.value[row][col]);
            v1 = long(m_terrain->indexMap.header->flag);
            v2 = long(m_terrain->indexMap.header->flag);
            v3 = long(m_terrain->indexMap.header->flag);

            if (v0 != long(m_terrain->indexMap.header->flag))
            {
                if (row < (m_terrain->indexMap.header->nrRows-1))
                    v1 = long(m_terrain->indexMap.value[row+1][col]);
                if (row < (m_terrain->indexMap.header->nrRows-1) && col < (m_terrain->indexMap.header->nrCols-1))
                    v2 = long(m_terrain->indexMap.value[row+1][col+1]);
                if (col < (m_terrain->indexMap.header->nrCols-1))
                    v3 = long(m_terrain->indexMap.value[row][col+1]);

                // clockwise
                if (v1 != long(m_terrain->indexMap.header->flag) && v2 != long(m_terrain->indexMap.header->flag))
                {
                    indexData[index*3] = uint(v0);
                    indexData[index*3+1] = uint(v2);
                    indexData[index*3+2] = uint(v1);
                    index++;
                }
                if (v2 != long(m_terrain->indexMap.header->flag) && v3 != long(m_terrain->indexMap.header->flag))
                {
                    indexData[index*3] = uint(v0);
                    indexData[index*3+1] = uint(v3);
                    indexData[index*3+2] = uint(v2);
                    index++;
                }

                // anti-clockwise
                if (v1 != long(m_terrain->indexMap.header->flag) && v2 != long(m_terrain->indexMap.header->flag))
                {
                    indexData[index*3] = uint(v0);
                    indexData[index*3+1] = uint(v1);
                    indexData[index*3+2] = uint(v2);
                    index++;
                }
                if (v2 != long(m_terrain->indexMap.header->flag) && v3 != long(m_terrain->indexMap.header->flag))
                {
                    indexData[index*3] = uint(v0);
                    indexData[index*3+1] = uint(v2);
                    indexData[index*3+2] = uint(v3);
                    index++;
                }
            }
        }
    }

    long nrTriangles = index;
    Qt3DRender::QBuffer *indexBuffer = new Qt3DRender::QBuffer();
    indexBuffer->setData(m_triangleIndexArray);

    Qt3DRender::QAttribute *indexAttribute = new Qt3DRender::QAttribute();
    indexAttribute->setAttributeType(Qt3DRender::QAttribute::IndexAttribute);
    indexAttribute->setVertexBaseType(Qt3DRender::QAttribute::UnsignedInt);
    indexAttribute->setBuffer(indexBuffer);

    // Geometry
    m_geometry = new Qt3DRender::QGeometry();
    m_geometry->addAttribute(positionAttribute);
    m_geometry->addAttribute(colorAttribute);
    m_geometry->addAttribute(indexAttribute);

    // Geometry renderer
    Qt3DRender::QGeometryRenderer *geometryRenderer = new Qt3DRender::QGeometryRenderer;
    geometryRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);
    geometryRenderer->setGeometry(m_geometry);
    geometryRenderer->setInstanceCount(1);
    geometryRenderer->setFirstVertex(0);
    geometryRenderer->setFirstInstance(0);
    geometryRenderer->setVertexCount(nrTriangles*3);

    // Material
    Qt3DRender::QMaterial *material = new Qt3DExtras::QPerVertexColorMaterial();

    m_rootEntity = new Qt3DCore::QEntity();

    // Entity
    m_rootEntity->addComponent(material);
    m_rootEntity->addComponent(geometryRenderer);
}

