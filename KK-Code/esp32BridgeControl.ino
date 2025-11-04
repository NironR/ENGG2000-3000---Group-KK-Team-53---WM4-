/*********
  ESP32 Opening Bridge Web Server
  WiFi control + Ultrasonic boat detection + Smart timer with sensor clearing
*********/

#include <WiFi.h>
#include <ESP32Servo.h>
#include "esp_wpa2.h"   // Required for enterprise authentication

// ===== CREDENTIALS ====== 
#define WIFI_SSID "Macquarie OneNet"
#define EAP_IDENTITY "STUDENT_NUMBER" 
#define EAP_PASSWORD "PASSWORD"                      

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
const int MAX_WAIT_TIME = 20000;
const int DETECTION_DISTANCE = 20;
const int CLEAR_DISTANCE = 50;
const int SAFETY_DELAY = 2000;
const int SENSOR_CHECK_INTERVAL = 200;

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
  
  long duration = pulseIn(echoPin, HIGH, 20000);
  if (duration == 0) return -1;
  
  float distance = duration * 0.034 / 2;
  return distance;
}

// Gate control functions
void closeGate(){
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, HIGH);
  delay(2000);
  myServo.write(90);
  gateIsOpen = false;
  Serial.println("Gate CLOSED");
}

void openGate(){
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);
  delay(2000);
  myServo.write(0);
  gateIsOpen = true;
  Serial.println("Gate OPENED");
}

// Automatic bridge sequence with smart timer
void automaticBridgeSequence() {
  sequenceInProgress = true;
  Serial.println("=== BOAT DETECTED! Starting automatic sequence ===");
  
  // 1. Flash yellow warning
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
    if (!autoMode) {
      Serial.println("Manual override activated - sequence interrupted");
      sequenceInProgress = false;
      return;
    }
    
    float leftDist = getDistance(TRIG_PIN_LEFT, ECHO_PIN_LEFT);
    float rightDist = getDistance(TRIG_PIN_RIGHT, ECHO_PIN_RIGHT);
    
    checkCount++;
    
    if (checkCount % 10 == 0) {
      Serial.print("  Sensors - Left: ");
      Serial.print(leftDist);
      Serial.print(" cm, Right: ");
      Serial.print(rightDist);
      Serial.print(" cm (Elapsed: ");
      Serial.print((millis() - startTime) / 1000);
      Serial.println("s)");
    }
    
    bool leftClear = (leftDist < 0 || leftDist > CLEAR_DISTANCE);
    bool rightClear = (rightDist < 0 || rightDist > CLEAR_DISTANCE);
    
    if (leftClear && rightClear) {
      Serial.println("✓ Both sensors cleared - boat has passed!");
      Serial.print("  Time taken: ");
      Serial.print((millis() - startTime) / 1000.0);
      Serial.println(" seconds");
      sensorsCleared = true;
      break;
    }
    
    delay(500);
  }
  
  if (!sensorsCleared) {
    Serial.println("Timeout reached - assuming boat has passed");
    Serial.print("  Maximum wait time: ");
    Serial.print(MAX_WAIT_TIME / 1000);
    Serial.println(" seconds");
  }
  
  Serial.println("Safety delay...");
  delay(SAFETY_DELAY);
  
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
  
  // Set initial states
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);
  
  // Initialize servo
  myServo.attach(SERVO_PIN);
  myServo.write(0);

  // ===== CONNECT TO MACQUARIE UNIVERSITY WIFI =====
  Serial.println("\n=== ESP32 Opening Bridge System ===");
  Serial.println("Connecting to Macquarie OneNet (WPA2-Enterprise)...");
  Serial.print("SSID: ");
  Serial.println(WIFI_SSID);
  Serial.print("Identity: ");
  Serial.println(EAP_IDENTITY);
  
  // Disconnect from any previous connections
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  
  // Configure WPA2-Enterprise authentication
  esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));
  esp_wifi_sta_wpa2_ent_enable();
  
  // Start connection
  WiFi.begin(WIFI_SSID);
  
  // Wait for connection (up to 20 seconds)
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 40) {
    delay(500);
    Serial.print(".");
    timeout++;
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n\nFAILED TO CONNECT TO MACQUARIE ONENET!");
    Serial.println("\n=== Troubleshooting Guide ===");
    Serial.println("1. Check your Student ID format:");
    Serial.println("\n2. Verify your OneID password is correct");
    Serial.println(WiFi.macAddress());
    Serial.println("\n4. Ensure you're in range of Macquarie OneNet");
  } else {
    Serial.println("");
    Serial.println("Successfully connected to Macquarie OneNet!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("MAC address: ");
    Serial.println(WiFi.macAddress());
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
    
    if ((leftDist > 0 && leftDist <= DETECTION_DISTANCE) || 
        (rightDist > 0 && rightDist <= DETECTION_DISTANCE)) {
      Serial.print("\nBOAT DETECTED! Left: ");
      Serial.print(leftDist);
      Serial.print("cm, Right: ");
      Serial.print(rightDist);
      Serial.println("cm");
      
      automaticBridgeSequence();
      delay(2000);
    }
  }
  
  // === WEB SERVER HANDLING ===
  WiFiClient client = server.available();

  if (client) {
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client connected");
    String currentLine = "";
    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // === HANDLE COMMANDS ===
            if (header.indexOf("GET /auto") >= 0) {
              Serial.println("Switching to AUTO mode");
              autoMode = true;
            } else if (header.indexOf("GET /manual") >= 0) {
              Serial.println("Switching to MANUAL mode");
              autoMode = false;
            }
            else if (header.indexOf("GET /open") >= 0) {
              if (!autoMode) {
                Serial.println("Manual OPEN command received");
                openGate();
              } else {
                Serial.println("OPEN blocked - Switch to MANUAL mode first");
              }
            } else if (header.indexOf("GET /close") >= 0) {
              if (!autoMode) {
                Serial.println("Manual CLOSE command received");
                closeGate();
              } else {
                Serial.println("CLOSE blocked - Switch to MANUAL mode first");
              }
            }
            
            float leftDist = getDistance(TRIG_PIN_LEFT, ECHO_PIN_LEFT);
            float rightDist = getDistance(TRIG_PIN_RIGHT, ECHO_PIN_RIGHT);
            
            // === BUILD HTML PAGE ===
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<meta http-equiv=\"refresh\" content=\"3\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            
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
            
            client.println("<body><h1>ESP32 Bridge Control</h1>");
            client.println("<p style=\"color: #666; font-size: 0.9rem;\">Connected to Macquarie OneNet</p>");
            
            client.println("<div class=\"mode-indicator " + String(autoMode ? "auto-mode" : "manual-mode") + "\">");
            client.println("<strong>Mode:</strong> " + String(autoMode ? "AUTOMATIC" : "MANUAL") + "</div>");
            
            client.println("<div class=\"status " + String(gateIsOpen ? "open" : "closed") + "\">");
            client.println("Bridge Status: <strong>" + String(gateIsOpen ? "OPEN (Cars can pass)" : "CLOSED (Boat passing)") + "</strong></div>");
            
            client.println("<div class=\"info-box\">");
            client.println("<strong>Smart Timer Active:</strong> Bridge opens when both sensors clear OR after 20 sec timeout");
            client.println("</div>");
            
            client.println("<h2>Manual Controls</h2>");
            if (autoMode) {
              client.println("<p style=\"color: #856404; background-color: #fff3cd; padding: 10px; border-radius: 5px;\">Controls locked in AUTO mode - Switch to MANUAL to use buttons</p>");
              client.println("<p><button class=\"button\" style=\"opacity: 0.5; cursor: not-allowed;\" disabled>OPEN BRIDGE</button>");
              client.println("<button class=\"button button2\" style=\"opacity: 0.5; cursor: not-allowed;\" disabled>CLOSE BRIDGE</button></p>");
            } else {
              client.println("<p><a href=\"/open\"><button class=\"button\">OPEN BRIDGE</button></a>");
              client.println("<a href=\"/close\"><button class=\"button button2\">CLOSE BRIDGE</button></a></p>");
            }
            
            client.println("<h2>Mode Control</h2>");
            client.println("<p><a href=\"/auto\"><button class=\"button button3\">AUTO MODE</button></a>");
            client.println("<a href=\"/manual\"><button class=\"button button4\">MANUAL MODE</button></a></p>");
            
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
            
            client.println("<hr>");
            client.println("<h2>Traffic Lights</h2>");
            client.println("<p>Red LED: " + String(digitalRead(RED_LED) ? "ON" : "OFF") + "</p>");
            client.println("<p>Yellow LED: " + String(digitalRead(YELLOW_LED) ? "ON" : "OFF") + "</p>");
            client.println("<p>Green LED: " + String(digitalRead(GREEN_LED) ? "ON" : "OFF") + "</p>");
            
            client.println("<hr>");
            client.println("<p style=\"font-size: 0.9rem; color: #666;\">Page auto-refreshes every 3 seconds | Detection: " + String(DETECTION_DISTANCE) + "cm | Clear: " + String(CLEAR_DISTANCE) + "cm</p>");
            
            client.println("</body></html>");
            
            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected\n");
  }
  
  delay(SENSOR_CHECK_INTERVAL);
}
