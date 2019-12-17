
/**
    G25 Shifter and Pedal standalone code for Arduino Pro Micro
    Uses https://github.com/MHeironimus/ArduinoJoystickLibrary/tree/version-2.0 as a base with the example code
    Inspired by :
    - pascalh's work : http://www.isrtv.com/forums/topic/13189-diy-g25-shifter-interface-with-h-pattern-sequential-and-handbrake-modes/
    - functionreturnfunction's work : https://github.com/functionreturnfunction/G27_Pedals_and_Shifter/blob/master/G27_Pedals_and_Shifter.ino
*/


#define TEENSY 0
#if !TEENSY
#include <Joystick.h>
Joystick_ joystick;
#endif

#define DEBUG_SERIAL 1
#define DEBUG_SHIFTER 1
#define DEBUG_PEDALS 0

#define DEBUG_SERIAL_MS 30

#define NORMAL_MS       8

#define ANALOG_READ_DELAY delayMicroseconds(100);
// Joystick for game controller emulation


// Pedal parameters
#define MAX_AXIS 1024
#define ARRAY_SIZE 1
#define DEAD_ZONE 22
  
// 35

//////////////////////////////// SHIFTER /////////////////////////////////
// Shifter pin definitions

#if TEENSY 
#define PIN_DATA        17
#define PIN_PARA        16
#define PIN_CLK         15

#define PIN_MODE_LED    13

#define PIN_X_AXIS      19
#define PIN_Y_AXIS      20

#define ANALOG_X        A5
#define ANALOG_Y        A6

#else // !TEESNY
#define PIN_DATA        14
#define PIN_PARA        16
#define PIN_CLK         10

#define PIN_MODE_LED    9

#define PIN_X_AXIS      18
#define PIN_Y_AXIS      19

#define ANALOG_X        A0
#define ANALOG_Y        A1

#endif

#if TEENSY
#define SHIFTER_X_12      600
#define SHIFTER_X_56      900
#define SHIFTER_Y_135     900
#define SHIFTER_Y_246     400

#define SHIFTER_SEQ_UP_THRES    450
#define SHIFTER_SEQ_DOWN_THRES  650

#else
#define SHIFTER_X_12      375
#define SHIFTER_X_56      650
#define SHIFTER_Y_135     800
#define SHIFTER_Y_246     300

#define SHIFTER_SEQ_UP_THRES    450
#define SHIFTER_SEQ_DOWN_THRES  650
#endif

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

#define SHIFTER_SEQ_DOWN_BTN 21
#define SHIFTER_SEQ_UP_BTN 22

// Shifter position registers
int shifter_x_axis = 0;
int shifter_y_axis = 0;

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
    //ANALOG_READ_DELAY
    //shifter_x_axis = analogRead(ANALOG_X);
    
    shifter_y_axis = analogRead(ANALOG_Y);
    //ANALOG_READ_DELAY
    //shifter_y_axis = analogRead(ANALOG_Y);
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
#if TEENSY
  #define SHIFTER_SEQ_NEU_TO_UP_THRESHOLD 500
  #define SHIFTER_SEQ_UP_TO_NEU_THRESHOLD 550
  #define SHIFTER_SEQ_NEU_TO_DO_THRESHOLD 900
  #define SHIFTER_SEQ_DO_TO_NEU_THRESHOLD 800    
#else    
  #define SHIFTER_SEQ_NEU_TO_UP_THRESHOLD 400
  #define SHIFTER_SEQ_UP_TO_NEU_THRESHOLD 400
  #define SHIFTER_SEQ_NEU_TO_DO_THRESHOLD 565
  #define SHIFTER_SEQ_DO_TO_NEU_THRESHOLD 565    
#endif
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
  for (int i = 1; i < 8; i++)
  {
    #if TEENSY
    Joystick.button(i, 0);
    #else
    joystick.setButton(i, 0);
    #endif
  }
#if TEENSY
    Joystick.button(SHIFTER_SEQ_DOWN_BTN, 0);
    Joystick.button(SHIFTER_SEQ_UP_BTN, 0);

#else
    joystick.setButton(SHIFTER_SEQ_DOWN_BTN, 0);
    joystick.setButton(SHIFTER_SEQ_UP_BTN, 0);
#endif

  if (shifter_mode == SHIFTER_MODE_H)
  {
    if (shifter_gear != SHIFTER_NEUTRAL)
    {
#if TEENSY
    Joystick.button(shifter_gear - 1, 1);

#else
      joystick.setButton(shifter_gear - 1, 1);
#endif
    }
  }
  else
  {
    if (shifter_seq_press == SHIFTER_SEQ_DOWN_SHIFT)
    {
#if TEENSY
    Joystick.button(SHIFTER_SEQ_DOWN_BTN, 1);
#else
      joystick.setButton(SHIFTER_SEQ_DOWN_BTN, 1);
#endif
    }
    else if (shifter_seq_press == SHIFTER_SEQ_UP_SHIFT)
    {
#if TEENSY
    Joystick.button(SHIFTER_SEQ_UP_BTN, 1);
#else
      joystick.setButton(SHIFTER_SEQ_UP_BTN, 1);
#endif
    }
  }

  // Starts at 4 to skip mode and reverse buttons which are not relevant
  for (int i = 4; i < SHIFTER_BUTTON_COUNT; i++)
  {
    
#if TEENSY
    Joystick.button(i + 4/* 7 - 4 */, shifter_buttons[i]);
#else
    joystick.setButton(i + 4/* 7 - 4 */, shifter_buttons[i]);
#endif
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

#if TEENSY 
#define PIN_GAS     23
#define PIN_BRAKE   22
#define PIN_CLUTCH  21

#define ANALOG_GAS      A9
#define ANALOG_BRAKE    A8
#define ANALOG_CLUTCH   A7

#else
#define PIN_GAS     4
#define PIN_BRAKE   6
#define PIN_CLUTCH  8

#define ANALOG_GAS      A6
#define ANALOG_BRAKE    A7
#define ANALOG_CLUTCH   A8
#endif

// Reading of pedal positions
struct Pedal {
  byte pin;
  int min, max, cur, axis;
  int values[ARRAY_SIZE];
  int current_index = 0;
};

// Pedal objects
Pedal Gas, Brake, Clutch;
int pedal_axis_value(struct Pedal* pedal)
{
  int physicalRange = pedal->max - pedal->min;
  if (physicalRange <= 0) {
    return 0;
  }
  int result = map(pedal->cur, pedal->min + DEAD_ZONE, pedal->max - DEAD_ZONE, 0, MAX_AXIS);

  if (result < DEAD_ZONE) {
    result = 0;
  }
  if (result > MAX_AXIS - DEAD_ZONE) {
    result = MAX_AXIS;
  }
  return result;
}

int get_average(struct Pedal* pedal) {
  int result = 0;
  for (int i = 0; i < ARRAY_SIZE; i++){
    result += pedal->values[i];
  }
  return result / ARRAY_SIZE;
}

void update_pedal(struct Pedal* pedal) {
  
  //pedal->cur = analogRead(pedal->pin);
#if DEBUG_SERIAL && DEBUG_PEDALS
 // Serial.print("First Read : ");
 // Serial.println(pedal->cur);
#endif
 // ANALOG_READ_DELAY
  pedal->cur = analogRead(pedal->pin);
#if DEBUG_SERIAL && DEBUG_PEDALS
  Serial.print("Second Read : ");
  Serial.println(pedal->cur);
#endif
 // ANALOG_READ_DELAY

  // Auto calibration
  pedal->max = pedal->cur > pedal->max ? pedal->cur : pedal->max;
  pedal->min = pedal->min == 0 || pedal->cur < pedal->min ? pedal->cur : pedal->min;

  // Get current value
  pedal->values[pedal->current_index % ARRAY_SIZE] = pedal_axis_value(pedal);
  pedal->current_index = ++pedal->current_index % ARRAY_SIZE;
  int result = get_average(pedal);  

  pedal->axis = result;
}

void setup_pedals()
{
  Gas.pin = ANALOG_GAS;
  Brake.pin = ANALOG_BRAKE;
  Clutch.pin = ANALOG_CLUTCH;
  Gas.min = MAX_AXIS;
  Brake.min = MAX_AXIS;
  Clutch.min = MAX_AXIS;
  Gas.max = 0;
  Brake.max = 0;
  Clutch.max = 0;

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
#if DEBUG_SERIAL && DEBUG_PEDALS
  Serial.println("Gas pedal data");
#endif
  update_pedal(&Gas);
 #if DEBUG_SERIAL && DEBUG_PEDALS
  Serial.println("//////////////");
#endif
  update_pedal(&Brake);
  update_pedal(&Clutch);

#if TEENSY
  Joystick.X(Gas.axis);
  Joystick.Y(Brake.axis);
  Joystick.Z(Clutch.axis);
#else
  joystick.setXAxis(Gas.axis);
  joystick.setYAxis(Brake.axis);
  joystick.setZAxis(Clutch.axis);
#endif 

#if DEBUG_SERIAL && DEBUG_PEDALS
  serial_debug_pedals();
#endif
}

//////////////////////////////// END PEDALS /////////////////////////////////


void setup() {
  setup_shifter();
  setup_pedals();


#if TEENSY
  Joystick.useManualSend(true);
#else
  // Initialize Joystick Library
  joystick.begin(false);
  joystick.setXAxisRange(0, MAX_AXIS);
  joystick.setYAxisRange(0, MAX_AXIS);
  joystick.setZAxisRange(0, MAX_AXIS);
#endif

#if DEBUG_SERIAL
  // Virtual serial interface configuration
  Serial.begin(38400);
#endif
}

void loop() {

  read_shifter();
  read_pedals();

#if TEENSY
  Joystick.send_now();
#else
  joystick.sendState();
#endif
  
#if DEBUG_SERIAL
  delay(DEBUG_SERIAL_MS);
#else
  delay(NORMAL_MS);
#endif
}
