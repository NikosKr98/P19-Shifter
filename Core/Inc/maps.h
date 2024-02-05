/*
 * maps.h
 *
 *  Created on: Jan 29, 2024
 *      Author: orestis
 */

#ifndef INC_MAPS_H_
#define INC_MAPS_H_

#include <Utils.h>

#define TOTAL_GEARS 6
#define MAX_GEAR 5

#define CLUTCH_PADDLE_TARGET_MAP_MAX_POINTS	11		// the number of points for the rClutchPaddle - xClutchTargetManual map

//    {3600, 3750},  // Gear 0
//    {3900, 4000},  // Gear 1
//    {3200, 3300},  // Gear 2
//    {2250, 2400},  // Gear 3
//    {1300, 1500},  // Gear 4
//    {400, 600}     // Gear 5


static const float NGearMap[2][TOTAL_GEARS] = {

	/* NGear  */	{  0,     1,     2,     3,     4,     5  },
	/* VNGear */	{2.981, 3.183, 2.619, 1.894, 1.128, 0.403}
};


static const float rClutchPaddle_xClutchTargetMap[2][CLUTCH_PADDLE_TARGET_MAP_MAX_POINTS] = {

	/* NGear  */	{  0,   10,   20,   30,   40,   50,   60,   70,   80,   90,   100},
	/* VNGear */	{1500, 1575, 1650, 1725, 1800, 1816, 1832, 1848, 1864, 1880, 1900}
};



static const uint16_t nEngineAntistallMap[TOTAL_GEARS-1] = { 	2000,	// 1st min rpm threshold (not needed for neutral gear)
																2000,	// 2nd
																2000,	// 3rd
																2000,	// 4th
																2000,	// 5th
};

static const uint16_t nEngineUpShiftMap[TOTAL_GEARS-1] = { 		0,	// neutral to 1st gear min rpm threshold
																0,	// 1st to 2nd
																0,	// 2nd to 3rd
																0,	// 3rd to 4th
																0,	// 4th to 5th
};


static const uint16_t nEngineDnShiftMap[TOTAL_GEARS-1] = {		10000,	// 1st to neutral gear max rpm threshold
																10000,	// 2nd to 1st
																10000,	// 3rd to 2nd
																10000,	// 4th to 3rd
																10000,	// 5th to 4th
};


static const uint32_t xClutchTargetUpShiftMap[TOTAL_GEARS-1] = { 	10000,	// neutral to 1st xClutchTarget
																	10000,	// 1st to 2nd
																	10000,	// 2nd to 3rd
																	10000,	// 3rd to 4th
																	10000,	// 4th to 5th
};

static const uint16_t xClutchTargetDnShiftMap[TOTAL_GEARS-1] = {	10000,		// 1st to neutral xClutchTarget (it is not needed here because we are pulling the clutch paddle)
																	10000,	// 2nd to 1st
																	10000,	// 3rd to 2nd
																	10000,	// 4th to 3rd
																	10000,	// 5th to 4th
};



static const uint32_t tUpShift[TOTAL_GEARS-1] = { 				100,	// neutral to 1st gear valve activation time (ms)
																100,	// 1st to 2nd
																100,	// 2nd to 3rd
																100,	// 3rd to 4th
																100,	// 4th to 5th
};


static const uint32_t tDnShift[TOTAL_GEARS-1] = {				10,	// 1st to neutral gear valve activation time (ms)
																200,	// 2nd to 1st
																200,	// 3rd to 2nd
																200,	// 4th to 3rd
																200,	// 5th to 4th
};

#endif /* INC_MAPS_H_ */
