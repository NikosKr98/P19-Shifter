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
uint32_t tControllerTimmer, tPreShiftTimer, tShiftTimer, tShifterMaxTransitTime, tShifterDelayForClutch, tShifterGearMatch, tPostShiftTimer, tAntistallTimmer, tControllerErrorStatusShadow;
uint32_t tToggleSwitch01, tToggleSwitch02, tToggleSwitch03, tToggleSwitch04;

// Local Variables
float rClutchPaddle_xClutchTargetMap[2][CLUTCH_PADDLE_TARGET_MAP_SIZE] = { // the variable used to store the selected clutch map, intentionally left empty

	/* In:  rClutchPaddle */		{  0,   10,   20,   30,   40,   50,   60,   70,   80,   90,  100 },
	/* Out: xClutchTarget */		{  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0 }
};

#define CheckEvent(event_) (MyInputs->nEventStatus >> (uint32_t)(event_)) & 0x1
#define CheckFault(fault_) (MyInputs->nFaultStatus >> (uint32_t)(fault_)) & 0x1

#define RaiseControlError(fault_) { do { MyOutputs->NControlErrorStatus |= (1 << (uint32_t)(fault_)); MyOutputs->NControlErrorStatusLogged = fault_; MyOutputs->NControlErrorStatusShadow |= MyOutputs->NControlErrorStatus; } while(0);}
#define ClearControlError(fault_) MyOutputs->NControlErrorStatus &= ~(1 << (uint32_t)(fault_))
#define CheckControlError(fault_) (MyOutputs->NControlErrorStatus >> (uint32_t)(fault_)) & 0x1


void InitController(InputStruct *inputs, OutputStruct *outputs) {

	MyInputs = inputs;
	MyOutputs = outputs;

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


	NMultifunctionActiveSwitchPrev = MyInputs->NSwitchA;

	// set the current positions to default
	for(uint8_t i=0; i<NMF; i++) {
		outputs->NMultifunction[i] = outputs->NMultifunctionDefMask[i];
	}

	// we initialize the min & max clutch targets, later they can be modified with the multifunction
	MyOutputs->xClutchTargetMin = CLUTCH_TARGET_MIN_DEF;
	MyOutputs->xClutchTargetMax = CLUTCH_TARGET_MAX_DEF;

	IDLE_Entry();
}



void Controller(InputStruct *inputs, OutputStruct *outputs){

	tControllerTimmer = HAL_GetTick();

	// ------------------------------------------------------------------------------------------------------------------------------------------------------

	// ANTISTALL

		#ifdef ANTISTALL_ENABLED

			// if the shut down is activated and we are at gear greater than neutral we can enter
			if(!MyInputs->BDriverKill && MyInputs->NGear >= ANTISTALL_MIN_ACTIVATION_GEAR && !MyInputs->BNGearInError && !MyInputs->BnEngineInError && !MyOutputs->BShiftInProgress && !MyInputs->BFalseNeutral) {

				if(MyOutputs->NAntistallState != Active && MyInputs->nEngine <= nEngineAntistallMap[MyInputs->NGear] && MyInputs->rClutchPaddle < ANTISTALL_CLUTCHPADDLE_RELEASED) {
					// Timer initialization of enable strategy
					if(MyOutputs->NAntistallState == Off) {
						MyOutputs->NAntistallState = Init;
						tAntistallTimmer = HAL_GetTick();
					}
					// Activation
					if(MyOutputs->NAntistallState == Init && (tAntistallTimmer + ANTISTALL_TRIGGER_TIME) < tControllerTimmer) {
						MyOutputs->NAntistallState = Active;
						MyOutputs->xClutchTargetProtection = MyOutputs->xClutchTargetMax;
					}
				}
				// Not activation due to engine rpm returning over the limit, or early clutch paddle press
				if(MyOutputs->NAntistallState == Init && (MyInputs->nEngine > nEngineAntistallMap[MyInputs->NGear] || MyInputs->rClutchPaddle > ANTISTALL_CLUTCHPADDLE_PRESSED)) {
					MyOutputs->NAntistallState = Off;
					MyOutputs->xClutchTargetProtection = MyOutputs->xClutchTargetMin;
				}
				// De-activation by Clutch paddle press
				if(MyOutputs->NAntistallState == Active && MyInputs->rClutchPaddle > ANTISTALL_CLUTCHPADDLE_PRESSED) {
					MyOutputs->NAntistallState = Off;
					MyOutputs->xClutchTargetProtection = MyOutputs->xClutchTargetMin;
				}
			}
			// De-activation by Driver Kill or Neutral or Errors
			else {
				MyOutputs->NAntistallState = Off;
				MyOutputs->xClutchTargetProtection = MyOutputs->xClutchTargetMin;
			}
		#elif
			MyOutputs->xClutchTargetProtection = MyOutputs->xClutchTargetMin;
		#endif

	// ------------------------------------------------------------------------------------------------------------------------------------------------------

	// CLUTCH CONTROLLER

		// Manual target mapping
		if(!MyInputs->BrClutchPaddleInError) {

			// we select the clutch paddle map based on the map index of the multifunction and copy it to the local array
//			memcpy((float*)rClutchPaddle_xClutchTargetMap[1], (float*)rClutchPaddle_xClutchTargetMaps[MyOutputs->NMultifunction[MULTIFUNCTION_CLUTCH_PADDLE_MAP_IDX-1]-1], CLUTCH_PADDLE_TARGET_MAP_SIZE);
			for(uint8_t i=0; i<CLUTCH_PADDLE_TARGET_MAP_SIZE; i++) {
				rClutchPaddle_xClutchTargetMap[1][i] = rClutchPaddle_xClutchTargetMaps[MyOutputs->NMultifunction[MULTIFUNCTION_CLUTCH_PADDLE_MAP_IDX-1]-1][i];
			}
			// we dynamically refine the clutch map
			for(uint8_t i=0; i<CLUTCH_PADDLE_TARGET_MAP_SIZE; i++) {

				// we clamp the xClutchTargetMap, to keep it inside the the min & max limits (dynamic) and
				rClutchPaddle_xClutchTargetMap[1][i] = CLAMP(rClutchPaddle_xClutchTargetMap[1][i], MyOutputs->xClutchTargetMin, MyOutputs->xClutchTargetMax);

				// we force the last element to the max clutch aperture
				if(i == CLUTCH_PADDLE_TARGET_MAP_SIZE-1) rClutchPaddle_xClutchTargetMap[1][i] = MyOutputs->xClutchTargetMax;
			}

			My2DMapInterpolate(CLUTCH_PADDLE_TARGET_MAP_SIZE, rClutchPaddle_xClutchTargetMap, MyInputs->rClutchPaddle, &MyOutputs->xClutchTargetManual, 0, 0);

			// we apply the  clutch paddle offset from the multifunction (inside the desired rClutchPaddle window)
			if(MyInputs->rClutchPaddle >= CLUTCH_PADDLE_ALLOW_OFFSET_MIN && MyInputs->rClutchPaddle <= CLUTCH_PADDLE_ALLOW_OFFSET_MAX) {
				MyOutputs->xClutchTargetManual *= rClutchPaddle_xClutchTargetOffsetMaps[MyOutputs->NMultifunction[MULTIFUNCTION_CLUTCH_PADDLE_OFFSET_IDX-1]];
			}

			// TODO: terminate potential array timed control that runs below

		}
		else {
			MyOutputs->xClutchTargetManual = MyOutputs->xClutchTargetMin;

			if(CheckEvent(DECLUTCH_RELEASE_EVT)) {
				// TODO: run the release array. here we initialize it
				// create lookup table running request for accel and emergency clutchButton release
				// xClutchReleaseMap[MyOutputs->NxClutchReleaseMapIdx][0..1..2..3....]
			}
			// TODO: Here we keep timers and counter and the state of the mini control and put the values in xClutchTargetManual
			// if the button gets pressed again we need to stop the declutch controller and open the clutch
		}

		// TODO: do the array running thing also for the launch sequence.
		// Decide if upshifts trigger will happen here, or we will be triggered in IDLE and start the clutch sequence here afterwards

		// we take the maximum target generated from the Antistall/Protection strategy, the one request
		// from the driver and the shifter requests when enabled from the respective strategy
		MyOutputs->xClutchTarget = MAX(MyOutputs->xClutchTargetProtection, MAX(MyOutputs->xClutchTargetManual, MyOutputs->xClutchTargetShift));

	// ------------------------------------------------------------------------------------------------------------------------------------------------------

	// TOGGLE SWITCHES

		// Toggle 1
		if(TOGGLE_SWITCH01_BUTTON && tToggleSwitch01 < tControllerTimmer) {
			if(!MyOutputs->NToggleSwitch01State) {
				MyOutputs->NToggleSwitch01State = 1;

				// Actions for toggle ON
				// remember the multifunction override
			}
			else {
				MyOutputs->NToggleSwitch01State = 0;

				// Actions for toggle OFF
				// remember the multifunction override

			}

			MyOutputs->NToggleSwitch01State ^= 1;
			tToggleSwitch01 = tControllerTimmer + TOGGLE_SWITCH_DEBOUNCE;
		}

		// Toggle 2
		if(TOGGLE_SWITCH02_BUTTON && tToggleSwitch02 < tControllerTimmer) {
			MyOutputs->NToggleSwitch02State ^= 1;
			tToggleSwitch02 = tControllerTimmer + TOGGLE_SWITCH_DEBOUNCE;
		}

		// Toggle 3
		if(TOGGLE_SWITCH03_BUTTON && tToggleSwitch03 < tControllerTimmer) {
			MyOutputs->NToggleSwitch03State ^= 1;
			tToggleSwitch03 = tControllerTimmer + TOGGLE_SWITCH_DEBOUNCE;
		}

		// Toggle 4
	//	if(TOGGLE_SWITCH04_BUTTON && tToggleSwitch04 < tControllerTimmer) {
	//		inputs->NToggleSwitch04State ^= 1;
	//		tToggleSwitch04 = tControllerTimmer + TOGGLE_SWITCH_DEBOUNCE;
	//	}


		MyOutputs->BSWLEDA = MyOutputs->NToggleSwitch01State;
		MyOutputs->BSWLEDB = MyOutputs->NToggleSwitch02State;
		MyOutputs->BSWLEDC = MyOutputs->NToggleSwitch03State;

	// ------------------------------------------------------------------------------------------------------------------------------------------------------

	// MULTIFUNCTION

		// inputs
		MyOutputs->NMultifunctionActiveSwitch = MyInputs->NSwitchA;
		MyOutputs->BMultifunctionNextPos = MULTIFUNCTION_NEXT_BUTTON;
		MyOutputs->BMultifunctionPrevPos = MULTIFUNCTION_PREV_BUTTON;

		if(MyOutputs->NMultifunctionActiveSwitch != NMultifunctionActiveSwitchPrev) {
			NMultifunctionActiveSwitchPrev = MyOutputs->NMultifunctionActiveSwitch;
			MyOutputs->tMultifunctionActiveOnRot = tControllerTimmer + MULTIFUNCTION_ACTIVE_TIME;
			MyOutputs->BUseButtonsForMultifunction = 1;
			NMFIdx = MyOutputs->NMultifunctionActiveSwitch - 1;	// to go from 1-14 to 0-13 indexing for the arrays

			if(MyOutputs->NDispalyPage != DISPLAY_MULTIFUNCTION_PAGE) MyOutputs->NDispalyPagePrev = MyOutputs->NDispalyPage;	// we save and change the page number here
			MyOutputs->NDispalyPage = DISPLAY_MULTIFUNCTION_PAGE;
		}

		// + Button (next position)
		if(MyOutputs->BMultifunctionNextPos && (MyOutputs->tMultifunctionActiveOnRot >= tControllerTimmer || ALLOW_MULTIFUNC_WITH_NO_ACTIVE_TIME) && !MyOutputs->BMultifunctionNextPosState) {
			MyOutputs->BMultifunctionNextPosState = 1;
			MyOutputs->tMultifunctionActiveOnRot = tControllerTimmer + MULTIFUNCTION_ACTIVE_TIME;

			if(MyOutputs->NMultifunction[NMFIdx] + 1 > MyOutputs->NMultifunctionMaxPos[NMFIdx]) {
				if(MyOutputs->BMultifunctionWrap[NMFIdx]) MyOutputs->NMultifunction[NMFIdx] = 1;
			}
			else {
				MyOutputs->NMultifunction[NMFIdx] ++;
			}
		}
		else if(!MyOutputs->BMultifunctionNextPos) {
			MyOutputs->BMultifunctionNextPosState = 0;
		}
		// - Button (previous position)
		if(MyOutputs->BMultifunctionPrevPos && (MyOutputs->tMultifunctionActiveOnRot >= tControllerTimmer || ALLOW_MULTIFUNC_WITH_NO_ACTIVE_TIME) && !MyOutputs->BMultifunctionPrevPosState) {
			MyOutputs->BMultifunctionPrevPosState = 1;
			MyOutputs->tMultifunctionActiveOnRot = tControllerTimmer + MULTIFUNCTION_ACTIVE_TIME;

			if(MyOutputs->NMultifunction[NMFIdx] - 1 <= 0 ) {
				if(MyOutputs->BMultifunctionWrap[NMFIdx]) MyOutputs->NMultifunction[NMFIdx] = MyOutputs->NMultifunctionMaxPos[NMFIdx] - 1;
			}
			else {
				MyOutputs->NMultifunction[NMFIdx] --;
			}
		}
		else if(!MyOutputs->BMultifunctionPrevPos) {
			MyOutputs->BMultifunctionPrevPosState = 0;
		}

		if(MyOutputs->tMultifunctionActiveOnRot < tControllerTimmer && MyOutputs->BUseButtonsForMultifunction) {
			MyOutputs->BUseButtonsForMultifunction = 0;
			// here we return to the actual page
			MyOutputs->NDispalyPage = MyOutputs->NDispalyPagePrev;
		}


		// Here we assign the various multifunction maps to the various indexes
		if(!MyOutputs->BMultifunctionOverride[MULTIFUNCTION_CLUTCH_TARGET_MAX_IDX-1]) MyOutputs->xClutchTargetMax = xClutchTargetMaxMap[MyOutputs->NMultifunction[MULTIFUNCTION_CLUTCH_TARGET_MAX_IDX-1]-1];
		if(!MyOutputs->BMultifunctionOverride[MULTIFUNCTION_CLUTCH_PADDLE_MAP_IDX-1]) MyOutputs->NxClutchPaddleMapIdx = MyOutputs->NMultifunction[MULTIFUNCTION_CLUTCH_PADDLE_MAP_IDX-1] - 1;
		if(!MyOutputs->BMultifunctionOverride[MULTIFUNCTION_CLUTCH_PADDLE_OFFSET_IDX-1]) MyOutputs->NxClutchPaddleOffsetIdx = MyOutputs->NMultifunction[MULTIFUNCTION_CLUTCH_PADDLE_OFFSET_IDX-1] - 1;
		if(!MyOutputs->BMultifunctionOverride[MULTIFUNCTION_CLUTCH_RELEASE_MAP_IDX-1]) MyOutputs->NxClutchReleaseMapIdx = MyOutputs->NMultifunction[MULTIFUNCTION_CLUTCH_RELEASE_MAP_IDX-1] - 1;
		if(!MyOutputs->BMultifunctionOverride[MULTIFUNCTION_CLUTCH_RELEASE_OFFSET_IDX-1]) MyOutputs->NxClutchReleaseOffsetIdx = MyOutputs->NMultifunction[MULTIFUNCTION_CLUTCH_RELEASE_OFFSET_IDX-1] - 1;
		if(!MyOutputs->BMultifunctionOverride[MULTIFUNCTION_UPSHIFT_TYPE_IDX-1]) MyOutputs->NUpShiftType = MyOutputs->NMultifunction[MULTIFUNCTION_UPSHIFT_TYPE_IDX-1] - 1;
		if(!MyOutputs->BMultifunctionOverride[MULTIFUNCTION_DNSHIFT_TYPE_IDX-1]) MyOutputs->NDnShiftType = MyOutputs->NMultifunction[MULTIFUNCTION_DNSHIFT_TYPE_IDX-1] - 1;

		// TODO: fill the rest...

	// ------------------------------------------------------------------------------------------------------------------------------------------------------

	// DISPLAY

		// inputs
		MyOutputs->BDisplayPageNext = DISPLAY_NEXT_BUTTON;
		MyOutputs->BDisplayPagePrev = DISPLAY_PREV_BUTTON;

		if(!ALLOW_MULTIFUNC_WITH_NO_ACTIVE_TIME) {	// we use the page buttons function only if we have the multifunction timing feature enabled

			if(!MyOutputs->BUseButtonsForMultifunction) {	// we only use them as page buttons when they are not used for the multifunction
				if(MyOutputs->BDisplayPageNext && (MyOutputs->tDisplayPageDebounce < tControllerTimmer) && !MyOutputs->BDisplayPageNextState) {
					MyOutputs->BDisplayPageNextState = 1;
					MyOutputs->tDisplayPageDebounce = tControllerTimmer + DISPLAY_PAGE_BUTTON_DEBOUNCE;

					MyOutputs->NDispalyPage ++;
					MyOutputs->NDispalyPage %= DISPLAY_MAX_PAGE;

				}
				else if(!MyOutputs->BDisplayPageNext) {
					MyOutputs->BDisplayPageNextState = 0;
				}

				if(MyOutputs->BDisplayPagePrev && (MyOutputs->tDisplayPageDebounce < tControllerTimmer) && !MyOutputs->BDisplayPagePrevState) {
					MyOutputs->BDisplayPagePrevState = 1;
					MyOutputs->tDisplayPageDebounce = tControllerTimmer + DISPLAY_PAGE_BUTTON_DEBOUNCE;

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
		MyOutputs->NControllerStatusWord |= MyOutputs->BShiftInProgress					<<6;
		MyOutputs->NControllerStatusWord |= MyOutputs->NToggleSwitch01State				<<7;
		MyOutputs->NControllerStatusWord |= MyOutputs->NToggleSwitch02State				<<8;
		MyOutputs->NControllerStatusWord |= MyOutputs->NToggleSwitch03State				<<9;
		MyOutputs->NControllerStatusWord |= MyOutputs->NToggleSwitch04State				<<10;
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

	MyOutputs->BShiftInProgress = 1;

	tPreShiftTimer = HAL_GetTick();
}
void PRE_UPSHIFT_Exit(void) {

}
void PRE_UPSHIFT_Event(void) {

	// if all ok we define the shifting targets and move on
	if(!MyOutputs->NControlErrorStatus) {
		MyOutputs->NGearTarget = MyInputs->NGear + 1;											// we go to the next gear

		if(((MyOutputs->NUpShiftType == WithClutch || MyOutputs->NUpShiftType == WithClutchAndSparkCut) && ALLOW_CLUTCH_ACT_DURING_UPSHIFT) || MyOutputs->BOverrideActuateClutchOnNextUpShift) {		// we check for clutch strategy during shift
			MyOutputs->xClutchTargetShiftShadow = xClutchTargetUpShiftMap[MyInputs->NGear];
			MyOutputs->BOverrideActuateClutchOnNextUpShift = 0; 									// reset the strat for the next gear
		}
		else {
			MyOutputs->xClutchTargetShiftShadow = 0;
		}

		if((MyOutputs->NUpShiftType == SparkCut || MyOutputs->NUpShiftType == WithClutchAndSparkCut) && ALLOW_SPARK_CUT_ON_UP_SHIFT) MyOutputs->BSparkCut = 1;

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

	if(MyInputs->nEngine < nEngineUpShiftMap[MyInputs->NGear] && !MyInputs->BnEngineInError && !(ALLOW_GEARS_WITH_CAR_STOPPED && MyInputs->nEngine == 0)) {	// trying to shift up with too low rpm
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

	MyOutputs->BShiftInProgress = 1;

	tPreShiftTimer = HAL_GetTick();
}
void PRE_DNSHIFT_Exit(void) {

}
void PRE_DNSHIFT_Event(void) {

	// if all ok we define the shifting targets and move on
	if(!MyOutputs->NControlErrorStatus) {
		MyOutputs->NGearTarget = MyInputs->NGear - 1;												// we go to the previous gear

		if(((MyOutputs->NDnShiftType == WithClutch || MyOutputs->NDnShiftType == WithClutchAndSparkCut) && ALLOW_CLUTCH_ACT_DURING_DNSHIFT) || MyOutputs->BOverrideActuateClutchOnNextDnShift) {		// we check for clutch strategy during shift
			MyOutputs->xClutchTargetShiftShadow = xClutchTargetDnShiftMap[MyInputs->NGear];
			MyOutputs->BOverrideActuateClutchOnNextDnShift = 0; 									// reset the strat for the next gear
		}
		else {
			MyOutputs->xClutchTargetShiftShadow = 0;
		}

		if((MyOutputs->NDnShiftType == SparkCut || MyOutputs->NDnShiftType == WithClutchAndSparkCut) && ALLOW_SPARK_CUT_ON_DN_SHIFT) MyOutputs->BSparkCut = 1;

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

	if(MyInputs->nEngine > nEngineDnShiftMap[MyInputs->NGear] && !MyInputs->BnEngineInError && !(ALLOW_GEARS_WITH_CAR_STOPPED && MyInputs->nEngine == 0)) {	// trying to shift down with too high rpm
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

	tShiftTimer = HAL_GetTick();

	MyOutputs->xClutchTargetShift = MyOutputs->xClutchTargetShiftShadow;

	if(NPreviousState == PRE_UPSHIFT_STATE) {
		NShiftRequest = Up;
		tShifterMaxTransitTime = tUpShift[MyInputs->NGear];

		if(MyOutputs->NUpShiftType == WithClutch) tShifterDelayForClutch = tUpShiftDelayForClutch[MyInputs->NGear];
		else tShifterDelayForClutch = 0;

		if(MyOutputs->NGearTarget == 1) {		// if going from neutral to 1st we need to actually downshift (it is how the gears work)
			MyOutputs->BDnShiftPortStateShadow = 1;
		}
		else {									// all other upshifts are normal
			MyOutputs->BUpShiftPortStateShadow = 1;
		}

	}
	else if(NPreviousState == PRE_DNSHIFT_STATE) {
		NShiftRequest = Down;
		tShifterMaxTransitTime = tDnShift[MyInputs->NGear];

		if(MyOutputs->NDnShiftType == WithClutch) tShifterDelayForClutch = tDnShiftDelayForClutch[MyInputs->NGear];
		else tShifterDelayForClutch = 0;

		if(MyOutputs->NGearTarget == 0) {		// if going from 1st to neutral we need to actually upshift (it is how the gears work)
			MyOutputs->BUpShiftPortStateShadow = 1;
		}
		else {									// all other downshifts are normal
			MyOutputs->BDnShiftPortStateShadow = 1;
		}

	}
	else {
		NCurrentState = Unknown;
		RaiseControlError(SHIFT_TARGET_UNKNOWN);
		tShifterDelayForClutch = 0;
	}

}
void SHIFTING_Exit(void) {
	MyOutputs->BNGearMatch = 0;
}
void SHIFTING_Event(void) {

	// we check for control errors and if present we abort
	if(MyOutputs->NControlErrorStatus) {
		SHIFTING_Exit();
		ERROR_Entry();
		return;
	}

	if((tShiftTimer + tShifterMaxTransitTime) <= tControllerTimmer) {	// the max time for the gear has expired
		// go out and determine if the shift was completed or not
		SHIFTING_Exit();
		POSTSHIFT_Entry();
		return;
	}

	// if the shift gets completed before the maximum time we terminate it
	if(MyInputs->NGear == MyOutputs->NGearTarget && ALLOW_END_OF_SHIFT_ON_GEAR_MATCH) {
		if(!MyOutputs->BNGearMatch) {
			MyOutputs->BNGearMatch = 1;
			tShifterGearMatch = tShiftTimer;
		}

		if((tShifterGearMatch + GEAR_MATCH_EARLY_THRESHOLD_TIME) <= tShiftTimer) {
			SHIFTING_Exit();
			POSTSHIFT_Entry();
			return;
		}
	}
	else {
		MyOutputs->BNGearMatch = 0;
	}

	// TODO: implement the shifting timing (include pre and post shift phases and create the metrics

	// TODO: think about the double action shift (pushing and then pulling the piston back)
}
void SHIFTING_Run(void) {

	if((tShiftTimer + tShifterDelayForClutch) <= tControllerTimmer) {	// when the actuator delay has passed
		MyOutputs->BUpShiftPortState = MyOutputs->BUpShiftPortStateShadow;
		MyOutputs->BDnShiftPortState = MyOutputs->BDnShiftPortStateShadow;
	}

}


void POSTSHIFT_Entry(void) {
	NPreviousState = NCurrentState;
	NCurrentState = POSTSHIFT_STATE;

	// reset all actuator states
	MyOutputs->BUpShiftPortState = 0;
	MyOutputs->BDnShiftPortState = 0;
	MyOutputs->BUpShiftPortStateShadow = 0;
	MyOutputs->BDnShiftPortStateShadow = 0;

	// reset all control variables for the next actuation
	MyOutputs->xClutchTargetShiftShadow = 0;
	MyOutputs->xClutchTargetShift = 0;
	MyOutputs->BSparkCut = 0;

}
void POSTSHIFT_Exit(void) {
	MyOutputs->BShiftInProgress = 0;

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

	MyOutputs->BShiftInProgress = 0;
	// TODO: we need to open a led to indicate the Error State !!!
	// or send it to the display via CAN

	// TODO: we set the page number to ERROR page and then we need to create a timeout in order to return to normal page operation

	// reset all actuator states
	MyOutputs->BUpShiftPortState = 0;
	MyOutputs->BDnShiftPortState = 0;
	MyOutputs->xClutchTargetShiftShadow = 0;
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
	// for now we exit with no condition
	ERROR_Exit();
	IDLE_Entry();
}
void ERROR_Run(void) {

	MyOutputs->NControlErrorStatus = 0;

}
