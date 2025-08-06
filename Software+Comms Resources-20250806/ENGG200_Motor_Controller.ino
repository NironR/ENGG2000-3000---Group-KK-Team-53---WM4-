#include <Servo.h>  //This comes with the Arduino

int Motor_pin0 = 12;
int Motor_pin1 = 11;
int Motor0_On_Pin = 4;
int Motor1_On_Pin = 5;
int led = 13;
Servo Motor0;
Servo Motor1;

/* The 'Servo.h' library allows you to control motors and servos by passing any value between
0 and 180 into the "write" method (Motor.write() in this program).
For a motor:   0 is full power in one direction,
             180 is full direction in the opposite direction,
              90 is stop.
For a servo:   0 is all the way one direction,
             180 is all the way in the opposite direction,
              90 is the middle.
*/

int Motor_max_forward = 2000;  //this might not actually be forward
int Motor_max_reverse = 1000;    //this might not actually be reverse
int Motor_stop = 1500;          //this is definitely stop (if the Motor is calibrated right)
unsigned long next_step;
unsigned long last_step;
int state = 0;

void setup() {
  pinMode(led, OUTPUT);     // setup the LED to blink as we run the loop
  pinMode(Motor0_On_Pin, OUTPUT);
  pinMode(Motor1_On_Pin, OUTPUT);
  Motor0.attach(Motor_pin0, 1000, 2000);  // This pin is the control signal for the Motor
  Motor0.writeMicroseconds(Motor_stop);
  Motor1.attach(Motor_pin1, 1000, 2000);  // and another Motor is out here  
  Motor1.writeMicroseconds(Motor_stop);
  last_step = millis();  // where we are
  next_step = last_step + 1000;
}

/*
 * states are:
 * 0 = Motor 0 is stopped
 *     Motor 1 is moving from stop to full forward
 *
 * 1 = Motor 0 is full forward
 *     Motor 1 is moving from full forward to stop
 *
 * 2 = Motor 0 is stopped
 *     Motor 1 is moving from stop to full reverse
 *
 * 3 = Motor 0 is full reverse
 *     Motor 1 is moving from full reverse to stop
 */
 
void loop() {
  unsigned long in_step;  // how long we are in the current step
  
  in_step = (millis() - last_step)/2;
  if (in_step > 1000) {
    in_step = 1000;  // don't go beyond the step time
  }
  switch(state) {
    case 0:
      digitalWrite(led, HIGH);
      digitalWrite(Motor0_On_Pin, LOW);
      digitalWrite(Motor1_On_Pin, LOW);
      Motor0.writeMicroseconds(Motor_stop);
      Motor1.writeMicroseconds(Motor_stop + in_step);
      break;
    case 1:
      digitalWrite(Motor0_On_Pin, HIGH);
      digitalWrite(Motor1_On_Pin, LOW);
      Motor0.writeMicroseconds(Motor_max_forward);
      Motor1.writeMicroseconds(Motor_max_forward - in_step);
      break;
    case 2:
      digitalWrite(led, LOW);
      digitalWrite(Motor0_On_Pin, LOW);
      digitalWrite(Motor1_On_Pin, HIGH);
      Motor0.writeMicroseconds(Motor_stop);
      Motor1.writeMicroseconds(Motor_stop - in_step);
      break;
    case 3:
      digitalWrite(Motor0_On_Pin, HIGH);
      digitalWrite(Motor1_On_Pin, HIGH);
      Motor0.writeMicroseconds(Motor_max_reverse);
      Motor1.writeMicroseconds(Motor_max_reverse + in_step);
      break;
  }
  
  // and move to the next state if necessary
  if (millis() > next_step) {
    state++;
    if (state > 3) {
      state= 0;
    }
    last_step = next_step;
    next_step += 1000;
  }
}
