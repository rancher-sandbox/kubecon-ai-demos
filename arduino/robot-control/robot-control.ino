#include <I2CSlaveMode.h>
#include <Adafruit_PWMServoDriver.h>

#define DEBUG 1
//#undef DEBUG

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
/********************
 * WARNING: these two arrays are tightly coupled with the servo enum as the index to the arrays
 * Use caution when modifying
 * char digit[6][7]
 * uint8_t signatures[6]
 ********************/
char digit[6][7] = { {"pinky"}, {"ring"}, {"middle"}, {"index"}, {"thumb"}, {"wrist"} };
uint8_t signatures[6] = { 0x10, 0x20, 0x30, 0x40, 0x50, 0x60 };
//Signature instruction set (signature offsets)
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
 *  C     : if > 0 enable debug output
 *  D-E   : UNUSED
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
enum signature_offsets {
  min_extend = 0x0,
  max_extend = 0x2,
  min_relax = 0x4,
  max_relax = 0x6,
  sig_position = 0x8,
  sig_relax = 0xA,
  sig_delay = 0xB,
  sig_debug = 0xC,
  sig_activate = 0xF,
};
/**************
 * End signature offsets data
 *****************************/

// Depending on your servo make, the pulse width min and max may vary, you 
// want these to be as small/large as possible without hitting the hard stop
// for max range. You'll have to tweak them as necessary to match the servos you
// have!
// Tweak them by setting values in EEPROM registers
// Min starts at about 600 microseconds 
// Max starts at about 2400 microseconds 
enum runtime_offset {
  gesture_hold = 0x04,//Register with a multiplier for number of seconds to hold the gesture before repeating it
  /*********************
   * the run_count register value can be used by the client to activate or deactivate the current gesture
   * If the state is less than 0xFF, the program will run the number of times
   * specified by the value.
   * If the run_count value is 0x00, the program will stop
   * The program checks the state periodically 
   * based on the timing interval specified using gesture_hold value
   *********************************/
  run_count = 0x05 // Register indicating how many cycles to run the program
};

// Will be set to the value of the run_count register if the cur and stored state are different
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

uint16_t get_double_reg(uint8_t reg) {
  uint16_t upper = agent.getRegister(reg);
  uint16_t lower = agent.getRegister(reg+1);
  uint16_t final_value = upper << 8;
  final_value = final_value | lower;
  return final_value;
}


void setup() {

  Serial.begin(9600);
  Serial.println("\nFinger Control");
  Serial.print("I2C address:");
  Serial.println(agent.getAddress(), HEX);
  pwm.begin(); //This calls Wire.begin()
  pwm.sleep();
  pwm.setPWMFreq(SERVO_FREQ);  // This is the maximum PWM frequency

  update_interval();
  state_changed();
  start_activity();   
}

void update_interval() {
  interval = agent.getRegister(gesture_hold) * 1000;
  // Arbitrarilly decided that the interval should not be less than 1 second 
  // or more than 20 seconds
  interval = constrain(interval, 1000, 20000);
}

void start_activity() {      
  active_state = cur_state;
}

bool state_changed() {
  uint8_t next_state = (uint8_t)agent.getRegister(run_count);

  if (next_state != cur_state) {
    cur_state = next_state;
    return true;
  } else {
    return false;
  }
}

void load_gesture() {
    for (uint8_t i = 0; i < signature_len; i++) {
    hand[pinky][i] = agent.getRegister(signatures[pinky]+i);
    hand[ring][i] = agent.getRegister(signatures[ring]+i);
    hand[middle][i] = agent.getRegister(signatures[middle]+i);
    hand[pointer][i] = agent.getRegister(signatures[pointer]+i);
    hand[thumb][i] = agent.getRegister(signatures[thumb]+i);
    hand[wrist][i] = agent.getRegister(signatures[wrist]+i);
  }
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
  uint16_t min = get_double_reg(signatures[f]+min_relax);
  uint16_t max = get_double_reg(signatures[f]+max_relax);
  uint16_t pos = get_double_reg(signatures[f]+sig_position);
  pwm.writeMicroseconds((byte)f, constrain(pos, min, max));
}

void set_position(servo f) {
  if ( hand[f][sig_activate] > 0 ) {
    if (agent.getRegister(signatures[f]+sig_debug) > 0) {
      _dump_digit(f);
    }
    uint16_t min = get_double_reg(signatures[f]+min_extend);
    uint16_t max = get_double_reg(signatures[f]+max_extend);
    uint16_t pos = get_double_reg(signatures[f]+sig_position);
    pwm.writeMicroseconds((byte)f, constrain(pos, min, max));
    if (hand[f][sig_relax] > 0) {
      delay(hand[f][sig_delay]*4);
      relax(f);
    }
  }
}


void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    update_interval();
    load_gesture();    

    if (state_changed()) {
      start_activity();
    }
    if (active_state > 0){      
      pwm.wakeup();
      gesture();      
      active_state -= 1;
    }
  }
  pwm.sleep();
}

void _dump_digit(servo f) {
  #ifdef DEBUG
    
    char buf[strlen(digit[f]) + 3] = {0};
    sprintf(buf, "%s: ", digit[f]);
    Serial.println();
    Serial.print(buf);
    _dump_instruction_set(f);
    Serial.flush();
  #endif
}

void _dump_instruction_set(servo f) {
  char finger[50] = {0};
  for( uint8_t i = 0; i < 16; i++ ) {
    sprintf(finger + (i*3), "%02X ", hand[f][i]);
  }
  Serial.println(finger);
}


#undef DEBUG
