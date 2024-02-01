/*
 * application.h
 *
 *  Created on: Jan 22, 2024
 *      Author: NikosKr
 */

#ifndef INC_CONTROLLER_H_
#define INC_CONTROLLER_H_

#include <Inputs.h>


// STRATEGIES
#define ALLOW_SPARK_CUT_ON_UP_SHIFT 1		// flag to allow/not allow spark cut during upshifts




// TIMING
#define PRE_UPSHIFT_THRESHOLD_TIME	100		// the time we keep trying to accept an upshift request before we deny it
#define PRE_DNSHIFT_THRESHOLD_TIME	300		// the time we keep trying to accept an downshift request before we deny it

// CLUTCH
#define CLUTCH_DNSHIFT_TARGET		100		// the clutch target opening during downshifts


// STATE MACHINE STATES
typedef enum _States {
	IDLE_STATE,
	PRE_UPSHIFT_STATE,
	PRE_DNSHIFT_STATE,
	SHIFTING_STATE,
	POSTSHIFT_STATE,
	ERROR_STATE
}States;


// CONTROL ERRORS
typedef enum _ControlError {
	NEUTRAL_TO_FIRST_WITH_NO_CLUTCH,
	RPM_ILLEGAL_FOR_UPSHIFT,
	TARGET_GEAR_EXCEEDS_MAX,
	RPM_ILLEGAL_FOR_DNSHIFT,
	TARGET_GEAR_LESS_THAN_NEUTRAL,
}ControlError;

//typedef enum _Shifts{
//	Up,
//	Down
//}Shifts;


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

	ControlError NControlErrorStatus;


	// GEAR
	uint8_t NGear;							// the current gear (copy from input struct)
	uint8_t NGearTarget;					// target gear for the controller

	// CLUTCH
	uint16_t xClutchTargetProtection;		// the clutch target opening requested from the protection/antistall strategy
	uint16_t xClutchTargetManual;			// the clutch target opening requested from the clutch pad
	uint16_t xClutchTargetShift;			// the clutch target opening requested from the shift control
	uint8_t BClutchActuated;				// 1 when the clutch is being actuated

	// SHIFTER
	uint8_t BUpShiftPortActuated;			// 1 when the UpShift port is being actuated
	uint8_t BDownShiftPortActuated;			// 1 when the DownShift port is being actuated
	uint32_t tLastUpShiftTransitTime_us;	// the time it actually took to shift the gear Up in the last actuation (for performance measurement)
	uint32_t tLastDnShiftTransitTime_us;	// the time it actually took to shift the gear Dn in the last actuation (for performance measurement)
	uint16_t NTotalShifts;					// total number of shifts done since powerup
	uint16_t NShiftsLeftEstimated;			// estimated number of shifts left

	// ECU
	uint8_t BSparkCut;						// flag to send to the ECU for spark cutting


}OutputStruct;


void IDLE_Entry(void);
void IDLE_Exit(void);
void IDLE_Event(void);
void IDLE_Run(void);

void PRE_UPSHIFT_Entry(void);
void PRE_UPSHIFT_Exit(void);
void PRE_UPSHIFT_Event(void);
void PRE_UPSHIFT_Run(void);

void PRE_DNSHIFT_Entry(void);
void PRE_DNSHIFT_Exit(void);
void PRE_DNSHIFT_Event(void);
void PRE_DNSHIFT_Run(void);

void SHIFTING_Entry(void);
void SHIFTING_Exit(void);
void SHIFTING_Event(void);
void SHIFTING_Run(void);

void POSTSHIFT_Entry(void);
void POSTSHIFT_Exit(void);
void POSTSHIFT_Event(void);
void POSTSHIFT_Run(void);

void ERROR_Entry(void);
void ERROR_Exit(void);
void ERROR_Event(void);
void ERROR_Run(void);


void InitApplication(InputStruct *inputs, OutputStruct *outputs);
void RunApplication(InputStruct *inputs, OutputStruct *outputs);


#endif /* INC_CONTROLLER_H_ */
