/*!
    \copyright 2010-2016 Fausto Tomei, Gabriele Antolini,
    Alberto Pistocchi, Marco Bittelli, Antonio Volta, Laura Costantini

    This file is part of CRITERIA3D.
    CRITERIA3D has been developed under contract issued by A.R.P.A. Emilia-Romagna

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

    contacts:
    fausto.tomei@gmail.com
    ftomei@arpae.it
*/

#include <algorithm>
#include <math.h>
#include "commonConstants.h"
#include "color.h"


Crit3DColor::Crit3DColor()
{
    red = 0;
    green = 0;
    blue = 0;
}

Crit3DColor::Crit3DColor(short myRed, short myGreen, short myBlue)
{
    red = myRed;
    green = myGreen;
    blue = myBlue;
}

Crit3DColorScale::Crit3DColorScale()
{
    nrKeyColors = 1;
    nrColors = 1;
    keyColor = new Crit3DColor();
    color = new Crit3DColor();
    minimum = NODATA;
    maximum = NODATA;
    classification = classificationMethod::EqualInterval;
}

bool Crit3DColorScale::setRange(float myMinimum, float myMaximum)
{
    if (myMaximum < myMinimum) return false;

    minimum = myMinimum;
    maximum = myMaximum;
    return true;
}


bool Crit3DColorScale::classify()
{
    int i, j, n, nrIntervals;
    float step, dRed, dGreen, dBlue;

    if (classification == classificationMethod::EqualInterval)
    {
        nrIntervals = nrKeyColors - 1;
        step = float(nrColors) / float(nrIntervals);

        for (i = 0; i < nrIntervals; i++)
        {
            dRed = (keyColor[i+1].red - keyColor[i].red) / step;
            dGreen = (keyColor[i+1].green - keyColor[i].green) / step;
            dBlue = (keyColor[i+1].blue - keyColor[i].blue) / step;

            for (j = 0; j < step; j++)
            {
                n = int(step) * i + j;
                color[n].red = keyColor[i].red + short(dRed * j);
                color[n].green = keyColor[i].green + short(dGreen * j);
                color[n].blue = keyColor[i].blue + short(dBlue * j);
            }
        }
        color[nrColors-1] = keyColor[nrKeyColors -1];
    }

    return (true);
}


Crit3DColor* Crit3DColorScale::getColor(float myValue)
{
    int myIndex = 0;

    if (myValue <= minimum)
        myIndex = 0;
    else if (myValue >= maximum)
        myIndex = nrColors-1;
    else
        if (classification == classificationMethod::EqualInterval)
        {
            myIndex = int(round((nrColors-1) * ((myValue - minimum) / (maximum - minimum))));
        }

    return(&color[myIndex]);
}


int Crit3DColorScale::getColorIndex(float myValue)
{
    if (myValue <= minimum)
        return 0;
    else if (myValue >= maximum)
        return nrColors-1;
    else if (classification == classificationMethod::EqualInterval)
        return int(round((nrColors-1) * ((myValue - minimum) / (maximum - minimum))));
    else return 0;
}


bool setDefaultDTMScale(Crit3DColorScale* myScale)
{
    myScale->nrKeyColors = 4;
    myScale->nrColors = 256;
    myScale->keyColor = new Crit3DColor[unsigned(myScale->nrKeyColors)];
    myScale->color = new Crit3DColor[unsigned(myScale->nrColors)];
    myScale->classification = classificationMethod::EqualInterval;

    myScale->keyColor[0] = Crit3DColor(32, 160, 32);        /*!<  green */
    myScale->keyColor[1] = Crit3DColor(255, 255, 0);        /*!<  yellow */
    myScale->keyColor[2] = Crit3DColor(160, 64, 0);         /*!<  red */
    myScale->keyColor[3] = Crit3DColor(255, 255, 255);      /*!<  gray */

    return(myScale->classify());
}


/*!
 * \brief roundColorScale round colorScale values on the second (or third) digit of each range.
 * It requires that nrColors is a multiply of nrIntervals for a correct visualization in the colors legend.
 * It is projected for a legend of nrIntervals+1 levels (i.e= 4 intervals, 5 levels)
 * \param myScale
 * \param nrIntervals
 * \param lessRounded if true the round is on third digit
 * \return
 */
bool roundColorScale(Crit3DColorScale* myScale, int nrIntervals, bool lessRounded)
{
    if (myScale == nullptr)
        return false;
    if (myScale->minimum == NODATA || myScale->maximum == NODATA)
        return false;
    if (nrIntervals < 1)
        return false;
    if (myScale->minimum == myScale->maximum)
        return false;

    double avg = myScale->minimum + (myScale->maximum - myScale->minimum) * 0.5;
    double level = (myScale->maximum - myScale->minimum) / nrIntervals;

    double logLevel = log10(level);

    double myExp;
    double roundAvg = avg;

    if (avg == 0)
    {
        myExp = floor(logLevel)-1;
    }
    else
    {
        double logAvg = log10(avg);
        if (lessRounded)
            myExp = std::min(floor(logLevel)-1, floor(logAvg)-1);
        else
            myExp = std::max(floor(logLevel)-1, floor(logAvg)-1);
    }

    double pow10 = pow(10.0, myExp);
    if (avg != 0)
    {
        roundAvg = round(avg / pow10) * pow10;
    }

    double roundLevel = ceil(level / pow10) * pow10;

    myScale->minimum = float(roundAvg - roundLevel*(nrIntervals/2));
    myScale->maximum = float(roundAvg + roundLevel*(nrIntervals/2));
    return true;
}

