#include <I2CSlaveMode.h>
#include <Adafruit_PWMServoDriver.h>

//Initialize arduino i2c for config data
I2CSlaveMode agent = new I2CSlaveMode();
//Initialize Adafruit servo driver board to address 0x40 (the default)
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVO_FREQ 50 // Analog servos run at ~50 Hz updates
unsigned long previousMillis = 0;
unsigned long interval = 5000;
byte cur_cycle = 0; //The current cycle being executed
byte max_cycle = 16; //The maximum number of cycles possible in a gesture

enum finger {
  pinky = 0,
  ring,
  middle,
  pointer,
  thumb
};

// Depending on your servo make, the pulse width min and max may vary, you 
// want these to be as small/large as possible without hitting the hard stop
// for max range. You'll have to tweak them as necessary to match the servos you
// have!
// Tweak them by setting values in EEPROM registers
// Min starts at about 600 microseconds 
// Max starts at about 2400 microseconds 
enum uS_offset {
  gesture_hold = 0x04,//Register with a multiplier for number of seconds to hold the gesture 
  uS_divisor = 0x05, //Register holding the number of microsecond offsets possible
  uS_min = 0x06,     //Register holding the minimum microsecond length
  uS_max = 0x08,     //Register holding the maximum microsecond length
  step_delay = 0xFF, //Register holding the delay between steps for a gesture
};

// Define microseconds min, max, divisor(num steps) and range.
// initialize it in setup
uint16_t _uS_min;
uint16_t _uS_max;
uint8_t _uS_divisor;
uint16_t _range;

/*************
 * These values represent fractions of the microsecond range with 0x00 being the lowest 
 * possible value (or pos_min) and 0xFF being the highest possible value
 */
const uint8_t finger_close[] = {0x08,0x18,0x28,0x38,0x48,0x58,0x68,0x78,0x88,0x98,0xA8,0xB8,0xC8,0xD8,0xE8,0xF8};
const uint8_t finger_open[] = {0xF8,0xE8,0xD8,0xC8,0xB8,0xA8,0x98,0x88,0x78,0x68,0x58,0x48,0x38,0x28,0x18,0x08};

// Default is do nothing
uint8_t* cur_pinky = NULL;
uint8_t* cur_ring = NULL;
uint8_t* cur_middle = NULL;
uint8_t* cur_pointer = NULL;
uint8_t* cur_thumb = NULL;

//Gesture registers
/*************** 
 *  Each finger can be uniquely manipulated across a 16 frame cycle
 *  Each frame can represent one of the following operations
 *  0x01 - finger open - step through the finger open sequence
 *  0x02 - finger closed - step through the finger closed sequence
 *  0x04 - no change - do not actuate finger in any direction
 *  All other values have no defined behavior
 *  
 *  An example frame for the peace sign would look like this
 *  register  : value
 *  0x10      : 0x02
 *  0x20      : 0x02
 *  0x30      : 0x01
 *  0x40      : 0x01
 *  0x50      : 0x02
 *  
 *  An example of opening and closing the pointer finger with a pause between each frame
 *  for 10 frames would look like this
 *  register 0x10: 0x01, 0x04, 0x02, 0x04, 0x01, 0x04, 0x02, 0x04, 0x01, 0x04 ...
 */
enum gesture_offset {
  g_pinky = 0x10,
  g_ring = 0x20,
  g_middle = 0x30,
  g_pointer = 0x40,
  g_thumb = 0x50,
} ;


uint16_t get_double_reg(uint8_t reg) {
  uint16_t value = agent.getRegister(reg);
  value <<= 8;
  value |= agent.getRegister(reg+1);
  return value;
}

uint16_t calculate_uS(byte next_uS) {
  return _uS_min + ( ((float)next_uS/_uS_divisor) * _range);
}




void setup() {

  Serial.begin(9600);
  Serial.println("\nFinger Control");
  Serial.print("I2C address:");
  Serial.println(agent.getAddress());

  pwm.begin(); //This calls Wire.begin()
  pwm.sleep();
  pwm.setPWMFreq(SERVO_FREQ);  // This is the maximum PWM frequency

  interval = agent.getRegister(gesture_hold) * 500;
  // Arbitrarilly decided that the interval should not be less than 1 second 
  // or more than 20 seconds
  interval = constrain(interval, 1000, 20000);
  
  _uS_min = get_double_reg(uS_min);
  _uS_max = get_double_reg(uS_max);
  _uS_divisor = (uint8_t)agent.getRegister(uS_divisor);
  _range = _uS_max - _uS_min;  
  
}

uint8_t* get_finger_gesture(byte fg) {
  uint8_t* finger_gesture = NULL;
  switch (fg) {
    case 0x01:
      finger_gesture = finger_open;
      break;
    case 0x02:
      finger_gesture = finger_close;
      break;
    default:
      finger_gesture = NULL;
      break;      
  }
  return finger_gesture;
}

void loop() {

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    cur_pinky = get_finger_gesture(agent.getRegister(g_pinky+cur_cycle));
    cur_ring = get_finger_gesture(agent.getRegister(g_ring+cur_cycle));
    cur_middle = get_finger_gesture(agent.getRegister(g_middle+cur_cycle));
    cur_pointer = get_finger_gesture(agent.getRegister(g_pointer+cur_cycle));
    cur_thumb = get_finger_gesture(agent.getRegister(g_thumb+cur_cycle));

    //If anything is not null do gesture()
    if (cur_pinky || cur_ring || cur_middle || cur_pointer || cur_thumb) {
      pwm.wakeup();
      gesture();
    }
    cur_cycle = (cur_cycle < max_cycle) ? cur_cycle+1 : 0;
  }
  pwm.sleep();
  

}

void gesture() {
  // The complete number of actions in a single finger motion is 16
  for (int i=0; i<16; i++) {  
    if (cur_pinky != NULL) {next_step(cur_pinky[i], pinky);}
    if (cur_ring != NULL) {next_step(cur_ring[i], ring);}
    if (cur_middle != NULL) {next_step(cur_middle[i], middle);}
    if (cur_pointer != NULL) {next_step(cur_pointer[i], pointer);}
    if (cur_thumb != NULL) {next_step(cur_thumb[i], thumb);}
    
    delay(50);
  }
  
}



void next_step(uint8_t step_index, finger f) {
  pwm.writeMicroseconds((byte)f, constrain(calculate_uS(step_index), _uS_min, _uS_max));
}
