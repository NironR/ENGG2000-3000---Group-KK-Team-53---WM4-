/*********
  ESP32 Opening Bridge Web Server
  WiFi control + Ultrasonic boat detection + Smart timer with sensor clearing
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

// Ultrasonic sensor pins (Left = West side, Right = East side)
const int TRIG_PIN_LEFT = 33;
const int ECHO_PIN_LEFT = 32;
const int TRIG_PIN_RIGHT = 19;
const int ECHO_PIN_RIGHT = 18;

// Timing constants (adjustable)
const int MAX_WAIT_TIME = 20000;           // 20 seconds maximum wait (safety timeout)
const int DETECTION_DISTANCE = 20;         // Detection range in cm
const int CLEAR_DISTANCE = 50;             // Distance considered "clear" in cm
const int SAFETY_DELAY = 2000;             // Extra safety buffer in ms
const int SENSOR_CHECK_INTERVAL = 200;     // How often to check sensors in ms

// State variables
bool gateIsOpen = true;
bool autoMode = true;
bool sequenceInProgress = false;

// Timeout
unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;

// Function to measure distance with ultrasonic sensor
float getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH, 20000); // 20ms timeout
  if (duration == 0) return -1; // No response
  
  float distance = duration * 0.034 / 2;  // Convert to cm
  return distance;
}

// Gate control functions
void closeGate(){
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, HIGH);
  delay(2000);
  myServo.write(90);   // close gate (adjust angle as needed)
  gateIsOpen = false;
  Serial.println("Gate CLOSED");
}

void openGate(){
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);
  delay(2000);
  myServo.write(0);    // open gate (adjust angle as needed)
  gateIsOpen = true;
  Serial.println("Gate OPENED");
}

// Automatic bridge sequence with smart timer
void automaticBridgeSequence() {
  sequenceInProgress = true;
  Serial.println("=== BOAT DETECTED! Starting automatic sequence ===");
  
  // 1. Flash yellow warning (boat approaching)
  Serial.println("Warning phase: Flashing yellow LED");
  for(int i = 0; i < 6; i++) {
    digitalWrite(YELLOW_LED, HIGH);
    delay(300);
    digitalWrite(YELLOW_LED, LOW);
    delay(300);
  }
  
  // 2. Close gate to car traffic
  Serial.println("Stopping car traffic...");
  if (gateIsOpen) {
    closeGate();
  }
  
  // 3. SMART TIMER: Wait for sensors to clear OR timeout
  Serial.println("Waiting for boat to pass (monitoring sensors)...");
  
  unsigned long startTime = millis();
  bool sensorsCleared = false;
  int checkCount = 0;
  
  while(millis() - startTime < MAX_WAIT_TIME) {
    // Check if manual override triggered
    if (!autoMode) {
      Serial.println("Manual override activated - sequence interrupted");
      sequenceInProgress = false;
      return;
    }
    
    // Check both sensors
    float leftDist = getDistance(TRIG_PIN_LEFT, ECHO_PIN_LEFT);
    float rightDist = getDistance(TRIG_PIN_RIGHT, ECHO_PIN_RIGHT);
    
    checkCount++;
    
    // Log sensor readings every 10 checks (~5 seconds)
    if (checkCount % 10 == 0) {
      Serial.print("  Sensors - Left: ");
      Serial.print(leftDist);
      Serial.print(" cm, Right: ");
      Serial.print(rightDist);
      Serial.print(" cm (Elapsed: ");
      Serial.print((millis() - startTime) / 1000);
      Serial.println("s)");
    }
    
    // Check if BOTH sensors show no boat (beyond clear distance)
    bool leftClear = (leftDist < 0 || leftDist > CLEAR_DISTANCE);
    bool rightClear = (rightDist < 0 || rightDist > CLEAR_DISTANCE);
    
    if (leftClear && rightClear) {
      Serial.println("✓ Both sensors cleared - boat has passed!");
      Serial.print("  Time taken: ");
      Serial.print((millis() - startTime) / 1000.0);
      Serial.println(" seconds");
      sensorsCleared = true;
      break;  // Exit early - boat has passed!
    }
    
    delay(500);
  }
  
  if (!sensorsCleared) {
    Serial.println("Timeout reached - assuming boat has passed");
    Serial.print("  Maximum wait time: ");
    Serial.print(MAX_WAIT_TIME / 1000);
    Serial.println(" seconds");
  }
  
  // 4. Additional safety delay
  Serial.println("Safety delay...");
  delay(SAFETY_DELAY);
  
  // 5. Reopen gate to car traffic
  Serial.println("Reopening to car traffic...");
  if (!gateIsOpen && autoMode) {
    openGate();
  }
  
  sequenceInProgress = false;
  Serial.println("=== Sequence complete! ===\n");
}

void setup() {
  Serial.begin(115200);
  
  // Initialize LED pins
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  
  // Initialize sensor pins
  pinMode(TRIG_PIN_LEFT, OUTPUT);
  pinMode(ECHO_PIN_LEFT, INPUT);
  pinMode(TRIG_PIN_RIGHT, OUTPUT);
  pinMode(ECHO_PIN_RIGHT, INPUT);
  
  // Set initial states - gate open, green light on
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);
  
  // Initialize servo - start with gate open
  myServo.attach(SERVO_PIN);
  myServo.write(0);  // Open position

  // Connect to Wi-Fi
  Serial.println("\n=== ESP32 Opening Bridge System ===");
  Serial.println("Smart Timer Mode: Sensors + Timeout");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 40) {
    delay(500);
    Serial.print(".");
    timeout++;
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n\nFAILED TO CONNECT TO WIFI!");
    Serial.println("Check your credentials:");
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("Password length: ");
    Serial.println(strlen(password));
    Serial.println("\nMake sure you're using 2.4GHz WiFi!");
  } else {
    Serial.println("");
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  
  Serial.println("Web server started");
  Serial.println("================================\n");
  server.begin();
}

void loop(){
  // === AUTOMATIC BOAT DETECTION ===
  if (autoMode && gateIsOpen && !sequenceInProgress) {
    float leftDist = getDistance(TRIG_PIN_LEFT, ECHO_PIN_LEFT);
    float rightDist = getDistance(TRIG_PIN_RIGHT, ECHO_PIN_RIGHT);
    
    // If EITHER sensor detects boat within detection range
    if ((leftDist > 0 && leftDist <= DETECTION_DISTANCE) || 
        (rightDist > 0 && rightDist <= DETECTION_DISTANCE)) {
      Serial.print("\nBOAT DETECTED! Left: ");
      Serial.print(leftDist);
      Serial.print("cm, Right: ");
      Serial.print(rightDist);
      Serial.println("cm");
      
      automaticBridgeSequence();
      delay(2000); // Debounce delay before checking sensors again
    }
  }
  
  // === WEB SERVER HANDLING ===
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
            // Mode switching works anytime
            if (header.indexOf("GET /auto") >= 0) {
              Serial.println("Switching to AUTO mode");
              autoMode = true;
            } else if (header.indexOf("GET /manual") >= 0) {
              Serial.println("Switching to MANUAL mode");
              autoMode = false;
            }
            // Manual controls only work in MANUAL mode
            else if (header.indexOf("GET /open") >= 0) {
              if (!autoMode) {
                Serial.println("Manual OPEN command received");
                openGate();
              } else {
                Serial.println("⚠ OPEN blocked - Switch to MANUAL mode first");
              }
            } else if (header.indexOf("GET /close") >= 0) {
              if (!autoMode) {
                Serial.println("Manual CLOSE command received");
                closeGate();
              } else {
                Serial.println("⚠ CLOSE blocked - Switch to MANUAL mode first");
              }
            }
            
            // Get current sensor readings for display
            float leftDist = getDistance(TRIG_PIN_LEFT, ECHO_PIN_LEFT);
            float rightDist = getDistance(TRIG_PIN_RIGHT, ECHO_PIN_RIGHT);
            
            // === BUILD HTML PAGE ===
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<meta http-equiv=\"refresh\" content=\"3\">"); // Auto-refresh every 3 seconds
            client.println("<link rel=\"icon\" href=\"data:,\">");
            
            // CSS styling
            client.println("<style>");
            client.println("html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println("h1 { margin-bottom: 10px; }");
            client.println(".status { font-size: 1.5rem; margin: 20px 0; padding: 15px; border-radius: 8px; }");
            client.println(".open { background-color: #d4edda; color: #155724; }");
            client.println(".closed { background-color: #f8d7da; color: #721c24; }");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 24px; margin: 8px; cursor: pointer; border-radius: 8px; display: inline-block;}");
            client.println(".button2 {background-color: #f44336;}");
            client.println(".button3 {background-color: #2196F3;}");
            client.println(".button4 {background-color: #555555;}");
            client.println(".sensor-box { display: inline-block; margin: 10px; padding: 15px; border: 2px solid #ddd; border-radius: 8px; }");
            client.println(".mode-indicator { font-size: 1.2rem; margin: 15px 0; padding: 10px; border-radius: 5px; }");
            client.println(".auto-mode { background-color: #cce5ff; }");
            client.println(".manual-mode { background-color: #fff3cd; }");
            client.println(".info-box { background-color: #e7f3ff; padding: 10px; margin: 15px; border-radius: 5px; font-size: 0.9rem; }");
            client.println("</style></head>");
            
            // Page content
            client.println("<body><h1>ESP32 Bridge Control</h1>");
            
            // Mode indicator
            client.println("<div class=\"mode-indicator " + String(autoMode ? "auto-mode" : "manual-mode") + "\">");
            client.println("<strong>Mode:</strong> " + String(autoMode ? "AUTOMATIC" : "MANUAL") + "</div>");
            
            // Gate status
            client.println("<div class=\"status " + String(gateIsOpen ? "open" : "closed") + "\">");
            client.println("Bridge Status: <strong>" + String(gateIsOpen ? "OPEN (Cars can pass)" : "CLOSED (Boat passing)") + "</strong></div>");
            
            // Smart timer info
            client.println("<div class=\"info-box\">");
            client.println("<strong>Smart Timer Active:</strong> Bridge opens when both sensors clear OR after 20 sec timeout");
            client.println("</div>");
            
            // Control buttons
            client.println("<h2>Manual Controls</h2>");
            if (autoMode) {
              client.println("<p style=\"color: #856404; background-color: #fff3cd; padding: 10px; border-radius: 5px;\">Controls locked in AUTO mode - Switch to MANUAL to use buttons</p>");
              client.println("<p><button class=\"button\" style=\"opacity: 0.5; cursor: not-allowed;\" disabled>OPEN BRIDGE</button>");
              client.println("<button class=\"button button2\" style=\"opacity: 0.5; cursor: not-allowed;\" disabled>CLOSE BRIDGE</button></p>");
            } else {
              client.println("<p><a href=\"/open\"><button class=\"button\">OPEN BRIDGE</button></a>");
              client.println("<a href=\"/close\"><button class=\"button button2\">CLOSE BRIDGE</button></a></p>");
            }
            
            // Mode toggle buttons
            client.println("<h2>Mode Control</h2>");
            client.println("<p><a href=\"/auto\"><button class=\"button button3\">AUTO MODE</button></a>");
            client.println("<a href=\"/manual\"><button class=\"button button4\">MANUAL MODE</button></a></p>");
            
            // Sensor readings
            client.println("<hr>");
            client.println("<h2>Sensor Status</h2>");
            client.println("<div class=\"sensor-box\">");
            client.println("<h3>Left Sensor (West)</h3>");
            client.println("<p>" + String(leftDist > 0 ? String(leftDist, 1) + " cm" : "No reading") + "</p>");
            client.println("<p>" + String((leftDist > 0 && leftDist <= DETECTION_DISTANCE) ? "BOAT DETECTED" : "Clear") + "</p>");
            client.println("</div>");
            
            client.println("<div class=\"sensor-box\">");
            client.println("<h3>Right Sensor (East)</h3>");
            client.println("<p>" + String(rightDist > 0 ? String(rightDist, 1) + " cm" : "No reading") + "</p>");
            client.println("<p>" + String((rightDist > 0 && rightDist <= DETECTION_DISTANCE) ? "BOAT DETECTED" : "Clear") + "</p>");
            client.println("</div>");
            
            // LED status
            client.println("<hr>");
            client.println("<h2>Traffic Lights</h2>");
            client.println("<p>Red LED: " + String(digitalRead(RED_LED) ? "ON" : "OFF") + "</p>");
            client.println("<p>Yellow LED: " + String(digitalRead(YELLOW_LED) ? "ON" : "OFF") + "</p>");
            client.println("<p>Green LED: " + String(digitalRead(GREEN_LED) ? "ON" : "OFF") + "</p>");
            
            // System info
            client.println("<hr>");
            client.println("<p style=\"font-size: 0.9rem; color: #666;\">Page auto-refreshes every 3 seconds | Detection: " + String(DETECTION_DISTANCE) + "cm | Clear: " + String(CLEAR_DISTANCE) + "cm</p>");
            
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
  
  delay(SENSOR_CHECK_INTERVAL); // Small delay between sensor checks
}
