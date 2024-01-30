/*
 * input.h
 *
 *  Created on: Jan 22, 2024
 *      Author: NikosKr
 */

#ifndef INC_INPUTS_H_
#define INC_INPUTS_H_

#include "utils.h"

// CAN
#define STEERING_RX_ID 0x310
#define ECU_RX_ID 0x311

// GEAR
#define TOTAL_GEARS 6
#define MAX_GEAR 5


// CLUTCH
#define CLUTCH_PADDLE_PRESSED_THRESHOLD 	10		// TODO: to be tuned
#define CLUTCH_PADDLE_MIN					0		// min clutch paddle percentage
#define CLUTCH_PADDLE_MAX 					100		// max clutch paddle percentage
#define CLUTCH_PADDLE_THRESHOLD_FOR_FIRST	60		// threshold for upshift from neutral to first

#define ADC_BUFFER_SIZE 375*2						// is the size of the buffer, 2 halves of 306 samples
#define ADC_BUFFER_HALF_SIZE ADC_BUFFER_SIZE/2		// we use it to do the division in compile time and not in run time

extern uint16_t adcRawValue[ADC_BUFFER_SIZE];

extern ADC_HandleTypeDef hadc1;
extern CAN_HandleTypeDef hcan;


/* EVENT DEFINITION */
typedef enum _Event {
	UPSHIFT_PRESS_EVT,
	DNSHIFT_PRESS_EVT,
	LAUNCH_PRESS_EVT,
	CLUTCH_PADDLE_PRESS_EVT
} Event;

/* FAULT DEFINITION */
typedef enum _Fault {
	NGEAR_IN_ERROR_FAULT,
	BOTH_PADS_PRESSED_FAULT,
	UPSHIFT_CAN_FAULT,
	DNSHIFT_CAN_FAULT,
	UPSHIFT_ANALOG_FAULT,
	DNSHIFT_ANALOG_FAULT,
	CLUTCHPADDLE_CAN_FAULT,
	CLUTCHPADDLE_ANALOG_FAULT,
} Fault;

typedef struct _InputStruct {
	uint32_t nEventStatus; 		// 32-bit bitfield for events
	uint32_t nFaultStatus; 		// 32-bit bitfield for faults

	uint8_t NGear;				// actual gear based on filtered gear potentiometer
	uint8_t BNGearInError;		// error flag for NGear
	uint8_t BUpShiftRequest;	// steering wheel UpShift request (reflects the state of the paddle)
	uint8_t BDnShiftRequest;	// steering wheel DownShift request (reflects the state of the paddle)
	uint8_t BLaunchRequest;		// steering wheel Launch control  request (reflects the state of the button)
	int8_t rClutchPaddleRaw;	// Steering wheel clutch paddle percentage (can be from -4% to 104% to allow margin)
	int8_t rClutchPaddle;		// Steering wheel clutch paddle Clipped percentage
	int16_t nEngine;			// engine RPM taken from the ECU

	uint8_t NCANErrors;			// CAN Bus error count
	uint8_t NCANRxErrors;		// CAN message receive error count

	float VSupply;				// PCB Voltage Input Diagnostic

} InputStruct;

void InitInputs(void);
void ReadInputs(InputStruct *input);
uint8_t CheckFaults(InputStruct *inputs);

#endif /* INC_INPUTS_H_ */
