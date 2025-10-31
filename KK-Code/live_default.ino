/*********
  ESP32 Opening Bridge - Manual Control Only
  WiFi web server for manual bridge control
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
  // Flash yellow warning
  for(int i = 0; i < 3; i++) {
    digitalWrite(YELLOW_LED, HIGH);
    delay(200);
    digitalWrite(YELLOW_LED, LOW);
    delay(200);
  }
  
  // Switch to red and close
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, HIGH);
  digitalWrite(YELLOW_LED, LOW);
  delay(1000);
  myServo.write(90);   // close gate (adjust angle as needed)
  gateIsOpen = false;
  Serial.println("Gate CLOSED");
}

void openGate(){
  // Flash yellow warning
  for(int i = 0; i < 3; i++) {
    digitalWrite(YELLOW_LED, HIGH);
    delay(200);
    digitalWrite(YELLOW_LED, LOW);
    delay(200);
  }
  
  // Switch to green and open
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(YELLOW_LED, LOW);
  delay(1000);
  myServo.write(0);    // open gate (adjust angle as needed)
  gateIsOpen = true;
  Serial.println("Gate OPENED");
}

void setup() {
  Serial.begin(115200);
  
  // Initialize LED pins
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  
  // Set initial states - gate open, green light on
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);
  
  // Initialize servo - start with gate open
  myServo.attach(SERVO_PIN);
  myServo.write(0);  // Open position

  // Connect to Wi-Fi
  Serial.println("\n=== ESP32 Opening Bridge - Manual Control ===");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Web server started");
  Serial.println("=============================================\n");
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client connected");
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {
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
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // === HANDLE COMMANDS ===
            if (header.indexOf("GET /open") >= 0) {
              Serial.println("Manual OPEN command received");
              openGate();
            } else if (header.indexOf("GET /close") >= 0) {
              Serial.println("Manual CLOSE command received");
              closeGate();
            }
            
            // === BUILD HTML PAGE ===
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            
            // CSS styling
            client.println("<style>");
            client.println("html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println("h1 { margin-bottom: 10px; }");
            client.println(".status { font-size: 1.8rem; margin: 30px 0; padding: 20px; border-radius: 8px; }");
            client.println(".open { background-color: #d4edda; color: #155724; }");
            client.println(".closed { background-color: #f8d7da; color: #721c24; }");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 20px 50px;");
            client.println("text-decoration: none; font-size: 28px; margin: 10px; cursor: pointer; border-radius: 8px; display: inline-block;}");
            client.println(".button2 {background-color: #f44336;}");
            client.println(".led-status { margin: 20px 0; padding: 15px; background-color: #f5f5f5; border-radius: 8px; }");
            client.println("</style></head>");
            
            // Page content
            client.println("<body><h1>ESP32 Opening Bridge</h1>");
            client.println("<h2>Manual Control</h2>");
            
            // Bridge status
            client.println("<div class=\"status " + String(gateIsOpen ? "open" : "closed") + "\">");
            client.println("Bridge Status: <strong>" + String(gateIsOpen ? "OPEN" : "CLOSED") + "</strong></div>");
            
            // Control buttons
            client.println("<p><a href=\"/open\"><button class=\"button\">OPEN BRIDGE</button></a></p>");
            client.println("<p><a href=\"/close\"><button class=\"button button2\">CLOSE BRIDGE</button></a></p>");
            
            // LED status
            client.println("<div class=\"led-status\">");
            client.println("<h3>Traffic Lights Status</h3>");
            client.println("<p>Red LED: " + String(digitalRead(RED_LED) ? "ON" : "OFF") + "</p>");
            client.println("<p>Yellow LED: " + String(digitalRead(YELLOW_LED) ? "ON" : "OFF") + "</p>");
            client.println("<p>Green LED: " + String(digitalRead(GREEN_LED) ? "ON" : "OFF") + "</p>");
            client.println("</div>");
            
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
    Serial.println("Client disconnected\n");
  }
}
