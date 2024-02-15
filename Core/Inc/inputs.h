/*
 * input.h
 *
 *  Created on: Jan 22, 2024
 *      Author: NikosKr
 */

#ifndef INC_INPUTS_H_
#define INC_INPUTS_H_

#include <Utils.h>

// CAN
#define STEERING_RX_ID 0x310
#define ECU_RX_ID 0x311

#define STEERING_WHEEL_FITTED_INTERVAL		500		// after this time that we have not seen the message from SW we declare it not fitted/dead
#define ECU_COMMS_LOST_INTERVAL				500		// after this time that we have not seen the message from the ECU we declare it not fitted/dead

// GEAR
#define VNGEAR_MARGIN_MIN 					0.2f	// the voltage below the min map voltage we accept to arrive before declaring out of bounds
#define VNGEAR_MARGIN_MAX 					0.2f	// the voltage above the max map voltage we accept to arrive before declaring out of bounds


// CLUTCH
#define CLUTCH_PADDLE_PRESSED_THRESHOLD 	80		// Threshold % to consider Clutch Paddle as pressed
#define CLUTCH_PADDLE_MIN					0		// min clutch paddle percentage !!!!! ATTENTION !!!!!, Changing these will affect the maps and the various controls! better to leave as is
#define CLUTCH_PADDLE_MAX 					100		// max clutch paddle percentage

#define ADC_BUFFER_SIZE 714							// is the size of the buffer, 2 times the samples needed for 1 cycle
#define ADC_BUFFER_HALF_SIZE 357					// we use it to not do the division in run time

extern uint16_t adcRawValue[ADC_BUFFER_SIZE];

extern ADC_HandleTypeDef hadc1;
extern CAN_HandleTypeDef hcan;


/* EVENT DEFINITION */
typedef enum _Event {
	UPSHIFT_PRESS_EVT,
	UPSHIFT_RELEASE_EVT,
	DNSHIFT_PRESS_EVT,
	DNSHIFT_RELEASE_EVT,
	LAUNCH_PRESS_EVT,
	LAUNCH_RELEASE_EVT,
	EMERGENCY_PRESS_EVT,
	EMERGENCY_RELEASE_EVT,
	CLUTCH_PADDLE_PRESS_EVT,
	CLUTCH_PADDLE_RELEASE_EVT
} Event;


/* FAULT DEFINITION */
typedef enum _Fault {
	NGEAR_FAULT,
	STEERING_WHEEL_FAULT,
	UPSHIFT_REQUEST_FAULT,
	DNSHIFT_REQUEST_FAULT,
	LAUNCH_REQUEST_FAULT,
	CLUTCHPADDLE_FAULT,
	ECU_COMMS_FAULT
} Fault;

/* SIGNAL SOURCE  */
typedef enum _SigSource {
	CAN,
	Analog
} SignalSource;

typedef struct _InputStruct {
	uint32_t nEventStatus; 			// 32-bit bitfield for events
	uint32_t nFaultStatus; 			// 32-bit bitfield for faults


	// GEAR
	uint8_t BNGearInError;					// error flag for NGear
	float VNGearRaw;						// the voltage of the gear potentiometer
	float NGearRaw;							// raw gear value interpolated from the NGear 2D map
	uint8_t NGear;							// actual gear based on filtered gear potentiometer voltage and conditioned value

	uint8_t BDriverKill;					// 1 if the shutdown is open and 0 if it is closed (armed)

	// Shift Inputs
	uint8_t BUpShiftButtonCANInError;		// 1 if steering wheel CAN UpShift button is in Error
	uint8_t BUpShiftButtonCAN;				// steering wheel CAN UpShift button (reflects the state of the paddle)
	uint8_t BDnShiftButtonCANInError;		// 1 if steering wheel CAN DnShift button is in Error
	uint8_t BDnShiftButtonCAN;				// steering wheel CAN DnShift button (reflects the state of the paddle)

	uint8_t BUpDnShiftButtonAnalogInError;	// 1 if the internal Analog measurement of UpDnShift button is in error
	uint8_t NBUpDnShiftButtonAnalog;		// 0:none 1:Up 2:Dn Internal Analog measurement of UpShift button (reflects the state of the paddle)

	uint8_t BUpShiftRequestInError;			// 1 if UpShift request is in error
	uint8_t BUpShiftRequest;				// UpShift request (reflects the state of the paddle)
	uint8_t BDnShiftRequestInError;			// 1 if DnShift request is in error
	uint8_t BDnShiftRequest;				// DnShift request (reflects the state of the paddle)
	SignalSource NBUpshiftRequestSource;	// CAN or Analog
	SignalSource NBDnshiftRequestSource;	// CAN or Analog

	// Launch Button
	uint8_t BLaunchButtonCANInError;		// 1 if steering wheel CAN Launch control Button is in error
	uint8_t BLaunchButtonCAN;				// steering wheel CAN Launch control Button (reflects the state of the button)
	uint8_t BLaunchRequestInError;			// 1 if steering wheel CAN Launch control Button is in error
	uint8_t BLaunchRequest;					// Launch control Button (reflects the state of the button)

	// Emergency Button
	uint8_t BEmergencyButtonCANInError;		// 1 if steering wheel CAN Emergency Button is in error
	uint8_t BEmergencyButtonCAN;			// steering wheel CAN Emergency Button (reflects the state of the button)
	uint8_t BEmergencyButtonAnalogInError;	// 1 if Analog Emergency Button is in error
	uint8_t BEmergencyButtonAnalog;			// Analog Emergency Button (reflects the state of the button)
	uint8_t BEmergencyRequestInError;		// 1 if steering wheel CAN Emergency Button is in error
	uint8_t BEmergencyRequest;				// Emergency Button (reflects the state of the button)
	SignalSource NBEmergencyRequestSource;	// CAN or Analog

	// Clutch Paddles
	uint8_t BrClutchPaddleRawCANInError;	// 1 if CAN clutch paddle value is in error
	int8_t rClutchPaddleRawCAN;				// Steering wheel CAN clutch paddle percentage (can be from -x% to 10x% to allow margin)
	uint8_t BrClutchPaddleRawAnalogInError;	// 1 if internal Clutch Analog Measurement in Error
	int8_t rClutchPaddleRawAnalog;			// Internal Clutch Paddle Analog Measurement (can be from -x% to 10x% to allow margin)
	uint8_t BrClutchPaddleInError;			// 1 if Clutch Paddle is in error (both Analog and CAN)
	int8_t rClutchPaddle;					// Steering wheel clutch paddle Clipped percentage
	SignalSource NrClutchPaddleSource;		// can be CAN or Analog

	// ECU
	uint8_t BnEngineInError;		// flag to determine that the Engine rpm are not reliable
	uint8_t BnEngineReliable;		// 1 if the ECU message arrives correctly and the measurement is reliable
	int16_t nEngine;				// engine RPM taken from the ECU

	// CAN
	uint8_t BSteeringWheelFitted;	// 1 if the SW is fitted and the SIU is communicating, otherwise 0
	uint8_t NCANErrors;				// CAN Bus error count
	uint8_t NCANRxErrors;			// CAN message receive error count

	float VSupply;					// PCB Voltage Input Diagnostic

} InputStruct;

void InitInputs(void);
void ReadInputs(InputStruct *input);
uint8_t CheckFaults(InputStruct *inputs);

#endif /* INC_INPUTS_H_ */
