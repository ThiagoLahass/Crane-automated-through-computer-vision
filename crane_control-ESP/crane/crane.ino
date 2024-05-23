#include <ESP32Servo.h>

// SETUP SERVO
#define VALOR_PARADO_SERVO 				      93
#define VELOCIDADE_SERVO 						    5
#define SERVO 									        23
Servo servo;

// SETUP ELETR0ÍMÃ
#define ELETROIMA 	                    22

// OUTRAS VARIÁVEIS DE SETUP
#define TEMPO_ABAIXAR_ELETROIMA 				        3000
#define RANGE_ERRO_PERMITIDO_POSICAO_CONTAINER 	20
#define TEMPO_CENTRO_PONTE_ATE_CARRINHO 		    5000


// SETUP MOTORES
#define EN_A		19
#define IN1  		18
#define IN2			15

#define EN_B 		17
#define IN3 		16
#define IN4 		4

#define VELOCIDADE_MOT_MOV 	0

// SENSORES DE FIM DE CURSO
#define LIM_X_MIN  	12
#define LIM_X_MAX   14
#define LIM_Y_MIN   27
#define LIM_Y_MAX   26

int delta_x = 0;
int delta_y = 0;

int lim_x_min = 0;
int lim_x_max = 0;
int lim_y_min = 0;
int lim_y_max = 0;

////////////////////////////// FUNCTIONS HEADERS ///////////////////////////////////
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

// INTERRUPT FUNCTIONS
void lim_min_x_interruption();
void lim_max_x_interruption();
void lim_min_y_interruption();
void lim_max_y_interruption();
//////////////////////////////////////////////////////////////////////////////////////

void setup(){
  Serial.begin(9600);
  servo.attach(SERVO);
  
  pinMode(EN_A, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  
  pinMode(EN_B, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  pinMode(VELOCIDADE_MOT_MOV, INPUT);
  
  pinMode(ELETROIMA, OUTPUT);
  
  // DEFINIR SENSORES DE FIM DE CURSO COMO PINOS DE INTERRUPÇÃO
  pinMode(LIM_X_MIN,INPUT);
  pinMode(LIM_X_MAX,INPUT);
  pinMode(LIM_Y_MIN,INPUT);
  pinMode(LIM_Y_MAX,INPUT);

  // Configuração das interrupções
  attachInterrupt(LIM_X_MIN, lim_min_x_interruption, RISING);
  // attachInterrupt(LIM_X_MAX, lim_max_x_interruption, RISING);
  // attachInterrupt(LIM_Y_MIN, lim_min_y_interruption, RISING);
  // attachInterrupt(LIM_Y_MAX, lim_max_x_interruption, RISING);
  
  // ESTADO DE SETUP
  // MOVE O CARRO PARA A POSICAO INICIAL (0, 0)
  // DEPOIS MOVER PARA O CENTRO DA PONTE PARA SE TER VISAO DOS CONTAINERS
  // move_to_central_position();
  
  delay(100);
}

void loop(){
  
  ///////////////////////////////////////////////////////////////////////////////////
  // ESTADO INICIAL
  // AGUARDANDO COMANDO PARA INICIAR A BUSCA DE CONTAINERS
  int comand_identified = 0;
  int start_comand 		  = 0;
  
  String str = "";
  
  Serial.println("ESP: Aguardando comando pela Serial para iniciar a busca... (begin)");
  
  while(!start_comand){
   	if (Serial.available() > 0) { 	// Verifica se há dados disponíveis na serial
      str = Serial.readString();    // Lê os dados recebidos como string
      str.trim();                   // Remove espaços em branco do início e do fim da string
      comand_identified = 1;
    }
      
    if(comand_identified){
      Serial.print("ESP: ");
      Serial.println(str);

      // Verifica se a string recebida é igual a "begin"
      if (str.equals("begin")) {
        Serial.println("ESP: Comando de inicio recebido!");
        start_comand = 1;
      } else {
        Serial.println("ESP: Comando invalido!");
        str = "";
      }

      // Reset string 
      str = "";
    }

    delay(5);
    comand_identified = 0;
  }
  
  ///////////////////////////////////////////////////////////////////////////////////
  
  // ESTADO DE BUSCA DO CONTAINER SELECIONADO
  /* 
  AS INFORMAÇÕES RECEBIDAS AQUI DEVEM SER NO FORMATO:
    "sxxx zyyy",
    ONDE:
    s é o sinal de delta_x (+ ou -);
    xxx são os 3 caracteres de delta_x;
    z é o sinal de delta_y (+ ou -);
    yyy são os 3 caracteres de delta_y.
  */
  int container_centralized = 0;
  str = "";
  
  Serial.println("ESP: Aguardando posicao do container...");
  
  unsigned long tempo_inicio_x = millis(); 	// Salva o tempo de início de centralização em x
  unsigned long tempo_inicio_y = millis(); 	// Salva o tempo de início de centralização em y
  unsigned long tempo_decorrido_x = 0; 		  // Variável para armazenar o tempo gasto para realizar a centralização em x
  unsigned long tempo_decorrido_y = 0; 		  // Variável para armazenar o tempo gasto para realizar a centralização em y
  
  while(!container_centralized){
    // SETAR VELOCIDADE DOS MOTORES DE TRANSLAÇÃO DE ACORDO COM O POTENCIOMETRO DE VELOCIDADE
    speed();

    if (Serial.available() > 0) { 	// Verifica se há dados disponíveis na serial
      str = Serial.readString();    // Lê os dados recebidos como string
      str.trim();                   // Remove espaços em branco do início e do fim da string
      comand_identified = 1;
    }
      
    if(comand_identified){
      // VERIFICAR COMANDO
      // Serial.print("ESP: ");
      // Serial.print(str);
      
      /* 
      AS INFORMAÇÕES RECEBIDAS AQUI DEVEM SER NO FORMATO:
        "sxxx zyyy",
        ONDE:
        s é o sinal de delta_x (+ ou -);
        xxx são os 3 caracteres de delta_x;
        z é o sinal de delta_y (+ ou -);
        yyy são os 3 caracteres de delta_y.
      */
      String str_delta_x = str.substring(0, 4); // Extrai os primeiros 4 caracteres (delta_x)
      String str_delta_y = str.substring(5, 9); // Extrai os ultimos   4 caracteres (delta_y)

      // Converte as strings para números inteiros
      delta_x = str_delta_x.toInt();
      delta_y = str_delta_y.toInt();
      
      // Serial.print("ESP: delta_x: ");
      // Serial.print(delta_x);
      // Serial.print("ESP: delta_y: ");
      // Serial.print(delta_y);
      
      if(delta_x > RANGE_ERRO_PERMITIDO_POSICAO_CONTAINER){             // CÂMERA ESTÁ À ESQUERDA DO CENTRO DO CONTAINER

        if(delta_y > RANGE_ERRO_PERMITIDO_POSICAO_CONTAINER){           // CÂMERA ESTÁ ABAIXO DO CENTRO DO CONTAINER
			    move_1_forward_2_forward();
        }
        else if(delta_y < - RANGE_ERRO_PERMITIDO_POSICAO_CONTAINER){    // CÂMERA ESTÁ ACIMA DO CENTRO DO CONTAINER
          move_1_forward_2_backward();
        }
        else{                                                           // CÂMERA ESTÁ CENTRALIZADA NO CONTAINER NO SENTIDO VERTICAL
          // Serial.print("ESP: Centralizado em y");
          if(tempo_decorrido_y == 0){
            tempo_decorrido_y = millis() - tempo_inicio_y;
          }
          move_1_forward_2_stop();
        }
      }
      else if(delta_x < - RANGE_ERRO_PERMITIDO_POSICAO_CONTAINER){      // CÂMERA ESTÁ À DIREITA DO CENTRO DO CONTAINER
        
        if(delta_y > RANGE_ERRO_PERMITIDO_POSICAO_CONTAINER){           // CÂMERA ESTÁ ABAIXO DO CENTRO DO CONTAINER
          move_1_backward_2_forward();
        }
        else if(delta_y < - RANGE_ERRO_PERMITIDO_POSICAO_CONTAINER){    // CÂMERA ESTÁ ACIMA DO CENTRO DO CONTAINER
          move_1_backward_2_backward();
        }
        else{                                                           // CÂMERA ESTÁ CENTRALIZADA NO CONTAINER NO SENTIDO VERTICAL
          // Serial.print("ESP: Centralizado em y");
          if(tempo_decorrido_y == 0){
            tempo_decorrido_y = millis() - tempo_inicio_y;
          }
          move_1_backward_2_stop();
        }
      }
      else{                                                             // CÂMERA ESTÁ CENTRALIZADA NO CONTAINER NO SENTIDO HORIZONTAL
        // Serial.print("ESP: Centralizado em x");
        if(tempo_decorrido_x == 0){
          tempo_decorrido_x = millis() - tempo_inicio_x;
        }
        
        if(delta_y > RANGE_ERRO_PERMITIDO_POSICAO_CONTAINER){           // CÂMERA ESTÁ ABAIXO DO CENTRO DO CONTAINER
          move_1_stop_2_forward();      
        }
        else if(delta_y < - RANGE_ERRO_PERMITIDO_POSICAO_CONTAINER){    // CÂMERA ESTÁ ACIMA DO CENTRO DO CONTAINER
          move_1_stop_2_backward();
        }
        else{                                                           // CÂMERA ESTÁ CENTRALIZADA EM AMBOS SENTIDOS !!!
          if(tempo_decorrido_y == 0){
            tempo_decorrido_y = millis() - tempo_inicio_y;
          }
          
          move_1_stop_2_stop();
          
          Serial.print("ESP: container centralizado");
          container_centralized = 1;
        }
      }
      
      if(!container_centralized){
        // Serial.print("ESP: Ainda nao chegou no centro do container...");
      }
       
      // Reseta as variáveis 
      str = "";
      comand_identified = 0; 
    }
    
    delay(10);
  }
  
  ///////////////////////////////////////////////////////////////////////////////////
  
  // ESTADO DE IÇAMENTO DA CARGA
  // DELAY PARA DAR TEMPO DO BACKEND RECEBER O COMANDO INDICANDO QUE O CONTAINER ESTA CENTRALIZADO
  delay(2000);

  Serial.println("ESP: Icando carga...");
  pegar_carga_e_icar();
  Serial.println("ESP: Carga icada");
  
  ///////////////////////////////////////////////////////////////////////////////////
  
  // ESTADO DE CARREGAMENTO DO CONTAINER PARA O CARRINHO AUTÔNOMO
  Serial.print("ESP: Tempo decorrido x: ");
  Serial.println(tempo_decorrido_x);
  Serial.print("ESP: Tempo decorrido y: ");
  Serial.println(tempo_decorrido_y);
  
  Serial.println("ESP: Levando container para o carrinho...");
  
  if(tempo_decorrido_x <= tempo_decorrido_y + TEMPO_CENTRO_PONTE_ATE_CARRINHO){
    move_1_backward_2_backward();
  	delay(tempo_decorrido_x);
    
    move_1_stop_2_backward();
  	delay(tempo_decorrido_y + TEMPO_CENTRO_PONTE_ATE_CARRINHO - tempo_decorrido_x);
  }
  else{
    move_1_backward_2_backward();
  	delay(tempo_decorrido_y + TEMPO_CENTRO_PONTE_ATE_CARRINHO);
    
    move_1_backward_2_stop();
  	delay(tempo_decorrido_x - (tempo_decorrido_y + TEMPO_CENTRO_PONTE_ATE_CARRINHO));
  }
  
  Serial.println("ESP: Container centralizado em cima do carrinho");
  move_1_stop_2_stop();
  
  ///////////////////////////////////////////////////////////////////////////////////
  
  // ESTADO DE COLOCAR O CONTAINER NO CARRINHO AUTÔNOMO
  Serial.println("ESP: Colocando container em cima do carrinho...");
  abaixar_carga_e_soltar();
  
  ///////////////////////////////////////////////////////////////////////////////////
  
  // ESTADO DE VOLTAR A PONTE PARA A POSIÇÃO CENTRAL
  Serial.println("ESP: Movendo carro de volta para a posicao central");

  move_1_stop_2_forward();
  delay(TEMPO_CENTRO_PONTE_ATE_CARRINHO);
  move_1_stop_2_stop();
  Serial.print("ESP: end");

  // DELAY PARA DAR TEMPO DO BACKEND RECEBER O COMANDO INDICANDO QUE O CICLO ESTA COMPLETO
  delay(2000);
  
  ///////////////////////////////////////////////////////////////////////////////////
}

void speed(){
  int aux = analogRead(VELOCIDADE_MOT_MOV);
  aux = map(aux, 0, 4095, 0 , 255);
  analogWrite(EN_A, aux);
  analogWrite(EN_B, aux);
}

void move(int in1_, int in2_, int in3_, int in4_) {
 // MOTOR 1 - x
 digitalWrite(IN1, in1_);
 digitalWrite(IN2, in2_);
 // MOTOR 2 - y
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
  
  Serial.println("ESP: Movendo o carro para a posicao inicial...");
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
      Serial.println("ESP: Posicao inicial atingida");
      break;
    }
    // SE APENAS O SENSOR LIMITE MININO X ESTIVER ATIVADO
    // APENAS O MOTOR DE TRANSLAÇÃO X DEVE SER DESATIVADO
    if(!initial_position_x_reached && lim_x_min == HIGH){
      initial_position_x_reached = true;
      move_1_stop_2_backward();
      Serial.println("ESP: Posicao x inicial atingida");
    }
    // SE AEPNAS O SENSOR LIMITE MININO Y ESTIVER ATIVADO
    // APENAS O MOTOR DE TRANSLAÇÃO X DEVE SER DESATIVADO
    if(!initial_position_y_reached && lim_y_min == HIGH){
      initial_position_y_reached = true;
      move_1_backward_2_stop();
      Serial.println("ESP: Posicao y inicial atingida");
    }
  }
}

// PRÉ REQUISITO: CARRINHO ESTAR NA POSIÇÃO (0, 0)
void move_to_central_position(){
  move_to_initial_position();
  
  Serial.println("ESP: Movendo o carro para a posicao central...");

  speed();

  move_1_forward_2_forward();
  delay(2000);
  move_1_stop_2_forward();
  delay(1000);
  move_1_stop_2_stop();
  
  Serial.println("ESP: Posicao central atingida!");
}

void pegar_carga_e_icar(){
  servo.write(VALOR_PARADO_SERVO - VELOCIDADE_SERVO); // sentido horário
  delay(TEMPO_ABAIXAR_ELETROIMA);
  servo.write(VALOR_PARADO_SERVO);
  digitalWrite(ELETROIMA, HIGH);
  delay(1000);
  servo.write(VALOR_PARADO_SERVO + VELOCIDADE_SERVO); // sentido anti-horário
  delay(TEMPO_ABAIXAR_ELETROIMA);
  servo.write(VALOR_PARADO_SERVO);
}

void abaixar_carga_e_soltar(){
  servo.write(VALOR_PARADO_SERVO - VELOCIDADE_SERVO); // sentido horário
  delay(TEMPO_ABAIXAR_ELETROIMA);
  servo.write(VALOR_PARADO_SERVO);
  digitalWrite(ELETROIMA, LOW);
  delay(1000);
  servo.write(VALOR_PARADO_SERVO + VELOCIDADE_SERVO); // sentido anti-horário
  delay(TEMPO_ABAIXAR_ELETROIMA);
  servo.write(VALOR_PARADO_SERVO);
}

void lim_min_x_interruption(){
  move_1_stop_2_stop();  
  //move_to_central_position();
  Serial.println("ESP: Interrupção ativada, parando os motores...");
}
void lim_max_x_interruption(){
  move_1_stop_2_stop();
  move_to_central_position();
}
void lim_min_y_interruption(){
  move_1_stop_2_stop();
  move_to_central_position();
}
void lim_max_y_interruption(){
  move_1_stop_2_stop();
  move_to_central_position();
}
