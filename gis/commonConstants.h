#ifndef COMMONCONSTANTS_H
#define COMMONCONSTANTS_H

    #ifndef minValue
        #define minValue(a, b) (((a) < (b))? (a) : (b))
    #endif

    #ifndef maxValue
        #define maxValue(a, b) (((a) > (b))? (a) : (b))
    #endif

    #ifndef NODATA
        #define NODATA -9999
    #endif

    // -----------------MATHEMATICS---------------------
    #ifndef PI
        #define PI 3.141592653589793238462643383
    #endif

    #ifndef EPSILON
        #define EPSILON 0.00001
    #endif

    #define DEG_TO_RAD 0.0174532925
    #define RAD_TO_DEG 57.2957795


#endif // COMMONCONSTANTS_H
