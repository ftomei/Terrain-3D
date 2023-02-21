#include <QApplication>
#include "geometry.h"
#include "viewer3D.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Crit3DGeometry myGeometry;
    Viewer3D w(&myGeometry);
    w.show();

    return a.exec();
}
