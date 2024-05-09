// Communication buffers lengths
#define MAX_BUFF_LEN        255 
#define CMD_BUFF_LEN        6

// LED configs
#define LED_PIN             2
#define DEFAULT_DELAY       1000

// Globals
char c; // IN char
char str[CMD_BUFF_LEN];
uint8_t idx = 0; // Reading index

int delta_x = DEFAULT_DELAY; // Default blinking delay
int delta_y = 0;
unsigned long prev_time;

void toggle_led();

void setup() {
  // Config serial port
  Serial.begin(9600);

  // Config LED
  pinMode(LED_PIN, OUTPUT);

  // 
  prev_time = millis();
}

void loop() {
  
  // Parse incoming command
  if(Serial.available() > 0){ // There's a command
    
    c = Serial.read(); // Read one byte
    
    if(c != '\n'){ // Still reading
      str[idx++] = c; // Parse the string byte (char) by byte
    }
    else{ // Done reading
      str[idx] = '\0'; // Convert it to a string
      
      // strtol(char*, Ref pointer, Base[Decimal-->10, Hex-->16, ...])
      delta_x = strtol(str, NULL, 10); // str+1 --> exclude the first char
      delta_y = strtol(str+2, NULL, 10);
      /* Some input checking could've been done here (like b15f2 --> invalid) */

      // Serial.println(str);

      if((delta_x < 20 && delta_x > -20) && (delta_y < 20 && delta_y > -20)){
        // Serial.println("ON");
        digitalWrite(LED_PIN, LOW);
      }
      else{
        // Serial.println("OFF");
        digitalWrite(LED_PIN, HIGH);
      }

      // Reset reading index 
      idx = 0;
    }
  }

  // if(millis() - prev_time > delay_t){
  //   toggle_led();
  //   prev_time = millis();
  // }
}

void toggle_led(){
  // Toggle the state of the LED pin (write the NOT of the current state to the LED pin)
  digitalWrite(LED_PIN, !digitalRead(LED_PIN));
}
