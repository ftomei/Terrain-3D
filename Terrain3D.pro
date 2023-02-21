#-----------------------------------------------------------
#
# Terrain-3D
# 3D terrain visualizer using Qt 3d libraries
#
# Terrain-3D is a test project for CRITERIA-3D distribution
# https://github.com/ARPA-SIMC/CRITERIA3D
# Author: Fausto Tomei
#
#-----------------------------------------------------------

QT       += core gui widgets
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat openglwidgets

TARGET = TERRAIN_3D
TEMPLATE = app

CONFIG += c++11

INCLUDEPATH += gis

SOURCES += main.cpp \
    geometry.cpp \
    glWidget.cpp \
    gis/color.cpp \
    gis/gis.cpp \
    gis/gisIO.cpp \
    viewer3D.cpp

HEADERS += \
    geometry.h \
    glWidget.h \
    gis/commonConstants.h \
    gis/color.h \
    gis/gis.h \
    viewer3D.h


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
