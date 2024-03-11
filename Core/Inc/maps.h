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

		/* In:  VNGear */	{0.403, 1.128, 1.894, 2.619, 2.981, 3.183},
		/* Out: NGear  */	{  5,     4,     3,     2,     0,     1  }

};


static const float rClutchPaddle_xClutchTargetMap[2][CLUTCH_PADDLE_TARGET_MAP_MAX_SIZE] = {

	/* In:  rClutchPaddle */		{  0,   10,   20,   30,   40,   50,   60,   70,   80,   90,   100},
	/* Out: xClutchTarget */		{1100, 1180, 1260, 1340, 1420, 1500, 1580, 1660, 1720, 1800, 1900}
};

static const float rClutchMap[2][CLUTCH_PADDLE_MAP_SIZE] = {

		/* In:  VrClutchPaddleRaw */	{		0.500			, 	 		3.000	 	},
		/* Out: rClutchPaddle 	  */	{ CLUTCH_PADDLE_MAP_MIN ,  CLUTCH_PADDLE_MAP_MAX}
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
