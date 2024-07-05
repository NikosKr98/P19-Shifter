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

#define CLUTCH_PADDLE_TARGET_MAP_SIZE		11		// the number of points for the rClutchPaddle - xClutchTargetManual map

#define CLUTCH_PADDLE_MAP_SIZE				2
#define SWITCHA_MAP_SIZE					14
#define CLUTCH_SERVO_MAP_SIZE				2

#define CLUTCH_PADDLE_MAP_MAX				104
#define CLUTCH_PADDLE_MAP_MIN				-4

#define CLUTCH_PADDLE_MAPS 					14
#define CLUTCH_PADDLE_MAP_OFFSETS 			13
#define CLUTCH_RELEASE_MAPS 				14
#define CLUTCH_RELEASE_MAP_POINTS			20
#define CLUTCH_TARGET_MAX_POINTS			14

#define CLUTCH_TARGET_MIN_DEF				0		// the default min clutch target
#define CLUTCH_TARGET_MAX_DEF				5		// the default max clutch target

#define CLUTCH_SERVO_MIN					1450
#define CLUTCH_SERVO_MAX					1600
#define CLUTCH_SERVO_ABSOLUTE_MIN			1400
#define CLUTCH_SERVO_ABSOLUTE_MAX			1700
#define CLUTCH_SERVO_ACTUATED				1480
// ------------------------------------------------------------------------------------------------------------------------------------------------------
// NGEAR

static const float NGearMap[2][TOTAL_GEARS] = {

		/* In:  VNGear */	{0.461, 1.226, 2.210, 2.763, 3.022, 3.339},
		/* Out: NGear  */	{  5,     4,     3,     2,     0,     1  }
};

static const float NGearRawLimsMinMap[TOTAL_GEARS] =

		/* 		VNGear */	{-0.20, 0.800, 1.800, 2.800, 3.800, 4.800};
		//		NGear		{  0,     1,     2,     3,     4,     5  }

static const float NGearRawLimsMaxMap[TOTAL_GEARS] =

		/* 		VNGear */	{0.200, 1.200, 2.200, 3.200, 4.200, 5.200};
		//		NGear		{  0,     1,     2,     3,     4,     5  }

// ------------------------------------------------------------------------------------------------------------------------------------------------------
// rClutchPaddle

static const float rClutchPaddle_xClutchTargetMaps[CLUTCH_PADDLE_MAPS][CLUTCH_PADDLE_TARGET_MAP_SIZE] = { // the various clutch maps

		/* 1:	Linear			*/		{0.00, 0.50, 1.00, 1.50, 2.00, 2.50, 3.00, 3.50, 4.00, 4.50, 5.00},
		/* 2:	Progressive 1	*/		{0.00, 1.00, 2.00, 3.00, 4.00, 4.17, 4.33, 4.50, 4.67, 4.83, 5.00},
		/* 3:	Progressive 2	*/		{0.00, 1.33, 2.67, 4.00, 4.14, 4.29, 4.43, 4.57, 4.71, 4.86, 5.00},
		/* 4:	Progressive 3	*/		{0.00, 2.00, 4.00, 4.13, 4.25, 4.38, 4.50, 4.63, 4.75, 4.88, 5.00},
		/* 5:	DeadBand 1		*/		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* 6:	Hill 1			*/		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* 7:	Hill 2			*/		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* 8:	S1				*/		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* 9:	S2				*/		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* 10:	Y1				*/		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* 11:	Y2				*/		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* 12:	Start 1			*/		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* 13:	Start 2			*/		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* 14:	Empty			*/		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

static const float rClutchPaddle_xClutchTargetOffsetMaps[CLUTCH_PADDLE_MAP_OFFSETS] = // the various clutch maps

								{ 0.7,  0.75,  0.8,  0.85,  0.9,  0.95,   1,  1.05,  1.1,  1.15,  1.2,  1.25,  1.3 };
		//		Offset %		{-30%,  -25%, -20%,	 -15%, -10%,   -5%,  0%,    5%,  10%,   15%,  20%,   25%,  30% }
		//		Position		{  1,     2,    3,     4,    5,     6,    7,     8,   9,    10,    11,   12,    13 }


static const float rClutchPaddleMap[2][CLUTCH_PADDLE_MAP_SIZE] = {

		/* In:  VrClutchPaddleRaw */	{		1.400			, 	 		2.470       },
		/* Out: rClutchPaddle 	  */	{ CLUTCH_PADDLE_MAP_MAX ,  CLUTCH_PADDLE_MAP_MIN}
};

// ------------------------------------------------------------------------------------------------------------------------------------------------------
// NSwitch

static const float NSWitchAmap[2][SWITCHA_MAP_SIZE] = {

		/* In:  VNSwitch 	*/	{ 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00 },
		/* Out: NSwitch 	*/	{	1,	  2	,	3 ,	  4 ,	5 ,	  6 ,	7 ,	  8 ,	9 ,	 10 ,  11 ,	 12 ,  13 ,	 14  }
};


// ------------------------------------------------------------------------------------------------------------------------------------------------------
// Shifting Times

static const uint32_t tUpShift[TOTAL_GEARS] = { 				100,	// neutral to 1st total shifting time (ms)
																100,	// 1st to 2nd
																100,	// 2nd to 3rd
																100,	// 3rd to 4th
																100,	// 4th to 5th
																0
};


static const uint32_t tDnShift[TOTAL_GEARS] = {					0,		// total shifting time (ms)
																10,		// 1st to neutral
																200,	// 2nd to 1st
																200,	// 3rd to 2nd
																200,	// 4th to 3rd
																200		// 5th to 4th
};

static const uint32_t tUpShiftDelayForClutch[TOTAL_GEARS] = {	80,		// delay before actuating the valves (ms)
																80,		// 1st to neutral
																80,		// 2nd to 1st
																80,		// 3rd to 2nd
																80,		// 4th to 3rd
																0		// 5th to 4th
};

static const uint32_t tDnShiftDelayForClutch[TOTAL_GEARS] = {	0,		// delay before actuating the valves (ms)
																80,		// 1st to neutral
																80,		// 2nd to 1st
																80,		// 3rd to 2nd
																80,		// 4th to 3rd
																80		// 5th to 4th
};

// ------------------------------------------------------------------------------------------------------------------------------------------------------
// nEngine

static const uint16_t nEngineAntistallMap[TOTAL_GEARS] = { 		0,		// min rpm threshold (not needed for neutral gear)
																800,	// 1st
																800,	// 2nd
																800,	// 3rd
																800,	// 4th
																800		// 5th
};

static const uint16_t nEngineUpShiftMap[TOTAL_GEARS] = { 		0,		// neutral to 1st gear min rpm threshold
																0,		// 1st to 2nd
																0,		// 2nd to 3rd
																0,		// 3rd to 4th
																0,		// 4th to 5th
																0
};


static const uint16_t nEngineDnShiftMap[TOTAL_GEARS] = {		0,		// gear max rpm threshold
																10000,	// 1st to neutral
																10000,	// 2nd to 1st
																10000,	// 3rd to 2nd
																10000,	// 4th to 3rd
																10000	// 5th to 4th
};

// ------------------------------------------------------------------------------------------------------------------------------------------------------
// xClutch

static const float xClutchTargetUpShiftMap[TOTAL_GEARS] = { 		3.00,	// neutral to 1st clutch target during upshift
																	3.00,	// 1st to 2nd
																	3.00,	// 2nd to 3rd
																	3.00,	// 3rd to 4th
																	3.00,	// 4th to 5th
																	0.00
};

static const float xClutchTargetDnShiftMap[TOTAL_GEARS] = {			0.00,		// clutch target during downshift
																	4.00,	// 1st to neutral
																	4.00,	// 2nd to 1st
																	4.00,	// 3rd to 2nd
																	4.00,	// 4th to 3rd
																	4.00	// 5th to 4th
};

static const float xClutchTargetMaxMap[CLUTCH_TARGET_MAX_POINTS] =

		/* 		xClutchTargetMax */	{5.0, 5.1, 5.2, 5.3, 5.4, 5.5, 5.6, 5.7, 5.8, 5.9, 6.0, 6.1, 6.2, 6.3};


static const float xClutchTarget_rServoDemandMap[2][CLUTCH_SERVO_MAP_SIZE] = {

		/* In:  xClutchTarget	*/	{ 	CLUTCH_TARGET_MIN_DEF  , 	CLUTCH_TARGET_MAX_DEF  },
		/* Out: rServoDemand	*/	{ CLUTCH_SERVO_ABSOLUTE_MAX, CLUTCH_SERVO_ABSOLUTE_MIN }
};


static const uint16_t xClutchReleaseMap[CLUTCH_RELEASE_MAPS][CLUTCH_RELEASE_MAP_POINTS] = {


		/* 1:	Basic (Easy)	*/		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* 2:	Dry Start 1		*/		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* 3:	Dry Start 2		*/		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* 4:	Dry Start 3		*/		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* 5:	Dry Start 4		*/		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* 6:	Wet Start 1		*/		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* 7:	Wet Start 2		*/		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* 8:	Empty			*/		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* 9:	Empty			*/		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* 10:	Empty			*/		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* 11:	Empty			*/		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* 12:	Empty			*/		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* 13:	Empty			*/		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* 14:	Empty			*/		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};


// ------------------------------------------------------------------------------------------------------------------------------------------------------

#endif /* INC_MAPS_H_ */
