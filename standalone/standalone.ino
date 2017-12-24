/**
    G25 Shifter and Pedal standalone code for Arduino Pro Micro
    Uses https://github.com/MHeironimus/ArduinoJoystickLibrary/tree/version-2.0 as a base with the example code
    Inspired by :
    - pascalh's work : http://www.isrtv.com/forums/topic/13189-diy-g25-shifter-interface-with-h-pattern-sequential-and-handbrake-modes/
    - functionreturnfunction's work : https://github.com/functionreturnfunction/G27_Pedals_and_Shifter/blob/master/G27_Pedals_and_Shifter.ino
*/

#include <Joystick.h>

#define DEBUG_SERIAL 0
#define DEBUG_SHIFTER 1
#define DEBUG_PEDALS 0

#define DEBUG_SERIAL_MS 30

#define NORMAL_MS       2

#define ANALOG_READ_DELAY delayMicroseconds(200);
// Joystick for game controller emulation
Joystick_ joystick;

//////////////////////////////// SHIFTER /////////////////////////////////
// Shifter pin definitions
#define PIN_DATA        14
#define PIN_PARA        16
#define PIN_CLK         10

#define PIN_MODE_LED    9

#define PIN_X_AXIS      18
#define PIN_Y_AXIS      19

#define ANALOG_X        A0
#define ANALOG_Y        A1

#define SHIFTER_X_12      375
#define SHIFTER_X_56      650


#define SHIFTER_Y_135     800
#define SHIFTER_Y_246     300

#define SHIFTER_SEQ_UP_THRES    450
#define SHIFTER_SEQ_DOWN_THRES  650

// Shifter button array register
#define SHIFTER_BUTTON_COUNT 16
int shifter_buttons[SHIFTER_BUTTON_COUNT];

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

#define SHIFTER_SEQ_DOWN_BTN 20
#define SHIFTER_SEQ_UP_BTN 21

// Shifter position registers
int shifter_x_axis = 0;
int shifter_y_axis = 0;

#define SHIFTER_NEUTRAL 0
#define SHIFTER_1       1
#define SHIFTER_2       2
#define SHIFTER_3       3
#define SHIFTER_4       4
#define SHIFTER_5       5
#define SHIFTER_6       6
#define SHIFTER_REVERSE 7

#define SHIFTER_MODE_H    0
#define SHIFTER_MODE_SEQ  1

#define SHIFTER_REV_OFF   0
#define SHIFTER_REV_ON    1

#define SHIFTER_SEQ_NO_SHIFT      0
#define SHIFTER_SEQ_UP_SHIFT      1
#define SHIFTER_SEQ_DOWN_SHIFT   -1

#define SHIFTER_SEQ_MODE_TAP      1
#define SHIFTER_SEQ_MODE_HOLD     0

int shifter_gear = SHIFTER_NEUTRAL;
int shifter_reverse = SHIFTER_REV_OFF;
int shifter_mode = SHIFTER_MODE_H;
int shifter_seq  = SHIFTER_SEQ_NO_SHIFT;
int shifter_seq_press = SHIFTER_SEQ_NO_SHIFT;

int shifter_seq_mode = SHIFTER_SEQ_MODE_HOLD;

// Functions
void read_shifter_buttons()
{
  digitalWrite(PIN_PARA, LOW);         // Parallel mode: inputs are read into shift register
  delayMicroseconds(10);               // Wait for signal to settle
  digitalWrite(PIN_PARA, HIGH);        // Serial mode: data bits are output on clock falling edge

  for (int i = 0; i < SHIFTER_BUTTON_COUNT; i++)         // Iteration over both 8 bit registers
  {
    digitalWrite(PIN_CLK, LOW);      // Generate clock falling edge
    delayMicroseconds(10);             // Wait for signal to settle

    shifter_buttons[i] = digitalRead(PIN_DATA);   // Read data bit and store it into bit array

    digitalWrite(PIN_CLK, HIGH);     // Generate clock rising edge
    delayMicroseconds(10);             // Wait for signal to settle
  }

  shifter_mode = shifter_buttons[BUT_MODE];
  shifter_reverse = shifter_buttons[BUT_REVERSE];
}

void read_shifter_analogs()
{
    shifter_x_axis = analogRead(ANALOG_X);
    ANALOG_READ_DELAY
    shifter_x_axis = analogRead(ANALOG_X);
    
    shifter_y_axis = analogRead(ANALOG_Y);
    ANALOG_READ_DELAY
    shifter_y_axis = analogRead(ANALOG_Y);
}

void select_shifter_gear()
{
  shifter_gear  = SHIFTER_NEUTRAL;     // Default position of shifter is neutral
  shifter_seq_press = SHIFTER_SEQ_NO_SHIFT;

  if (shifter_mode == SHIFTER_MODE_H)
  {
    if (shifter_x_axis < SHIFTER_X_12) // Shifter on the left?
    {
      if (shifter_y_axis > SHIFTER_Y_135)
      {
        shifter_gear = SHIFTER_1;  // 1st gear
      }
      if (shifter_y_axis < SHIFTER_Y_246)
      {
        shifter_gear = SHIFTER_2;  // 2nd gear
      }
    }
    else if (shifter_x_axis > SHIFTER_X_56) // Shifter on the right?
    {
      if (shifter_y_axis > SHIFTER_Y_135)
      {
        shifter_gear = SHIFTER_5;  // 5th gear
      }
      if (shifter_y_axis < SHIFTER_Y_246)
      {
        if (shifter_reverse == SHIFTER_REV_OFF)
        {
          shifter_gear = SHIFTER_6;  // 6th gear
        }
        else
        {
          shifter_gear = SHIFTER_REVERSE; // Reverse
        }
      }
    }
    else // Shifter is in the middle
    {
      if (shifter_y_axis > SHIFTER_Y_135)
      {
        shifter_gear = SHIFTER_3;  // 3rd gear
      }
      if (shifter_y_axis < SHIFTER_Y_246)
      {
        shifter_gear = SHIFTER_4;  // 4th gear
      }
    }
  }
  else  // If there shifter is set as a sequential mode
  {
    // shifter_seq   = SHIFTER_SEQ_NO_SHIFT;
#define SHIFTER_SEQ_NEU_TO_UP_THRESHOLD 300
#define SHIFTER_SEQ_UP_TO_NEU_THRESHOLD 500
#define SHIFTER_SEQ_NEU_TO_DO_THRESHOLD 590
#define SHIFTER_SEQ_DO_TO_NEU_THRESHOLD 600    
    if (shifter_seq_mode == SHIFTER_SEQ_MODE_TAP)
    {
        if (shifter_seq == SHIFTER_SEQ_NO_SHIFT)
        {
            if (shifter_y_axis > SHIFTER_SEQ_NEU_TO_DO_THRESHOLD)
            {
              shifter_seq = SHIFTER_SEQ_DOWN_SHIFT;
              shifter_seq_press = SHIFTER_SEQ_DOWN_SHIFT;
            }
            else if (shifter_y_axis < SHIFTER_SEQ_NEU_TO_UP_THRESHOLD)
            {
              shifter_seq = SHIFTER_SEQ_UP_SHIFT;
              shifter_seq_press = SHIFTER_SEQ_UP_SHIFT;
            }
        }
        else if (shifter_seq == SHIFTER_SEQ_UP_SHIFT)
        {
            if (shifter_y_axis > SHIFTER_SEQ_UP_TO_NEU_THRESHOLD)
            {
              shifter_seq = SHIFTER_SEQ_NO_SHIFT;
            }
        }
        else
        {
            if (shifter_y_axis < SHIFTER_SEQ_DO_TO_NEU_THRESHOLD)
            {
              shifter_seq = SHIFTER_SEQ_NO_SHIFT;
            }
        }
    }
    else
    {
        if (shifter_y_axis > SHIFTER_SEQ_NEU_TO_DO_THRESHOLD)
        {
          shifter_seq = SHIFTER_SEQ_DOWN_SHIFT;
          shifter_seq_press = SHIFTER_SEQ_DOWN_SHIFT;
        }
        else if (shifter_y_axis < SHIFTER_SEQ_NEU_TO_UP_THRESHOLD)
        {
          shifter_seq = SHIFTER_SEQ_UP_SHIFT;
          shifter_seq_press = SHIFTER_SEQ_UP_SHIFT;
        }       
    }    
  }
}

void set_shifter_inputs()
{
  // Reset shifter position buttons
  for (int i = 0; i < 7; i++)
  {
    joystick.setButton(i, 0);
  }
  joystick.setButton(SHIFTER_SEQ_DOWN_BTN, 0);
  joystick.setButton(SHIFTER_SEQ_UP_BTN, 0);

  if (shifter_mode == SHIFTER_MODE_H)
  {
    if (shifter_gear != SHIFTER_NEUTRAL)
    {
      joystick.setButton(shifter_gear - 1, 1);
    }
  }
  else
  {
    if (shifter_seq_press == SHIFTER_SEQ_DOWN_SHIFT)
    {
      joystick.setButton(SHIFTER_SEQ_DOWN_BTN, 1);
    }
    else if (shifter_seq_press == SHIFTER_SEQ_UP_SHIFT)
    {
      joystick.setButton(SHIFTER_SEQ_UP_BTN, 1);
    }
  }

  // Starts at 4 to skip mode and reverse buttons which are not relevant
  for (int i = 4; i < SHIFTER_BUTTON_COUNT; i++)
  {
    joystick.setButton(i + 3/* 7 - 4 */, shifter_buttons[i]);
  }
}

#if DEBUG_SERIAL && DEBUG_SHIFTER
void serial_debug_shifter()
{
  Serial.print(" X axis: ");
  Serial.println(shifter_x_axis);

  Serial.print(" Y axis: ");
  Serial.println(shifter_y_axis);

  Serial.print(" Digital inputs: ");
  for (int i = 0; i < SHIFTER_BUTTON_COUNT; i++)
  {
    Serial.print(shifter_buttons[i]);
    Serial.print(" ");
  }
  Serial.println("");
  Serial.print(" Gear: ");
  Serial.println(shifter_gear);
  Serial.print(" Mode: ");
  Serial.println(shifter_mode);
  Serial.print(" Shift: ");
  Serial.println(shifter_seq_press);
}
#endif

void read_shifter()
{
  // Basic shifter position detection code
  read_shifter_buttons();
  read_shifter_analogs();
  select_shifter_gear();
  set_shifter_inputs();
  if (shifter_seq_mode == SHIFTER_SEQ_MODE_HOLD)
  {
      digitalWrite(PIN_MODE_LED, HIGH);    
  }
  else 
  {
      digitalWrite(PIN_MODE_LED, LOW);    
  }
#if DEBUG_SERIAL && DEBUG_SHIFTER
  // Display shifter state in serial port
  serial_debug_shifter();
#endif
}

void setup_shifter()
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

//////////////////////////////// END SHIFTER /////////////////////////////////

//////////////////////////////// BEGIN PEDALS /////////////////////////////////

#define PIN_GAS     4
#define PIN_BRAKE   6
#define PIN_CLUTCH  8

#define ANALOG_GAS      A6
#define ANALOG_BRAKE    A7
#define ANALOG_CLUTCH   A8

#define MAX_AXIS 255

// Reading of pedal positions
struct Pedal {
  byte pin;
  int min, max, cur, axis;
};

// Pedal objects
Pedal Gas, Brake, Clutch;

int pedal_axis_value(struct Pedal* pedal)
{
  int physicalRange = pedal->max - pedal->min;
  if (physicalRange <= 0) {
    return 0;
  }
  int result = map(pedal->cur, pedal->min, pedal->max, 0, MAX_AXIS);

  if (result < 5) {
    return 0;
  }
  if (result > MAX_AXIS - 5) {
    return MAX_AXIS;
  }
  return result;
}

void update_pedal(struct Pedal* pedal) {
  pedal->cur = analogRead(pedal->pin);
  ANALOG_READ_DELAY
  pedal->cur = analogRead(pedal->pin);
  ANALOG_READ_DELAY

  // Auto calibration
  pedal->max = pedal->cur > pedal->max ? pedal->cur : pedal->max;
  pedal->min = pedal->min == 0 || pedal->cur < pedal->min ? pedal->cur : pedal->min;

  pedal->axis = pedal_axis_value(pedal);
}

void setup_pedals()
{
  Gas.pin = ANALOG_GAS;
  Brake.pin = ANALOG_BRAKE;
  Clutch.pin = ANALOG_CLUTCH;
  Gas.min = MAX_AXIS;
  Brake.min = MAX_AXIS;
  Clutch.min = MAX_AXIS;

}
#if DEBUG_SERIAL && DEBUG_PEDALS
void serial_debug_pedals()
{
  Serial.print("Gas pedal : ");
  Serial.println(Gas.axis);
  Serial.print("Brake pedal : ");
  Serial.println(Brake.axis);
  Serial.print("Clutch pedal : ");
  Serial.println(Clutch.axis);
}
#endif

void read_pedals()
{
  // Get wheel inputs
  update_pedal(&Gas);
  update_pedal(&Brake);
  update_pedal(&Clutch);

  joystick.setXAxis(Gas.axis);
  joystick.setYAxis(Brake.axis);
  joystick.setZAxis(Clutch.axis);

#if DEBUG_SERIAL && DEBUG_PEDALS
  serial_debug_pedals();
#endif
}

//////////////////////////////// END PEDALS /////////////////////////////////


void setup() {
  setup_shifter();
  setup_pedals();

  // Initialize Joystick Library
  joystick.begin(false);

  joystick.setXAxisRange(0, MAX_AXIS);
  joystick.setYAxisRange(0, MAX_AXIS);
  joystick.setZAxisRange(0, MAX_AXIS);

#if DEBUG_SERIAL
  // Virtual serial interface configuration
  Serial.begin(38400);
#endif
}

void loop() {

  read_shifter();
  read_pedals();

  joystick.sendState();
#if DEBUG_SERIAL
  delay(DEBUG_SERIAL_MS);
#else
  delay(NORMAL_MS);
#endif
}
