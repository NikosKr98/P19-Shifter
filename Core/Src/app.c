/*
 * app.c
 *
 *  Created on: Jan 22, 2024
 *      Author: NikosKr
 */

#include "app.h"

InputStruct myLocalInputs;

States NCurrentState, NPreviousState;

void InitApplication(void) {
	IDLE_Entry();
}



void RunApplication(InputStruct *input, OutputStruct *output){

//	myLocalInputs = &input;
//	if(app->current_gear != app -> actual_gear){
//		app->current_gear = app -> actual_gear;
//		app->target_gear = app -> actual_gear;
//	}


	// UP BUTTON REQUEST CONFIRMATION CHECK !
//	if(input->up_button_request && !app->up_button_status && button_previous<current && !app-> up_port_state){
//		button_previous=current;
//		button_previous+=button_interval;
//		app->up_button_output = 1;
//		app->up_button_status=1;
//	}
//	else if(!input->up_button_request && app->up_button_status){
//		app->up_button_output =0;
//		app->up_button_status=0;
//	}


	// DOWNM BUTTON REQUEST CONFIRMATION CHECK !
//	if(input-> down_button_request && !app->down_button_status && button_previous<current && !app-> down_port_state){
//		button_previous=current;
//		button_previous+=button_interval;
//		app->down_button_output = 1;
//		app->down_button_status=1;
//
//	}
//	else if(!input->down_button_request && app->down_button_status){
//		app->down_button_output =0;
//		app->down_button_status=0;
//
//	}


//	output->clutch_position = input->clutch_position;

//	output->clutch_detection = (output->clutch_position > clutch_detection_threshold) ? 1 : 0;
	//CLUTCH REACHED THE THRESHOLD

//	app->actual_gear = ;

	// SHIFTER STATE MACHINE
	switch (NCurrentState) {

	case SHIFTER_IDLE:
		IDLE_Run();
		break;
	case SHIFTER_PRESHIFT:
		PRESHIFT_Run();
		break;
	case SHIFTER_SHIFTING:
		SHIFTING_Run();
		break;
	case SHIFTER_POSTSHIFT:
		POSTSHIFT_Run();
		break;
	case SHIFTER_ERROR:
		ERROR_Run();
		break;
//	case default:
//		break;
	}


	// CLUTCH CONTROL



}


void IDLE_Entry(void) {
	NPreviousState = NCurrentState;
	NCurrentState = SHIFTER_IDLE;
}
void IDLE_Run(void) {


//	if(myInput.BDownShiftRequest || myInput.BUpShiftRequest) {
//		IDLE_Exit();
//		PRESHIFT_Entry();
//	}
}
void IDLE_Exit(void) {

}


void PRESHIFT_Entry(void) {
	NPreviousState = NCurrentState;
	NCurrentState = SHIFTER_PRESHIFT;
}
void PRESHIFT_Run(void) {

}
void PRESHIFT_Exit(void) {

}


void SHIFTING_Entry(void) {
	NPreviousState = NCurrentState;
	NCurrentState = SHIFTER_SHIFTING;
}
void SHIFTING_Run(void) {

}
void SHIFTING_Exit(void) {

}


void POSTSHIFT_Entry(void) {
	NPreviousState = NCurrentState;
	NCurrentState = SHIFTER_POSTSHIFT;
}
void POSTSHIFT_Run(void) {

}
void POSTSHIFT_Exit(void) {

}


void ERROR_Entry(void) {
	NPreviousState = NCurrentState;
	NCurrentState = SHIFTER_ERROR;
}
void ERROR_Run(void) {

}
void ERROR_Exit(void) {

}
