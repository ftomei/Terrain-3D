#ifndef TERRAIN_H
#define TERRAIN_H

    #include "gis.h"
    #include <QString>

    class Terrain
    {
    public:
        Terrain();

        gis::Crit3DRasterGrid DTM;
        gis::Crit3DRasterGrid indexMap;
        gis::Crit3DRasterGrid slopeMap;
        gis::Crit3DRasterGrid aspectMap;

        bool loadDTM(QString myFileName);
        bool createIndexMap();
    };

#endif // TERRAIN_H
