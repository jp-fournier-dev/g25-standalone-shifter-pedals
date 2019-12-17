#pragma once

#include "../common.h"
#include "../shifter.h"
#include <Joystick.h>

// Shifter pin definitions
#define PIN_DATA        14
#define PIN_PARA        16
#define PIN_CLK         10

#define PIN_MODE_LED    9

#define PIN_X_AXIS      18
#define PIN_Y_AXIS      19

#define ANALOG_X        A0
#define ANALOG_Y        A1

// Shifter zone definitions
#define SHIFTER_X_12      375
#define SHIFTER_X_56      650
#define SHIFTER_Y_135     800
#define SHIFTER_Y_246     300

// Shifter seq zone definition
#define SHIFTER_SEQ_UP_THRES    450
#define SHIFTER_SEQ_DOWN_THRES  650

#define SHIFTER_SEQ_NEU_TO_UP_THRESHOLD 400
#define SHIFTER_SEQ_UP_TO_NEU_THRESHOLD 400
#define SHIFTER_SEQ_NEU_TO_DO_THRESHOLD 565
#define SHIFTER_SEQ_DO_TO_NEU_THRESHOLD 565   

#define BUTTON_OFFSET 8

void setup_pro_micro();

void read_buttons(ShifterInput *input);

void read_analog(ShifterInput *input);

void read_shifter(ShifterInput *input);

int h_shifter_select(ShifterInput *input);

int seq_shifter_value(ShifterInput *input);

ShifterOutput get_joystick_values(ShifterInput *input);

void set_shifter_outputs(ShifterOutput *output, Joystick_ *joystick);

#if DEBUG_SHIFTER_OUTPUT
void serial_debug_shifter_output(ShifterOutput *output);
#endif

#if DEBUG_SHIFTER_INPUT
void serial_debug_shifter_input(ShifterInput *input);
#endif