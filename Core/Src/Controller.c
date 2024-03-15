/*
 * app.c
 *
 *  Created on: Jan 22, 2024
 *      Author: NikosKr
 */

#include <Controller.h>
#include <Maps.h>

// Local Structs
InputStruct *MyInputs;
OutputStruct *MyOutputs;

States NCurrentState, NPreviousState;

Shifts NShiftRequest;



// timing variables
uint32_t tControllerTimmer, tPreShiftTimer, tShiftTimer, tShifterMaxTransitTime, tAntistallTimmer;

#define CheckEvent(event_) (MyInputs->nEventStatus >> (uint32_t)(event_)) & 0x1
#define CheckFault(fault_) (MyInputs->nFaultStatus >> (uint32_t)(fault_)) & 0x1

#define RaiseControlError(fault_) {do{ MyOutputs->NControlErrorStatus |= (1 << (uint32_t)(fault_)); MyOutputs->NControlErrorStatusLogged = fault_; }while(0);}
#define ClearControlError(fault_) MyOutputs->NControlErrorStatus &= ~(1 << (uint32_t)(fault_))
#define CheckControlError(fault_) (MyOutputs->NControlErrorStatus >> (uint32_t)(fault_)) & 0x1


void InitController(InputStruct *inputs, OutputStruct *outputs) {

	MyInputs = inputs;
	MyOutputs = outputs;

	IDLE_Entry();
}



void Controller(InputStruct *inputs, OutputStruct *outputs){

	tControllerTimmer = HAL_GetTick();

	// ------------------------------------------------------------------------------------------------------------------------------------------------------

	// ANTISTALL
		#ifdef ANTISTALL_ACTIVE

			// if the shut down is activated and we are at gear greater than neutral
			if(!MyInputs->BDriverKill && MyInputs->NGear > 0 && !MyInputs->BNGearInError && !MyInputs->BnEngineInError) {

				if(MyOutputs->NAntistallState != Active && MyInputs->nEngine <= nEngineAntistallMap[MyInputs->NGear] && MyInputs->rClutchPaddle < ANTISTALL_CLUTCHPADDLE_RELEASED) {
					// Timer initialization of enable strategy
					if(MyOutputs->NAntistallState == Off) {
						MyOutputs->NAntistallState = Init;
						tAntistallTimmer = HAL_GetTick();
					}
					// Activation
					if(MyOutputs->NAntistallState == Init && (tAntistallTimmer + ANTISTALL_TRIGGER_TIME) < tControllerTimmer) {
						MyOutputs->NAntistallState = Active;
						MyOutputs->xClutchTargetProtection = xCLUTCH_ABSOLUTE_MAX;
					}
				}
				// Not activation due to engine rpm returning over the limit, or early clutch paddle press
				if(MyOutputs->NAntistallState == Init && (MyInputs->nEngine > nEngineAntistallMap[MyInputs->NGear] || MyInputs->rClutchPaddle > ANTISTALL_CLUTCHPADDLE_PRESSED)) {
					MyOutputs->NAntistallState = Off;
					MyOutputs->xClutchTargetProtection = 0;
				}
				// De-activation by Clutch paddle press
				if(MyOutputs->NAntistallState == Active && MyInputs->rClutchPaddle > ANTISTALL_CLUTCHPADDLE_PRESSED) {
					MyOutputs->NAntistallState = Off;
					MyOutputs->xClutchTargetProtection = 0;
				}
			}
			// De-activation by Driver Kill or Neutral or Errors
			else {
				MyOutputs->NAntistallState = Off;
				MyOutputs->xClutchTargetProtection = 0;
			}
		#endif

	// ------------------------------------------------------------------------------------------------------------------------------------------------------

	// CLUTCH CONTROLLER

		// Manual target mapping
		if(!MyInputs->BrClutchPaddleInError) {
			My2DMapInterpolate(CLUTCH_PADDLE_TARGET_MAP_MAX_SIZE, rClutchPaddle_xClutchTargetMap, MyInputs->rClutchPaddle, &MyOutputs->xClutchTargetManual, 0, 0);
			// TODO: terminate potential array timed control that runs below
		}
		else {
			if(CheckEvent(DECLUTCH_RELEASE_EVT)) {
				// TODO: run the release array. here we initialize it
				// create lookup table running request for accel and emergency clutchButton release
			}
			// TODO: Here we keep timers and counter and the state of the mini control and put the values in xClutchTargetManual
		}

		// TODO: do the array running thing also for the launch sequence.
		// Decide if upshifts trigger will happen here, or we will be triggered in IDLE and start the clutch sequence here afterwards

		// we take the maximum target generated from the Antistall/Protection strategy, the request
		// from the driver and the shifter requests when enabled from the respective strategy
		MyOutputs->xClutchTarget = MAX(MyOutputs->xClutchTargetProtection, MAX((uint16_t)MyOutputs->xClutchTargetManual, MyOutputs->xClutchTargetShift));


	// ------------------------------------------------------------------------------------------------------------------------------------------------------

	// TOGGLE SWITCHES & LEDS
		MyOutputs->BSWLEDA = MyInputs->NToggleSwitch01State;
		MyOutputs->BSWLEDB = MyInputs->NToggleSwitch02State;
		MyOutputs->BSWLEDC = MyInputs->NToggleSwitch03State;


	// ------------------------------------------------------------------------------------------------------------------------------------------------------

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
		MyOutputs->NGearTarget = MyInputs->NGear + 1;											// we go to the next gear

		if(CLUTCH_ACTUATION_DURING_UPSHIFT || MyOutputs->BOverrideActuateClutchOnUpShift) {		// we check for clutch strategy during shift
			MyOutputs->xClutchTargetShift = xClutchTargetUpShiftMap[MyInputs->NGear];
			MyOutputs->BOverrideActuateClutchOnUpShift = 0; 									// reset the strat for the next gear
		}
		else {
			MyOutputs->xClutchTargetShift = 0;
		}

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

	if(MyInputs->NGear == 0 && MyInputs->rClutchPaddle <= CLUTCH_PADDLE_THRESHOLD_FOR_FIRST && !ALLOW_FIRST_WITHOUT_CLUTCH) {	// trying to put 1st gear without clutch
		RaiseControlError(NEUTRAL_TO_FIRST_WITH_NO_CLUTCH);
	}
	else { ClearControlError(NEUTRAL_TO_FIRST_WITH_NO_CLUTCH); }

	if(MyInputs->nEngine < nEngineUpShiftMap[MyInputs->NGear] && !MyInputs->BnEngineInError) {									// trying to shift up with too low rpm
		RaiseControlError(RPM_ILLEGAL_FOR_UPSHIFT);
	}
	else { ClearControlError(RPM_ILLEGAL_FOR_UPSHIFT); }

	if(MyInputs->NGear + 1 > MAX_GEAR)	{																					// trying to shift up after last gear
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
		MyOutputs->NGearTarget = MyInputs->NGear - 1;												// we go to the previous gear

		if(CLUTCH_ACTUATION_DURING_DNSHIFT || MyOutputs->BOverrideActuateClutchOnDnShift) {		// we check for clutch strategy during shift
			MyOutputs->xClutchTargetShift = xClutchTargetDnShiftMap[MyInputs->NGear];
			MyOutputs->BOverrideActuateClutchOnDnShift = 0; 									// reset the strat for the next gear
		}
		else {
			MyOutputs->xClutchTargetShift = 0;
		}

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

	if(MyInputs->nEngine > nEngineDnShiftMap[MyInputs->NGear] && !MyInputs->BnEngineInError) {									// trying to shift down with too high rpm
		RaiseControlError(RPM_ILLEGAL_FOR_DNSHIFT);
	}
	else { ClearControlError(RPM_ILLEGAL_FOR_DNSHIFT); }

	if(MyInputs->NGear == 0)	{																								// trying to shift down from neutral
		RaiseControlError(TARGET_GEAR_LESS_THAN_NEUTRAL);
	}
	else { ClearControlError(TARGET_GEAR_LESS_THAN_NEUTRAL); }

}

void SHIFTING_Entry(void) {
	NPreviousState = NCurrentState;
	NCurrentState = SHIFTING_STATE;

	if(NPreviousState == PRE_UPSHIFT_STATE) {
		tShifterMaxTransitTime = tUpShift[MyInputs->NGear];
		NShiftRequest = Up;

		if(MyOutputs->NGearTarget == 1) {		// if going from neutral to 1st we need to actually downshift (it is how the gears work)
			MyOutputs->BDnShiftPortState = 1;
		}
		else {									// all other upshifts are normal
			MyOutputs->BUpShiftPortState = 1;
		}

	}
	else if(NPreviousState == PRE_DNSHIFT_STATE) {
		tShifterMaxTransitTime = tDnShift[MyInputs->NGear];
		NShiftRequest = Down;

		if(MyOutputs->NGearTarget == 0) {		// if going from 1st to neutral we need to actually upshift (it is how the gears work)
			MyOutputs->BUpShiftPortState = 1;
		}
		else {									// all other downshifts are normal
			MyOutputs->BDnShiftPortState = 1;
		}

	}
	else {
		NCurrentState = Unknown;
		RaiseControlError(SHIFT_TARGET_UNKNOWN);
	}

	tShiftTimer = HAL_GetTick();
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


	if((tShiftTimer + tShifterMaxTransitTime) < tControllerTimmer) {	// the max time for the gear has expired
		// go out and determine if the shift was completed or not
		SHIFTING_Exit();
		POSTSHIFT_Entry();
		return;
	}

}
void SHIFTING_Run(void) {

	// based on the error status and the srat preferences decide in which controller to enter



	// PID


	// FEED FORWARD

}


void POSTSHIFT_Entry(void) {
	NPreviousState = NCurrentState;
	NCurrentState = POSTSHIFT_STATE;

	// reset all actuator states
	MyOutputs->BUpShiftPortState = 0;
	MyOutputs->BDnShiftPortState = 0;

	// reset all control variables for the next actuation
	MyOutputs->xClutchTargetShift = 0;
	MyOutputs->BSparkCut = 0;

}
void POSTSHIFT_Exit(void) {

	// TODO: probably here we need to set the MyOutputs->NGear = MyInputs->NGear

}
void POSTSHIFT_Event(void) {


	// think about the condition
	POSTSHIFT_Exit();
	IDLE_Entry();
	return;
	// remember return in all functions
}
void POSTSHIFT_Run(void) {

	// maybe we need to spend sometime here to let the shifting system stabilize and then determine the new current gear
}


void ERROR_Entry(void) {
	NPreviousState = NCurrentState;
	NCurrentState = ERROR_STATE;

	// TODO: evaluate if it is correct to stop all output actions here...maybe not
	// clutch should always work... if we enter here during an actuation, not sure if it is correct to interrupt it
}

void ERROR_Exit(void) {

}
void ERROR_Event(void) {

	// check that all faults are cleared
	if(!CheckFaults(MyInputs)) {
		ERROR_Exit();
		IDLE_Entry();
		return;
	}
		// for some faults that are very critical we could make a counter and when it expires we declare a default hardcoded value to be able to move on
	// TODO: it must not be completely blocking to be able to comeback from an error.
	// the concept is to keep a counter for the number of errors of each type and after a certain point come back and continue normal running with less features
	// check that all control errors are cleared
	// and do not zero the logged error status
	// remember return in all functions
}
void ERROR_Run(void) {

	MyOutputs->NControlErrorStatus = 0;


	// TODO: find a way to read the Control Errors and then reset them in order to clear them for the next cycle

}
