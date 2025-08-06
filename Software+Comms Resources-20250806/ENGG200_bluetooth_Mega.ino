/*
  BlueTooth Serial controlling a MEGA

 Receives from the main serial port, sends to the others.
 Receives from serial port 1, sends to the main serial (Serial 0).

 */

boolean NL = true;  // we need to delay this one loop through the loop() routine

void setup() {
  // initialize both serial ports:
  Serial.begin(9600);
  Serial.print("Sketch:   ");   Serial.println(__FILE__);
  Serial.print("Uploaded: ");   Serial.println(__DATE__);
  Serial.println(" ");
  
  Serial1.begin(9600);
}

void loop() {
  char  c;
  
  // read from port 1, send to port 0:
  if (Serial1.available()) {
    int inByte = Serial1.read();
    Serial.write(inByte);
  }

  // read from port 0, send to port 1:
  if (Serial.available()) {
    c = Serial.read();

    if (c != '\r' && c != '\n') {
      Serial1.write(c);
    }
    if (NL) {
      Serial.print("\r\n> ");
      NL = false;
    }
    Serial.write(c);
    if (c == '\n') {
      NL = true;
    }
  }
}
