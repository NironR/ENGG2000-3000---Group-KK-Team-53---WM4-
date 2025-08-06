#include <Metro.h> // Include the Metro library

Metro serialMetro = Metro(250);  // Instantiate an instance
int countLeft=0;
int countRight=0;
  
void setup() {  // put your setup code here, to run once:
   Serial.begin(115200); // Start the Serial communication
  attachInterrupt(digitalPinToInterrupt(18), ISRleft, RISING);
  attachInterrupt(digitalPinToInterrupt(19), ISRright, RISING);
  pinMode(22,INPUT); // 18 and 22 come from left
  pinMode(23,INPUT); // 19 and 23 come from right
 
}

void loop() {
  if (serialMetro.check() == 1) { // check if the metro has passed it's interval .
  // Output all the analog readings seperated by a space character
      Serial.print(countLeft);
      Serial.print('\t');
      Serial.println(countRight);
      countLeft++;
      countRight++;
    }
  // put your main code here, to run repeatedly:
}
void ISRleft(){
  countLeft=countLeft+1;
}
void ISRright(){
  countRight=countRight+1;
}

