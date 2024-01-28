/*
 * app.c
 *
 *  Created on: Jan 22, 2024
 *      Author: NikosKr
 */

#include "app.h"

void InitApplication(void) {

}



void RunApplication(InputStruct *input, OutputStruct *output){

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


	output->clutch_position = input->clutch_position;

	output->clutch_detection = (output->clutch_position > clutch_detection_threshold) ? 1 : 0;
	//CLUTCH REACHED THE THRESHOLD

//	app->actual_gear = ;
}





