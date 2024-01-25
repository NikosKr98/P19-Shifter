/*
 * output.h
 *
 *  Created on: Jan 22, 2024
 *      Author: NikosKr
 */

#ifndef INC_OUTPUT_H_
#define INC_OUTPUT_H_

#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "application.h"
#include "gpio_init.h"


#define shifting_interval 300
#define neutral_interval 10
#define STEERING_ID 0x320


extern uint32_t msg_interval, msg_previous, button_previous, button_interval, msg_interval,shift_end_time;
extern uint8_t TxData[8];
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
extern CAN_HandleTypeDef hcan;
extern CAN_TxHeaderTypeDef TxHeader;


void Output(ApplicationStruct *application);
void CAN_Tx(uint32_t ID, uint8_t dlc, uint8_t* data);
void shiftup_activation(ApplicationStruct *application);
void shiftdown_activation(ApplicationStruct *application);
void neutral_activation(ApplicationStruct *application);
void end_of_shift(ApplicationStruct *application);

#endif /* INC_OUTPUT_H_ */
