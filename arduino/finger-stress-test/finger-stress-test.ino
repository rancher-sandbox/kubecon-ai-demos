/*
 Try this test sketch with the Servo library to see how your
 servo responds to different settings, type a position from
 544 to 2400 in the top of serial monitor and hit [ENTER],
 start at 1472 and work your way toward zero (544) 50 micros at
 a time, then toward 2400. 
*/
#include <Servo.h>
Servo servo;
bool MANUAL_CTL = false;
int pos=1472;

void setup() {
  // initialize serial:
  Serial.begin(9600); //set serial monitor baud rate to match
  servo.writeMicroseconds(1472);
  servo.attach(9);
  prntIt();
}

void loop() {

  if (Serial.available() > 0) {
    int pos = Serial.parseInt(SKIP_WHITESPACE);
    if (pos == 0) {
      MANUAL_CTL = true;        
    } else if (pos == 1) {
      MANUAL_CTL = false;
      while (Serial.available() > 0)
        Serial.read();
    } 
    if (MANUAL_CTL == true) {
      if (pos != 0) {
        pos = constrain(pos, 550, 2400);    
        servo.writeMicroseconds(pos);
        prntIt();
      }
    }
  } 
  if (!MANUAL_CTL) {
    pos=random(1350,2300);
    servo.writeMicroseconds(pos);
    prntIt();
  }
  delay(400);
 
}

void prntIt()
{
  Serial.print("  degrees = "); 
  Serial.print(servo.read());
  Serial.print("\t");
  Serial.print("microseconds =  ");
  Serial.println(servo.readMicroseconds());
}
