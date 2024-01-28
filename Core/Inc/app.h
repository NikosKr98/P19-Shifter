/*
 * application.h
 *
 *  Created on: Jan 22, 2024
 *      Author: NikosKr
 */

#ifndef INC_APP_H_
#define INC_APP_H_

#include "inputs.h"
#include "outputs.h"

#define clutch_detection_threshold 1000



// TODO: create state machine functions and variables. Remember current state, previous state and init, body and end for every state

void InitApplication(void);
void RunApplication(InputStruct *input, OutputStruct *output);

#endif /* INC_APP_H_ */
