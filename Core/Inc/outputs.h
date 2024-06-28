/*
 * output.h
 *
 *  Created on: Jan 22, 2024
 *      Author: NikosKr
 */

#ifndef INC_OUTPUTS_H_
#define INC_OUTPUTS_H_

#include <Controller.h>
#include <Utils.h>

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern CAN_HandleTypeDef hcan;


#define SHIFTER_TX_ID01 	0x320
#define SHIFTER_TX_ID02 	0x321
#define SHIFTER_TX_ID03 	0x322
#define SHIFTER_TX_ID04 	0x323

#define CAN_TX_TIMEOUT 200		// us of CAN timeout when sending a frame

// DIGITAL OUT
	// DO01: CLUTCH PWM
	// DO02: VALVE2 (DOWN)
	// DO03: VALVE1 (UP)
	// DO04: ECU (SPRACK CUT)

void InitOutputs(void);
void WriteOutputs(InputStruct *inputs, OutputStruct *outputs);


#endif /* INC_OUTPUTS_H_ */
