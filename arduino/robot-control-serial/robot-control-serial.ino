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

// Gesture run state. 
// Used inside loop() to indicate turning on/off the pwm
uint8_t RUN = 0x01;
uint8_t STOP = 0x00;
uint8_t cur_state = STOP;
// Finger run state
// Used to determine each fingers activation when running a gesture
uint8_t YES = 0x01;
uint8_t NO = 0x00;

uint16_t POSITION_OPEN = 0x0564;
uint16_t POSITION_CLOSED = 0x0960;
uint16_t POSITION_MIDDLE = 0x05C0;
uint16_t POINTER_POS = POSITION_CLOSED;
uint8_t RELAX_DELAY = .5; //Delay in seconds between extend and relax 

// Servo signatures
// Indexes into the servo signatures
// extend min, extend max, relax min, relax max, position
enum signature {
  MIN_EXTEND = 0,
  MAX_EXTEND,
  MIN_RELAX,
  MAX_RELAX,
  POSITION,
  ACTIVATE,
  RELAX,
};

//servo signature mapping  
struct servo_map {
  servo s;
  uint16_t sig[7];
};

servo_map sm[6] = {
  { pinky, {0x0564, 0x0960, 0x05C0, 0x08D0, POSITION_CLOSED, YES, YES} },
  { ring, {0x0564, 0x0960, 0x05C0, 0x08D0, POSITION_CLOSED, YES, YES} },
  {middle, {0x0564, 0x0960, 0x05C0, 0x08D0, POSITION_CLOSED, YES, YES} },
  {pointer, {0x0564, 0x0960, 0x05C0, 0x08D0, POSITION_CLOSED, YES, YES} },
  {thumb, {0x0564, 0x0960, 0x05C0, 0x08D0, POSITION_CLOSED, YES, YES} },
  {wrist, {0x0280, 0x0960, 0x05C0, 0x05C0, POSITION_MIDDLE, YES, YES} }
};

void setup() {
  Serial.begin(115200);
  Serial.println("\nRobot Hand Control");
  pwm.begin(); //This calls Wire.begin()
  pwm.sleep();
  pwm.setPWMFreq(SERVO_FREQ);  // This is the maximum PWM frequency

}


void gesture() {
  set_position(sm[pinky]);
  set_position(sm[ring]);
  set_position(sm[middle]);
  set_position(sm[pointer]);
  set_position(sm[thumb]);
  set_position(sm[wrist]);
}

void relax(servo_map sm) {
  pwm.writeMicroseconds((byte)sm.s, constrain(sm.sig[POSITION], sm.sig[MIN_RELAX], sm.sig[MAX_RELAX]));
}

void set_position(servo_map sm) {
  if ( sm.sig[ACTIVATE] == YES ) {
    pwm.writeMicroseconds((byte)sm.s, constrain(sm.sig[POSITION], sm.sig[MIN_EXTEND], sm.sig[MAX_EXTEND]));
    if (sm.sig[RELAX] == YES) {
      delay(RELAX_DELAY);
      relax(sm);
    }
  }
}

void flip_state() {
  cur_state = (cur_state == RUN ? STOP : RUN);
}

void loop() {

  while (Serial.available() > 0) {
    // look for the next valid integer in the incoming serial stream:
    int cmd = Serial.parseInt(SKIP_WHITESPACE);
    Serial.print("Command is: ");
    Serial.println(cmd);    
    if (cmd == 1) {
      //ROCK
      sm[pinky].sig[POSITION] = POSITION_CLOSED;
      sm[ring].sig[POSITION] = POSITION_CLOSED;
      sm[middle].sig[POSITION] = POSITION_CLOSED;
      sm[pointer].sig[POSITION] = POSITION_CLOSED;
      sm[thumb].sig[POSITION] = POSITION_CLOSED;
      sm[pinky].sig[RELAX] = NO;
      sm[ring].sig[RELAX] = NO;
      sm[middle].sig[RELAX] = NO;
      sm[pointer].sig[RELAX] = NO;
      sm[thumb].sig[RELAX] = NO;
      sm[wrist].sig[ACTIVATE] = NO;
      flip_state();
    } else if (cmd == 0) {
      Serial.flush();
    } else if (cmd == 2) {
      //PAPER
      sm[pinky].sig[POSITION] = POSITION_OPEN;
      sm[ring].sig[POSITION] = POSITION_OPEN;
      sm[middle].sig[POSITION] = POSITION_OPEN;
      sm[pointer].sig[POSITION] = POSITION_OPEN;
      sm[thumb].sig[POSITION] = POSITION_OPEN;
      sm[wrist].sig[ACTIVATE] = NO;
      flip_state();
    } else if (cmd == 3) {
      //SCISSORS
      sm[pinky].sig[POSITION] = POSITION_CLOSED;
      sm[ring].sig[POSITION] = POSITION_CLOSED;
      sm[middle].sig[POSITION] = POSITION_OPEN;
      sm[pointer].sig[POSITION] = POSITION_OPEN;
      sm[thumb].sig[POSITION] = POSITION_CLOSED;
      sm[wrist].sig[ACTIVATE] = NO;
      flip_state();
    }

  }

  if (cur_state == RUN) {
    pwm.wakeup();
    gesture();
    pwm.sleep();
    flip_state();
  }
}

#undef DEBUG
