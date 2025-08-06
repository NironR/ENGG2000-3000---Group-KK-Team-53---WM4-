#include <Metro.h> // Include the Metro library
#include <Servo.h>

Metro serialMetro = Metro(250);  // Instantiate an instance
int countLeft=0;
int countRight=0;
int targetPos = 1000;
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
 int throttleLeft = (int) 90.0-0.9*(targetPos-countLeft);
 int throttleRight = (int) 90.0+1.3*(targetPos-countRight);
 if(digitalRead(6)==HIGH){ // put your main code here, to run repeatedly:
   myservoLeft.write(constrain(throttleLeft,0,180));
   myservoRight.write(constrain(throttleRight,0,180));
  }
 else{
    myservoLeft.write(90);
    myservoRight.write(90);
 }  

 
}
void ISRleft(){
  if(digitalRead(22)==LOW){
  countLeft=countLeft+1;}
  else{countLeft=countLeft-1;}
}
void ISRright(){
   if(digitalRead(23)==HIGH){
  countRight=countRight+1;}
  else{countRight=countRight-1;}
}

