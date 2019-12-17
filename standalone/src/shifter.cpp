#include "shifter.h"

ShifterOutput init_output()
{
    ShifterOutput output;
    output.current_gear = SHIFTER_NEUTRAL;
    output.sequential_direction = SHIFTER_SEQ_NO_SHIFT;
    output.shifter_mode = SHIFTER_MODE_H;
    for (int i = 0; i < SHIFTER_BUTTON_COUNT; i++)
    {
        output.shifter_buttons[i] = 0;
    }
    return output;
}