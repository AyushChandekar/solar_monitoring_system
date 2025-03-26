#include <Servo.h>

Servo solarServo;   // Servo motor object
int servoPos = 90;  // Initial servo position (middle)

// LDR sensor pins
const int topLDR = A1;     // Top LDR
const int bottomLDR = A0;  // Bottom LDR

// Servo movement limits
const int servoMin = 0;
const int servoMax = 180;

// Sensor pins
const int TEMP_SENSOR = A5;
const int RAIN_SENSOR = 4;

// Timing variables
unsigned long previousServoUpdate = 0;
const long servoInterval = 300;  // Servo update interval

void setup() {
  solarServo.attach(3);        // Attach servo to pin 3 (PWM)
  solarServo.write(servoPos);  // Set initial position
  Serial.begin(9600);
  pinMode(RAIN_SENSOR, INPUT_PULLUP);  // Use internal pull-up resistor
}

void loop() {
  // Read sensors
  int topValue = analogRead(topLDR);
  int bottomValue = analogRead(bottomLDR);
  int diff = topValue - bottomValue;

  // Update servo position at intervals
  if (millis() - previousServoUpdate >= servoInterval) {
    previousServoUpdate = millis();
    
    int tolerance = 20;
    if (abs(diff) > tolerance) {
      int stepSize = map(abs(diff), 20, 400, 2, 10);  // Dynamic movement speed

      servoPos += (diff > 0) ? stepSize : -stepSize;
      servoPos = constrain(servoPos, servoMin, servoMax);
      solarServo.write(servoPos);
    }
  }

  // Read and send sensor data
  static unsigned long previousDataSend = 0;
  if (millis() - previousDataSend >= 2000) {  // Send data every 2 seconds
    previousDataSend = millis();

    // Temperature reading (assuming LM35 sensor)
    int tempValue = analogRead(TEMP_SENSOR);
    float temperatureC = (tempValue * 5.0 / 1023.0) * 100.0;

    // Rain sensor reading (LOW when raining)
    bool isRaining = digitalRead(RAIN_SENSOR) == LOW;
    String rainCondition = isRaining ? "Not Raining" : "Raining";

    // Send data via Serial
    Serial.print(temperatureC, 1);  // 1 decimal place
    Serial.print(',');
    Serial.println(rainCondition);
  }
}