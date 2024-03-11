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
#define ALLOW_SPARK_CUT_ON_UP_SHIFT 		1		// spark cut during upshifts
#define ALLOW_SPARK_CUT_ON_DN_SHIFT 		0		// spark cut during dnshifts

#define ALLOW_NEUTRAL_WITHOUT_CLUTCH		0		// downshift from 1st to neutral without pulling the clutch paddle
#define ALLOW_FIRST_WITHOUT_CLUTCH			0		// upshift from neutral to 1st without pulling the clutch paddle
#define CLUTCH_ACTUATION_DURING_UPSHIFT		0		// clutch actuation during upshift
#define CLUTCH_ACTUATION_DURING_DNSHIFT		1		// clutch actuation during dnshift

#define ANTISTALL_ACTIVE					1		// antistall enable strategy
#define ANTISTALL_TRIGGER_TIME				1000	// antistall ms timeout for triggering
#define ANTISTALL_CLUTCHPADDLE_PRESSED		95		// the clutch paddle % we need to press the paddle to deactivate the antistall
#define ANTISTALL_CLUTCHPADDLE_RELEASED		40		// the clutch paddle % we need to have released the paddle for the antistall control to start working

// TIMING
#define PRE_UPSHIFT_THRESHOLD_TIME			100		// the time we keep trying to accept an upshift request before we deny it
#define PRE_DNSHIFT_THRESHOLD_TIME			300		// the time we keep trying to accept an downshift request before we deny it

// CLUTCH
#define CLUTCH_PADDLE_THRESHOLD_FOR_FIRST	90		// Threshold % of clutch paddle for upshift from neutral to first

#define xCLUTCH_DNSHIFT_TARGET				1700	// the clutch target opening during downshifts
#define xCLUTCH_REST_POSITION				1500	// the clutch position when not actuated
#define xCLUTCH_ABSOLUTE_MIN				1100	// min clutch position value
#define xCLUTCH_ABSOLUTE_MAX				1900	// max clutch position value



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
	NO_ERROR,
	NEUTRAL_TO_FIRST_WITH_NO_CLUTCH,
	FIRST_TO_NEUTRAL_WITH_NO_CLUTCH,
	RPM_ILLEGAL_FOR_UPSHIFT,
	TARGET_GEAR_EXCEEDS_MAX,
	RPM_ILLEGAL_FOR_DNSHIFT,
	TARGET_GEAR_LESS_THAN_NEUTRAL,
	SHIFT_TARGET_UNKNOWN
}ControlError;

typedef enum _Shifts{
	Up,
	Down,
	Unknown
}Shifts;

typedef enum _AntistallState{
	Off,
	Init,
	Active
} AntistallState;


typedef struct {

	ControlError NControlErrorStatus;
	ControlError NControlErrorStatusLogged;

	// GEAR
	uint8_t NGear;							// the current gear (copy from input struct)
	uint8_t NGearTarget;					// target gear for the controller

	// CLUTCH
	uint16_t xClutchTargetProtection;		// the clutch target opening requested from the protection/antistall strategy
	float xClutchTargetManual;				// the clutch target opening requested from the clutch pad
	uint16_t xClutchTargetShift;			// the clutch target opening requested from the shift control
	uint16_t xClutchTarget;					// the clutch target opening used for the servo control
	uint8_t BClutchActuated;				// 1 when the clutch is being actuated

	// SHIFTER
	uint8_t BUpShiftPortState;				// 1 when the UpShift port is being actuated
	uint8_t BDnShiftPortState;				// 1 when the DownShift port is being actuated
	uint32_t tLastUpShiftTransitTime_us;	// the time it actually took to shift the gear Up in the last actuation (for performance measurement)
	uint32_t tLastDnShiftTransitTime_us;	// the time it actually took to shift the gear Dn in the last actuation (for performance measurement)
	uint16_t NTotalShifts;					// total number of shifts done since powerup
	uint16_t NShiftsLeftEstimated;			// estimated number of shifts left

	// ECU
	uint8_t BSparkCut;						// flag to send to the ECU for spark cutting
	uint8_t BLaunchControl;					// flag to send to the ECU for launch control

	// STRATEGIES
	uint8_t BOverrideActuateClutchOnUpShift;	// if one it will actuate the clutch on the next UpShift, then it gets automatically disabled
	uint8_t BOverrideActuateClutchOnDnShift;	// if one it will actuate the clutch on the next DnShift, then it gets automatically disabled
	AntistallState NAntistallState;				// the state of the antistall strategy

	// PID Control
	uint8_t BPIDRunning;					// 1 when the PID controller is running
	uint8_t BPIDTimeout;					// 1 if the PID controller did not manage to reach the target before the timeout

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


void InitController(InputStruct *inputs, OutputStruct *outputs);
void Controller(InputStruct *inputs, OutputStruct *outputs);


#endif /* INC_CONTROLLER_H_ */
