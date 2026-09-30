// ---------- Ultrasonic sensor pins ----------
const int trigPin = 5;
const int echoPin = 6;

// ---------- L298N motor-driver pins ----------
const int IN1 = 8;    // Left motor direction pin 1
const int IN2 = 9;    // Left motor direction pin 2
const int IN3 = 10;   // Right motor direction pin 1
const int IN4 = 11;   // Right motor direction pin 2

// Change this to control how close an object must be
// before the robot avoids it.
const int obstacleDistance = 10;  // centimeters

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  stopMotors();

  Serial.begin(9600);
}

void loop() {
  int distance = getDistanceCM();

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // A zero reading means no echo arrived.
  // Treat it as unsafe and avoid moving forward blindly.
  if (distance == 0 || distance <= obstacleDistance) {
    avoidObstacle();
  } 
  else {
    moveForward();
  }

  delay(50);
}

// Returns the measured distance in centimeters.
// Returns 0 if the sensor receives no echo in time.
int getDistanceCM() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // 30 ms timeout prevents the code from getting stuck
  // when there is no returned echo.
  unsigned long duration = pulseIn(echoPin, HIGH, 30000UL);

  if (duration == 0) {
    return 0;
  }

  // Sound moves about 0.0343 cm per microsecond.
  // Divide by 2 because sound travels to the object and back.
  int distance = duration * 0.0343 / 2;

  return distance;
}

// Both motors forward
void moveForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

// Both motors backward
void moveBackward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

// Stop both motors
void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

// Pivot left: left wheel backward, right wheel forward
void turnLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

// Stop, reverse, then pivot left
void avoidObstacle() {
  stopMotors();
  delay(200);

  moveBackward();
  delay(500);

  stopMotors();
  delay(150);

  turnLeft();
  delay(550);

  stopMotors();
  delay(150);
}