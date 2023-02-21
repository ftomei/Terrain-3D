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

TARGET = TERRAIN_3D
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += _CRT_SECURE_NO_WARNINGS

CONFIG += c++11

INCLUDEPATH +=  gis

SOURCES += \
        main.cpp \
    gis/color.cpp \
    gis/gis.cpp \
    gis/gisIO.cpp

HEADERS += \
    gis/commonConstants.h \
    gis/color.h \
    gis/gis.h


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
