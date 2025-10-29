/*********
  ESP32 Boom Gate Web Server
  Combines web server controls with servo gate control
*********/

#include <WiFi.h>
#include <ESP32Servo.h>

// WiFi credentials
const char* ssid = "SSID";
const char* password = "PASSWORD";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Servo and LED pins
Servo myServo;
const int SERVO_PIN   = 23;
const int RED_LED     = 27;
const int YELLOW_LED  = 26; 
const int GREEN_LED   = 25; 

// Gate state
bool gateIsOpen = true;

// Timeout
unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;

// Gate control functions
void closeGate(){
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, HIGH);
  delay(2000);
  myServo.write(90);   // close gate
  gateIsOpen = false;
  Serial.println("Gate CLOSED");
}

void openGate(){
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);
  delay(2000);
  myServo.write(0);    // open gate
  gateIsOpen = true;
  Serial.println("Gate OPENED");
}

void setup() {
  Serial.begin(115200);
  
  // Initialize LED pins
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  
  // Set initial states
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);  // Start with green (gate open)
  
  // Initialize servo
  myServo.attach(SERVO_PIN);
  myServo.write(0);  // Start with gate open

  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // Handle gate control
            if (header.indexOf("GET /open") >= 0) {
              openGate();
            } else if (header.indexOf("GET /close") >= 0) {
              closeGate();
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the buttons
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #f44336;}");
            client.println(".button3 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP32 Boom Gate Control</h1>");
            
            // Display current gate state
            client.println("<p>Gate Status: <strong>" + String(gateIsOpen ? "OPEN" : "CLOSED") + "</strong></p>");
            
            // Display control buttons
            client.println("<p><a href=\"/open\"><button class=\"button\">OPEN GATE</button></a></p>");
            client.println("<p><a href=\"/close\"><button class=\"button button2\">CLOSE GATE</button></a></p>");
            
            // Display LED status
            client.println("<hr>");
            client.println("<p>Red LED: " + String(digitalRead(RED_LED) ? "ON" : "OFF") + "</p>");
            client.println("<p>Green LED: " + String(digitalRead(GREEN_LED) ? "ON" : "OFF") + "</p>");
            
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
