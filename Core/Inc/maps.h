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

#define CLUTCH_PADDLE_TARGET_MAP_MAX_SIZE	11		// the number of points for the rClutchPaddle - xClutchTargetManual map

#define CLUTCH_PADDLE_MAP_SIZE	2

#define CLUTCH_PADDLE_MAP_MAX		104
#define CLUTCH_PADDLE_MAP_MIN		-4


static const float NGearMap[2][TOTAL_GEARS] = {

		/* In:  VNGear */	{0.461, 1.226, 2.210, 2.763, 3.022, 3.339},
		/* Out: NGear  */	{  5,     4,     3,     2,     0,     1  }

};

//static const float rClutchPaddle_xClutchTargetMap[2][CLUTCH_PADDLE_TARGET_MAP_MAX_SIZE] = {
//
//	/* In:  rClutchPaddle */		{  0,   10,   20,   30,   40,   50,   60,   70,   80,   90,  100},
//	/* Out: xClutchTarget */		{900, 1100, 1300, 1500, 1700, 1750, 1800, 1850, 1900, 1950, 2100}
//};

static const float rClutchPaddle_xClutchTargetMap[2][CLUTCH_PADDLE_TARGET_MAP_MAX_SIZE] = {

	/* In:  rClutchPaddle */		{  0,   10,   20,   30,   40,   50,   60,   70,   80,   90,  100},
	/* Out: xClutchTarget */		{1720, 1688, 1656, 1624, 1592, 1560, 1528, 1496, 1464, 1432, 1400}
};

//BITE Point : 1580

static const float rClutchMap[2][CLUTCH_PADDLE_MAP_SIZE] = {

		/* In:  VrClutchPaddleRaw */	{		1.400			, 	 		2.470       },
		/* Out: rClutchPaddle 	  */	{ CLUTCH_PADDLE_MAP_MAX ,  CLUTCH_PADDLE_MAP_MIN}
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


static const uint32_t xClutchTargetUpShiftMap[TOTAL_GEARS-1] = { 	1400,	// neutral to 1st xClutchTarget
																	1400,	// 1st to 2nd
																	1400,	// 2nd to 3rd
																	1400,	// 3rd to 4th
																	1400,	// 4th to 5th
};

static const uint16_t xClutchTargetDnShiftMap[TOTAL_GEARS-1] = {	1400,		// 1st to neutral xClutchTarget (it is not needed here because we are pulling the clutch paddle)
																	1400,	// 2nd to 1st
																	1400,	// 3rd to 2nd
																	1400,	// 4th to 3rd
																	1400,	// 5th to 4th
};



static const uint32_t tUpShift[TOTAL_GEARS-1] = { 				100,	// neutral to 1st gear valve activation time (ms)
																300,	// 1st to 2nd
																300,	// 2nd to 3rd
																300,	// 3rd to 4th
																300,	// 4th to 5th
};


static const uint32_t tDnShift[TOTAL_GEARS-1] = {				5,	// 1st to neutral gear valve activation time (ms)
																300,	// 2nd to 1st
																300,	// 3rd to 2nd
																300,	// 4th to 3rd
																300,	// 5th to 4th
};


#endif /* INC_MAPS_H_ */
