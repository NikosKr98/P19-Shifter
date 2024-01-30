/*
 * app.c
 *
 *  Created on: Jan 22, 2024
 *      Author: NikosKr
 */

#include "app.h"
#include "maps.h"

// Local Structs
InputStruct *MyInputs; // TODO: they need to go in app.h
OutputStruct *MyOutputs;

States NCurrentState, NPreviousState;

//Shifts NShiftRequest;

// Control variables
uint8_t NGearTarget=0;
uint16_t xClutchTarget=0;

uint32_t tPreShiftThreshold=0;

#define CheckEvent(event_) (MyInputs->nEventStatus >> (uint32_t)(event_)) & 0x1
#define CheckFault(fault_) (MyInputs->nFaultStatus >> (uint32_t)(fault_)) & 0x1

#define RaiseControlError(fault_) MyOutputs->NControlErrorStatus |= (1 << (uint32_t)(fault_))
#define ClearControlError(fault_) MyOutputs->NControlErrorStatus &= ~(1 << (uint32_t)(fault_))
#define CheckControlError(fault_) (MyOutputs->NControlErrorStatus >> (uint32_t)(fault_)) & 0x1


void InitApplication(InputStruct *inputs, OutputStruct *outputs) {

	MyInputs = inputs;
	MyOutputs = outputs;

	IDLE_Entry();
}



void RunApplication(InputStruct *inputs, OutputStruct *outputs){

//	myInputs = inputs;   // TODO: previously here... we should not need to do the copy every time, they are pointers
//	myOutputs = outputs;


	// SHIFTER STATE MACHINE
	switch (NCurrentState) {

	case IDLE_STATE:
		IDLE_Run();
		IDLE_Event();
		break;
	case PRE_UPSHIFT_STATE:
		PRE_UPSHIFT_Run();
		PRE_UPSHIFT_Event();
		break;
	case PRE_DNSHIFT_STATE:
		PRE_DNSHIFT_Run();
		PRE_DNSHIFT_Event();
		break;
	case SHIFTING_STATE:
		SHIFTING_Run();
		SHIFTING_Event();
		break;
	case POSTSHIFT_STATE:
		POSTSHIFT_Run();
		POSTSHIFT_Event();
		break;
	case ERROR_STATE:
		ERROR_Run();
		ERROR_Event();
		break;
	}


}


void IDLE_Entry(void) {
	NPreviousState = NCurrentState;
	NCurrentState = IDLE_STATE;
}
void IDLE_Exit(void) {

}
void IDLE_Event(void) {

	if(CheckFaults(MyInputs)) {		// TODO: not sure how to handle this...how do we come back if we see faults?
		IDLE_Exit();
		ERROR_Entry();
		return;
	}


    if(CheckEvent(UPSHIFT_PRESS_EVT)) {
        IDLE_Exit();
        PRE_UPSHIFT_Entry();
        return;
    }

    if(CheckEvent(DNSHIFT_PRESS_EVT)) {
        IDLE_Exit();
        PRE_DNSHIFT_Entry();
        return;
    }

    if(CheckEvent(LAUNCH_PRESS_EVT)) {
//        IDLE_Exit();
//        LAUNCH_Entry();
//        return;
    }

}
void IDLE_Run(void) {

	 // TODO: Clutch handle with Rpm check and conditioning (Map)

}



void PRE_UPSHIFT_Entry(void) {
	NPreviousState = NCurrentState;
	NCurrentState = PRE_UPSHIFT_STATE;

	tPreShiftThreshold = HAL_GetTick();
}
void PRE_UPSHIFT_Exit(void) {

}
void PRE_UPSHIFT_Event(void) {

	// if all ok we define the shifting targets and move on
	if(!MyOutputs->NControlErrorStatus) {
		NGearTarget = MyInputs->NGear + 1;		// we go to the next gear
		xClutchTarget = 0;						// we do not need any clutch opening
		PRE_UPSHIFT_Exit();
		SHIFTING_Entry();
		return;
	}

	// we check for control errors and if present after the time threshold, we abort
	if(MyOutputs->NControlErrorStatus && (tPreShiftThreshold + PRE_UPSHIFT_THRESHOLD_TIME) <= HAL_GetTick()) {
		PRE_UPSHIFT_Exit();
		ERROR_Entry();
		return;
	}


}
void PRE_UPSHIFT_Run(void) {

	if(MyInputs->NGear == 0 && MyInputs->rClutchPaddle <= CLUTCH_PADDLE_THRESHOLD_FOR_FIRST)
		RaiseControlError(NEUTRAL_TO_FIRST_WITH_NO_CLUTCH);
	else
		ClearControlError(NEUTRAL_TO_FIRST_WITH_NO_CLUTCH);

	if(MyInputs->nEngine < nEngineUpShiftMap[MyInputs->NGear])
		RaiseControlError(RPM_ILLEGAL_FOR_UPSHIFT);
	else
		ClearControlError(RPM_ILLEGAL_FOR_UPSHIFT);

	if(MyInputs->NGear + 1 > TOTAL_GEARS)
		RaiseControlError(TARGET_GEAR_EXCEEDS_MAX);
	else
		ClearControlError(TARGET_GEAR_EXCEEDS_MAX);
}



void PRE_DNSHIFT_Entry(void) {
	NPreviousState = NCurrentState;
	NCurrentState = PRE_DNSHIFT_STATE;
}
void PRE_DNSHIFT_Exit(void) {

}
void PRE_DNSHIFT_Event(void) {

}
void PRE_DNSHIFT_Run(void) {


}

void SHIFTING_Entry(void) {
	NPreviousState = NCurrentState;
	NCurrentState = SHIFTING_STATE;
}
void SHIFTING_Exit(void) {

}
void SHIFTING_Event(void) {

}
void SHIFTING_Run(void) {

	// remember to copy NGear from input struct to output struct because we need it in outputs
	// TODO: attention to xClutch , (always??? take the max of Manualtarget and ControlTarget)
}


void POSTSHIFT_Entry(void) {
	NPreviousState = NCurrentState;
	NCurrentState = POSTSHIFT_STATE;
}
void POSTSHIFT_Exit(void) {

}
void POSTSHIFT_Event(void) {

}
void POSTSHIFT_Run(void) {

}


void ERROR_Entry(void) {
	NPreviousState = NCurrentState;
	NCurrentState = ERROR_STATE;
}
void ERROR_Exit(void) {

}
void ERROR_Event(void) {

}
void ERROR_Run(void) {

	// TODO: find a way to read the Control Errors and then reset them in order to clear them for the next cycle
}
