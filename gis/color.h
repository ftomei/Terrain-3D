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

#ifndef CRIT3DCOLOR_H
#define CRIT3DCOLOR_H

    namespace classificationMethod
    {
        enum type{EqualInterval, Gaussian, Quantile, Categories, UserDefinition };
    }

    class Crit3DColor {
    public:
        short red;
        short green;
        short blue;

        Crit3DColor();
        Crit3DColor(short,short,short);
    };

    class Crit3DColorScale {
    public:
        int nrColors, nrKeyColors;
        Crit3DColor *color, *keyColor;
        float minimum, maximum;
        int classification;

        Crit3DColorScale();
        bool classify();

        Crit3DColor* getColor(float myValue);
        int getColorIndex(float myValue);
        bool setRange(float myMinimum, float myMaximum);
    };

    bool setDefaultDTMScale(Crit3DColorScale* myScale);
    bool setTemperatureScale(Crit3DColorScale* myScale);
    bool setPrecipitationScale(Crit3DColorScale* myScale);
    bool setRelativeHumidityScale(Crit3DColorScale* myScale);
    bool setRadiationScale(Crit3DColorScale* myScale);
    bool setWindIntensityScale(Crit3DColorScale* myScale);
    bool setLeafWetnessScale(Crit3DColorScale* myScale);
    bool setZeroCenteredScale(Crit3DColorScale* myScale);
    bool roundColorScale(Crit3DColorScale* myScale, int nrIntervals, bool lessRounded);


#endif // CRIT3DCOLOR_H
