/*
 * maps.h
 *
 *  Created on: Jan 29, 2024
 *      Author: orestis
 */

#ifndef INC_MAPS_H_
#define INC_MAPS_H_

#include "utils.h"


//uint16_t NGearMap[TOTAL_GEARS][2] = {	// TODO: needs to go in maps.h file
//
//    {3600, 3750},  // Gear 0
//    {3900, 4000},  // Gear 1
//    {3200, 3300},  // Gear 2
//    {2250, 2400},  // Gear 3
//    {1300, 1500},  // Gear 4
//    {400, 600}     // Gear 5
//
//};

uint16_t nEngineUpShiftMap[TOTAL_GEARS] = { 0,	// neutral to 1st gear min threshold
											0,	// 1st to 2nd
											0,	// 2nd to 3rd
											0,	// 3rd to 4th
											0,	// 4th to 5th
};


uint16_t nEngineDnShiftMap[TOTAL_GEARS] = {	10000,	// 1st to neutral gear max threshold
												10000,	// 2nd to 1st
												10000,	// 3rd to 2nd
												10000,	// 4th to 3rd
												10000,	// 5th to 4th
};



#endif /* INC_MAPS_H_ */
