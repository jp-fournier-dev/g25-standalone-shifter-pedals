#include <Joystick.h>

#include "../shifter.h"
#include "micro.h"


void setup_pro_micro()
{
  // Initialize shifter analog inputs
  // Initialize shifter data pins
  pinMode(PIN_DATA, INPUT);
  pinMode(PIN_PARA, OUTPUT);
  pinMode(PIN_CLK, OUTPUT);
  pinMode(PIN_MODE_LED, OUTPUT);


  // Digital outputs initialization
  digitalWrite(PIN_PARA, HIGH);
  digitalWrite(PIN_CLK, HIGH);
  digitalWrite(PIN_MODE_LED, HIGH);
}



void read_buttons(ShifterInput *input)
{    
    digitalWrite(PIN_PARA, LOW);         // Parallel mode: inputs are read into shift register
    delayMicroseconds(10);               // Wait for signal to settle
    digitalWrite(PIN_PARA, HIGH);        // Serial mode: data bits are output on clock falling edge

    for (int i = 0; i < SHIFTER_INPUT_COUNT; i++)         // Iteration over both 8 bit registers
    {
        digitalWrite(PIN_CLK, LOW);      // Generate clock falling edge
        delayMicroseconds(10);             // Wait for signal to settle

        input->shifter_buttons[i] = digitalRead(PIN_DATA);   // Read data bit and store it into bit array

        digitalWrite(PIN_CLK, HIGH);     // Generate clock rising edge
        delayMicroseconds(10);             // Wait for signal to settle
    }

    input->shifter_mode = input->shifter_buttons[BUT_MODE];
    input->shifter_reverse_enable = input->shifter_buttons[BUT_REVERSE];
}

void read_analog(ShifterInput *input)
{
    input->shifter_x_axis = analogRead(ANALOG_X);
    input->shifter_y_axis = analogRead(ANALOG_Y);
}

void read_shifter(ShifterInput *input)
{
    read_buttons(input);
    read_analog(input);
}

int h_shifter_select(ShifterInput *input)
{
    int shifter_x_axis = input->shifter_x_axis;
    int shifter_y_axis = input->shifter_y_axis;
    int shifter_reverse = input->shifter_reverse_enable;

    if (shifter_x_axis < SHIFTER_X_12) // Shifter on the left?
    {
        if (shifter_y_axis > SHIFTER_Y_135)
        {
            return SHIFTER_1;  // 1st gear
        }
        if (shifter_y_axis < SHIFTER_Y_246)
        {
            return SHIFTER_2;  // 2nd gear
        }
    }
    else if (shifter_x_axis > SHIFTER_X_56) // Shifter on the right?
    {
        if (shifter_y_axis > SHIFTER_Y_135)
        {
            return SHIFTER_5;  // 5th gear
        }
        if (shifter_y_axis < SHIFTER_Y_246)
        {
            if (shifter_reverse == SHIFTER_REV_OFF)
            {
                return SHIFTER_6;  // 6th gear
            }
            else
            {
                return SHIFTER_REVERSE; // Reverse
            }
        }
    }
    else // Shifter is in the middle
    {
        if (shifter_y_axis > SHIFTER_Y_135)
        {
            return SHIFTER_3;  // 3rd gear
        }
        if (shifter_y_axis < SHIFTER_Y_246)
        {
            return SHIFTER_4;  // 4th gear
        }
    }
    return SHIFTER_NEUTRAL;
}


int seq_shifter_value(ShifterInput *input)
{
    int shifter_x_axis = input->shifter_x_axis;
    int shifter_y_axis = input->shifter_y_axis;

    if (shifter_y_axis > SHIFTER_SEQ_NEU_TO_DO_THRESHOLD)
    {        
        return SHIFTER_SEQ_DOWN_SHIFT;
    }
    else if (shifter_y_axis < SHIFTER_SEQ_NEU_TO_UP_THRESHOLD)
    {
        return SHIFTER_SEQ_UP_SHIFT;
    }       
}

ShifterOutput get_joystick_values(ShifterInput *input)
{
#if DEBUG_SHIFTER_INPUT
    serial_debug_shifter_input(input);
#endif
    ShifterOutput output = init_output();

    if (input->shifter_mode == SHIFTER_MODE_H)
    {
        output.current_gear = h_shifter_select(input);
    }
    else 
    {
        output.sequential_direction = seq_shifter_value(input);
    }

    return output;
}


void set_shifter_outputs(ShifterOutput *output, Joystick_ *joystick)
{
#if DEBUG_SHIFTER_OUTPUT
    serial_debug_shifter_output(output);
#endif

    // Reset shifter position buttons
    for (int i = 1; i < 8; i++)
    {
        joystick->setButton(i, 0);
    }
    
    joystick->setButton(SHIFTER_SEQ_DOWN_BTN, 0);
    joystick->setButton(SHIFTER_SEQ_UP_BTN, 0);

    if (output->shifter_mode == SHIFTER_MODE_H)
    {
        if (output->current_gear != SHIFTER_NEUTRAL)
        {
            joystick->setButton(output->current_gear - 1, 1);
        }
    }
    else
    {
        if (output->sequential_direction == SHIFTER_SEQ_DOWN_SHIFT)
        {
            joystick->setButton(SHIFTER_SEQ_DOWN_BTN, 1);
        }
        else if (output->sequential_direction == SHIFTER_SEQ_UP_SHIFT)
        {
            joystick->setButton(SHIFTER_SEQ_UP_BTN, 1);
        }
    }

    // Starts at 4 to skip mode and reverse buttons which are not relevant
    for (int i = 0; i < SHIFTER_BUTTON_COUNT; i++)
    {
        joystick->setButton(i + BUTTON_OFFSET/* 7 - 4 */, output->shifter_buttons[i]);
    }
}

#if DEBUG_SHIFTER_OUTPUT
void serial_debug_shifter_output(ShifterOutput *output)
{

  Serial.print(" Digital inputs: ");
  for (int i = 0; i < SHIFTER_BUTTON_COUNT; i++)
  {
    Serial.print(output->shifter_buttons[i]);
    Serial.print(" ");
  }
  Serial.println("");
  Serial.print(" Gear: ");
  Serial.println(output->current_gear);
  Serial.print(" Mode: ");
  Serial.println(output->shifter_mode);
  Serial.print(" Shift: ");
  Serial.println(output->sequential_direction);
}
#endif

#if DEBUG_SHIFTER_INPUT
void serial_debug_shifter_input(ShifterInput *input)
{
    Serial.print(" X axis: ");
    Serial.println(input->shifter_x_axis);

    Serial.print(" Y axis: ");
    Serial.println(input->shifter_y_axis);
    
    Serial.print(" Reverse button: ");
    Serial.println(input->shifter_reverse_enable);
    Serial.print(" Mode: ");
    Serial.println(input->shifter_mode);

    for (int i = 0; i < SHIFTER_INPUT_COUNT; i++)
    {
        Serial.print(input->shifter_buttons[i]);
        Serial.print(" ");
    }    
}
#endif