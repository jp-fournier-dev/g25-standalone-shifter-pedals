/**
 *  G25 Shifter and Pedal standalone code for Arduino Pro Micro
 *  Uses https://github.com/MHeironimus/ArduinoJoystickLibrary/tree/version-2.0 as a base with the example code
 *  Inspired by : 
 *  - pascalh's work : http://www.isrtv.com/forums/topic/13189-diy-g25-shifter-interface-with-h-pattern-sequential-and-handbrake-modes/
 *  - functionreturnfunction's work : https://github.com/functionreturnfunction/G27_Pedals_and_Shifter/blob/master/G27_Pedals_and_Shifter.ino
 */

#include <Joystick.h>

#define DEBUG_SERIAL 1

//////////////////////////////// SHIFTER /////////////////////////////////
// Shifter pin definitions
#define PIN_DATA        14
#define PIN_PARA        16
#define PIN_CLK         10

#define PIN_X_AXIS      18
#define PIN_Y_AXIS      19

#define ANALOG_X        A0
#define ANALOG_Y        A1

#define SHIFTER_X_12      500
#define SHIFTER_X_56      700

#define SHIFTER_Y_135     800
#define SHIFTER_Y_246     300

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

int shifter_gear = SHIFTER_NEUTRAL;

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
}

void read_shifter_analogs()
{
    shifter_x_axis = analogRead(ANALOG_X); 
    shifter_y_axis = analogRead(ANALOG_Y); 
}

void select_shifter_gear()
{
    shifter_gear = SHIFTER_NEUTRAL;     // Default position of shifter is neutral

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
            shifter_gear = SHIFTER_6;  // 6th gear
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

#if DEBUG_SERIAL
void serial_debug_shifter()
{  
    Serial.print(" X axis: ");
    Serial.println(shifter_x_axis);
    
    Serial.print(" Y axis: ");
    Serial.println(shifter_y_axis);
    
    Serial.print(" Digital inputs: ");
    for(int i = 0; i < SHIFTER_BUTTON_COUNT; i++)
    {
        Serial.print(shifter_buttons[i]);
        Serial.print(" ");
    }  
    Serial.println("");
    Serial.println(" Gear: ");
    Serial.println(shifter_gear);  
}
#endif

void read_shifter()
{
    // Basic shifter position detection code
    read_shifter_buttons();
    read_shifter_analogs();
    select_shifter_gear();
    
#if DEBUG_SERIAL
    // Display shifter state in serial port
    serial_debug_shifter();
#endif
}

void setup_shifter()
{
    // Initialize shifter analog inputs
    pinMode(ANALOG_X, INPUT_PULLUP);
    pinMode(ANALOG_Y, INPUT_PULLUP);
  
    // Initialize shifter data pins
    pinMode(PIN_DATA, INPUT);
    pinMode(PIN_PARA, OUTPUT);
    pinMode(PIN_CLK, OUTPUT);
    
    // Digital outputs initialization
    digitalWrite(PIN_PARA, HIGH);
    digitalWrite(PIN_CLK, HIGH);  
}

//////////////////////////////// END SHIFTER /////////////////////////////////

// Create the Joystick
Joystick_ Joystick;

// Constant that maps the phyical pin to the joystick button.
const int pinToButtonMap = 9;

void setup() {
    setup_shifter();
    
    // Initialize Joystick Library
    Joystick.begin();

#if DEBUG_SERIAL
    // Virtual serial interface configuration
    Serial.begin(38400);
#endif
}

void loop() {

  read_shifter();

  delay(50);
}
