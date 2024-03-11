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

#define shifting_interval 500
#define neutral_interval 10

#define DISPLAY_TX_ID 0x320

#define CAN_TX_TIMEOUT 200		// us of CAN timeout when sending a frame

// DIGITAL OUT
	// DO01: CLUTCH PWM
	// DO02: VALVE2 (DOWN)
	// DO03: VALVE1 (UP)
	// DO04: ECU (SPACK CUT)

void InitOutputs(void);
void WriteOutputs(OutputStruct *output);


#endif /* INC_OUTPUTS_H_ */
