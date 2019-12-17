#pragma once

#include "common.h"

#define DEBUG_SHIFTER_OUTPUT 1 && DEBUG_SERIAL
#define DEBUG_SHIFTER_INPUT 1 && DEBUG_SERIAL

#define SHIFTER_INPUT_COUNT 16
#define SHIFTER_BUTTON_COUNT 12

#define SHIFTER_NEUTRAL 1
#define SHIFTER_1       2
#define SHIFTER_2       3
#define SHIFTER_3       4
#define SHIFTER_4       5
#define SHIFTER_5       6
#define SHIFTER_6       7
#define SHIFTER_REVERSE 8

#define SHIFTER_MODE_H    0
#define SHIFTER_MODE_SEQ  1

#define SHIFTER_REV_OFF   0
#define SHIFTER_REV_ON    1

#define SHIFTER_SEQ_NO_SHIFT      0
#define SHIFTER_SEQ_UP_SHIFT      1
#define SHIFTER_SEQ_DOWN_SHIFT   -1

#define SHIFTER_SEQ_MODE_TAP      1
#define SHIFTER_SEQ_MODE_HOLD     0

typedef struct ShifterInput
{
    int shifter_buttons[SHIFTER_INPUT_COUNT];
    int shifter_mode;
    int shifter_reverse_enable;

    int shifter_x_axis;
    int shifter_y_axis;
};


// Shifter button definitions
#define BUT_REVERSE         1
#define BUT_MODE            3
#define BUT_RED_CENTERRIGHT 4
#define BUT_RED_CENTERLEFT  5
#define BUT_RED_RIGHT       6
#define BUT_RED_LEFT        7
#define BUT_BLACK_TOP       8
#define BUT_BLACK_RIGHT     9
#define BUT_BLACK_LEFT      10
#define BUT_BLACK_BOTTOM    11
#define BUT_DPAD_RIGHT      12
#define BUT_DPAD_LEFT       13
#define BUT_DPAD_BOTTOM     14
#define BUT_DPAD_TOP        15

#define SHIFTER_SEQ_DOWN_BTN 21
#define SHIFTER_SEQ_UP_BTN 22

typedef struct ShifterOutput
{
    int current_gear;

    int sequential_direction;

    int shifter_mode;

    int shifter_buttons[SHIFTER_BUTTON_COUNT];
};

ShifterOutput init_output();