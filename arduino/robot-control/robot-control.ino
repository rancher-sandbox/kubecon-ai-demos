#include <I2CSlaveMode.h>
#include <Adafruit_PWMServoDriver.h>

#define DEBUG 1

//Initialize arduino i2c for config data
I2CSlaveMode agent = new I2CSlaveMode();
//Initialize Adafruit servo driver board to address 0x40 (the default)
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVO_FREQ 50 // Analog servos run at ~50 Hz updates
unsigned long previousMillis = 0;
unsigned long interval = 5000;

enum servo {
  pinky = 0,
  ring,
  middle,
  pointer,
  thumb,
  wrist
};

// Depending on your servo make, the pulse width min and max may vary, you 
// want these to be as small/large as possible without hitting the hard stop
// for max range. You'll have to tweak them as necessary to match the servos you
// have!
// Tweak them by setting values in EEPROM registers
// Min starts at about 600 microseconds 
// Max starts at about 2400 microseconds 
enum runtime_offset {
  gesture_hold = 0x04,//Register with a multiplier for number of seconds to hold the gesture 
  /*********************
   * the activity_state register value can be used by the client to activate or deactivate the current gesture
   * If the state is less than 0xFF, the program will run the number of times
   * specified by the value.
   * If the activity_state value is 0x00, the program will stop
   * The program checks the state periodically 
   * based on the timing interval specified using gesture_hold value
   *********************************/
  activity_state = 0x05 // Register indicating whether to DELAY, RUN, or STOP processing
};

// Will be set to the value of the activity_state register if the cur and stored state are different
uint8_t active_state = 0x00;
uint8_t cur_state = active_state;

uint8_t const signature_len = 16;
// Default is do nothing
uint8_t sig_pinky[signature_len] = {0};
uint8_t sig_ring[signature_len] = {0};
uint8_t sig_middle[signature_len] = {0};
uint8_t sig_pointer[signature_len] = {0};
uint8_t sig_thumb[signature_len] = {0};
uint8_t sig_wrist[signature_len] = {0};

uint8_t hand[][signature_len] = { {sig_pinky}, {sig_ring}, {sig_middle}, {sig_pointer}, {sig_thumb}, {sig_wrist} };

//Gesture registers (signatures)
/*************** 
 *  Each servo can be uniquely manipulated 
 *  Each servo has 16 registers for detailing its range and desired position
 * 
 *  Register offsets from the gesture_offset
 *  0-1   : Min extend - the absolute minimum uS this servo can go
 *  2-3   : Max extend - the absolute maximum uS this servo can go
 *  4-5   : Min relax - min extension in uS but relieve the tension slightly toward max
 *  6-7   : Max relax - max extention in uS but relieve the tension slightly toward min
 *  8-9   : position to move to in uS - will not exceed extend values
 *  A     : if > 0 relax after move
 *  B     : delay between extend and relax moves. Range is between 0x00 and 0xFF
 *  C-E   : UNUSED
 *  F     : if > 0 activate this signature otherwise do nothing (IMPORTANT this is what indicates to move the servo or not)
 *    
 *  An example signature to make the peace sign gesture would look like this
 *  register start  : value
 *  0x10            : 0x05 0x64 0x09 0x60 0x05 0xC0 0x07 0xD0 0x09 0x60 0x01 0x32 0x00 0x00 0x00 0x01
 *  0x20            : 0x05 0x64 0x09 0x60 0x05 0xC0 0x07 0xD0 0x09 0x60 0x01 0x32 0x00 0x00 0x00 0x01
 *  0x30            : 0x05 0x64 0x09 0x60 0x05 0xC0 0x07 0xD0 0x05 0x64 0x01 0x32 0x00 0x00 0x00 0x01
 *  0x40            : 0x05 0x64 0x09 0x60 0x05 0xC0 0x07 0xD0 0x05 0x64 0x01 0x32 0x00 0x00 0x00 0x01
 *  0x50            : 0x05 0x64 0x09 0x60 0x05 0xC0 0x07 0xD0 0x09 0x60 0x01 0x32 0x00 0x00 0x00 0x01
 *  0x60            : 0x02 0x80 0x09 0x60 0x05 0xC0 0x05 0xC0 0x05 0xC0 0x01 0x32 0x00 0x00 0x00 0x00
 *  
 *  In the example above, the first 5 servos are enabled as indicated by the 16th register [F] 
 *    and the last servo is disabled
 */
enum gesture_offset {
  g_pinky   = 0x10,
  g_ring    = 0x20,
  g_middle  = 0x30,
  g_pointer = 0x40,
  g_thumb   = 0x50,
  g_wrist   = 0x60,
} ;

enum signature_offset {
  min_extend = 0x0,
  max_extend = 0x2,
  min_relax = 0x4,
  max_relax = 0x6,
  sig_position = 0xA,
  sig_delay = 0xB,
  sig_activate = 0xF,
};
/**************
 * End Gesture registers (signatures) data
 *****************************/


uint16_t get_double_reg(uint8_t reg) {
  uint16_t value = agent.getRegister(reg);
  value <<= 8;
  value |= agent.getRegister(reg+1);
  return value;
}


void setup() {

  Serial.begin(9600);
  Serial.println("\nFinger Control");
  Serial.print("I2C address:");
  Serial.println(agent.getAddress(), HEX);

  pwm.begin(); //This calls Wire.begin()
  pwm.sleep();
  pwm.setPWMFreq(SERVO_FREQ);  // This is the maximum PWM frequency

  interval = agent.getRegister(gesture_hold) * 1000;
  // Arbitrarilly decided that the interval should not be less than 1 second 
  // or more than 20 seconds
  interval = constrain(interval, 1000, 20000);

  start_activity();   
}

void start_activity() {
    uint8_t next_state = (uint8_t)agent.getRegister(activity_state);
    if (next_state != cur_state) {
      cur_state = next_state;
      active_state = next_state;
    }
}

void load_gesture() {
  for (uint8_t i = 0; i < signature_len; i++) {
    hand[pinky][i] = agent.getRegister(g_pinky+i);
    hand[ring][i] = agent.getRegister(g_ring+i);
    hand[middle][i] = agent.getRegister(g_middle+i);
    hand[pointer][i] = agent.getRegister(g_pointer+i);
    hand[thumb][i] = agent.getRegister(g_thumb+i);
    hand[wrist][i] = agent.getRegister(g_wrist+i);
  }
  #ifdef DEBUG
    char finger[20];
    sprintf(finger, "Pinky: 0x%02X%02x", hand[pinky][sig_position],hand[pinky][sig_position+1]);
    Serial.println(finger);
  #endif
}

void gesture() {
  set_position(pinky);
  set_position(ring);
  set_position(middle);
  set_position(pointer);
  set_position(thumb);
  set_position(wrist);
}

void relax(servo f) {
  uint16_t relax_min = get_double_reg(hand[f][min_relax]);
  uint16_t relax_max = get_double_reg(hand[f][max_relax]);
  uint16_t position = get_double_reg(hand[f][sig_position]);
  pwm.writeMicroseconds((byte)f, constrain(position, relax_min, relax_max));
}

void set_position(servo f) {
  uint16_t extend_min = get_double_reg(hand[f][min_extend]);
  uint16_t extend_max = get_double_reg(hand[f][max_extend]);
  uint16_t position = get_double_reg(hand[f][sig_position]);
  pwm.writeMicroseconds((byte)f, constrain(position, extend_min, extend_max));
  if (hand[f][0xA] > 0) {
    delay(hand[f][sig_delay]);
    relax(f);
  }
}


void loop() {

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    load_gesture();

    //If anything is not null do gesture()
    if (active_state > 0){
      pwm.wakeup();
      gesture();
      active_state -= 1;
    }
  }
  pwm.sleep();
}
