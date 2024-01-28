/*
 * application.c
 *
 *  Created on: Jan 22, 2024
 *      Author: NikosKr
 */

#include "application.h"

void Application_init(ApplicationStruct *application){

	application->current_gear = 0;
	application->target_gear = 0;
	application->up_port_state = 0;
	application->down_port_state = 0;
	application->shift_end_time= 0;
	application->up_button_out= 0;
	application->down_button_out= 0;
	application->clutch_position =0;
	application->actual_gear =0;
}

uint16_t gearTable[][2] = {

    {3600, 3750},  // Gear 0
    {3900, 4000},  // Gear 1
    {3200, 3300},  // Gear 2
    {2250, 2400},  // Gear 3
    {1300, 1500},  // Gear 4
    {400, 600}     // Gear 5

};


uint16_t calculateActualGear(InputStruct *input) {

    for (int gear = 0; gear < sizeof(gearTable) / sizeof(gearTable[0]); ++gear) {
        if (input ->actual_gear > gearTable[gear][0] && input->actual_gear < gearTable[gear][1]) {
            return gear;
        }
    }
    return 255; // If no match found, return 255!
}


void Application(InputStruct *input, ApplicationStruct *application){

//	if(application->current_gear != application -> actual_gear){
//		application->current_gear = application -> actual_gear;
//		application->target_gear = application -> actual_gear;
//	}


	// UP BUTTON REQUEST CONFIRMATION CHECK !
	if(input->up_button_request && !application->up_button_status && button_previous<current && !application-> up_port_state){
		button_previous=current;
		button_previous+=button_interval;
		application->up_button_out = 1;
		application->up_button_status=1;
	}
	else if(!input->up_button_request && application->up_button_status){
		application->up_button_out =0;
		application->up_button_status=0;
	}


	// DOWNM BUTTON REQUEST CONFIRMATION CHECK !
	if(input-> down_button_request && !application->down_button_status && button_previous<current && !application-> down_port_state){
		button_previous=current;
		button_previous+=button_interval;
		application->down_button_out = 1;
		application->down_button_status=1;

	}
	else if(!input->down_button_request && application->down_button_status){
		application->down_button_out =0;
		application->down_button_status=0;

	}


	application->clutch_position = input->clutch_position;

	application->clutch_detection = (application->clutch_position > clutch_detection_threshold) ? 1 : 0;
	//CLUTCH REACHED THE THRESHOLD

	application->actual_gear = calculateActualGear(input);
}







