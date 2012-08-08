/* 
 * File:   globals.h
 * Author: Abotea
 *
 * Created on April 12, 2010, 4:42 PM
 */

#ifndef _GLOBALS_H
#define	_GLOBALS_H

#include <string>
#include <limits>

using namespace std;

#ifdef	__cplusplus
extern "C" {
#endif

    const int MAX_WIDTH = 1; //1000;
    const int MAX_HEIGHT = 1; //1000;
    const int NR_DIRECTIONS = 8;
    const int NORTH = 8;
    const int NORTHEAST = 1;
    const int EAST = 2;
    const int SOUTHEAST = 3;
    const int SOUTH = 4;
    const int SOUTHWEST = 5;
    const int WEST = 6;
    const int NORTHWEST = 7;
    const int NODIRECTION = 0;
    const int DIAGDISTANCE = 141;
    const int STRAIGHTDISTANCE = 100;

    const double SQRT2 = 1.414213;
    const double ONE = 1.0;
    
    const int MAX_NODE_DEGREE = 10;

    const string NOID = "noid";
    
    const unsigned int NO_VALUE = numeric_limits<unsigned int>::max();
    const int NR_COMPONENTS_CPD2 = 5;

#ifdef	__cplusplus
}
#endif

#endif	/* _GLOBALS_H */

