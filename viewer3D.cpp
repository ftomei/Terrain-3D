/*!
    \file viewer3D.cpp

    \abstract Viewer 3D widget

    This file is part of CRITERIA3D.

    CRITERIA3D has been developed by A.R.P.A.E. Emilia-Romagna.

    \copyright
    CRITERIA3D is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    CRITERIA3D is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public License
    along with CRITERIA3D.  If not, see <http://www.gnu.org/licenses/>.

    \authors
    Fausto Tomei ftomei@arpae.it
*/

#include "commonConstants.h"
#include "glWidget.h"
#include "viewer3D.h"

#include <QSlider>
#include <QLabel>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>


Viewer3D::Viewer3D(Crit3DGeometry *myGeometry)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Terrain 3D"));

    QHBoxLayout *glLayout = new QHBoxLayout;
    glWidget = new Crit3DOpenGLWidget(myGeometry);
    glLayout->addWidget(glWidget);
    turnSlider = verticalSlider(0, 360 * DEGREE_MULTIPLY, DEGREE_MULTIPLY, 15 * DEGREE_MULTIPLY);
    glLayout->addWidget(turnSlider);

    QHBoxLayout *rotateLayout = new QHBoxLayout;
    QLabel *rotateLabel = new QLabel("Rotation:");
    rotateLayout->addWidget(rotateLabel);
    rotateSlider = horizontalSlider(0, 360 * DEGREE_MULTIPLY, DEGREE_MULTIPLY, 15 * DEGREE_MULTIPLY);
    rotateLayout->addWidget(rotateSlider);

    QHBoxLayout *magnifyLayout = new QHBoxLayout;
    QLabel *magnifyLabel = new QLabel("Magnify: ");
    magnifyLayout->addWidget(magnifyLabel);
    magnifySlider = horizontalSlider(1, 100, 1, 5);
    magnifyLayout->addWidget(magnifySlider);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(glLayout);
    mainLayout->addLayout(rotateLayout);
    mainLayout->addLayout(magnifyLayout);
    setLayout(mainLayout);

    QStatusBar *statusBar = new QStatusBar(this);
    mainLayout->addWidget(statusBar);

    connect(turnSlider, &QSlider::valueChanged, glWidget, &Crit3DOpenGLWidget::setXRotation);
    connect(glWidget, &Crit3DOpenGLWidget::xRotationChanged, turnSlider, &QSlider::setValue);
    connect(rotateSlider, &QSlider::valueChanged, glWidget, &Crit3DOpenGLWidget::setZRotation);
    connect(glWidget, &Crit3DOpenGLWidget::zRotationChanged, rotateSlider, &QSlider::setValue);
    connect(magnifySlider, &QSlider::valueChanged, glWidget, &Crit3DOpenGLWidget::setMagnify);

    turnSlider->setValue(30 * DEGREE_MULTIPLY);
    rotateSlider->setValue(0 * DEGREE_MULTIPLY);
    magnifySlider->setValue(myGeometry->magnify() * 10);
}


QSlider* Viewer3D::verticalSlider(int minimum, int maximum, int step, int tick)
{
    QSlider *slider = new QSlider(Qt::Vertical);
    slider->setRange(minimum, maximum);
    slider->setSingleStep(step);
    slider->setPageStep(tick);
    slider->setTickInterval(tick);
    slider->setTickPosition(QSlider::TicksRight);
    return slider;
}


QSlider* Viewer3D::horizontalSlider(int minimum, int maximum, int step, int tick)
{
    QSlider *slider = new QSlider(Qt::Horizontal);
    slider->setRange(minimum, maximum);
    slider->setSingleStep(step);
    slider->setPageStep(tick);
    slider->setTickInterval(tick);
    slider->setTickPosition(QSlider::TicksBelow);
    return slider;
}

