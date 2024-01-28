/*
 * application.h
 *
 *  Created on: Jan 22, 2024
 *      Author: NikosKr
 */

#ifndef INC_APP_H_
#define INC_APP_H_

#include "inputs.h"

#define clutch_detection_threshold 1000

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

typedef enum {
	SHIFTER_IDLE,
	SHIFTER_PRESHIFT,
	SHIFTER_SHIFTING,
	SHIFTER_POSTSHIFT,
	SHIFTER_ERROR
}States;


void InitApplication(void);
void RunApplication(InputStruct *input, OutputStruct *output);


void IDLE_Entry(void);
void IDLE_Run(void);
void IDLE_Exit(void);

void PRESHIFT_Entry(void);
void PRESHIFT_Run(void);
void PRESHIFT_Exit(void);

void SHIFTING_Entry(void);
void SHIFTING_Run(void);
void SHIFTING_Exit(void);

void POSTSHIFT_Entry(void);
void POSTSHIFT_Run(void);
void POSTSHIFT_Exit(void);

void ERROR_Entry(void);
void ERROR_Run(void);
void ERROR_Exit(void);

#endif /* INC_APP_H_ */
