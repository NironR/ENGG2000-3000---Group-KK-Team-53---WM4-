#include <Servo.h>

// VARIABLES
Servo gateServo;

// LED Pins
const int RED_LED = 8;
const int GREEN_LED = 10;

// Ultrasonic Sensors - Entry and Exit
const int TRIG_ENTRY = 2;
const int ECHO_ENTRY = 3;
const int TRIG_EXIT = 4;
const int ECHO_EXIT = 5;

bool gateClosed = false; // false = gate open, true = gate closed
float vehicleDistanceEntry = 0.0;
float vehicleDistanceExit = 0.0;

// FUNCTIONS + HELPERS
void setup() {
    gateServo.attach(6); // servo signal pin
    pinMode(RED_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(TRIG_ENTRY, OUTPUT);
    pinMode(ECHO_ENTRY, INPUT);
    pinMode(TRIG_EXIT, OUTPUT);
    pinMode(ECHO_EXIT, INPUT);
    // Start with green light ON and gate open
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    gateServo.write(0); // open gate
    Serial.begin(9600);
}

float getDistance(int trigPin, int echoPin) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    long duration = pulseIn(echoPin, HIGH, 20000);
    float distance = duration * 0.034 / 2;
    return distance;
}

void closeGate() {
    if (!gateClosed) {
        digitalWrite(GREEN_LED, LOW);
        digitalWrite(RED_LED, HIGH);
        gateServo.write(90); // close gate
        gateClosed = true;
    }
}

void openGate() {
    if (gateClosed) {
        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, HIGH);
        gateServo.write(0); // open gate
        gateClosed = false;
    }
}

// MAIN LOGIC
void loop() {
    vehicleDistanceEntry = getDistance(TRIG_ENTRY, ECHO_ENTRY);
    vehicleDistanceExit = getDistance(TRIG_EXIT, ECHO_EXIT);

    Serial.print("Entry Sensor: ");
    Serial.print(vehicleDistanceEntry);
    Serial.print(" cm | Exit Sensor: ");
    Serial.print(vehicleDistanceExit);
    Serial.println(" cm");

    // Entry logic: close gate if vehicle detected
    if (vehicleDistanceEntry > 0 && vehicleDistanceEntry <= 15) {
        closeGate();
    }
    // Exit logic: open gate if vehicle detected
    if (vehicleDistanceExit > 0 && vehicleDistanceExit <= 15) {
        openGate();
    }
    delay(200);
}