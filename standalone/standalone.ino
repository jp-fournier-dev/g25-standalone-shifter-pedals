/**
    G25 Shifter and Pedal standalone code for Arduino Pro Micro
    Uses https://github.com/MHeironimus/ArduinoJoystickLibrary/tree/version-2.0 as a base with the example code
    Inspired by :
    - pascalh's work : http://www.isrtv.com/forums/topic/13189-diy-g25-shifter-interface-with-h-pattern-sequential-and-handbrake-modes/
    - functionreturnfunction's work : https://github.com/functionreturnfunction/G27_Pedals_and_Shifter/blob/master/G27_Pedals_and_Shifter.ino
*/
#include <Joystick.h>

#include "src/common.h"
#include "src/promicro/micro.h"

Joystick_ joystick;

#define DEBUG_PEDALS 0

#define DEBUG_SERIAL_MS 30
#define NORMAL_MS       8


// Pedal parameters
#define MAX_AXIS 1024
#define ARRAY_SIZE 1
#define DEAD_ZONE 22

ShifterInput input;

//////////////////////////////// SHIFTER /////////////////////////////////
void read_shifter()
{
    read_shifter(&input);
    ShifterOutput output = get_joystick_values(&input);
    set_shifter_outputs(&output, &joystick);
}

//////////////////////////////// END SHIFTER /////////////////////////////////

//////////////////////////////// BEGIN PEDALS /////////////////////////////////
#define TEENSY 0
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


void setup() 
{

  setup_pro_micro();
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
