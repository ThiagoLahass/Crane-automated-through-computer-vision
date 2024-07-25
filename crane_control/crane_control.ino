#include <ESP32Servo.h>

#define DEBOUNCE_TIME 200 //ms
unsigned long timestamp_last_activation= 0;

// SETUP SERVO
#define SERVO_STOPPED_VALUE            90
#define SERVO_SPEED                    5
#define SERVO_PIN                      13
Servo servo;

// SETUP ELECTROMAGNET
#define ELECTROMAGNET_PIN              15

// SETUP INFRARED
#define INFRARED_PIN                   25

// OTHER SETUP VARIABLES
#define ELECTROMAGNET_DOWN_TIME        1900
#define ELECTROMAGNET_UP_TIME          7000
#define CONTAINER_POSITION_ERROR_RANGE 20
#define TIME_BRIDGE_CENTER_TO_CART     5000

// SETUP MOTORS
#define EN_PIN                2
#define IN1_PIN               5
#define IN2_PIN               17
#define IN3_PIN               23
#define IN4_PIN               22
#define MOTOR_MOV_SPEED_PIN   4
#define MOTOR_MOV_SPEED       120

// LIMIT SWITCH SENSORS
#define LIM_X_MIN_PIN  12
#define LIM_X_MAX_PIN  14
#define LIM_Y_MIN_PIN  27
#define LIM_Y_MAX_PIN  26

// CRANE TRANSLATION VARIABLES
int delta_x   = 0;
int delta_y   = 0;
int lim_x_min = 0;
int lim_x_max = 0;
int lim_y_min = 0;
int lim_y_max = 0;
int end_of_course = 0;
int infrared_value = 0;

///////////////////////////////////////////////////////////////////////////////////
////////////////////////////// FUNCTION HEADERS ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Sets the motor speed based on the input.
 * 
 * @param motor_mov_speed Speed value for the motor.
 */
void setSpeed(int motor_mov_speed);

/**
 * @brief Controls the movement of motors in different directions based on the input pins.
 * 
 * @param in1_ State for motor IN1 pin.
 * @param in2_ State for motor IN2 pin.
 * @param in3_ State for motor IN3 pin.
 * @param in4_ State for motor IN4 pin.
 * @param motor_mov_speed Speed value for the motors.
 */
void move(int in1_, int in2_, int in3_, int in4_, int motor_mov_speed);

/**
 * @brief Moves both motors forward.
 * 
 * @param motor_mov_speed Speed value for the motors.
 */
void move_1_forward_2_forward(int motor_mov_speed);

/**
 * @brief Moves motor 1 forward and stops motor 2.
 * 
 * @param motor_mov_speed Speed value for the motors.
 */
void move_1_forward_2_stop(int motor_mov_speed);

/**
 * @brief Moves motor 1 forward and motor 2 backward.
 * 
 * @param motor_mov_speed Speed value for the motors.
 */
void move_1_forward_2_backward(int motor_mov_speed);

/**
 * @brief Stops motor 1 and moves motor 2 forward.
 * 
 * @param motor_mov_speed Speed value for the motors.
 */
void move_1_stop_2_forward(int motor_mov_speed);

/**
 * @brief Stops both motors.
 * 
 * @param motor_mov_speed Speed value for the motors.
 */
void move_1_stop_2_stop(int motor_mov_speed);

/**
 * @brief Stops motor 1 and moves motor 2 backward.
 * 
 * @param motor_mov_speed Speed value for the motors.
 */
void move_1_stop_2_backward(int motor_mov_speed);

/**
 * @brief Moves motor 1 backward and motor 2 forward.
 * 
 * @param motor_mov_speed Speed value for the motors.
 */
void move_1_backward_2_forward(int motor_mov_speed);

/**
 * @brief Stops motor 1 and moves motor 2 backward.
 * 
 * @param motor_mov_speed Speed value for the motors.
 */
void move_1_backward_2_stop(int motor_mov_speed);

/**
 * @brief Moves both motors backward.
 * 
 * @param motor_mov_speed Speed value for the motors.
 */
void move_1_backward_2_backward(int motor_mov_speed);

/**
 * @brief Moves the system to the initial position (0, 0) using the limit switch sensors.
 * 
 * @param motor_mov_speed Speed value for the motors.
 */
void move_to_initial_position(int motor_mov_speed);

/**
 * @brief Moves the system to the central position after reaching the initial position.
 * 
 * @param motor_mov_speed Speed value for the motors.
 */
void move_to_central_position(int motor_mov_speed);

/**
 * @brief Lifts the load using a servo motor and an electromagnet.
 */
void lift_load();

/**
 * @brief Lowers the load by releasing it from the electromagnet.
 */
void lower_load();

/**
 * @brief Sets up the servo motor by rotating it counterclockwise until a certain condition is met, then clockwise for a fixed duration, and finally stops it.
 * 
 * This function operates the servo motor based on the readings from an infrared sensor. 
 * The servo motor rotates counterclockwise initially, stops when the infrared sensor 
 * detects a value above zero for three consecutive readings, then rotates clockwise 
 * for 10.5 seconds, and finally stops.
 */
void setup_servo();

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
  Serial.begin(115200);
  Serial.setTimeout(100);   // Set timeout to 100 milliseconds
  servo.attach(SERVO_PIN);
  
  pinMode(EN_PIN, OUTPUT);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(IN3_PIN, OUTPUT);
  pinMode(IN4_PIN, OUTPUT);
  
  pinMode(MOTOR_MOV_SPEED_PIN, INPUT);
  
  pinMode(ELECTROMAGNET_PIN, OUTPUT);

  pinMode(INFRARED_PIN, INPUT);
  
  // SET LIMIT SWITCH SENSORS AS INTERRUPT PINS
  pinMode(LIM_X_MIN_PIN, INPUT);
  pinMode(LIM_X_MAX_PIN, INPUT);
  pinMode(LIM_Y_MIN_PIN, INPUT);
  pinMode(LIM_Y_MAX_PIN, INPUT);

  // Interrupt configuration
  attachInterrupt(LIM_X_MIN_PIN, lim_min_x_interrupt, RISING);
  attachInterrupt(LIM_X_MAX_PIN, lim_max_x_interrupt, RISING);
  attachInterrupt(LIM_Y_MIN_PIN, lim_min_y_interrupt, RISING);
  attachInterrupt(LIM_Y_MAX_PIN, lim_max_x_interrupt, RISING);
  
  // SETUP STATE
  digitalWrite(ELECTROMAGNET_PIN, LOW);

  // ADJUST SERVO INITIAL POSITION
  Serial.print("ESP: Setuping servo");
  setup_servo();

  // MOVE THE CAR TO THE INITIAL POSITION (0, 0)
  // THEN MOVE TO THE CENTER OF THE BRIDGE TO HAVE A HOLISCT VIEW OF THE CONTAINERS
  move_to_central_position(MOTOR_MOV_SPEED);
  delay(500);
  Serial.print("ESP: Cpr"); // Central position reached!

  delay(1000);
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// MAIN LOOP ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
void loop(){

  // RESET CRANE TRANSLATION VARIABLES
  delta_x = 0;
  delta_y = 0;
  lim_x_min = 0;
  lim_x_max = 0;
  lim_y_min = 0;
  lim_y_max = 0;
  end_of_course = 0;
  
  ///////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////// INITIAL STATE ////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////
  // -> WAITING FOR COMMAND 'begin' FROM ESP TO START SEARCHING FOR CONTAINERS

  int command_identified = 0;
  int start_command      = 0;
  int direction_go_back  = 0; // 1 - LEFT; 2 RIGHT
  
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
  // -> 'begin' COMMAND RECEIVED FROM ESP, START SEARCHING CONTAINER BASED ON DELTAS
  // OF CONTAINER CENTER POSITION TO CAMERA CENTER POSITION
  /* THE DATA RECEIVED HERE FROM ESP MUST BE IN THE FORMAT:
      "sxxx zyyy",
      WHERE:
      s is the sign of delta_x (+ or -);
      xxx are the 3 characters of delta_x;
      z is the sign of delta_y (+ or -);
      yyy are the 3 characters of delta_y.
    */
  command_identified = 0;
  int container_centralized = 0;
  str = "";
  
  Serial.println("ESP: Receiving container position...");
  
  unsigned long start_time_x    = 0;           // Save the start time of x centralization
  unsigned long start_time_y    = 0;           // Save the start time of y centralization
  unsigned long elapsed_time_x  = 0;         // Variable to store the time taken for x centralization
  unsigned long elapsed_time_y  = 0;         // Variable to store the time taken for y centralization
  
  while(!container_centralized){

    // CHECKING IF ANY OF THE END OF COURSE SENSORS HAVE BEEN ACTIVATED
    if(lim_x_min == 1 || lim_x_max == 1 || lim_y_min == 1 || lim_y_max == 1){
      // We could make so:
      // Serial.print("ESP: End of course activated, going to the center position");
      // But because it is an interruption, putting a small string reduces the chances of sending only part of the string
      Serial.print("ESP: EoC1");
      delay(500);

      lim_x_min = 0;
      lim_x_max = 0;
      lim_y_min = 0;
      lim_y_max = 0;

      // GOING BACK TO THE CENTRAL POSITION
      if(elapsed_time_x == 0){
        elapsed_time_x = millis() - start_time_x;
      }
      if(elapsed_time_y == 0){
        elapsed_time_y = millis() - start_time_y;
      }
      
      if(direction_go_back == 1){     // 1 == LEFT
        move_1_forward_2_backward(MOTOR_MOV_SPEED);
        if(elapsed_time_x <= elapsed_time_y){
          delay(elapsed_time_x);
          move_1_stop_2_backward(MOTOR_MOV_SPEED);
          delay(elapsed_time_y - elapsed_time_x);
        }
        else{
          delay(elapsed_time_y);
          move_1_forward_2_stop(MOTOR_MOV_SPEED);
          delay(elapsed_time_x - elapsed_time_y);
        }
      }
      else{                           // 2 == RIGHT
        move_1_backward_2_backward(MOTOR_MOV_SPEED);
        if(elapsed_time_x <= elapsed_time_y){
          delay(elapsed_time_x);
          move_1_stop_2_backward(MOTOR_MOV_SPEED);
          delay(elapsed_time_y - elapsed_time_x);
        }
        else{
          delay(elapsed_time_y);
          move_1_backward_2_stop(MOTOR_MOV_SPEED);
          delay(elapsed_time_x - elapsed_time_y);
        }
      }
      
      move_1_stop_2_stop(MOTOR_MOV_SPEED);

      Serial.print("ESP: EoC2");
      delay(1000);

      end_of_course = 1;
      
      break;
    }

    if (Serial.available() > 0) {          // Check if data is available on the serial
      str = Serial.readStringUntil('\n');  // Read the received data as a string until newline
      str.trim();                          // Remove whitespace from the start and end of the string
      if(!str.equals("")){
        command_identified = 1;
      }
    }
      
    if(command_identified){
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

      if(start_time_x == 0 && start_time_y == 0){
        start_time_x = millis();
        start_time_y = millis();
      }

      if( delta_x == 365 && delta_y == 240){                     // IN THIS CASE, THERE ARE NO OBJECT VISIBLE ON THE CAMERA
        move_1_stop_2_stop(MOTOR_MOV_SPEED);
        if(millis() - start_time_x > 5000){
          Serial.println("ESP: Cc");
          container_centralized = 1;
          end_of_course = 1;
          delay(500);
          Serial.print("ESP: end");
          delay(500);
        }
      }
      else if(delta_x > CONTAINER_POSITION_ERROR_RANGE){         // CAMERA IS TO THE LEFT OF THE CONTAINER CENTER
        if(direction_go_back == 0){
          direction_go_back = 1;                                 // TO GO BACK TO THE CENTER POSITION, WE NEED TO GO TO RIGHT AFTER GET THE CONTAINER
        }

        if(delta_y > CONTAINER_POSITION_ERROR_RANGE){            // CAMERA IS BELOW THE CONTAINER CENTER
          move_1_backward_2_forward(MOTOR_MOV_SPEED);
        }
        else if(delta_y < -CONTAINER_POSITION_ERROR_RANGE){      // CAMERA IS ABOVE THE CONTAINER CENTER
          move_1_backward_2_backward(MOTOR_MOV_SPEED);
        }
        else{                                                    // CAMERA IS CENTERED VERTICALLY ON THE CONTAINER
          if(elapsed_time_y == 0){
            elapsed_time_y = millis() - start_time_y;
          }
          move_1_backward_2_stop(MOTOR_MOV_SPEED);
        }
      }
      else if(delta_x < -CONTAINER_POSITION_ERROR_RANGE){        // CAMERA IS TO THE RIGHT OF THE CONTAINER CENTER
        if(direction_go_back == 0){
          direction_go_back = 2;                                 // TO GO BACK TO THE CENTER POSITION, WE NEED TO GO TO RIGHT AFTER GET THE CONTAINER
        }
        
        if(delta_y > CONTAINER_POSITION_ERROR_RANGE){            // CAMERA IS BELOW THE CONTAINER CENTER
          move_1_forward_2_forward(MOTOR_MOV_SPEED);
        }
        else if(delta_y < -CONTAINER_POSITION_ERROR_RANGE){      // CAMERA IS ABOVE THE CONTAINER CENTER
          move_1_forward_2_backward(MOTOR_MOV_SPEED);
        }
        else{                                                    // CAMERA IS CENTERED VERTICALLY ON THE CONTAINER
          if(elapsed_time_y == 0){
            elapsed_time_y = millis() - start_time_y;
          }
          move_1_forward_2_stop(MOTOR_MOV_SPEED);
        }
      }
      else{                                                      // CAMERA IS CENTERED HORIZONTALLY ON THE CONTAINER
        if(elapsed_time_x == 0){
          elapsed_time_x = millis() - start_time_x;
        }
        
        if(delta_y > CONTAINER_POSITION_ERROR_RANGE){            // CAMERA IS BELOW THE CONTAINER CENTER
          move_1_stop_2_forward(MOTOR_MOV_SPEED);      
        }
        else if(delta_y < -CONTAINER_POSITION_ERROR_RANGE){      // CAMERA IS ABOVE THE CONTAINER CENTER
          move_1_stop_2_backward(MOTOR_MOV_SPEED);
        }
        else{                                                    // CAMERA IS CENTERED IN BOTH DIRECTIONS !!!
          if(elapsed_time_y == 0){
            elapsed_time_y = millis() - start_time_y;
          }
          
          move_1_stop_2_stop(MOTOR_MOV_SPEED);
          
          Serial.print("ESP: Cc");
          container_centralized = 1;
        }
      }
       
      // Reset variables 
      str = "";
      command_identified = 0;
    }
    
    delay(50);
  }

  container_centralized = 0;

  // IF END OF COURSE IS ACTIVATED, WE MUST RESTART THE MAIN LOOP
  if(end_of_course == 1){
    end_of_course = 0;
  }
  else{
    // DELAY TO ALLOW ESP TO RECEIVE INFORMATION FROM THE SERIAL THAT THE CONTAINER IS CENTRALIZED
    delay(2000);
    
    ///////////////////////////////////////////////////////////////////////////////////
    //////////////////////////// PICKING UP CONTAINER STATE ///////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////

    Serial.print("ESP: Picking up container...");
    lift_load();
    delay(1000);
    
    ///////////////////////////////////////////////////////////////////////////////////
    /////////////////// LOADING CONTAINER TO THE AUTONOMOUS CART STATE ////////////////
    ///////////////////////////////////////////////////////////////////////////////////

    Serial.print("ESP: Elapsed time x: ");
    Serial.println(elapsed_time_x);
    Serial.print("ESP: Elapsed time y: ");
    Serial.println(elapsed_time_y);
    
    Serial.println("ESP: Transporting container to the cart...");

    if(direction_go_back == 1){     // 1 == LEFT
      move_1_forward_2_backward(MOTOR_MOV_SPEED + 2);
      delay(elapsed_time_x);

      while(!lim_y_min){            // WHILE THE CONTAINER ISNT ON THE INITIAL POSITION OF Y AXIS
        move_1_stop_2_backward(MOTOR_MOV_SPEED);
      }
      lim_y_min = 0;
    }
    else{                           // 2 == RIGHT
      move_1_backward_2_backward(MOTOR_MOV_SPEED - 3);
      delay(elapsed_time_x);

      while(!lim_y_min){            // WHILE THE CONTAINER ISNT ON THE INITIAL POSITION OF Y AXIS
        move_1_stop_2_backward(MOTOR_MOV_SPEED);
      }
      lim_y_min = 0;
    }

    move_1_stop_2_stop(MOTOR_MOV_SPEED);
    
    Serial.println("ESP: Container centered above the cart");
    
    ///////////////////////////////////////////////////////////////////////////////////
    ///////////// PLACING THE CONTAINER ON THE AUTONOMOUS CART STATE //////////////////
    ///////////////////////////////////////////////////////////////////////////////////
    
    Serial.println("ESP: Placing container on the cart...");
    lower_load();
    Serial.println("ESP: Container placed on the cart");
    
    ///////////////////////////////////////////////////////////////////////////////////
    //////////// MOVING THE BRIDGE BACK TO THE CENTRAL POSITION STATE /////////////////
    ///////////////////////////////////////////////////////////////////////////////////
    
    Serial.println("ESP: Moving the car back to the central position");

    int count = 0;
    while(count < TIME_BRIDGE_CENTER_TO_CART){
      move_1_stop_2_forward(MOTOR_MOV_SPEED);
      delay(100);
      count += 100;
    }
    move_1_stop_2_stop(MOTOR_MOV_SPEED);

    Serial.print("ESP: end");

    // DELAY TO ALLOW THE BACKEND TO RECEIVE THE COMMAND INDICATING THE CYCLE IS COMPLETE
    delay(2000);
    
    ///////////////////////////////////////////////////////////////////////////////////
    //////////////////////////// END OF THE CYCLE STATE ///////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////
  }
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////// FUNCTION DEFINITIONS ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
void setSpeed(int motor_mov_speed){
  analogWrite(EN_PIN, motor_mov_speed);
}

void move(int in1_, int in2_, int in3_, int in4_, int motor_mov_speed){
  setSpeed(motor_mov_speed);
  digitalWrite(IN1_PIN, in1_);
  digitalWrite(IN2_PIN, in2_);
  digitalWrite(IN3_PIN, in3_);
  digitalWrite(IN4_PIN, in4_);
}

void move_1_forward_2_forward(int motor_mov_speed){
  move(LOW, HIGH, LOW, HIGH, motor_mov_speed);
}

void move_1_forward_2_stop(int motor_mov_speed){
  move(LOW, HIGH, LOW, LOW, motor_mov_speed);
}

void move_1_forward_2_backward(int motor_mov_speed){
  move(LOW, HIGH, HIGH, LOW, motor_mov_speed);
}

void move_1_stop_2_forward(int motor_mov_speed){
  move(LOW, LOW, LOW, HIGH, motor_mov_speed);
}

void move_1_stop_2_stop(int motor_mov_speed){
  move(LOW, LOW, LOW, LOW, motor_mov_speed);
}

void move_1_stop_2_backward(int motor_mov_speed){
  move(LOW, LOW, HIGH, LOW, motor_mov_speed);
}

void move_1_backward_2_forward(int motor_mov_speed){
  move(HIGH, LOW, LOW, HIGH, motor_mov_speed);
}

void move_1_backward_2_stop(int motor_mov_speed){
  move(HIGH, LOW, LOW, LOW, motor_mov_speed);
}

void move_1_backward_2_backward(int motor_mov_speed){
  move(HIGH, LOW, HIGH, LOW, motor_mov_speed);
}

void move_to_initial_position(int motor_mov_speed){
  bool initial_position_x_reached = false;
  bool initial_position_y_reached = false;
  
  Serial.println("ESP: Moving the car to the initial position (0,0)...");
  move_1_backward_2_backward(motor_mov_speed);
  
  while(1){
    
    // IF BOTH SENSORS ARE ACTIVATED, WE ARE IN THE
    // INITIAL POSITION x = 0 and y = 0
    // BOTH MOTORS MUST BE DEACTIVATED
    // AND WE EXIT THIS LOOP
    if(initial_position_x_reached && initial_position_y_reached){
      Serial.println("ESP: Initial position reached");
      move_1_stop_2_stop(motor_mov_speed);
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
      move_1_stop_2_backward(motor_mov_speed);
      Serial.println("ESP: Initial x position reached");
    }
    // IF ONLY THE Y MINIMUM LIMIT SENSOR IS ACTIVATED
    // ONLY THE Y TRANSLATION MOTOR SHOULD BE DEACTIVATED
    if(!initial_position_y_reached && lim_y_min == 1){
      initial_position_y_reached = true;
      move_1_backward_2_stop(motor_mov_speed);
      Serial.println("ESP: Initial y position reached");
    }
  }
}

void move_to_central_position(int motor_mov_speed){
  move_to_initial_position(motor_mov_speed);
  
  move_1_forward_2_forward(motor_mov_speed);
  lim_x_min = 0;
  lim_x_max = 0;
  lim_y_min = 0;
  lim_y_max = 0;

  Serial.println("ESP: Moving the car to the central position...");

  delay(2600);
  move_1_stop_2_forward(motor_mov_speed);

  delay(2500);
  move_1_stop_2_stop(motor_mov_speed);
  
  Serial.println("ESP: Central position reached!");
}

void lift_load(){
  servo.write(SERVO_STOPPED_VALUE - SERVO_SPEED); // counterclockwise

  // while loop to lower the electromagnet while it is not on top of the container
  int stop_flag = 0;
  int count = 0;
  while( !stop_flag ){
    infrared_value = analogRead(INFRARED_PIN);    // Read the value of infrared sensor
    if (infrared_value > 0){
      count++;
    }
    else{
      count = 0;
    }
    if( count >= 1){
      stop_flag = 1;
    }

    delay(10);
  }

  delay(300);

  servo.write(SERVO_STOPPED_VALUE);
  delay(500);
  digitalWrite(ELECTROMAGNET_PIN, HIGH);
  delay(500);
  servo.write(SERVO_STOPPED_VALUE + SERVO_SPEED); // clockwise
  delay(ELECTROMAGNET_UP_TIME);
  servo.write(SERVO_STOPPED_VALUE);
}

void lower_load(){
  servo.write(SERVO_STOPPED_VALUE - SERVO_SPEED); // counterclockwise
  delay(ELECTROMAGNET_DOWN_TIME);
  servo.write(SERVO_STOPPED_VALUE);
  delay(500);
  digitalWrite(ELECTROMAGNET_PIN, LOW);
  delay(500);
  servo.write(SERVO_STOPPED_VALUE + SERVO_SPEED); // clockwise
  delay(ELECTROMAGNET_UP_TIME + 800);
  servo.write(SERVO_STOPPED_VALUE);
}

void setup_servo(){
  servo.write(SERVO_STOPPED_VALUE - SERVO_SPEED); // counterclockwise
  int stop_flag = 0;
  int count = 0;
  while( !stop_flag ){
    infrared_value = analogRead(INFRARED_PIN);    // Read the value of the infrared sensor
    if (infrared_value > 0){
      count++;
    }
    else{
      count = 0;
    }
    if( count >= 1){
      stop_flag = 1;
    }

    delay(50);
  }

  servo.write(SERVO_STOPPED_VALUE);
  delay(500);

  servo.write(SERVO_STOPPED_VALUE + SERVO_SPEED);   // clockwise
  delay(10500);

  servo.write(SERVO_STOPPED_VALUE);
}

// INTERRUPT FUNCTION DEFINITIONS
void lim_min_x_interrupt(){
  // Checks if the debounce time has been met
  if ( (millis() - timestamp_last_activation) >= DEBOUNCE_TIME ) {
    move_1_stop_2_stop(MOTOR_MOV_SPEED);
    lim_x_min = 1;
    timestamp_last_activation = millis();
  }
}

void lim_max_x_interrupt(){
  // Checks if the debounce time has been met
  if ( (millis() - timestamp_last_activation) >= DEBOUNCE_TIME ) {
    move_1_stop_2_stop(MOTOR_MOV_SPEED);
    lim_x_max = 1;
    timestamp_last_activation = millis();
  }
}

void lim_min_y_interrupt(){
  // Checks if the debounce time has been met
  if ( (millis() - timestamp_last_activation) >= DEBOUNCE_TIME ) {
    move_1_stop_2_stop(MOTOR_MOV_SPEED);
    lim_y_min = 1;
    timestamp_last_activation = millis();
  }
}

void lim_max_y_interrupt(){
  // Checks if the debounce time has been met
  if ( (millis() - timestamp_last_activation) >= DEBOUNCE_TIME ) {
    move_1_stop_2_stop(MOTOR_MOV_SPEED);
    lim_y_max = 1;
    timestamp_last_activation = millis();
  }
}