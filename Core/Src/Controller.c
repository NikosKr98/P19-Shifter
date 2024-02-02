/*
 * app.c
 *
 *  Created on: Jan 22, 2024
 *      Author: NikosKr
 */

#include <Controller.h>
#include <Maps.h>

// Local Structs
InputStruct *MyInputs; // TODO: they need to go in app.h
OutputStruct *MyOutputs;

States NCurrentState, NPreviousState;

Shifts NShiftRequest;




uint32_t tPreShiftTimer=0;

#define CheckEvent(event_) (MyInputs->nEventStatus >> (uint32_t)(event_)) & 0x1
#define CheckFault(fault_) (MyInputs->nFaultStatus >> (uint32_t)(fault_)) & 0x1

#define RaiseControlError(fault_) {do{ MyOutputs->NControlErrorStatus |= (1 << (uint32_t)(fault_)); MyOutputs->NControlErrorStatusLogged = (1 << (uint32_t)(fault_)); }while(0);}
#define ClearControlError(fault_) MyOutputs->NControlErrorStatus &= ~(1 << (uint32_t)(fault_))
#define CheckControlError(fault_) (MyOutputs->NControlErrorStatus >> (uint32_t)(fault_)) & 0x1


void InitController(InputStruct *inputs, OutputStruct *outputs) {

	MyInputs = inputs;
	MyOutputs = outputs;

	IDLE_Entry();
}



void Controller(InputStruct *inputs, OutputStruct *outputs){

//	myInputs = inputs;   // TODO: previously here... we should not need to do the copy every time, they are pointers
//	myOutputs = outputs;


	// ANTISTALL
		// make the check that we are not actually pressing the clutch...if yes disable the strat
	// make check if rpm =0 to not be active all the time if the car is stuck with gear inside and 0 rpm
	// only if BDriverKill is active (shutdown is closed (armed)). Think about other scenarios

	// CLUTCH CONTROLLER
		// TODO: attention to xClutch , (always??? take the max of Manualtarget and ControlTarget)




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

	if(CheckFaults(MyInputs)) {
		IDLE_Exit();
		ERROR_Entry();
		return;
	}

	// TODO: do we need to also check controller errors here? I think no...

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

}



void PRE_UPSHIFT_Entry(void) {
	NPreviousState = NCurrentState;
	NCurrentState = PRE_UPSHIFT_STATE;

	tPreShiftTimer = HAL_GetTick();
}
void PRE_UPSHIFT_Exit(void) {

}
void PRE_UPSHIFT_Event(void) {

	if(CheckFaults(MyInputs)) {
		PRE_UPSHIFT_Exit();
		ERROR_Entry();
		return;
	}

	// if all ok we define the shifting targets and move on
	if(!MyOutputs->NControlErrorStatus) {
		MyOutputs->NGearTarget = MyInputs->NGear + 1;			// we go to the next gear
		MyOutputs->xClutchTargetShift = 0;						// we do not need any clutch opening

		if(ALLOW_SPARK_CUT_ON_UP_SHIFT) MyOutputs->BSparkCut = 1;

		PRE_UPSHIFT_Exit();
		SHIFTING_Entry();
		return;
	}

	// we check for control errors and if present after the time threshold, we abort
	if(MyOutputs->NControlErrorStatus && (tPreShiftTimer + PRE_UPSHIFT_THRESHOLD_TIME) <= HAL_GetTick()) {
		PRE_UPSHIFT_Exit();
		ERROR_Entry();
		return;
	}


}
void PRE_UPSHIFT_Run(void) {

	if(MyInputs->NGear == 0 && MyInputs->rClutchPaddle <= CLUTCH_PADDLE_THRESHOLD_FOR_FIRST) {	// trying to put 1st gear without clutch
		RaiseControlError(NEUTRAL_TO_FIRST_WITH_NO_CLUTCH);
	}
	else { ClearControlError(NEUTRAL_TO_FIRST_WITH_NO_CLUTCH); }

	if(MyInputs->nEngine < nEngineUpShiftMap[MyInputs->NGear]) {		// trying to shift up with too low rpm
		RaiseControlError(RPM_ILLEGAL_FOR_UPSHIFT);
	}
	else { ClearControlError(RPM_ILLEGAL_FOR_UPSHIFT); }

	if(MyInputs->NGear + 1 > TOTAL_GEARS)	{	// trying to shift up after last gear
		RaiseControlError(TARGET_GEAR_EXCEEDS_MAX);
	}
	else { ClearControlError(TARGET_GEAR_EXCEEDS_MAX); }
}



void PRE_DNSHIFT_Entry(void) {
	NPreviousState = NCurrentState;
	NCurrentState = PRE_DNSHIFT_STATE;

	tPreShiftTimer = HAL_GetTick();
}
void PRE_DNSHIFT_Exit(void) {

}
void PRE_DNSHIFT_Event(void) {

	if(CheckFaults(MyInputs)) {
		PRE_DNSHIFT_Exit();
		ERROR_Entry();
		return;
	}

	// if all ok we define the shifting targets and move on
	if(!MyOutputs->NControlErrorStatus) {
		MyOutputs->NGearTarget = MyInputs->NGear - 1;								// we go to the previous gear
		MyOutputs->xClutchTargetShift = xClutchTargetDnShiftMap[MyOutputs->NGear];	// we set the xClutch target to the target of the current gear

		if(ALLOW_SPARK_CUT_ON_DN_SHIFT) MyOutputs->BSparkCut = 1;

		PRE_DNSHIFT_Exit();
		SHIFTING_Entry();
		return;
	}

	// we check for control errors and if present after the time threshold, we abort
	if(MyOutputs->NControlErrorStatus && (tPreShiftTimer + PRE_DNSHIFT_THRESHOLD_TIME) <= HAL_GetTick()) {
		PRE_DNSHIFT_Exit();
		ERROR_Entry();
		return;
	}

}
void PRE_DNSHIFT_Run(void) {

	if(MyInputs->NGear == 1 && MyInputs->rClutchPaddle <= CLUTCH_PADDLE_THRESHOLD_FOR_FIRST && !ALLOW_NEUTRAL_WITHOUT_CLUTCH) {	// trying to put neutral gear without clutch
		RaiseControlError(FIRST_TO_NEUTRAL_WITH_NO_CLUTCH);
	}
	else { ClearControlError(FIRST_TO_NEUTRAL_WITH_NO_CLUTCH); }

	if(MyInputs->nEngine > nEngineDnShiftMap[MyInputs->NGear]) {		// trying to shift down with too high rpm
		RaiseControlError(RPM_ILLEGAL_FOR_DNSHIFT);
	}
	else { ClearControlError(RPM_ILLEGAL_FOR_DNSHIFT); }

	if(MyInputs->NGear == 0)	{	// trying to shift down from neutral
		RaiseControlError(TARGET_GEAR_LESS_THAN_NEUTRAL);
	}
	else { ClearControlError(TARGET_GEAR_LESS_THAN_NEUTRAL); }

}

void SHIFTING_Entry(void) {
	NPreviousState = NCurrentState;
	NCurrentState = SHIFTING_STATE;

	if(NPreviousState == PRE_UPSHIFT_STATE) NShiftRequest = Up;
	else if(NPreviousState == PRE_DNSHIFT_STATE) NShiftRequest = Down;
	else {
		NCurrentState = Unknown;
		RaiseControlError(SHIFT_TARGET_UNKNOWN);
	}
}
void SHIFTING_Exit(void) {

}
void SHIFTING_Event(void) {

	if(CheckFaults(MyInputs)) {
		SHIFTING_Exit();
		ERROR_Entry();
		return;
	}

	// TODO: keep checking for control errors
}
void SHIFTING_Run(void) {

	// run the timer for each gear/up-down (there are tables for that) and NShiftRequest tells us the direction

}


void POSTSHIFT_Entry(void) {
	NPreviousState = NCurrentState;
	NCurrentState = POSTSHIFT_STATE;

	// reset all control variables for the next actuation
	MyOutputs->xClutchTargetShift = 0;
	MyOutputs->BSparkCut = 0;

}
void POSTSHIFT_Exit(void) {

	// TODO: probably here we need to set the MyOutputs->NGear = MyInputs->NGear

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

	// check that all faults are cleared
		// for some faults that are very critical we could make a counter and when it expires we declare a default hardcoded value to be able to move on
	// TODO: it must not be completely blocking to be able to comeback from an error.
	// the concept is to keep a counter for the number of errors of each type and after a certain point come back and continue normal running with less features
	// check that all control errors are cleared
	// and do not zero the logged error status

}
void ERROR_Run(void) {

	// TODO: find a way to read the Control Errors and then reset them in order to clear them for the next cycle
}
