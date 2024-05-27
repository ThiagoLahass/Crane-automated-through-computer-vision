#include <ESP32Servo.h>

#define DEBOUNCE_TIME 200 //ms
unsigned long timestamp_last_activation= 0;

// SETUP SERVO
#define SERVO_STOPPED_VALUE            93
#define SERVO_SPEED                    5
#define SERVO_PIN                      23
Servo servo;

// SETUP ELECTROMAGNET
#define ELECTROMAGNET_PIN              22

// OTHER SETUP VARIABLES
#define ELECTROMAGNET_DOWN_TIME        3000
#define CONTAINER_POSITION_ERROR_RANGE 20
#define TIME_BRIDGE_CENTER_TO_CART     5000

// SETUP MOTORS
#define EN_A_PIN              19
#define IN1_PIN               18
#define IN2_PIN               15
#define EN_B_PIN              17
#define IN3_PIN               16
#define IN4_PIN               4
#define MOTOR_MOV_SPEED_PIN   0

// LIMIT SWITCH SENSORS
#define LIM_X_MIN_PIN  12
#define LIM_X_MAX_PIN  14
#define LIM_Y_MIN_PIN  27
#define LIM_Y_MAX_PIN  26

// CRANE TRANSLATION VARIABLES
int delta_x = 0;
int delta_y = 0;
int lim_x_min = 0;
int lim_x_max = 0;
int lim_y_min = 0;
int lim_y_max = 0;
int end_of_course = 0;

///////////////////////////////////////////////////////////////////////////////////
////////////////////////////// FUNCTION HEADERS ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Sets the motor speed based on the input from a speed potentiometer.
 */
void speed();

/**
 * @brief Controls the movement of motors in different directions based on the input pins.
 * 
 * @param in1_ State for motor IN1 pin.
 * @param in2_ State for motor IN2 pin.
 * @param in3_ State for motor IN3 pin.
 * @param in4_ State for motor IN4 pin.
 */
void move(int in1_, int in2_, int in3_, int in4_);

/**
 * @brief Moves both motors forward.
 */
void move_1_forward_2_forward();

/**
 * @brief Moves motor 1 forward and stops motor 2.
 */
void move_1_forward_2_stop();

/**
 * @brief Moves motor 1 forward and motor 2 backward.
 */
void move_1_forward_2_backward();

/**
 * @brief Stops motor 1 and moves motor 2 forward.
 */
void move_1_stop_2_forward();

/**
 * @brief Stops both motors.
 */
void move_1_stop_2_stop();

/**
 * @brief Stops motor 1 and moves motor 2 backward.
 */
void move_1_stop_2_backward();

/**
 * @brief Moves motor 1 backward and motor 2 forward.
 */
void move_1_backward_2_forward();

/**
 * @brief Stops motor 1 and moves motor 2 backward.
 */
void move_1_backward_2_stop();

/**
 * @brief Moves both motors backward.
 */
void move_1_backward_2_backward();

/**
 * @brief Moves the system to the initial position (0, 0) using the limit switch sensors.
 */
void move_to_initial_position();

/**
 * @brief Moves the system to the central position after reaching the initial position.
 */
void move_to_central_position();

/**
 * @brief Lifts the load using a servo motor and an electromagnet.
 */
void lift_load();

/**
 * @brief Lowers the load by releasing it from the electromagnet.
 */
void lower_load();

/**
 * @brief Interrupt function for X minimum limit switch.
 */
void IRAM_ATTR lim_min_x_interrupt();

/**
 * @brief Interrupt function for X maximum limit switch.
 */
void IRAM_ATTR lim_max_x_interrupt();

/**
 * @brief Interrupt function for Y minimum limit switch.
 */
void IRAM_ATTR lim_min_y_interrupt();

/**
 * @brief Interrupt function for Y maximum limit switch.
 */
void IRAM_ATTR lim_max_y_interrupt();

///////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// SETUP /////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void setup(){
  Serial.begin(9600);
  servo.attach(SERVO_PIN);
  
  pinMode(EN_A_PIN, OUTPUT);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  
  pinMode(EN_B_PIN, OUTPUT);
  pinMode(IN3_PIN, OUTPUT);
  pinMode(IN4_PIN, OUTPUT);
  
  pinMode(MOTOR_MOV_SPEED_PIN, INPUT);
  
  pinMode(ELECTROMAGNET_PIN, OUTPUT);
  
  // SET LIMIT SWITCH SENSORS AS INTERRUPT PINS
  pinMode(LIM_X_MIN_PIN, INPUT);
  pinMode(LIM_X_MAX_PIN, INPUT);
  pinMode(LIM_Y_MIN_PIN, INPUT);
  pinMode(LIM_Y_MAX_PIN, INPUT);

  // Interrupt configuration
  attachInterrupt(LIM_X_MIN_PIN, lim_min_x_interrupt, RISING);
  // attachInterrupt(LIM_X_MAX_PIN, lim_max_x_interrupt, RISING);
  // attachInterrupt(LIM_Y_MIN_PIN, lim_min_y_interrupt, RISING);
  // attachInterrupt(LIM_Y_MAX_PIN, lim_max_x_interrupt, RISING);
  
  // SETUP STATE
  // MOVE THE CAR TO THE INITIAL POSITION (0, 0)
  // THEN MOVE TO THE CENTER OF THE BRIDGE TO HAVE A HOLISCT VIEW OF THE CONTAINERS
  // move_to_central_position();
  
  delay(100);
}

///////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// LOOP /////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void loop(){
  
  ///////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////// INITIAL STATE ////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  // -> WAITING FOR COMMAND 'begin' FROM ESP TO START SEARCHING FOR CONTAINERS

  int command_identified = 0;
  int start_command      = 0;
  
  String str = "";
  
  Serial.println("ESP: Waiting for command via Serial to start search... (begin)");
  
  while(!start_command){
    if (Serial.available() > 0) {  // Check if data is available on the serial
      str = Serial.readString();   // Read the received data as a string
      str.trim();                  // Remove whitespace from the start and end of the string
      command_identified = 1;
    }
      
    if(command_identified){
      Serial.print("ESP: ");
      Serial.println(str);

      // Check if the received string equals "begin"
      if (str.equals("begin")) {
        Serial.println("ESP: Start command received!");
        start_command = 1;
      } else {
        Serial.println("ESP: Invalid command!");
        str = "";
      }

      // Reset string 
      str = "";
    }

    delay(5);
    command_identified = 0;
  }
  
  ///////////////////////////////////////////////////////////////////////////////////
  ///////////////////////// SELECTED CONTAINER SEARCH STATE /////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  // -> 'begin' COMMAND RECEIVED FROM ESP, START SEARCHING CONTAINER
  /* -> THE DATA RECEIVED HERE FROM ESP MUST BE IN THE FORMAT:
          "sxxx zyyy",
          WHERE:
          s is the sign of delta_x (+ or -);
          xxx are the 3 characters of delta_x;
          z is the sign of delta_y (+ or -);
          yyy are the 3 characters of delta_y.
        */
  int container_centralized = 0;
  str = "";
  
  Serial.println("ESP: Waiting for container position...");
  
  unsigned long start_time_x = millis();   // Save the start time of x centralization
  unsigned long start_time_y = millis();   // Save the start time of y centralization
  unsigned long elapsed_time_x = 0;        // Variable to store the time taken for x centralization
  unsigned long elapsed_time_y = 0;        // Variable to store the time taken for y centralization
  
  while(!container_centralized){

    // CHECKING IF ANY OF THE END OF COURSE SENSORS HAVE BEEN ACTIVATED
    if(lim_x_min == 1 || lim_x_max == 1 || lim_y_min == 1 || lim_y_max == 1){
      Serial.println("ESP: End of course activated");
      delay(1000);
      move_to_central_position();
      lim_x_min = 0;
      lim_x_max = 0;
      lim_y_min = 0;
      lim_y_max = 0;

      end_of_course = 1;
      
      break;
    }

    // SET MOTOR SPEED ACCORDING TO THE SPEED CONTROL POTENTIOMETER
    speed();

    if (Serial.available() > 0) {  // Check if data is available on the serial
      str = Serial.readString();   // Read the received data as a string
      str.trim();                  // Remove whitespace from the start and end of the string
      command_identified = 1;
    }
      
    if(command_identified){
      // CHECK COMMAND
      // Serial.print("ESP: ");
      // Serial.print(str);
      
      /* 
      THE INFORMATION RECEIVED HERE MUST BE IN THE FORMAT:
        "sxxx zyyy",
        WHERE:
        s is the sign of delta_x (+ or -);
        xxx are the 3 characters of delta_x;
        z is the sign of delta_y (+ or -);
        yyy are the 3 characters of delta_y.
      */
      String str_delta_x = str.substring(0, 4); // Extract the first 4 characters (delta_x)
      String str_delta_y = str.substring(5, 9); // Extract the last  4 characters (delta_y)

      // Convert strings to integers
      delta_x = str_delta_x.toInt();
      delta_y = str_delta_y.toInt();
      
      // Serial.print("ESP: delta_x: ");
      // Serial.print(delta_x);
      // Serial.print("ESP: delta_y: ");
      // Serial.print(delta_y);
      
      if(delta_x > CONTAINER_POSITION_ERROR_RANGE){              // CAMERA IS TO THE LEFT OF THE CONTAINER CENTER

        if(delta_y > CONTAINER_POSITION_ERROR_RANGE){            // CAMERA IS BELOW THE CONTAINER CENTER
          move_1_forward_2_forward();
        }
        else if(delta_y < -CONTAINER_POSITION_ERROR_RANGE){      // CAMERA IS ABOVE THE CONTAINER CENTER
          move_1_forward_2_backward();
        }
        else{                                                    // CAMERA IS CENTERED VERTICALLY ON THE CONTAINER
          // Serial.print("ESP: Centered in y");
          if(elapsed_time_y == 0){
            elapsed_time_y = millis() - start_time_y;
          }
          move_1_forward_2_stop();
        }
      }
      else if(delta_x < -CONTAINER_POSITION_ERROR_RANGE){        // CAMERA IS TO THE RIGHT OF THE CONTAINER CENTER
        
        if(delta_y > CONTAINER_POSITION_ERROR_RANGE){            // CAMERA IS BELOW THE CONTAINER CENTER
          move_1_backward_2_forward();
        }
        else if(delta_y < -CONTAINER_POSITION_ERROR_RANGE){      // CAMERA IS ABOVE THE CONTAINER CENTER
          move_1_backward_2_backward();
        }
        else{                                                    // CAMERA IS CENTERED VERTICALLY ON THE CONTAINER
          // Serial.print("ESP: Centered in y");
          if(elapsed_time_y == 0){
            elapsed_time_y = millis() - start_time_y;
          }
          move_1_backward_2_stop();
        }
      }
      else{                                                      // CAMERA IS CENTERED HORIZONTALLY ON THE CONTAINER
        // Serial.print("ESP: Centered in x");
        if(elapsed_time_x == 0){
          elapsed_time_x = millis() - start_time_x;
        }
        
        if(delta_y > CONTAINER_POSITION_ERROR_RANGE){            // CAMERA IS BELOW THE CONTAINER CENTER
          move_1_stop_2_forward();      
        }
        else if(delta_y < -CONTAINER_POSITION_ERROR_RANGE){      // CAMERA IS ABOVE THE CONTAINER CENTER
          move_1_stop_2_backward();
        }
        else{                                                    // CAMERA IS CENTERED IN BOTH DIRECTIONS !!!
          if(elapsed_time_y == 0){
            elapsed_time_y = millis() - start_time_y;
          }
          
          move_1_stop_2_stop();
          
          Serial.print("ESP: container centered");
          container_centralized = 1;
        }
      }
      
      if(!container_centralized){
        // Serial.print("ESP: Not yet centered on the container...");
      }
       
      // Reset variables 
      str = "";
      command_identified = 0; 
    }
    
    delay(10);
  }

  // IF END OF COURSE IS ACTIVATED, WE MUST RESTART THE LOOP
  if(end_of_course == 1){
    end_of_course = 0;
    continue;
  }

  // DELAY TO ALLOW ESP TO RECEIVE INFORMATION FROM THE SERIAL THAT THE CONTAINER IS CENTRALIZED
  delay(2000);
  
  ///////////////////////////////////////////////////////////////////////////////////
  //////////////////////////// PICKING UP CONTAINER STATE ///////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////

  Serial.print("ESP: Picking up container...");
  lift_load();
  delay(ELECTROMAGNET_DOWN_TIME);
  
  ///////////////////////////////////////////////////////////////////////////////////
  /////////////////// LOADING CONTAINER TO THE AUTONOMOUS CART STATE ////////////////
  ///////////////////////////////////////////////////////////////////////////////////

  Serial.print("ESP: Elapsed time x: ");
  Serial.println(elapsed_time_x);
  Serial.print("ESP: Elapsed time y: ");
  Serial.println(elapsed_time_y);
  
  Serial.println("ESP: Transporting container to the cart...");
  
  if(elapsed_time_x <= elapsed_time_y + TIME_BRIDGE_CENTER_TO_CART){
    move_1_backward_2_backward();
    delay(elapsed_time_x);
    
    move_1_stop_2_backward();
    delay(elapsed_time_y + TIME_BRIDGE_CENTER_TO_CART - elapsed_time_x);
  }
  else{
    move_1_backward_2_backward();
    delay(elapsed_time_y + TIME_BRIDGE_CENTER_TO_CART);
    
    move_1_backward_2_stop();
    delay(elapsed_time_x - (elapsed_time_y + TIME_BRIDGE_CENTER_TO_CART));
  }
  
  Serial.println("ESP: Container centered above the cart");
  move_1_stop_2_stop();
  
  ///////////////////////////////////////////////////////////////////////////////////
  ///////////// PLACING THE CONTAINER ON THE AUTONOMOUS CART STATE //////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  
  Serial.println("ESP: Placing container on the cart...");
  lower_load();
  delay(ELECTROMAGNET_DOWN_TIME);
  
  ///////////////////////////////////////////////////////////////////////////////////
  //////////// MOVING THE BRIDGE BACK TO THE CENTRAL POSITION STATE /////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  
  Serial.println("ESP: Moving the car back to the central position");

  move_1_stop_2_forward();
  delay(TIME_BRIDGE_CENTER_TO_CART);
  move_1_stop_2_stop();
  Serial.print("ESP: end");

  // DELAY TO ALLOW THE BACKEND TO RECEIVE THE COMMAND INDICATING THE CYCLE IS COMPLETE
  delay(2000);
  
  ///////////////////////////////////////////////////////////////////////////////////
  //////////////////////////// END OF THE CYCLE STATE ///////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////// FUNCTION DEFINITIONS ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
void speed(){
  int motor_speed = analogRead(MOTOR_MOV_SPEED_PIN);
  motor_speed = map(motor_speed, 0, 4095, 0, 255);
  analogWrite(EN_A_PIN, motor_speed);
  analogWrite(EN_B_PIN, motor_speed);
}

void move(int in1_, int in2_, int in3_, int in4_){
  digitalWrite(IN1_PIN, in1_);
  digitalWrite(IN2_PIN, in2_);
  digitalWrite(IN3_PIN, in3_);
  digitalWrite(IN4_PIN, in4_);
}

void move_1_forward_2_forward(){
  move(HIGH, LOW, HIGH, LOW);
}

void move_1_forward_2_stop(){
  move(HIGH, LOW, LOW, LOW);
}

void move_1_forward_2_backward(){
  move(HIGH, LOW, LOW, HIGH);
}

void move_1_stop_2_forward(){
  move(LOW, LOW, HIGH, LOW);
}

void move_1_stop_2_stop(){
  move(LOW, LOW, LOW, LOW);
}

void move_1_stop_2_backward(){
  move(LOW, LOW, LOW, HIGH);
}

void move_1_backward_2_forward(){
  move(LOW, HIGH, HIGH, LOW);
}

void move_1_backward_2_stop(){
  move(LOW, HIGH, LOW, LOW);
}

void move_1_backward_2_backward(){
  move(LOW, HIGH, LOW, HIGH);
}

void move_to_initial_position(){
  bool initial_position_x_reached = false;
  bool initial_position_y_reached = false;
  
  Serial.println("ESP: Moving the car to the initial position...");
  speed();
  move_1_backward_2_backward();
  
  while(1){
    speed();
    
    // IF BOTH SENSORS ARE ACTIVATED, WE ARE IN THE
    // INITIAL POSITION x = 0 and y = 0
    // BOTH MOTORS MUST BE DEACTIVATED
    // AND WE EXIT THIS LOOP
    if(lim_x_min == 1 && lim_y_min == 1){
      move_1_stop_2_stop();
      Serial.println("ESP: Initial position reached");
      lim_x_min = 0;
      lim_x_max = 0;
      lim_y_min = 0;
      lim_y_max = 0;
      break;
    }
    // IF ONLY THE X MINIMUM LIMIT SENSOR IS ACTIVATED
    // ONLY THE X TRANSLATION MOTOR SHOULD BE DEACTIVATED
    if(!initial_position_x_reached && lim_x_min == 1){
      initial_position_x_reached = true;
      move_1_stop_2_backward();
      Serial.println("ESP: Initial x position reached");
    }
    // IF ONLY THE Y MINIMUM LIMIT SENSOR IS ACTIVATED
    // ONLY THE Y TRANSLATION MOTOR SHOULD BE DEACTIVATED
    if(!initial_position_y_reached && lim_y_min == 1){
      initial_position_y_reached = true;
      move_1_backward_2_stop();
      Serial.println("ESP: Initial y position reached");
    }
  }
}

void move_to_central_position(){
  move_to_initial_position();
  
  Serial.println("ESP: Moving the car to the central position...");

  speed();

  move_1_forward_2_forward();
  delay(2000);
  move_1_stop_2_forward();
  delay(1000);
  move_1_stop_2_stop();
  
  Serial.println("ESP: Central position reached!");
}

void lift_load(){
  servo.write(SERVO_STOPPED_VALUE - SERVO_SPEED); // clockwise
  delay(ELECTROMAGNET_DOWN_TIME);
  servo.write(SERVO_STOPPED_VALUE);
  digitalWrite(ELECTROMAGNET_PIN, HIGH);
  delay(1000);
  servo.write(SERVO_STOPPED_VALUE + SERVO_SPEED); // counterclockwise
  delay(ELECTROMAGNET_DOWN_TIME);
  servo.write(SERVO_STOPPED_VALUE);
}

void lower_load(){
  servo.write(SERVO_STOPPED_VALUE - SERVO_SPEED); // clockwise
  delay(ELECTROMAGNET_DOWN_TIME);
  servo.write(SERVO_STOPPED_VALUE);
  digitalWrite(ELECTROMAGNET_PIN, LOW);
  delay(1000);
  servo.write(SERVO_STOPPED_VALUE + SERVO_SPEED); // counterclockwise
  delay(ELECTROMAGNET_DOWN_TIME);
  servo.write(SERVO_STOPPED_VALUE);
}

// INTERRUPT FUNCTION DEFINITIONS
void lim_min_x_interrupt(){
  // Checks if the debounce time has been met
  if ( (millis() - timestamp_last_activation) >= DEBOUNCE_TIME ) {
    move_1_stop_2_stop();
    // Serial.println("ESP: LIM MIN X reached");
    // delay(1000);
    // move_to_initial_position();

    lim_x_min = 1;
    timestamp_last_activation = millis();
  }
}

void lim_max_x_interrupt(){
  // Checks if the debounce time has been met
  if ( (millis() - timestamp_last_activation) >= DEBOUNCE_TIME ) {
    move_1_stop_2_stop();
    // Serial.println("ESP: LIM MAX X reached");
    // delay(1000);
    // move_to_initial_position();
    
    lim_x_max = 1;
    timestamp_last_activation = millis();
  }
}

void lim_min_y_interrupt(){
  // Checks if the debounce time has been met
  if ( (millis() - timestamp_last_activation) >= DEBOUNCE_TIME ) {
    move_1_stop_2_stop();
    // Serial.println("ESP: LIM MIX Y reached");
    // delay(1000);
    // move_to_initial_position();
    
    lim_y_min = 1;
    timestamp_last_activation = millis();
  }
}

void lim_max_y_interrupt(){
  // Checks if the debounce time has been met
  if ( (millis() - timestamp_last_activation) >= DEBOUNCE_TIME ) {
    move_1_stop_2_stop();
    // Serial.println("ESP: LIM MAX Y reached");
    // delay(1000);
    // move_to_initial_position();
    
    lim_y_max = 1;
    timestamp_last_activation = millis();
  }
}
