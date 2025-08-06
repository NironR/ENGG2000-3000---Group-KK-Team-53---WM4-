#include <Metro.h> // Include the Metro library
#include <Servo.h>

Metro serialMetro = Metro(250);  // Instantiate an instance
int countLeft=0;
int countRight=0;
int throttle = 0;
Servo myservoLeft;
Servo myservoRight;    
  
void setup() {  // put your setup code here, to run once:
   Serial.begin(115200); // Start the Serial communication
  attachInterrupt(digitalPinToInterrupt(18), ISRleft, RISING);
  attachInterrupt(digitalPinToInterrupt(19), ISRright, RISING);
  pinMode(22,INPUT); // 18 and 22 come from left
  pinMode(23,INPUT); // 19 and 23 come from right
  pinMode(6,INPUT); //enable pin
  myservoLeft.attach(4);  // attaches the servo on pin 9 to the servo object
  myservoRight.attach(5);  // attaches the servo on pin 9 to the servo object
 
}

void loop() {
  if (serialMetro.check() == 1) { // check if the metro has passed it's interval .
  // Output all the analog readings seperated by a space character
      Serial.print(countLeft);
      Serial.print('\t');
      Serial.println(countRight);
    }
 if(digitalRead(6)==HIGH){ // put your main code here, to run repeatedly:
 
 for (throttle = 0; throttle <= 180; throttle += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservoLeft.write(throttle);
    myservoRight.write(180-throttle);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  for (throttle = 180; throttle >= 0; throttle -= 1) { // goes from 180 degrees to 0 degrees
    myservoLeft.write(throttle);
    myservoRight.write(180-throttle);// tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
 }
 else{
    myservoLeft.write(90);
    myservoRight.write(90);
 }  
}
void ISRleft(){
  if(digitalRead(22)==HIGH){
  countLeft=countLeft+1;}
  else{countLeft=countLeft-1;}
}
void ISRright(){
   if(digitalRead(23)==LOW){
  countRight=countRight+1;}
  else{countRight=countRight-1;}
}

