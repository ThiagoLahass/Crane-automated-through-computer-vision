#include <Servo.h>

#define VALOR_PWM_SERVO360_PARADO 93
#define VELOCIDADE_SERVO 5
#define SERVO A0
Servo servo;

int EN_A = 5;
int IN1 = 7;
int IN2 = 6;

int EN_B = 3;
int IN3 = 4;
int IN4 = 2;

int ELETROIMA = 8;

int VELOCIDADE_MOT_MOV = A1;
int MOVIMENTO_MOT_1 = A2;
int MOVIMENTO_MOT_2 = A3;
int userInput1 = NULL;
int userInput2 = NULL;

void speed();
void move(int in1_, int in2_, int in3_, int in4_);
void move_1_foward_2_forward();
void move_1_foward_2_stop();
void move_1_foward_2_backward();
void move_1_stop_2_forward();
void move_1_stop_2_stop();
void move_1_stop_2_backward();
void move_1_backward_2_forward();
void move_1_backward_2_stop();
void move_1_backward_2_backward();
void pegar_carga_e_icar();
  
void setup(){
  Serial.begin(9600);
  servo.attach(SERVO);
  
  pinMode(LED_BUILTIN, OUTPUT);
  
  pinMode(EN_A, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  
  pinMode(EN_B,OUTPUT);
  pinMode(IN3,OUTPUT);
  pinMode(IN4,OUTPUT);
  
  pinMode(VELOCIDADE_MOT_MOV,INPUT);
  pinMode(MOVIMENTO_MOT_1,INPUT);
  pinMode(MOVIMENTO_MOT_2,INPUT);
  
  pinMode(ELETROIMA, OUTPUT);
}

void loop(){
  speed();
  userInput1 = analogRead(MOVIMENTO_MOT_1);
  userInput2 = analogRead(MOVIMENTO_MOT_2);
  if(userInput1 < 400){
    if(userInput2 < 400){
      move_1_foward_2_forward();
    }
    else if(userInput2 < 800){
      move_1_foward_2_stop();
    }
    else{
      move_1_foward_2_backward();
    }
  }
  else if(userInput1 < 800){
    if(userInput2 < 400){
      move_1_stop_2_forward();
    }
    else if(userInput2 < 800){
      move_1_stop_2_stop();
      pegar_carga_e_icar();
    }
    else{
      move_1_stop_2_backward();
    }
  }
  else{
    if(userInput2 < 400){
      move_1_backward_2_forward();
    }
    else if(userInput2 < 800){
      move_1_backward_2_stop();
    }
    else{
      move_1_backward_2_backward();
    }
  }
  
  while( userInput1 > 400 && userInput1 < 800 && userInput2 > 400 && userInput2 < 800){
    userInput1 = analogRead(MOVIMENTO_MOT_1);
  	userInput2 = analogRead(MOVIMENTO_MOT_2);
    delay(100);
  }
  
  delay(100);
}
    
void pegar_carga_e_icar(){
  servo.write(VALOR_PWM_SERVO360_PARADO - VELOCIDADE_SERVO); // sentido horário
  delay(3000);
  servo.write(VALOR_PWM_SERVO360_PARADO);
  digitalWrite(ELETROIMA, HIGH);
  delay(2000);
  digitalWrite(ELETROIMA, LOW);
  servo.write(VALOR_PWM_SERVO360_PARADO + VELOCIDADE_SERVO); // sentido anti-horário
  delay(3000);
  servo.write(VALOR_PWM_SERVO360_PARADO);
}

void speed(){
 int aux = analogRead(VELOCIDADE_MOT_MOV);
 aux = map(aux, 0, 1023, 0 , 255);
 analogWrite(EN_A, aux);
 analogWrite(EN_B, aux);
}

void move(int in1_, int in2_, int in3_, int in4_) {
 digitalWrite(IN1, in1_);
 digitalWrite(IN2, in2_);
 
 digitalWrite(IN3, in3_);
 digitalWrite(IN4, in4_);
}

void move_1_foward_2_forward(){
  move(1,0,1,0);
}
void move_1_foward_2_stop(){
  move(1,0,0,0);
}
void move_1_foward_2_backward(){
  move(1,0,0,1);
}
void move_1_stop_2_forward(){
  move(0,0,1,0);
}
void move_1_stop_2_stop(){
  move(0,0,0,0);
}
void move_1_stop_2_backward(){
  move(0,0,0,1);
}
void move_1_backward_2_forward(){
  move(0,1,1,0);
}
void move_1_backward_2_stop(){
  move(0,1,0,0);
}
void move_1_backward_2_backward(){
  move(0,1,0,1);
}