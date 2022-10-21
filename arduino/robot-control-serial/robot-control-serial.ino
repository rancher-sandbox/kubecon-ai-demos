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
uint16_t POSITION_OPEN_RELAX = 0x05C0;
uint16_t POSITION_CLOSED_RELAX = 0x08D0;
uint16_t POSITION_MIDDLE = 0x05C0;
uint16_t POSITION_WRIST_OPEN_CW = 0x1F4;
uint16_t POSITION_WRIST_OPEN_CCW = 0x0960;
uint16_t POINTER_POS = POSITION_CLOSED;
uint8_t RELAX_DELAY = .4; //Delay in seconds between extend and relax 

// Servo signatures
// Indexes into the servo signatures
// extend min, extend max, relax min, relax max, position
enum signature {
  MIN_EXTEND = 0,
  MAX_EXTEND,
  MIN_RELAX,
  MAX_RELAX,
  POSITION,
  RELAX_POSITION,
  ACTIVATE,
  RELAX,
};

//servo signature mapping  
struct servo_map {
  servo s;
  uint16_t sig[8];
};

servo_map sm[6] = {
  { pinky, {0x0564, 0x0960, 0x05C0, 0x08D0, POSITION_CLOSED, POSITION_CLOSED_RELAX, YES, YES} },
  { ring, {0x0564, 0x0960, 0x05C0, 0x08D0, POSITION_CLOSED, POSITION_CLOSED_RELAX, YES, YES} },
  { middle, {0x0564, 0x0960, 0x05C0, 0x08D0, POSITION_CLOSED, POSITION_CLOSED_RELAX, YES, YES} },
  { pointer, {0x0564, 0x0960, 0x05C0, 0x08D0, POSITION_CLOSED, POSITION_CLOSED_RELAX, YES, YES} },
  { thumb, {0x0564, 0x0960, 0x05C0, 0x08D0, POSITION_CLOSED, POSITION_CLOSED_RELAX, YES, YES} },
  { wrist, {0x0280, 0x0960, 0x1F4, 0x0960, POSITION_MIDDLE, POSITION_CLOSED_RELAX, YES, YES} }
};

void setup() {
  Serial.begin(115200);
  #ifdef DEBUG
  Serial.println("\nRobot Hand Control");
  #endif
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
  uint16_t constrained_uS = constrain(sm.sig[RELAX_POSITION], sm.sig[MIN_RELAX], sm.sig[MAX_RELAX]);
  #ifdef DEBUG
    pwm.writeMicroseconds((byte)sm.s, constrained_uS);
    char buf[22 + sizeof(constrained_uS) + sizeof(sm.s)] = {0};
    sprintf(buf, "final position: %d", constrained_uS);
  #endif
  Serial.println(buf);
}

void set_position(servo_map sm) {
  if ( sm.sig[ACTIVATE] == YES ) {
    uint16_t constrained_uS = constrain(sm.sig[POSITION], sm.sig[MIN_EXTEND], sm.sig[MAX_EXTEND]);
    #ifdef DEBUG
      char buf[30 + sizeof(sm.s)] = {0};
      sprintf(buf, "[finger: %d]\nstart position: %d", sm.s, constrained_uS);
      Serial.println(buf);
    #endif

    pwm.writeMicroseconds((byte)sm.s, constrained_uS);
    if (sm.sig[RELAX] == YES) {
      delay(RELAX_DELAY);
      relax(sm);
    }
  }
}

void rock() {
  sm[pinky].sig[POSITION] = POSITION_CLOSED;
  sm[ring].sig[POSITION] = POSITION_CLOSED;
  sm[middle].sig[POSITION] = POSITION_CLOSED;
  sm[pointer].sig[POSITION] = POSITION_CLOSED;
  sm[thumb].sig[POSITION] = POSITION_CLOSED;
  sm[wrist].sig[POSITION] = POSITION_MIDDLE; 
  sm[pinky].sig[RELAX] = NO;
  sm[ring].sig[RELAX] = NO;
  sm[middle].sig[RELAX] = NO;
  sm[pointer].sig[RELAX] = NO;
  sm[thumb].sig[RELAX] = NO;
  sm[wrist].sig[RELAX] = NO;
  sm[wrist].sig[ACTIVATE] = YES;
  cur_state = RUN;
}

void paper() {
  sm[pinky].sig[POSITION] = POSITION_OPEN;
  sm[ring].sig[POSITION] = POSITION_OPEN;
  sm[middle].sig[POSITION] = POSITION_OPEN;
  sm[pointer].sig[POSITION] = POSITION_OPEN;
  sm[thumb].sig[POSITION] = POSITION_OPEN;
  sm[wrist].sig[POSITION] = POSITION_MIDDLE; 
  sm[pinky].sig[RELAX_POSITION] = POSITION_OPEN_RELAX;
  sm[ring].sig[RELAX_POSITION] = POSITION_OPEN_RELAX;
  sm[middle].sig[RELAX_POSITION] = POSITION_OPEN_RELAX;
  sm[pointer].sig[RELAX_POSITION] = POSITION_OPEN_RELAX;
  sm[thumb].sig[RELAX_POSITION] = POSITION_OPEN_RELAX;
  sm[pinky].sig[RELAX] = YES;
  sm[ring].sig[RELAX] = YES;
  sm[middle].sig[RELAX] = YES;
  sm[pointer].sig[RELAX] = YES;
  sm[thumb].sig[RELAX] = YES;
  sm[wrist].sig[RELAX] = NO;
  sm[wrist].sig[ACTIVATE] = YES;
  cur_state = RUN;
}

void scissors() {
  sm[pinky].sig[POSITION] = POSITION_CLOSED;
  sm[ring].sig[POSITION] = POSITION_CLOSED;
  sm[middle].sig[POSITION] = POSITION_OPEN;
  sm[pointer].sig[POSITION] = POSITION_OPEN;
  sm[thumb].sig[POSITION] = POSITION_CLOSED;
  sm[wrist].sig[POSITION] = POSITION_MIDDLE; 
  sm[middle].sig[RELAX_POSITION] = POSITION_OPEN_RELAX;
  sm[pointer].sig[RELAX_POSITION] = POSITION_OPEN_RELAX;
  sm[pinky].sig[RELAX] = NO;
  sm[ring].sig[RELAX] = NO;
  sm[middle].sig[RELAX] = YES;
  sm[pointer].sig[RELAX] = YES;
  sm[thumb].sig[RELAX] = NO;
  sm[wrist].sig[RELAX] = NO;
  sm[wrist].sig[ACTIVATE] = YES;
  cur_state = RUN;
}

void win() {
  sm[pinky].sig[POSITION] = POSITION_OPEN;
  sm[ring].sig[POSITION] = POSITION_CLOSED;
  sm[middle].sig[POSITION] = POSITION_CLOSED;
  sm[pointer].sig[POSITION] = POSITION_CLOSED;
  sm[thumb].sig[POSITION] = POSITION_OPEN;
  sm[wrist].sig[POSITION] = POSITION_MIDDLE; 
  sm[pinky].sig[RELAX_POSITION] = POSITION_OPEN;
  sm[ring].sig[RELAX_POSITION] = POSITION_CLOSED;
  sm[middle].sig[RELAX_POSITION] = POSITION_CLOSED;
  sm[pointer].sig[RELAX_POSITION] = POSITION_CLOSED;
  sm[thumb].sig[RELAX_POSITION] = POSITION_OPEN;
  sm[wrist].sig[RELAX_POSITION] = POSITION_WRIST_OPEN_CW; 
  sm[pinky].sig[RELAX] = YES;
  sm[ring].sig[RELAX] = NO;
  sm[middle].sig[RELAX] = NO;
  sm[pointer].sig[RELAX] = NO;
  sm[thumb].sig[RELAX] = YES;
  sm[wrist].sig[RELAX] = YES;
  sm[wrist].sig[ACTIVATE] = YES;
  cur_state = RUN;
}

void loop() {
  while (Serial.available() > 0) {
    // look for the next valid integer in the incoming serial stream:
    int cmd = Serial.parseInt(SKIP_WHITESPACE);
    //#ifdef DEBUG
    Serial.print("Command is: ");
    Serial.println(cmd);    
    //#endif
    if (cmd == 1) {
      //ROCK
      rock();
    } else if (cmd == 0) {
      Serial.flush();
    } else if (cmd == 2) {
      //PAPER
      paper();
    } else if (cmd == 3) {
      //SCISSORS
      scissors();
    } else if (cmd == 4) {
      //Winner
      win();
    }

  }

  if (cur_state == RUN) {
    pwm.wakeup();
    gesture();
    delay(2400);
    pwm.sleep();
    cur_state = STOP;
  }
}

#undef DEBUG
