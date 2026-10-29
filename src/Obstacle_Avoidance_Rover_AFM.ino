#include <AFMotor.h>
#include <Servo.h>

// --- Adjusted Motor Calibration ---
// If it still veers LEFT: Increase MAX_SPEED_R or Decrease MAX_SPEED_L
const int MAX_SPEED_L = 150; // M3 (Left)
const int MAX_SPEED_R = 120; // M2 (Right) - Increased to counter left drift
const int MIN_SPEED_L = 116;  
const int MIN_SPEED_R = 90; 

const int TRIG_PIN = A3;
const int ECHO_PIN = A2;
const int STOP_DIST = 12;
const int SLOW_DIST = 25;

AF_DCMotor motorR(2); // Physical M2
AF_DCMotor motorL(3); // Physical M3
Servo scanServo;

void setup() {
  Serial.begin(9600);
  scanServo.attach(9);
  scanServo.write(90);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  motorL.run(RELEASE);
  motorR.run(RELEASE);
  Serial.println("Rover Initialized...");
}

void loop() {
  int currentDist = getDistance();
  
  // Real-time Telemetry
  printData("Forward", currentDist);

  if (currentDist <= STOP_DIST) {
    obstacleRoutine();
  } else {
    adjustVelocity(currentDist);
    motorL.run(FORWARD);
    motorR.run(FORWARD);
  }
  delay(50);
}

void printData(String label, int dist) {
  Serial.print(label);
  Serial.print(" Dist: ");
  Serial.print(dist);
  Serial.println("cm");
}

void adjustVelocity(int dist) {
  int speedL, speedR;
  if (dist > SLOW_DIST) {
    speedL = MAX_SPEED_L;
    speedR = MAX_SPEED_R;
  } else {
    speedL = map(dist, STOP_DIST, SLOW_DIST, MIN_SPEED_L, MAX_SPEED_L);
    speedR = map(dist, STOP_DIST, SLOW_DIST, MIN_SPEED_R, MAX_SPEED_R);
  }
  motorL.setSpeed(speedL);
  motorR.setSpeed(speedR);
}

int getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 25000);
  int dist = duration * 0.034 / 2;

  if (dist <= 0 || dist > 400) return 400;
  return dist;
}

void obstacleRoutine() {
  Serial.println("--- Obstacle! Scanning ---");
  motorL.run(RELEASE);
  motorR.run(RELEASE);
  
  int scans[5];
  int angles[] = {0, 45, 90, 135, 180};
  int maxDist = 0;
  int bestAngleIndex = 2;

  for (int i = 0; i < 5; i++) {
    scanServo.write(angles[i]);
    delay(400); // Stabilization delay
    scans[i] = getDistance();
    
    Serial.print("Angle "); Serial.print(angles[i]);
    Serial.print(": "); Serial.println(scans[i]);

    if (scans[i] > maxDist) {
      maxDist = scans[i];
      bestAngleIndex = i;
    }
  }

  executeTurn(angles[bestAngleIndex], maxDist);
  
  scanServo.write(90);
  delay(300);
}

void executeTurn(int angle, int maxDist) {
  motorL.setSpeed(MAX_SPEED_L);
  motorR.setSpeed(MAX_SPEED_R);

  // If even the best path is blocked, it's a trap
  if (maxDist < 15) { 
    Serial.println("Decision: U-TURN");
    motorL.run(BACKWARD); motorR.run(FORWARD);
    delay(800); 
  } else if (angle < 90) { 
    Serial.println("Decision: RIGHT");
    motorL.run(FORWARD); motorR.run(BACKWARD);
    delay(350); 
  } else if (angle > 90) { 
    Serial.println("Decision: LEFT");
    motorL.run(BACKWARD); motorR.run(FORWARD);
    delay(450);
  }
  
  motorL.run(RELEASE); 
  motorR.run(RELEASE);
}