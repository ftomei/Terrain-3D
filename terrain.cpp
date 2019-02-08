#include "terrain.h"


Terrain::Terrain() {}


bool Terrain::loadDTM(QString myFileName)
{
    std::string* myError = new std::string();
    std::string fileName = myFileName.left(myFileName.length()-4).toStdString();

    if (! gis::readEsriGrid(fileName, &DTM, myError))
    {
        qDebug("Load raster failed!");
        return (false);
    }

    if (! gis::computeSlopeAspectMaps(DTM, &slopeMap, &aspectMap))
    {
        qDebug("Error in computeSlopeAspectMaps!");
        return (false);
    }

    setDefaultDTMScale(DTM.colorScale);

    this->createIndexMap();

    return (true);
}


bool Terrain::createIndexMap()
{
    if (! DTM.isLoaded)
    {
        qDebug("Missing DTM.");
        return false;
    }

    indexMap.initializeGrid(*(DTM.header));

    long index = 0;
    for (int row = 0; row < indexMap.header->nrRows; row++)
    {
        for (int col = 0; col < indexMap.header->nrCols; col++)
        {
            if (int(DTM.value[row][col]) != int(DTM.header->flag))
            {
                indexMap.value[row][col] = float(index);
                index++;
            }
        }
    }

    gis::updateMinMaxRasterGrid(&indexMap);
    indexMap.isLoaded = true;
    return true;
}
