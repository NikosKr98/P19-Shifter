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

uint8_t NMultifunctionActiveSwitchPrev, NMFIdx;

// timing variables
uint32_t tControllerTimmer, tPreShiftTimer, tShiftTimer, tShifterMaxTransitTime, tPostShiftTimer, tAntistallTimmer, tControllerErrorStatusShadow;

#define CheckEvent(event_) (MyInputs->nEventStatus >> (uint32_t)(event_)) & 0x1
#define CheckFault(fault_) (MyInputs->nFaultStatus >> (uint32_t)(fault_)) & 0x1

#define RaiseControlError(fault_) {do{ MyOutputs->NControlErrorStatus |= (1 << (uint32_t)(fault_)); MyOutputs->NControlErrorStatusLogged = fault_; MyOutputs->NControlErrorStatusShadow |= MyOutputs->NControlErrorStatus; }while(0);}
#define ClearControlError(fault_) MyOutputs->NControlErrorStatus &= ~(1 << (uint32_t)(fault_))
#define CheckControlError(fault_) (MyOutputs->NControlErrorStatus >> (uint32_t)(fault_)) & 0x1


void InitController(InputStruct *inputs, OutputStruct *outputs) {

	MyInputs = inputs;
	MyOutputs = outputs;

	MyOutputs->xClutchBitepoint = xCLUTCH_BITE_POINT;


	// Multifunction

	// map map size
	outputs->NMultifunctionMaxPos[0] = MULTIFUNCTION01_MAX_POS;
	outputs->NMultifunctionMaxPos[1] = MULTIFUNCTION02_MAX_POS;
	outputs->NMultifunctionMaxPos[2] = MULTIFUNCTION03_MAX_POS;
	outputs->NMultifunctionMaxPos[3] = MULTIFUNCTION04_MAX_POS;
	outputs->NMultifunctionMaxPos[4] = MULTIFUNCTION05_MAX_POS;
	outputs->NMultifunctionMaxPos[5] = MULTIFUNCTION06_MAX_POS;
	outputs->NMultifunctionMaxPos[6] = MULTIFUNCTION07_MAX_POS;
	outputs->NMultifunctionMaxPos[7] = MULTIFUNCTION08_MAX_POS;
	outputs->NMultifunctionMaxPos[8] = MULTIFUNCTION09_MAX_POS;
	outputs->NMultifunctionMaxPos[9] = MULTIFUNCTION10_MAX_POS;
	outputs->NMultifunctionMaxPos[10] = MULTIFUNCTION11_MAX_POS;
	outputs->NMultifunctionMaxPos[11] = MULTIFUNCTION12_MAX_POS;
	outputs->NMultifunctionMaxPos[12] = MULTIFUNCTION13_MAX_POS;
	outputs->NMultifunctionMaxPos[13] = MULTIFUNCTION13_MAX_POS;

	// default values
	outputs->NMultifunctionDefMask[0] = (MULTIFUNCTION01_DEF_POS <= MULTIFUNCTION01_MAX_POS ? MULTIFUNCTION01_DEF_POS : MULTIFUNCTION01_MAX_POS);
	outputs->NMultifunctionDefMask[1] = (MULTIFUNCTION02_DEF_POS <= MULTIFUNCTION02_MAX_POS ? MULTIFUNCTION02_DEF_POS : MULTIFUNCTION02_MAX_POS);;
	outputs->NMultifunctionDefMask[2] = (MULTIFUNCTION03_DEF_POS <= MULTIFUNCTION03_MAX_POS ? MULTIFUNCTION03_DEF_POS : MULTIFUNCTION03_MAX_POS);;
	outputs->NMultifunctionDefMask[3] = (MULTIFUNCTION04_DEF_POS <= MULTIFUNCTION04_MAX_POS ? MULTIFUNCTION04_DEF_POS : MULTIFUNCTION04_MAX_POS);;
	outputs->NMultifunctionDefMask[4] = (MULTIFUNCTION05_DEF_POS <= MULTIFUNCTION05_MAX_POS ? MULTIFUNCTION05_DEF_POS : MULTIFUNCTION05_MAX_POS);;
	outputs->NMultifunctionDefMask[5] = (MULTIFUNCTION06_DEF_POS <= MULTIFUNCTION06_MAX_POS ? MULTIFUNCTION06_DEF_POS : MULTIFUNCTION06_MAX_POS);;
	outputs->NMultifunctionDefMask[6] = (MULTIFUNCTION07_DEF_POS <= MULTIFUNCTION07_MAX_POS ? MULTIFUNCTION07_DEF_POS : MULTIFUNCTION07_MAX_POS);;
	outputs->NMultifunctionDefMask[7] = (MULTIFUNCTION08_DEF_POS <= MULTIFUNCTION08_MAX_POS ? MULTIFUNCTION08_DEF_POS : MULTIFUNCTION08_MAX_POS);;
	outputs->NMultifunctionDefMask[8] = (MULTIFUNCTION09_DEF_POS <= MULTIFUNCTION09_MAX_POS ? MULTIFUNCTION09_DEF_POS : MULTIFUNCTION09_MAX_POS);;
	outputs->NMultifunctionDefMask[9] = (MULTIFUNCTION10_DEF_POS <= MULTIFUNCTION10_MAX_POS ? MULTIFUNCTION10_DEF_POS : MULTIFUNCTION10_MAX_POS);;
	outputs->NMultifunctionDefMask[10] = (MULTIFUNCTION11_DEF_POS <= MULTIFUNCTION11_MAX_POS ? MULTIFUNCTION11_DEF_POS : MULTIFUNCTION11_MAX_POS);;
	outputs->NMultifunctionDefMask[11] = (MULTIFUNCTION12_DEF_POS <= MULTIFUNCTION12_MAX_POS ? MULTIFUNCTION12_DEF_POS : MULTIFUNCTION12_MAX_POS);;
	outputs->NMultifunctionDefMask[12] = (MULTIFUNCTION13_DEF_POS <= MULTIFUNCTION13_MAX_POS ? MULTIFUNCTION13_DEF_POS : MULTIFUNCTION13_MAX_POS);;
	outputs->NMultifunctionDefMask[13] = (MULTIFUNCTION14_DEF_POS <= MULTIFUNCTION14_MAX_POS ? MULTIFUNCTION14_DEF_POS : MULTIFUNCTION14_MAX_POS);;

	// wrapping
	outputs->BMultifunctionWrap[0] = MULTIFUNCTION01_WRAP;
	outputs->BMultifunctionWrap[1] = MULTIFUNCTION02_WRAP;
	outputs->BMultifunctionWrap[2] = MULTIFUNCTION03_WRAP;
	outputs->BMultifunctionWrap[3] = MULTIFUNCTION04_WRAP;
	outputs->BMultifunctionWrap[4] = MULTIFUNCTION05_WRAP;
	outputs->BMultifunctionWrap[5] = MULTIFUNCTION06_WRAP;
	outputs->BMultifunctionWrap[6] = MULTIFUNCTION07_WRAP;
	outputs->BMultifunctionWrap[7] = MULTIFUNCTION08_WRAP;
	outputs->BMultifunctionWrap[8] = MULTIFUNCTION09_WRAP;
	outputs->BMultifunctionWrap[9] = MULTIFUNCTION10_WRAP;
	outputs->BMultifunctionWrap[10] = MULTIFUNCTION11_WRAP;
	outputs->BMultifunctionWrap[11] = MULTIFUNCTION12_WRAP;
	outputs->BMultifunctionWrap[12] = MULTIFUNCTION13_WRAP;
	outputs->BMultifunctionWrap[13] = MULTIFUNCTION13_WRAP;


	NMultifunctionActiveSwitchPrev = MyInputs->NSwitchA;

	// set the current positions to default
	for(uint8_t i=0; i<NMF; i++) {
		outputs->NMultifunction[i] = outputs->NMultifunctionDefMask[i];
	}


	IDLE_Entry();
}



void Controller(InputStruct *inputs, OutputStruct *outputs){

	tControllerTimmer = HAL_GetTick();

	// ------------------------------------------------------------------------------------------------------------------------------------------------------

	// ANTISTALL

		#ifdef ANTISTALL_ENABLED

			// if the shut down is activated and we are at gear greater than neutral we can enter
			if(!MyInputs->BDriverKill && MyInputs->NGear > 0 && !MyInputs->BNGearInError && !MyInputs->BnEngineInError && !MyOutputs->BShiftingInProgress && !MyInputs->BFalseNeutral) {

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
				// xClutchReleaseMap[MyOutputs->NxClutchReleaseMapIdx][0..1..2..3....]
			}
			// TODO: Here we keep timers and counter and the state of the mini control and put the values in xClutchTargetManual
		}

		// TODO: do the array running thing also for the launch sequence.
		// Decide if upshifts trigger will happen here, or we will be triggered in IDLE and start the clutch sequence here afterwards

		// we take the maximum target generated from the Antistall/Protection strategy, the one request
		// from the driver and the shifter requests when enabled from the respective strategy
		MyOutputs->xClutchTarget = MAX(MyOutputs->xClutchTargetProtection, MAX((uint16_t)MyOutputs->xClutchTargetManual, MyOutputs->xClutchTargetShift));

		MyOutputs->BClutchActuated = (MyOutputs->xClutchTarget >= xCLUTCH_TARGET_ACTUATED ? 1 : 0);

	// ------------------------------------------------------------------------------------------------------------------------------------------------------

	// TOGGLE SWITCHES & LEDS

		MyOutputs->BSWLEDA = MyInputs->NToggleSwitch01State;
		MyOutputs->BSWLEDB = MyInputs->NToggleSwitch02State;
		MyOutputs->BSWLEDC = MyInputs->NToggleSwitch03State;


	// ------------------------------------------------------------------------------------------------------------------------------------------------------

	// MULTIFUNCTION

		// inputs
		MyOutputs->NMultifunctionActiveSwitch = MyInputs->NSwitchA;
		MyOutputs->BMultifunctionNextPos = MyInputs->BSWButtonD;
		MyOutputs->BMultifunctionPrevPos = MyInputs->BSWButtonC;

		if(MyOutputs->NMultifunctionActiveSwitch != NMultifunctionActiveSwitchPrev) {
			NMultifunctionActiveSwitchPrev = MyOutputs->NMultifunctionActiveSwitch;
			MyOutputs->tMultifunctionActiveOnRot = tControllerTimmer + MULTIFUNCTION_ACTIVE_TIME;
			MyOutputs->BUseButtonsForMultifunction = 1;
			NMFIdx = MyOutputs->NMultifunctionActiveSwitch - 1;	// to go from 1-14 to 0-13 indexing for the arrays
			// TODO: not sure if better to change the page number here and then return it to the previous one
		}

		// + Button (next position)
		if(MyOutputs->BMultifunctionNextPos && (MyOutputs->tMultifunctionActiveOnRot >= tControllerTimmer || ALLOW_MULTIFUNC_WITH_NO_ACTIVE_TIME) && !MyOutputs->BMultifunctionNextPosState) {
			MyOutputs->BMultifunctionNextPosState = 1;
			MyOutputs->tMultifunctionActiveOnRot = tControllerTimmer + MULTIFUNCTION_ACTIVE_TIME;

			if(MyOutputs->NMultifunction[NMFIdx] + 1 >= MyOutputs->NMultifunctionMaxPos[NMFIdx]) {
				if(MyOutputs->BMultifunctionWrap[NMFIdx]) MyOutputs->NMultifunction[NMFIdx] = 0;
			}
			else {
				MyOutputs->NMultifunction[NMFIdx] ++;
			}
		}
		else if(!MyOutputs->BMultifunctionNextPos) {
			MyOutputs->BMultifunctionNextPosState = 0;
		}

		if(MyOutputs->BMultifunctionPrevPos && (MyOutputs->tMultifunctionActiveOnRot >= tControllerTimmer || ALLOW_MULTIFUNC_WITH_NO_ACTIVE_TIME) && !MyOutputs->BMultifunctionPrevPosState) {
			MyOutputs->BMultifunctionPrevPosState = 1;
			MyOutputs->tMultifunctionActiveOnRot = tControllerTimmer + MULTIFUNCTION_ACTIVE_TIME;

			if(MyOutputs->NMultifunction[NMFIdx] - 1 < 0 ) {
				if(MyOutputs->BMultifunctionWrap[NMFIdx]) MyOutputs->NMultifunction[NMFIdx] = MyOutputs->NMultifunctionMaxPos[NMFIdx] - 1;
			}
			else {
				MyOutputs->NMultifunction[NMFIdx] --;
			}
		}
		else if(!MyOutputs->BMultifunctionPrevPos) {
			MyOutputs->BMultifunctionPrevPosState = 0;
		}

		if(MyOutputs->tMultifunctionActiveOnRot < tControllerTimmer) {
			MyOutputs->BUseButtonsForMultifunction = 0;
			// TODO: and here to return to the actual page
		}


		// Here we assign the various multifunction maps to the various indexes
		MyOutputs->NxClutchReleaseMapIdx = MyOutputs->NMultifunction[MULTIFUNCTION_CLUTCH_RELEASE_IDX-1];
		// TODO: fill the rest...

	// ------------------------------------------------------------------------------------------------------------------------------------------------------

	// DISPLAY

		// inputs
		MyOutputs->BDisplayPageNext = MyInputs->BSWButtonD;
		MyOutputs->BDisplayPagePrev = MyInputs->BSWButtonC;

		if(!ALLOW_MULTIFUNC_WITH_NO_ACTIVE_TIME) {	// we use the page buttons function only if we have the multifunction timing feature enabled

			if(!MyOutputs->BUseButtonsForMultifunction) {	// we only use them as page buttons when they are not used for the multifunction
				if(MyOutputs->BDisplayPageNext && (MyOutputs->tDisplayPageDebounce < tControllerTimmer) && !MyOutputs->BDisplayPageNextState) {
					MyOutputs->BDisplayPageNextState = 1;
					MyOutputs->tDisplayPageDebounce = tControllerTimmer + DISPLAY_PAGE_DEBOUNCE;

					MyOutputs->NDispalyPage ++;
					MyOutputs->NDispalyPage %= DISPLAY_MAX_PAGE;

				}
				else if(!MyOutputs->BDisplayPageNext) {
					MyOutputs->BDisplayPageNextState = 0;
				}

				if(MyOutputs->BDisplayPagePrev && (MyOutputs->tDisplayPageDebounce < tControllerTimmer) && !MyOutputs->BDisplayPagePrevState) {
					MyOutputs->BDisplayPagePrevState = 1;
					MyOutputs->tDisplayPageDebounce = tControllerTimmer + DISPLAY_PAGE_DEBOUNCE;

					MyOutputs->NDispalyPage --;
					if(MyOutputs->NDispalyPage < 0) MyOutputs->NDispalyPage = DISPLAY_MAX_PAGE - 1;

				}
				else if(!MyOutputs->BDisplayPagePrev) {
					MyOutputs->BDisplayPagePrevState = 0;
				}
			}
		}

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


	// ------------------------------------------------------------------------------------------------------------------------------------------------------

	// CONTROLLER STATUS

		MyOutputs->NControllerStatusWord = 0;
		MyOutputs->NControllerStatusWord |= MyOutputs->BUpShiftPortState				<<0;
		MyOutputs->NControllerStatusWord |= MyOutputs->BDnShiftPortState				<<1;
		MyOutputs->NControllerStatusWord |= MyOutputs->BClutchActuated					<<2;
		MyOutputs->NControllerStatusWord |= MyOutputs->BSparkCut						<<3;
		MyOutputs->NControllerStatusWord |= MyOutputs->BLaunchControl					<<4;
		MyOutputs->NControllerStatusWord |= (MyOutputs->NAntistallState == 2 ? 1 : 0)	<<5;
		MyOutputs->NControllerStatusWord |= MyOutputs->BShiftingInProgress				<<6;
		MyOutputs->NControllerStatusWord |= 0											<<7;
		MyOutputs->NControllerStatusWord |= 0											<<8;
		MyOutputs->NControllerStatusWord |= 0											<<9;
		MyOutputs->NControllerStatusWord |= 0											<<10;
		MyOutputs->NControllerStatusWord |= 0											<<11;
		MyOutputs->NControllerStatusWord |= 0											<<12;
		MyOutputs->NControllerStatusWord |= 0											<<13;
		MyOutputs->NControllerStatusWord |= 0											<<14;
		MyOutputs->NControllerStatusWord |= 0											<<15;

		MyOutputs->NControllerStatusWord |= (MyOutputs->NControlErrorStatusShadow  && 0xffff) <<16;	// the controller errors (without the first which is "No error"), taken only the 16 first bits out of the 32

		if(tControllerErrorStatusShadow < tControllerTimmer) {
			tControllerErrorStatusShadow = tControllerTimmer + CONTROLLER_STATUS_SHADOW_REFRESH;
			MyOutputs->NControlErrorStatusShadow = 0;
		}

	// ------------------------------------------------------------------------------------------------------------------------------------------------------

	// DISPLAY DIAGNOSTICS

		MyOutputs->NDisplayFlags = 0;
		MyOutputs->NDisplayFlags |=	(MyOutputs->NControlErrorStatusShadow >> RPM_ILLEGAL_FOR_UPSHIFT) <<0;
		MyOutputs->NDisplayFlags |=	(MyOutputs->NControlErrorStatusShadow >> RPM_ILLEGAL_FOR_DNSHIFT) <<1;
		MyOutputs->NDisplayFlags |=	(MyOutputs->NControlErrorStatusShadow >> GEAR_TARGET_MISMATCH) <<2;
		MyOutputs->NDisplayFlags |=	(MyOutputs->NControlErrorStatusShadow >> FALSE_NEUTRAL_WITH_NO_CLUTCH) <<3;
		MyOutputs->NDisplayFlags |= 0 <<4;
		MyOutputs->NDisplayFlags |= 0 <<5;
		MyOutputs->NDisplayFlags |= 0 <<6;
		MyOutputs->NDisplayFlags |= 0 <<7;

	// ------------------------------------------------------------------------------------------------------------------------------------------------------

}

void IDLE_Entry(void) {
	NPreviousState = NCurrentState;
	NCurrentState = IDLE_STATE;
}
void IDLE_Exit(void) {

}
void IDLE_Event(void) {

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

	MyOutputs->BShiftingInProgress = 1;

	tPreShiftTimer = HAL_GetTick();
}
void PRE_UPSHIFT_Exit(void) {

}
void PRE_UPSHIFT_Event(void) {

	// if all ok we define the shifting targets and move on
	if(!MyOutputs->NControlErrorStatus) {
		MyOutputs->NGearTarget = MyInputs->NGear + 1;											// we go to the next gear

		if(CLUTCH_ACTUATION_DURING_UPSHIFT || MyOutputs->BOverrideActuateClutchOnNextUpShift) {		// we check for clutch strategy during shift
			MyOutputs->xClutchTargetShift = xClutchTargetUpShiftMap[MyInputs->NGear];
			MyOutputs->BOverrideActuateClutchOnNextUpShift = 0; 									// reset the strat for the next gear
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

	if(MyInputs->NGear == 0 && MyInputs->rClutchPaddle < CLUTCH_PADDLE_THRESHOLD_FOR_FIRST && !ALLOW_FIRST_WITHOUT_CLUTCH) {	// trying to put 1st gear without clutch
		RaiseControlError(NEUTRAL_TO_FIRST_WITH_NO_CLUTCH);
	}
	else { ClearControlError(NEUTRAL_TO_FIRST_WITH_NO_CLUTCH); }

	if(MyInputs->nEngine < nEngineUpShiftMap[MyInputs->NGear] && !MyInputs->BnEngineInError && !(ALLOW_GEARS_WITH_CAR_STOPPED && MyInputs->rClutchPaddle >= CLUTCH_PADDLE_THRESHOLD_FOR_FIRST)) {	// trying to shift up with too low rpm
		RaiseControlError(RPM_ILLEGAL_FOR_UPSHIFT);
	}
	else { ClearControlError(RPM_ILLEGAL_FOR_UPSHIFT); }

	if(MyInputs->NGear + 1 > MAX_GEAR)	{																					// trying to shift up after last gear
		RaiseControlError(TARGET_GEAR_EXCEEDS_MAX);
	}
	else { ClearControlError(TARGET_GEAR_EXCEEDS_MAX); }

	if(MyInputs->BFalseNeutral && !MyInputs->BNGearInError && MyInputs->rClutchPaddle < CLUTCH_PADDLE_THRESHOLD_FOR_FIRST) {	// trying to shift during False Neutral without clutch pressed
		RaiseControlError(FALSE_NEUTRAL_WITH_NO_CLUTCH);
	}
	else { ClearControlError(FALSE_NEUTRAL_WITH_NO_CLUTCH); }
}



void PRE_DNSHIFT_Entry(void) {
	NPreviousState = NCurrentState;
	NCurrentState = PRE_DNSHIFT_STATE;

	MyOutputs->BShiftingInProgress = 1;

	tPreShiftTimer = HAL_GetTick();
}
void PRE_DNSHIFT_Exit(void) {

}
void PRE_DNSHIFT_Event(void) {

	// if all ok we define the shifting targets and move on
	if(!MyOutputs->NControlErrorStatus) {
		MyOutputs->NGearTarget = MyInputs->NGear - 1;												// we go to the previous gear

		if(CLUTCH_ACTUATION_DURING_DNSHIFT || MyOutputs->BOverrideActuateClutchOnNextDnShift) {		// we check for clutch strategy during shift
			MyOutputs->xClutchTargetShift = xClutchTargetDnShiftMap[MyInputs->NGear];
			MyOutputs->BOverrideActuateClutchOnNextDnShift = 0; 									// reset the strat for the next gear
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

	if(MyInputs->NGear == 1 && MyInputs->rClutchPaddle < CLUTCH_PADDLE_THRESHOLD_FOR_FIRST && !ALLOW_NEUTRAL_WITHOUT_CLUTCH && !(MyInputs->BrClutchPaddleInError && ALLOW_NEUTRAL_WHEN_PADDLE_IN_ERROR)) {	// trying to put neutral gear without clutch
		RaiseControlError(FIRST_TO_NEUTRAL_WITH_NO_CLUTCH);
	}
	else { ClearControlError(FIRST_TO_NEUTRAL_WITH_NO_CLUTCH); }

	if(MyInputs->nEngine > nEngineDnShiftMap[MyInputs->NGear] && !MyInputs->BnEngineInError && !(ALLOW_GEARS_WITH_CAR_STOPPED && MyInputs->rClutchPaddle >= CLUTCH_PADDLE_THRESHOLD_FOR_FIRST)) {	// trying to shift down with too high rpm
		RaiseControlError(RPM_ILLEGAL_FOR_DNSHIFT);
	}
	else { ClearControlError(RPM_ILLEGAL_FOR_DNSHIFT); }

	if(MyInputs->NGear == 0)	{																								// trying to shift down from neutral
		RaiseControlError(TARGET_GEAR_LESS_THAN_NEUTRAL);
	}
	else { ClearControlError(TARGET_GEAR_LESS_THAN_NEUTRAL); }

	if(MyInputs->BFalseNeutral && !MyInputs->BNGearInError && MyInputs->rClutchPaddle < CLUTCH_PADDLE_THRESHOLD_FOR_FIRST) {	// trying to shift during False Neutral without clutch pressed
		RaiseControlError(FALSE_NEUTRAL_WITH_NO_CLUTCH);
	}
	else { ClearControlError(FALSE_NEUTRAL_WITH_NO_CLUTCH); }
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
		tShifterMaxTransitTime = tDnShift[MyInputs->NGear - 1];
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


	// TODO: keep checking for control errors ??


	if((tShiftTimer + tShifterMaxTransitTime) < tControllerTimmer) {	// the max time for the gear has expired
		// go out and determine if the shift was completed or not
		SHIFTING_Exit();
		POSTSHIFT_Entry();
		return;
	}

}
void SHIFTING_Run(void) {

	// based on the error status and the strat preferences decide in which controller to enter



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
	MyOutputs->BShiftingInProgress = 0;

	// we rest the False Neutral flag TODO: not sure if correct here
	MyInputs->BFalseNeutral = 0;
}
void POSTSHIFT_Event(void) {


	if(!MyOutputs->NControlErrorStatus) {

		POSTSHIFT_Exit();
		IDLE_Entry();
		return;
	}


	// we check for control errors and if present after the time threshold, we abort
	if(MyOutputs->NControlErrorStatus && (tPostShiftTimer + POSTSHIFT_THRESHOLD_TIME) <= HAL_GetTick()) {
		POSTSHIFT_Exit();
		ERROR_Entry();
		return;
	}
}
void POSTSHIFT_Run(void) {

	if(CHECK_POST_SHIFT_GEAR && MyInputs->NGear != MyOutputs->NGearTarget && !MyInputs->BFalseNeutral) {
		RaiseControlError(GEAR_TARGET_MISMATCH);
	}
	else { ClearControlError(GEAR_TARGET_MISMATCH); }

}


void ERROR_Entry(void) {
	NPreviousState = NCurrentState;
	NCurrentState = ERROR_STATE;

	MyOutputs->BShiftingInProgress = 0;
	// TODO: we need to open a led to indicate the Error State !!!
	// or send it to the display via CAN

	// reset all actuator states
	MyOutputs->BUpShiftPortState = 0;
	MyOutputs->BDnShiftPortState = 0;
	MyOutputs->xClutchTargetShift = 0;
	MyOutputs->BSparkCut = 0;

}

void ERROR_Exit(void) {

}
void ERROR_Event(void) {

		// for some faults that are very critical we could make a counter and when it expires we declare a default hardcoded value to be able to move on
	// TODO: it must not be completely blocking to be able to comeback from an error.
	// the concept is to keep a counter for the number of errors of each type and after a certain point come back and continue normal running with less features
	// check that all control errors are cleared
	// and do not zero the logged error status
	// remember return in all functions

	// Remember to create the Strategy (and a way to exit the error) to be able to function without NGear (complete open loop)
}
void ERROR_Run(void) {

	MyOutputs->NControlErrorStatus = 0;


	// TODO: find a way to read the Control Errors and then reset them in order to clear them for the next cycle

}
