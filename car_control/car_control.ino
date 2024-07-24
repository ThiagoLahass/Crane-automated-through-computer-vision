// Motor_A pin configuration on H-Bridge
int IN1 = 5;
int IN2 = 4;
int SPEED_A_PIN = 9;

// Motor_B pin configuration on H-Bridge
int IN3 = 3;
int IN4 = 2;
int SPEED_B_PIN = 10;

// Reflective Sensor pin configuration
int RIGHT_SENSOR_PIN  = A0;
int CENTER_SENSOR_PIN = A1;
int LEFT_SENSOR_PIN   = A2;

// LDR Sensor pin configuration
int LDR_PIN = A3;

// Auxiliary variables
float kp = 0.05;
float baseSpeed = 85;
int flag = 0;

/**
 * @brief Sets the flag based on the LDR sensor value.
 * 
 * @param ldr_value Value read from the LDR sensor.
 * @return int 1 if the LDR value is above the threshold, otherwise 0.
 */
int set_flag(float ldr_value);

/**
 * @brief Stops the car by setting all motor pins to LOW.
 */
void stop_car();

/**
 * @brief Moves the car by setting the motor pins and speeds.
 * 
 * @param SPEED_A_PIN PIN Speed for motor A.
 * @param leftSpeed Speed for the left side.
 * @param SPEED_B_PIN PIN Speed for motor B.
 * @param rightSpeed Speed for the right side.
 */
void move_car(float SPEED_A_PIN, float leftSpeed, float SPEED_B_PIN, float rightSpeed);

// Pin initialization
void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(SPEED_A_PIN, OUTPUT);
  pinMode(SPEED_B_PIN, OUTPUT);

  pinMode(RIGHT_SENSOR_PIN, INPUT);
  pinMode(CENTER_SENSOR_PIN, INPUT);
  pinMode(LEFT_SENSOR_PIN, INPUT);

  pinMode(LDR_PIN, INPUT);
}

void loop() {
  // Sensor reading
  float ldr_value;

  int right_ = digitalRead(RIGHT_SENSOR_PIN);
  int center_ = digitalRead(CENTER_SENSOR_PIN);
  int left_ = digitalRead(LEFT_SENSOR_PIN);

  while (right_ == 1 && center_ == 1 && left_ == 1) {
    stop_car();
    ldr_value = analogRead(LDR_PIN);
    set_flag(ldr_value);

    delay(3000);

    if (flag == 1) {
      delay(300);
      while (right_ == 1 && center_ == 1 && left_ == 1) {
        right_ = digitalRead(RIGHT_SENSOR_PIN);
        center_ = digitalRead(CENTER_SENSOR_PIN);
        left_ = digitalRead(LEFT_SENSOR_PIN);
        move_car(SPEED_A_PIN, 85, SPEED_B_PIN, 85);
      }

      break;
    }
  }

  while (1) {
    // Analog sensor reading
    float right = analogRead(RIGHT_SENSOR_PIN);
    float center = analogRead(CENTER_SENSOR_PIN);
    float left = analogRead(LEFT_SENSOR_PIN);

    // Calculating Proportional control
    float error = (right - left);
    float adjustment = error * kp;
    float leftSpeed = baseSpeed - adjustment;
    float rightSpeed = baseSpeed + adjustment;

    // Digital sensor reading
    right_ = digitalRead(RIGHT_SENSOR_PIN);
    center_ = digitalRead(CENTER_SENSOR_PIN);
    left_ = digitalRead(LEFT_SENSOR_PIN);

    move_car(SPEED_A_PIN, leftSpeed, SPEED_B_PIN, rightSpeed);

    if (right_ == 1 && center_ == 1 && left_ == 1) {
      flag = 0;
      break;
    }
  }

  delay(50);
}

int set_flag(float ldr_value) {
  if (ldr_value > 700) {
    flag = 1;
    return 1;
  }

  flag = 0;
  return 0;
}

void stop_car() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void move_car(float SPEED_A_PIN, float leftSpeed, float SPEED_B_PIN, float rightSpeed) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(SPEED_A_PIN, leftSpeed);
  analogWrite(SPEED_B_PIN, rightSpeed);
}
