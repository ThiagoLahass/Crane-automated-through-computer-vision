#include <Servo.h>

#define VALOR_PWM_SERVO360_PARADO 93
#define VELOCIDADE_SERVO 5
#define TEMPO_ABAIXAR_ELETROIMA 3000
#define RANGE_ERRO_PERMITIDO_POSICAO_CONTAINER 20
#define TEMPO_CENTRO_PONTE_ATE_CARRINHO 5000
#define SERVO A0
Servo servo;

int EN_A = 5;
int IN1 = 7;
int IN2 = 6;

int EN_B = 3;
int IN3 = 4;
int IN4 = 2;

int ELETROIMA = 8;

int LIM_X_MIN = 12;
int LIM_X_MAX = 11;
int LIM_Y_MIN = 10;
int LIM_Y_MAX = 9;

int lim_x_min = 0;
int lim_x_max = 0;
int lim_y_min = 0;
int lim_y_max = 0;

int VELOCIDADE_MOT_MOV = A1;
int MOVIMENTO_MOT_1 = A2;
int MOVIMENTO_MOT_2 = A3;
int userInput1 = NULL;
int userInput2 = NULL;

//////////////////////////////////////////////////////
int ENABLE_CONTAINER_READ = 13;

// Communication buffers lengths
#define MAX_BUFF_LEN        255 
#define CMD_BUFF_LEN        6

// Globals
char c; 					// IN char
char str[CMD_BUFF_LEN];
uint8_t idx = 0; 			// Reading index

int delta_x = 0;
int delta_y = 0;
unsigned long prev_time;

//////////////////////////////////////////////////////////

void speed();
void move(int in1_, int in2_, int in3_, int in4_);
void move_1_forward_2_forward();
void move_1_forward_2_stop();
void move_1_forward_2_backward();
void move_1_stop_2_forward();
void move_1_stop_2_stop();
void move_1_stop_2_backward();
void move_1_backward_2_forward();
void move_1_backward_2_stop();
void move_1_backward_2_backward();
void move_to_initial_position();
void move_to_central_position();

void pegar_carga_e_icar();
void abaixar_carga_e_soltar();

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
  
  pinMode(LIM_X_MIN,INPUT);
  pinMode(LIM_X_MAX,INPUT);
  pinMode(LIM_Y_MIN,INPUT);
  pinMode(LIM_Y_MAX,INPUT);
  
  pinMode(ENABLE_CONTAINER_READ,INPUT);
  
  //move_to_initial_position();
  move_to_central_position();
  
  delay(1000);
}

void loop(){
  
  // ESTADO INICIAL
  // AGUARDANDO COMANDO PARA INICIAR BUSCA DE CONTAINER
  int comand_identified = 0;
  int start_comand = 0;
  String str = "";
  
  Serial.println("Aguardando comando pela Serial para iniciar... (begin)");
  
  while(!start_comand){
   	while (Serial.available() > 0) { 	// Enquanto houver dados disponíveis na serial
      char c = Serial.read(); 			// Lê um caractere da serial
      str += c; 						// Adiciona o caractere à string
      delay(10); 						// Pequena pausa para garantir que todos os dados sejam lidos
      comand_identified = 1;
    }
      
    if(comand_identified){
      Serial.println(str);

      // Verifica se a string recebida é igual a "begin"
      if (str.equals("begin")) {
        // Ação a ser realizada quando a string recebida for igual a "begin"
        Serial.println("Comando de inicio recebido!");
        start_comand = 1;
      } else {
        // Ação a ser realizada quando a string recebida não for igual a "begin"
        Serial.println("Comando invalido!");
        str = "";
      }

      // Reset reading index 
      idx = 0;
    }

    delay(5);
    comand_identified = 0;
  }
  
  delay(500);
  
  ///////////////////////////////////////////////////
  
  // ESTADO DA BUSCA DO CONTAINER SELECIONADO
  // AS INFORMAÇÕES RECEBIDAS AQUI DEVEM SER NO FORMATO
  
  //MUDAR PARA 4 DIGITOS CADA POR CAUSA DO SINAL NEGATIVO
  
  // "xxxyyy", ONDE OS 3 PRIMEIROS DIGITOS REPRESENTAM O
  // DELTA_X RECEBIDO, E OS 3 ULTIMOS O DELTA_Y
  int container_centralized = 0;
  str = "";
  
  Serial.println("Aguardando posicao do container...");
  
  unsigned long tempo_inicio_x = millis(); 	// Salva o tempo de início de centralização em x
  unsigned long tempo_inicio_y = millis(); 	// Salva o tempo de início de centralização em y
  unsigned long tempo_decorrido_x = 0; 		// Variável para armazenar o tempo decorrido de centralização em x
  unsigned long tempo_decorrido_y = 0; 		// Variável para armazenar o tempo decorrido de centralização em y

  
  while(!container_centralized){
    
    // SETAR VELOCIDADE DOS MOTORES DE TRANSLAÇÃO
    speed();

   	while (Serial.available() > 0) { 	// Enquanto houver dados disponíveis na serial
      char c = Serial.read(); 			// Lê um caractere da serial
      str += c; 						// Adiciona o caractere à string
      delay(10); 						// Pequena pausa para garantir que todos os dados sejam lidos
      comand_identified = 1;
    }
      
    if(comand_identified){
      Serial.println(str);
      
      String str_delta_x = str.substring(0, 4); // Extrai os primeiros 3 caracteres (delta_x)
      String str_delta_y = str.substring(4, 8); // Extrai os próximos 3 caracteres (delta_y)

      // Converte as strings para números inteiros
      delta_x = str_delta_x.toInt();
      delta_y = str_delta_y.toInt();
      
      Serial.print("delta_x: ");
      Serial.println(delta_x);
      Serial.print("delta_y: ");
      Serial.println(delta_y);
      
      // CÂMERA ESTÁ À ESQUERDA DO CENTRO DO CONTAINER
      if(delta_x > RANGE_ERRO_PERMITIDO_POSICAO_CONTAINER){
        
        // CÂMERA ESTÁ ABAIXO DO CENTRO DO CONTAINER
        if(delta_y > RANGE_ERRO_PERMITIDO_POSICAO_CONTAINER){
          
			move_1_forward_2_forward();
          
        } // CÂMERA ESTÁ ACIMA DO CENTRO DO CONTAINER
        else if(delta_y < - RANGE_ERRO_PERMITIDO_POSICAO_CONTAINER){

          move_1_forward_2_backward();
          
        } // CÂMERA ESTÁ CENTRALIZADA NO CONTAINER NO SENTIDO VERTICAL
        else{
          Serial.println("Centralizado em y");
          if(tempo_decorrido_y == 0){
            tempo_decorrido_y = millis() - tempo_inicio_y;
          }
          
          move_1_forward_2_stop();
          
        }
      } // CÂMERA ESTÁ À DIREITA DO CENTRO DO CONTAINER
      else if(delta_x < - RANGE_ERRO_PERMITIDO_POSICAO_CONTAINER){
        // CÂMERA ESTÁ ABAIXO DO CENTRO DO CONTAINER
        if(delta_y > RANGE_ERRO_PERMITIDO_POSICAO_CONTAINER){
          
          move_1_backward_2_forward();

        } // CÂMERA ESTÁ ACIMA DO CENTRO DO CONTAINER
        else if(delta_y < - RANGE_ERRO_PERMITIDO_POSICAO_CONTAINER){
          
          move_1_backward_2_backward();

        } // CÂMERA ESTÁ CENTRALIZADA NO CONTAINER NO SENTIDO VERTICAL
        else{
          Serial.println("Centralizado em y");
          
          if(tempo_decorrido_y == 0){
            tempo_decorrido_y = millis() - tempo_inicio_y;
          }
          
          move_1_backward_2_stop();
        }
        
      } // CÂMERA ESTÁ CENTRALIZADA NO CONTAINER NO SENTIDO HORIZONTAL
      else{
        
        Serial.println("Centralizado em x");
        
        if(tempo_decorrido_x == 0){
          tempo_decorrido_x = millis() - tempo_inicio_x;
        }
        
        // CÂMERA ESTÁ ABAIXO DO CENTRO DO CONTAINER
        if(delta_y > RANGE_ERRO_PERMITIDO_POSICAO_CONTAINER){

          move_1_stop_2_forward();
          
        } // CÂMERA ESTÁ ACIMA DO CENTRO DO CONTAINER
        else if(delta_y < - RANGE_ERRO_PERMITIDO_POSICAO_CONTAINER){

          move_1_stop_2_backward();
          
        } // CÂMERA ESTÁ CENTRALIZADA NO CONTAINER NO SENTIDO VERTICAL
        else{
          if(tempo_decorrido_y == 0){
            tempo_decorrido_y = millis() - tempo_inicio_y;
          }
          
          move_1_stop_2_stop();
          
          Serial.println("Carro centralizado em cima do container alvo");
          container_centralized = 1;
        }
      }
      
      if(!container_centralized){
        Serial.println("Ainda nao chegou no centro do container...");
      }
       
      str = "";
      comand_identified = 0;
      // Reset reading index 
      idx = 0;
    }
    
    delay(10);
  }
  
  delay(500);
  ///////////////////////////////////////////////////////
  
  ///////////////////////////////////////////////////////
  // ESTADO DE IÇAMENTO DA CARGA
  Serial.println("Icando carga...");
  pegar_carga_e_icar();
  Serial.println("Carga icada");
  ///////////////////////////////////////////////////////
  
  
  ////////////////////////////////////////////////////////
  // ESTADO DE CARREGAMENTO DO CONTAINER PARA O CARRINHO AUTÔNOMO
  Serial.print("Tempo decorrido x: ");
  Serial.println(tempo_decorrido_x);
  Serial.print("Tempo decorrido y: ");
  Serial.println(tempo_decorrido_y);
  
  Serial.println("Levando container para o carrinho...");
  
  if(tempo_decorrido_x <= (tempo_decorrido_y + TEMPO_CENTRO_PONTE_ATE_CARRINHO)){
    move_1_backward_2_backward();
  	delay(tempo_decorrido_x);
    
    move_1_stop_2_backward();
  	delay((tempo_decorrido_y + TEMPO_CENTRO_PONTE_ATE_CARRINHO) - tempo_decorrido_x);
  }
  else{
    move_1_backward_2_backward();
  	delay(tempo_decorrido_y + TEMPO_CENTRO_PONTE_ATE_CARRINHO);
    
    move_1_backward_2_stop();
  	delay(tempo_decorrido_x - (tempo_decorrido_y + TEMPO_CENTRO_PONTE_ATE_CARRINHO));
  }
 
  move_1_stop_2_stop();
  
  ///////////////////////////////////////////////////////
  
  ////////////////////////////////////////////////////////
  // ESTADO DE COLOCAR O CONTAINER NO CARRINHO AUTÔNOMO
	
  abaixar_carga_e_soltar();
  
  ///////////////////////////////////////////////////////
  
  ////////////////////////////////////////////////////////
  // ESTADO DE VOLTAR A PONTE PARA A POSIÇÃO CENTRAL
	
  move_1_stop_2_forward();
  delay(TEMPO_CENTRO_PONTE_ATE_CARRINHO);
  move_1_stop_2_stop();
  
  ///////////////////////////////////////////////////////
  
  // VERIFICAR OS SENSORES DE FIM DE CURSO
  lim_x_min = digitalRead(LIM_X_MIN);
  lim_x_max = digitalRead(LIM_X_MAX);
  lim_y_min = digitalRead(LIM_Y_MIN);
  lim_y_max = digitalRead(LIM_Y_MAX);
  
  Serial.print("lim_x_min: ");
  Serial.println(lim_x_min);
  Serial.print("lim_x_max: ");
  Serial.println(lim_x_max);
  Serial.print("lim_y_min: ");
  Serial.println(lim_y_min);
  Serial.print("lim_y_max: ");
  Serial.println(lim_y_max);
  Serial.println();
  
  // DIRECAO DOS MOTORES DE TRANSLAÇÃO (HORARIO, PARADO, ANTI-HORARIO)
  // userInput1 = analogRead(MOVIMENTO_MOT_1);
  // userInput2 = analogRead(MOVIMENTO_MOT_2);
  
  // if(userInput1 < 400){
  //   if(userInput2 < 400){
  //     move_1_forward_2_forward();
  //   }
  //   else if(userInput2 < 800){
  //     move_1_forward_2_stop();
  //   }
  //   else{
  //     move_1_forward_2_backward();
  //   }
  // }
  // else if(userInput1 < 800){
  //   if(userInput2 < 400){
  //     move_1_stop_2_forward();
  //   }
  //   else if(userInput2 < 800){
  //     move_1_stop_2_stop();
  //     pegar_carga_e_icar();
  //   }
  //   else{
  //     move_1_stop_2_backward();
  //   }
  // }
  // else{
  //   if(userInput2 < 400){
  //     move_1_backward_2_forward();
  //   }
  //   else if(userInput2 < 800){
  //     move_1_backward_2_stop();
  //   }
  //   else{
  //     move_1_backward_2_backward();
  //   }
  // }
  
  // // SE A DIRECAO DOS DOIS FOR "PARADO" NAO PRECISAMOS FAZE NADA ENQUANTO NAO SAIR DESSE ESTADO
  // while( userInput1 > 400 && userInput1 < 800 && userInput2 > 400 && userInput2 < 800){
  //   userInput1 = analogRead(MOVIMENTO_MOT_1);
  // 	userInput2 = analogRead(MOVIMENTO_MOT_2);
  //   delay(100);
  // }
  
  delay(100);
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

void move_1_forward_2_forward(){
  move(1,0,1,0);
}
void move_1_forward_2_stop(){
  move(1,0,0,0);
}
void move_1_forward_2_backward(){
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

void move_to_initial_position(){
  bool initial_position_x_reached = false;
  bool initial_position_y_reached = false;
  
  Serial.println("Movendo o carro para a posicao inicial...");
  speed();
  move_1_backward_2_backward();
  
  while(1){
    speed();
    
    // VERIFICAR OS SENSORES DE FIM DE CURSO
    lim_x_min = digitalRead(LIM_X_MIN);
    lim_y_min = digitalRead(LIM_Y_MIN);
    
    // SE AMBOS OS SENSORES ESTIVEREM ATIVADOS, ESTAMOS
    // NA POSIÇÃO INICIAL x = 0 e y = 0
    // AMBOS OS MOTORES DEVEM SER DESATIVADOS
    // E SAÍMOS DESSE LOOP INICIAL
    if(lim_x_min == HIGH && lim_y_min == HIGH){
      move_1_stop_2_stop();
      Serial.println("Posicao inicial atingida");
      break;
    }
    // SE APENAS O SENSOR LIMITE MININO X ESTIVER ATIVADO
    // APENAS O MOTOR DE TRANSLAÇÃO X DEVE SER DESATIVADO
    if(!initial_position_x_reached && lim_x_min == HIGH){
      initial_position_x_reached = true;
      move_1_stop_2_backward();
      Serial.println("Posicao x inicial atingida");
    }
    // SE AEPNAS O SENSOR LIMITE MININO Y ESTIVER ATIVADO
    // APENAS O MOTOR DE TRANSLAÇÃO X DEVE SER DESATIVADO
    if(!initial_position_y_reached && lim_y_min == HIGH){
      initial_position_y_reached = true;
      move_1_backward_2_stop();
      Serial.println("Posicao y inicial atingida");
    }
  }
}

void move_to_central_position(){
  move_to_initial_position();
  
  Serial.println("Movendo o carro para a posicao central...");
  speed();
  move_1_forward_2_forward();
  delay(2000);
  move_1_stop_2_forward();
  delay(1000);
  move_1_stop_2_stop();
  
  Serial.println("Posicao central atingida!");
}

void pegar_carga_e_icar(){
  servo.write(VALOR_PWM_SERVO360_PARADO - VELOCIDADE_SERVO); // sentido horário
  delay(TEMPO_ABAIXAR_ELETROIMA);
  servo.write(VALOR_PWM_SERVO360_PARADO);
  digitalWrite(ELETROIMA, HIGH);
  delay(1000);
  servo.write(VALOR_PWM_SERVO360_PARADO + VELOCIDADE_SERVO); // sentido anti-horário
  delay(TEMPO_ABAIXAR_ELETROIMA);
  servo.write(VALOR_PWM_SERVO360_PARADO);
}

void abaixar_carga_e_soltar(){
  servo.write(VALOR_PWM_SERVO360_PARADO - VELOCIDADE_SERVO); // sentido horário
  delay(TEMPO_ABAIXAR_ELETROIMA);
  servo.write(VALOR_PWM_SERVO360_PARADO);
  digitalWrite(ELETROIMA, LOW);
  delay(1000);
  servo.write(VALOR_PWM_SERVO360_PARADO + VELOCIDADE_SERVO); // sentido anti-horário
  delay(TEMPO_ABAIXAR_ELETROIMA);
  servo.write(VALOR_PWM_SERVO360_PARADO);
}