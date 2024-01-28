/*
 * output.h
 *
 *  Created on: Jan 22, 2024
 *      Author: NikosKr
 */

#ifndef INC_OUTPUTS_H_
#define INC_OUTPUTS_H_

#include "inputs.h"


#define shifting_interval 500
#define neutral_interval 10
#define STEERING_ID 0x320

typedef struct {

	int8_t current_gear;
	int8_t target_gear;
	uint8_t up_port_state;
	uint8_t down_port_state;
	uint32_t shift_end_time;
	uint8_t up_button_out;
	uint8_t down_button_out;
	uint8_t up_button_status;
	uint8_t down_button_status;
	uint16_t clutch_position;
	uint32_t actual_gear;
	uint8_t clutch_detection;


	uint8_t BUpShiftPortActuated;	// 1 when the UpShift port is being actuated
	uint8_t BDownShiftPortActuated;	// 1 when the DownShift port is being actuated
	uint8_t NGear;					// the current gear (copy from input struct)
	uint8_t BClutchActuated;		// 1 when the clutch is being actuated
	uint32_t tLastShift_us;			// time of last shift in usec

}OutputStruct;

void InitOutputs(void);
void WriteOutputs(OutputStruct *output);


#endif /* INC_OUTPUTS_H_ */
