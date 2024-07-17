
//Pinagem do motor_A na ponte H
int IN1 = 5 ;
int IN2 = 4 ;
int velocidadeA = 9;

//Pinagem do motor_B na ponte H
int IN3 = 3 ;
int IN4 = 2 ;
int velocidadeB = 10;

//Pinagem dos Sensores Reflexiveis
int sensor_direita = A0;
int sensor_centro = A1;
int sensor_esquerda = A2;

//Pinagem do Sensor Ldr
int ldr = A3;

//Variávies auxiliares 
float kp = 0.05;
float velocidadeBase = 75;
int flag = 0;


//Inicialização dos Pinos
void setup(){
  
  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);
  pinMode(IN3,OUTPUT);
  pinMode(IN4,OUTPUT);
  pinMode(velocidadeA,OUTPUT);
  pinMode(velocidadeB,OUTPUT);

  pinMode(sensor_direita, INPUT); 
  pinMode(sensor_centro, INPUT); 
  pinMode(sensor_esquerda, INPUT);

  pinMode(ldr, INPUT);
}

void loop(){

  //Leitura dos Sensores 
  float sensor_ldr;
  
  int direita_ = digitalRead(sensor_direita);
  int centro_ = digitalRead(sensor_centro);
  int esquerda_ = digitalRead(sensor_esquerda);

  while (direita_ == 1 && centro_ == 1 && esquerda_ == 1){
    parar_carro();
    sensor_ldr = analogRead(ldr);
    return_flag( sensor_ldr);

    delay(3000);
    
    if (flag == 1){
      delay(300);
      while(direita_ == 1 && centro_ == 1 && esquerda_ == 1){
        direita_ = digitalRead(sensor_direita);
        centro_ = digitalRead(sensor_centro);
        esquerda_ = digitalRead(sensor_esquerda);
        andar_carro(velocidadeA,  85, velocidadeB, 85 );
      }

      break;
    }
  }

  while (1){
    //Leitura analogica dos sensores 
    float direita = analogRead(sensor_direita);
    float centro = analogRead(sensor_centro);
    float esquerda = analogRead(sensor_esquerda);
    
    //Calculando controle Proporcional
    float erro = (direita - esquerda);
    float ajuste = erro * kp;
    float velocidadeEsquerda = velocidadeBase - ajuste;
    float velocidadeDireita = velocidadeBase + ajuste;

    //Leitura digital dos sensores 
    direita_ = digitalRead(sensor_direita);
    centro_ = digitalRead(sensor_centro);
    esquerda_ = digitalRead(sensor_esquerda);
    
    andar_carro(velocidadeA,velocidadeEsquerda,velocidadeB,velocidadeDireita);

    if (direita_ == 1 && centro_ == 1 && esquerda_ == 1){
      flag = 0;
      break;
    }
  }

 delay(50);

}


int return_flag(float sensor_ldr){
 if (sensor_ldr > 700){
   flag = 1;
   return 1;
 }

 flag = 0;
 return 0;
}

void parar_carro(){
  digitalWrite(IN1,LOW);
  digitalWrite(IN2,LOW);
  digitalWrite(IN3,LOW);
  digitalWrite(IN4,LOW);
}

void andar_carro(float velocidadeA, float velocidadeEsquerda, float velocidadeB, float velocidadeDireita ){
  digitalWrite(IN1,HIGH);
  digitalWrite(IN2,LOW);
  digitalWrite(IN3,LOW);
  digitalWrite(IN4,HIGH);
  analogWrite(velocidadeA,velocidadeEsquerda);
  analogWrite(velocidadeB,velocidadeDireita);
}