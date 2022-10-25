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
unsigned long interval = 300000; //5 minutes

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
const uint8_t RUN = 0x01;
const uint8_t STOP = 0x00;
uint8_t cur_state = STOP;
uint8_t run_count = 0x00; //How many times to run the routine
uint8_t run_delay = 3600;

enum win_pos {
  cw = 0,
  ccw
} cur_win_pos = cw;

// Finger run state
// Used to determine each fingers activation when running a gesture
const uint8_t YES = 0x01;
const uint8_t NO = 0x00;

const uint16_t POSITION_EXTREME_OPEN = 0x464;
const uint16_t POSITION_OPEN = 0x0564;
const uint16_t POSITION_CLOSED = 0x0960;
const uint16_t POSITION_OPEN_RELAX = 0x05C0;
const uint16_t POSITION_CLOSED_RELAX = 0x08D0;
const uint16_t POSITION_MIDDLE = 0x05C0;
const uint16_t POSITION_WRIST_OPEN_CW = 0x1F4;
const uint16_t POSITION_WRIST_OPEN_CCW = 0x0960;
const uint16_t POINTER_POS = POSITION_CLOSED;
const uint8_t RELAX_DELAY = .6; //Delay in seconds between extend and relax 

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
  { pinky, {0x0564, 0x0960, 0x05C0, 0x08D0, POSITION_MIDDLE, POSITION_CLOSED, YES, YES} },
  { ring, {0x0564, 0x0960, 0x05C0, 0x08D0, POSITION_MIDDLE, POSITION_CLOSED, YES, YES} },
  { middle, {0x0564, 0x0960, 0x05C0, 0x08D0, POSITION_MIDDLE, POSITION_CLOSED, YES, YES} },
  { pointer, {0x0564, 0x0960, 0x05C0, 0x08D0, POSITION_MIDDLE, POSITION_CLOSED, YES, YES} },
  { thumb, {0x0564, 0x0960, 0x05C0, 0x08D0, POSITION_MIDDLE, POSITION_CLOSED, YES, YES} },
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

  //Seed random with unconnected analog pin 0
  randomSeed(analogRead(0));
  //Set a default gesture when it turns on or resets
  rock();
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
    Serial.println(buf);
  #endif
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
  sm[wrist].sig[POSITION] = POSITION_WRIST_OPEN_CCW; 
  sm[wrist].sig[RELAX_POSITION] = POSITION_MIDDLE; 
  sm[pinky].sig[RELAX] = NO;
  sm[ring].sig[RELAX] = NO;
  sm[middle].sig[RELAX] = NO;
  sm[pointer].sig[RELAX] = NO;
  sm[thumb].sig[RELAX] = NO;
  sm[wrist].sig[RELAX] = YES;
  sm[wrist].sig[ACTIVATE] = YES;
  cur_state = RUN;
}

void paper() {
  sm[pinky].sig[POSITION] = POSITION_OPEN;
  sm[ring].sig[POSITION] = POSITION_OPEN;
  sm[middle].sig[POSITION] = POSITION_EXTREME_OPEN;
  sm[pointer].sig[POSITION] = POSITION_OPEN;
  sm[thumb].sig[POSITION] = POSITION_OPEN;
  sm[wrist].sig[POSITION] = POSITION_WRIST_OPEN_CCW; 
  sm[pinky].sig[RELAX_POSITION] = POSITION_OPEN_RELAX;
  sm[ring].sig[RELAX_POSITION] = POSITION_OPEN_RELAX;
  sm[middle].sig[RELAX_POSITION] = POSITION_OPEN;
  sm[pointer].sig[RELAX_POSITION] = POSITION_OPEN_RELAX;
  sm[thumb].sig[RELAX_POSITION] = POSITION_OPEN_RELAX;
  sm[wrist].sig[RELAX_POSITION] = POSITION_MIDDLE;
  sm[pinky].sig[RELAX] = YES;
  sm[ring].sig[RELAX] = YES;
  sm[middle].sig[RELAX] = YES;
  sm[pointer].sig[RELAX] = YES;
  sm[thumb].sig[RELAX] = YES;
  sm[wrist].sig[RELAX] = YES;
  sm[wrist].sig[ACTIVATE] = YES;
  cur_state = RUN;
}

void scissors() {
  sm[pinky].sig[POSITION] = POSITION_CLOSED;
  sm[ring].sig[POSITION] = POSITION_CLOSED;
  sm[middle].sig[POSITION] = POSITION_EXTREME_OPEN;
  sm[pointer].sig[POSITION] = POSITION_OPEN;
  sm[thumb].sig[POSITION] = POSITION_CLOSED;
  sm[wrist].sig[POSITION] = POSITION_WRIST_OPEN_CW; 
  sm[middle].sig[RELAX_POSITION] = POSITION_OPEN;
  sm[pointer].sig[RELAX_POSITION] = POSITION_OPEN_RELAX;
  sm[wrist].sig[RELAX_POSITION] = POSITION_MIDDLE; 
  sm[pinky].sig[RELAX] = NO;
  sm[ring].sig[RELAX] = NO;
  sm[middle].sig[RELAX] = YES;
  sm[pointer].sig[RELAX] = YES;
  sm[thumb].sig[RELAX] = NO;
  sm[wrist].sig[RELAX] = YES;
  sm[wrist].sig[ACTIVATE] = YES;
  cur_state = RUN;
}

void win() {
  cur_win_pos == cw ? win_ccw() : win_cw();
}

void win_cw() {
  cur_win_pos = cw;
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

void win_ccw() {
  cur_win_pos = ccw;
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
  sm[wrist].sig[RELAX_POSITION] = POSITION_WRIST_OPEN_CCW; 
  sm[pinky].sig[RELAX] = YES;
  sm[ring].sig[RELAX] = NO;
  sm[middle].sig[RELAX] = NO;
  sm[pointer].sig[RELAX] = NO;
  sm[thumb].sig[RELAX] = YES;
  sm[wrist].sig[RELAX] = YES;
  sm[wrist].sig[ACTIVATE] = YES;
  cur_state = RUN;
}

void lose() {
  cur_win_pos = ccw;
  sm[pinky].sig[POSITION] = POSITION_CLOSED;
  sm[ring].sig[POSITION] = POSITION_CLOSED;
  sm[middle].sig[POSITION] = POSITION_CLOSED;
  sm[pointer].sig[POSITION] = POSITION_OPEN;
  sm[thumb].sig[POSITION] = POSITION_OPEN;
  sm[wrist].sig[POSITION] = POSITION_WRIST_OPEN_CCW; 
  sm[pinky].sig[RELAX_POSITION] = POSITION_CLOSED;
  sm[ring].sig[RELAX_POSITION] = POSITION_CLOSED;
  sm[middle].sig[RELAX_POSITION] = POSITION_CLOSED;
  sm[pointer].sig[RELAX_POSITION] = POSITION_OPEN;
  sm[thumb].sig[RELAX_POSITION] = POSITION_OPEN;
  sm[wrist].sig[RELAX_POSITION] = POSITION_MIDDLE; 
  sm[pinky].sig[RELAX] = NO;
  sm[ring].sig[RELAX] = NO;
  sm[middle].sig[RELAX] = NO;
  sm[pointer].sig[RELAX] = NO;
  sm[thumb].sig[RELAX] = NO;
  sm[wrist].sig[RELAX] = YES;
  sm[wrist].sig[ACTIVATE] = YES;
  cur_state = RUN;
}

void lonely() {
  uint8_t random_gesture = random(0,5);
  switch (random_gesture) {
    case 0:
      reset_count_and_delay();
          rock();
      break;
    case 1:
      reset_count_and_delay();
      scissors();
      break;
    case 2:
      reset_count_and_delay();
      paper();
      break;
    case 3:      
      run_count = 8;
      run_delay = 400;
          win();
      break;
    case 4:
      reset_count_and_delay();
      lose();
      break;
    default:
      break;
  }
}

void reset_count_and_delay() {
  //Everything but win uses this run_count and run_delay
  run_delay = 3600;
  run_count = 0;
}

void loop() {
  unsigned long currentMillis = millis();

  while (Serial.available() > 0) {
    // look for the next valid integer in the incoming serial stream:
    int cmd = Serial.parseInt(SKIP_WHITESPACE);
    // there is activity, reset the interval counter
    previousMillis = currentMillis;  
    #ifdef DEBUG
    Serial.print("Command is: ");
    Serial.println(cmd);    
    #endif
    if (cmd == 1) {
      //ROCK
      reset_count_and_delay();
      rock();
    } else if (cmd == 0) {
      Serial.flush();
    } else if (cmd == 2) {
      //PAPER
      reset_count_and_delay();
      paper();
    } else if (cmd == 3) {
      //SCISSORS
      reset_count_and_delay();
      scissors();
    } else if (cmd == 4) {
      //Winner
      run_count = 8;
      run_delay = 500;
      win();
    } else if (cmd == 5) {
      //Lose
      reset_count_and_delay();
      lose();
    }

  }

  if (currentMillis - previousMillis >= interval) {
    //reset the interval counter
    previousMillis = currentMillis;
    lonely();
  }

  if (run_count > 0) {
    win();
    cur_state = RUN;
    run_count--;
    run_delay = 500;
  }

  if (cur_state == RUN) {
    pwm.wakeup();
    gesture();
    delay(run_delay);
    pwm.sleep();
    cur_state = STOP;
  }
}

#undef DEBUG
