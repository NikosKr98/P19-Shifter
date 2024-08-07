/*
 * application.h
 *
 *  Created on: Jan 22, 2024
 *      Author: NikosKr
 */

#ifndef INC_CONTROLLER_H_
#define INC_CONTROLLER_H_

#include <Inputs.h>

// GEAR STRATEGIES
#define ALLOW_NEUTRAL_WHEN_PADDLE_IN_ERROR	1		// allow downshift from 1st to neutral when all Clutch paddle inputs are in error
#define ALLOW_NEUTRAL_WITHOUT_CLUTCH		0		// downshift from 1st to neutral without pulling the clutch paddle
#define ALLOW_FIRST_WITHOUT_CLUTCH			0		// upshift from neutral to 1st without pulling the clutch paddle
#define ALLOW_GEARS_WITH_CAR_STOPPED		1		// we allow changing gear with the car stopped and the clutch paddle pressed
#define ALLOW_END_OF_SHIFT_ON_GEAR_MATCH	0		// we allow to end a gear change before the defined shift time if we see that the gear is matching with the target for some time

// SHIFT STRATEGIES
#define ALLOW_SPARK_CUT_ON_UP_SHIFT 		1		// allow spark cut during upshifts strategie, enabled by multifunction
#define ALLOW_SPARK_CUT_ON_DN_SHIFT 		1		// allow spark cut during dnshifts strategie, enabled by multifunction
#define ALLOW_CLUTCH_ACT_DURING_UPSHIFT		1		// allow clutch actuation during upshift strategy, enabled by multifunction
#define ALLOW_CLUTCH_ACT_DURING_DNSHIFT		1		// allow clutch actuation during dnshift strategy, enabled by multifunction
#define CHECK_POST_SHIFT_GEAR				1		// during the post shift phase we check if the current gear has become equal to the target

// ANTISTALL
#define ANTISTALL_ENABLED					1		// antistall enable strategy
#define ANTISTALL_MIN_ACTIVATION_GEAR		1		// the gear from which we enable antistall
#define ANTISTALL_TRIGGER_TIME				500		// time for antistall to be triggered
#define ANTISTALL_CLUTCHPADDLE_PRESSED		95		// the clutch paddle % we need to press the paddle to deactivate the antistall
#define ANTISTALL_CLUTCHPADDLE_RELEASED		40		// the clutch paddle % we need to have released the paddle for the antistall control to start working

// TIMING
#define PRE_UPSHIFT_THRESHOLD_TIME			100		// the time we keep trying to accept an upshift request before we deny it
#define PRE_DNSHIFT_THRESHOLD_TIME			300		// the time we keep trying to accept an downshift request before we deny it
#define POSTSHIFT_THRESHOLD_TIME			500		// the time we allow for all the conditions to return to nominal post shifting
#define GEAR_MATCH_EARLY_THRESHOLD_TIME		50		// the time we wait for the gears to remain matched to declare that they are actually the same

// CLUTCH PADDLE
#define CLUTCH_PADDLE_THRESHOLD_FOR_FIRST	90		// Threshold % of clutch paddle for upshift from neutral to first
#define CLUTCH_PADDLE_ALLOW_OFFSET_MIN		5		// the threshold below (and equal) which we do not apply
#define CLUTCH_PADDLE_ALLOW_OFFSET_MAX		95		// the threshold below (and equal) which we do not apply

// CLUTCH
#define CLUTCH_TARGET_MIN_MARGIN			0.0f	// the margin that we accept for the servo demand map, below the map's min value
#define CLUTCH_TARGET_MAX_MARGIN			15.0f	// the margin that we accept for the servo demand map, abovbe the map's maximum value

// MULTIFUNCTION
#define ALLOW_MULTIFUNC_WITH_NO_ACTIVE_TIME	0		// allow the use of +/- buttons all the times with no need or the rotary to turn first. This strategy deactivates the screen page function
#define NMF									12		// the number of multifunction maps (must be the same as the rotary positions)
#define MULTIFUNCTION_ACTIVE_TIME			2000	// the time the display shows the map position and value and the buttons work as +/-

#define MULTIFUNCTION_NEXT_BUTTON			inputs->BSWButtonD
#define MULTIFUNCTION_PREV_BUTTON			inputs->BSWButtonC

// Max position										//		ATTENTION	!! (do not exceed the array size, currently 14)
#define MULTIFUNCTION01_MAX_POS				14		// the maximum position (size) of each map (select value = 0 to deactivate the map)
#define MULTIFUNCTION02_MAX_POS				14
#define MULTIFUNCTION03_MAX_POS				13
#define MULTIFUNCTION04_MAX_POS				14
#define MULTIFUNCTION05_MAX_POS				13
#define MULTIFUNCTION06_MAX_POS				4
#define MULTIFUNCTION07_MAX_POS				4
#define MULTIFUNCTION08_MAX_POS				2
#define MULTIFUNCTION09_MAX_POS				14
#define MULTIFUNCTION10_MAX_POS				14
#define MULTIFUNCTION11_MAX_POS				14
#define MULTIFUNCTION12_MAX_POS				14

// Default position
#define MULTIFUNCTION01_DEF_POS				1		// the default positions of the maps on power up
#define MULTIFUNCTION02_DEF_POS				1
#define MULTIFUNCTION03_DEF_POS				7
#define MULTIFUNCTION04_DEF_POS				1
#define MULTIFUNCTION05_DEF_POS				7
#define MULTIFUNCTION06_DEF_POS				3
#define MULTIFUNCTION07_DEF_POS				1
#define MULTIFUNCTION08_DEF_POS				1
#define MULTIFUNCTION09_DEF_POS				1
#define MULTIFUNCTION10_DEF_POS				1
#define MULTIFUNCTION11_DEF_POS				1
#define MULTIFUNCTION12_DEF_POS				1

// Wrapping Enable
#define MULTIFUNCTION01_WRAP				0		// 1 to enable wrapping of the map
#define MULTIFUNCTION02_WRAP				0
#define MULTIFUNCTION03_WRAP				0
#define MULTIFUNCTION04_WRAP				0
#define MULTIFUNCTION05_WRAP				0
#define MULTIFUNCTION06_WRAP				0
#define MULTIFUNCTION07_WRAP				0
#define MULTIFUNCTION08_WRAP				1
#define MULTIFUNCTION09_WRAP				1
#define MULTIFUNCTION10_WRAP				1
#define MULTIFUNCTION11_WRAP				1
#define MULTIFUNCTION12_WRAP				1

// Multifunction associations
#define MULTIFUNCTION_CLUTCH_TARGET_MAX_IDX		1
#define MULTIFUNCTION_CLUTCH_PADDLE_MAP_IDX		2
#define MULTIFUNCTION_CLUTCH_PADDLE_OFFSET_IDX	3
#define MULTIFUNCTION_CLUTCH_RELEASE_MAP_IDX	4
#define MULTIFUNCTION_CLUTCH_RELEASE_OFFSET_IDX	5
#define MULTIFUNCTION_UPSHIFT_TYPE_IDX			6
#define MULTIFUNCTION_DNSHIFT_TYPE_IDX			7

// TOGGLE SWITCHES
#define TOGGLE_SWITCH_DEBOUNCE				1000	// time interval for next toggle
#define TOGGLE_SWITCH01_BUTTON				inputs->BSWButtonE
#define TOGGLE_SWITCH02_BUTTON				inputs->BSWButtonA
#define TOGGLE_SWITCH03_BUTTON				inputs->BSWButtonB
#define TOGGLE_SWITCH04_BUTTON				inputs->BSWButtonB

// DISPLAY
#define DISPLAY_MAX_PAGE					5		// the maximum page number we have in the screen, not the index
#define DISPLAY_PAGE_BUTTON_DEBOUNCE		200		// debounce time for consecutive page changes
#define DISPLAY_NEXT_BUTTON					inputs->BSWButtonD
#define DISPLAY_PREV_BUTTON					inputs->BSWButtonC

#define CONTROLLER_STATUS_SHADOW_REFRESH	1000	// the time we accumulate all controller errors before zeroing them (for display reasons)

#define DISPLAY_MULTIFUNCTION_PAGE			6		// the pop up multifunction page
#define DISPLAY_ERROR_PAGE					7		// the pop up error page
#define DISPLAY_CLUTCH_PAGE					8		// the pop up page on clutch paddle activation

#define DISPLAY_ERROR_PAGE_INTERVAL			1500	// the time the error page will be active in the screen on every error occurrence
#define DISPLAY_CLUTCH_PAGE_INTERVAL		1000	// the time the clutch paddle page will be active in the screen while the clutch paddle value is inside the defined limits
#define DISPLAY_CLUTCH_PAGE_CLUTCH_MIN		5		// the min clutch paddle value to enter the clutch screen and the min to exit after the interval

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
	SHIFT_TARGET_UNKNOWN,
	GEAR_TARGET_MISMATCH,
	FALSE_NEUTRAL_WITH_NO_CLUTCH
}ControlError;

typedef enum _Shifts{
	Up,
	Down,
	Unknown
}Shifts;

typedef enum _ShiftType{
	WithClutchAndSparkCut,
	WithClutch,
	SparkCut,
	NoClutchNoSparkCut
}ShiftType;

typedef enum _AntistallState{
	Off,
	Init,
	Active
} AntistallState;


typedef struct {

	uint32_t NControlErrorStatus;		// bit word of the various controller errors
	uint32_t NControlErrorStatusShadow;	// it gets used for the diagnostics and thud gets zeroed after 1 second, in order to be able to see the various errors
	ControlError NControlErrorStatusLogged;	// it keeps the last error seen

	// GEAR
	uint8_t NGearTarget;					// target gear for the controller
	uint8_t BNGearMatch;					// used for shifting control, indicates that NGear matched the target gear

	// CLUTCH
	float xClutchTargetProtection;			// the clutch target opening requested from the protection/antistall strategy
	float xClutchTargetManual;				// the clutch target opening requested from the clutch pad
	float xClutchTargetShift;				// the clutch target opening requested from the shift control
	float xClutchTargetShiftShadow;			// the shadow variable used in order to save the target and applied at the right moment
	float xClutchTarget;					// the clutch target opening used for the servo control
	float xClutchBitepoint;					// the clutch bite point
	float xClutchTargetMin;					// the min opening point (can be changed from multifunction and strategies)
	float xClutchTargetMax;					// the max opening point (can be changed from multifunction and strategies)
	uint8_t BClutchActuated;				// 1 when the clutch is being actuated
	uint8_t NxClutchPaddleMapIdx;			// the index for the clutch paddle maps
	uint8_t NxClutchPaddleOffsetIdx;		// the index for the clutch paddle offset
	uint8_t NxClutchReleaseMapIdx;			// the index for the clutch release maps
	uint8_t NxClutchReleaseOffsetIdx;		// the index for the clutch release offset

	// SHIFTER
	uint8_t BUpShiftPortState;				// 1 when the UpShift port is being actuated
	uint8_t BUpShiftPortStateShadow;		// the shadow variable in order to control the timing
	uint8_t BDnShiftPortState;				// 1 when the DownShift port is being actuated
	uint8_t BDnShiftPortStateShadow;		// the shadow variable in order to control the timing
	uint32_t tLastUpShiftTransitTime_us;	// the time it actually took to shift the gear Up in the last actuation (for performance measurement)
	uint32_t tLastDnShiftTransitTime_us;	// the time it actually took to shift the gear Dn in the last actuation (for performance measurement)
	uint16_t NTotalShifts;					// total number of shifts done since power up
	uint16_t NShiftsLeftEstimated;			// estimated number of shifts left
	uint8_t BShiftInProgress;				// 1 when the state machine is performing a shift
	ShiftType NUpShiftType;					// UpShift type index selected from the multifunction
	ShiftType NDnShiftType;					// DpShift type index selected from the multifunction

	// Steering Wheel LED
	uint8_t BSWLEDA;
	uint8_t BSWLEDB;
	uint8_t BSWLEDC;
	uint8_t BSWLEDD;

	// Toggle Switches
	uint8_t NToggleSwitch01State;
	uint8_t NToggleSwitch02State;
	uint8_t NToggleSwitch03State;
	uint8_t NToggleSwitch04State;

	// Display Buttons & control
	int8_t NDispalyPage;					// the currently selected display page
	int8_t NDispalyPagePrev;				// the previous page, used for smart page switching
	uint8_t NDisplayFlags;					// flags for display messages and diagnostics
	uint8_t BDisplayPageNext;				// the button state from the input structure
	uint8_t BDisplayPagePrev;				// the button state from the input structure
	uint8_t BDisplayPageNextState;			// the state of the Next page button for control reasons
	uint8_t BDisplayPagePrevState;			// the state of the Prev page button for control reasons
	uint32_t tDisplayPageDebounce;			// timer for display control debouncing

	// Multifunction
	uint8_t NMultifunctionActiveSwitch;		// copy of NSwitchA from inputs
	uint8_t BMultifunctionNextPos;			// the button state from the input structure
	uint8_t BMultifunctionPrevPos;			// the button state from the input structure
	uint8_t BMultifunctionNextPosState;		// the state of the Next page button for control reasons
	uint8_t BMultifunctionPrevPosState;		// the state of the Next page button for control reasons

	uint8_t NMultifunctionDefMask[NMF];		// the default values of the multifunction on power up
	int8_t NMultifunction[NMF];				// the currently selected values for each map
	uint8_t BMultifunctionWrap[NMF];		// if it contains 1 we allow the overflow of the map (from last we go to first with next button pressed)
	uint8_t NMultifunctionMaxPos[NMF];		// the size of each map, used for wrapping and general control
	uint8_t BMultifunctionOverride[NMF];	// if a position contains 1 the mapped function does not get changed by the multifunction, used for toggles and for bloching strategies
	uint32_t tMultifunctionActiveOnRot;		// the time the screen shows the multifunction map position and value & the time the SW buttons function as +/- instead of display page control
	uint8_t BUseButtonsForMultifunction;	// a flag to indicate the use of the +/- buttons for multifunction and display page control

	// ECU
	uint8_t BSparkCut;						// flag to send to the ECU for spark cutting
	uint8_t BLaunchControl;					// flag to send to the ECU for launch control
	uint8_t NLaunchControlState;			// the state of the Launch controller

	// STRATEGIES
	uint8_t BOverrideActuateClutchOnNextUpShift;	// if one it will actuate the clutch on the next UpShift, then it gets automatically disabled
	uint8_t BOverrideActuateClutchOnNextDnShift;	// if one it will actuate the clutch on the next DnShift, then it gets automatically disabled
	AntistallState NAntistallState;			// the state of the antistall strategy

	// SERVO OUTPUT
	float rServoDemandRaw;					// the raw timer value from the map interpolation
	uint16_t rServoDemand;					// the timer value we use to command the SERVO PWM

	// PID Control
	uint8_t BPIDRunning;					// 1 when the PID controller is running
	uint8_t BPIDTimeout;					// 1 if the PID controller did not manage to reach the target before the timeout

	uint32_t NControllerStatusWord;			// bit word for flags and various status bits (


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
