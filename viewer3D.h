#ifndef VIEWER3D_H
#define VIEWER3D_H

    #include <QWidget>

    class Crit3DGeometry;
    class Crit3DOpenGLWidget;
    class QSlider;

    class Viewer3D : public QWidget
    {
        Q_OBJECT

    public:
        Viewer3D(Crit3DGeometry *myGeometry);

        Crit3DOpenGLWidget *glWidget;

    protected:

    private:
        QSlider *turnSlider;
        QSlider *rotateSlider;
        QSlider *magnifySlider;

        QSlider *verticalSlider(int minimum, int maximum, int step, int tick);
        QSlider *horizontalSlider(int minimum, int maximum, int step, int tick);
    };

#endif
