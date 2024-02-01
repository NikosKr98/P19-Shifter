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

extern TIM_HandleTypeDef htim2;
extern CAN_HandleTypeDef hcan;

#define shifting_interval 500
#define neutral_interval 10
#define STEERING_ID 0x320



void InitOutputs(void);
void WriteOutputs(OutputStruct *output);


#endif /* INC_OUTPUTS_H_ */
