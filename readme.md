# Sistema de Automação Logística com Visão Computacional

<p align="center">
  <img src="media/logo.png" />
</p>

## Descrição do Projeto

O Sistema de Automação Logística com Visão Computacional é uma solução inovadora que combina robótica, visão computacional e comunicação serial para otimizar a logística em um ambiente automatizado. Utilizando uma ponte rolante controlada por um ESP32, o sistema é capaz de identificar, localizar e mover contêineres de diferentes cores de forma precisa e eficiente.

## Componentes do Sistema

### Hardware:
   - **ESP32:** Microcontrolador responsável pelo controle dos motores e comunicação serial.
   - **Motores de Translação:** Permitem o movimento longitudinal (frente/tras) e movimento transversal (esquerda/direita) da ponte rolante.
   - **Servo Motor 360° Contínuo:** Utilizado para elevar e baixar a carga.
   - **Eletroímã:** Para pegar e soltar os contêineres.
   - **Sensores de Fim de Curso:** 4 sensores para limitar a translação da ponte.
   - **Câmera:** Integrada à ponte rolante para capturar imagens dos contêineres.
   - **Carrinho Seguidor de Linha:** Transporta os contêineres da área de carga para a área de descarga e depois volta à área de carga.

### Software:
- **OpenCV:** Biblioteca de visão computacional para processamento de imagens.
- **Comunicação Serial:** Interface de comunicação entre o ESP32 e o sistema de visão computacional.

## Funcionamento

O sistema captura imagens através da câmera integrada na ponte rolante e utiliza a biblioteca OpenCV para processar essas imagens e identificar containers por suas respectivas cores. O usuário pode selecionar, através de uma interface, a quantidade e tipo de containers desejados, e o sistema irá operar conforme as escolhas feitas.

## Estrutura do Repositório

- **crane.ino:** Código em C++ embarcado no ESP32, responsável pelo controle de todos os componentes da ponte rolante.
- **thresholds-identifier.py:** Utilizado para identificar bons valores no espaço de cores HSV para identificar cada uma das cores antes da execução do código principal.
- **containers_identification.py:** Código principal que executa simultaneamente com o microcontrolador ESP32, processando a imagem e enviando as informações via comunicação serial.

## Instalação e Configuração

### Requisitos

- ESP32
- Python 3.x
- OpenCV
- Bibliotecas adicionais conforme especificado nos arquivos `.ino` e `.py`

### Passos para Configuração

1. Clone o repositório:
   ```bash
   git clone https://github.com/ThiagoLahass/Crane-automated-through-computer-vision.git
   cd sistema-automacao-logistica
   ```

2. Instale as dependências Python:
   ```bash
   pip install -r requirements.txt
   ```

3. Compile e carregue o código `crane.ino` no ESP32 utilizando a Arduino IDE.

4. Execute `thresholds-identifier.py` para identificar os valores de cor HSV ideais. Após isso, ajuste os valores das variáveis para cada uma das cores em  `containers_identification.py`. Por exemplo, os valores para a cor vermelha devem ser definidos em `H_MIN_RED`, `H_MAX_RED`, `S_MIN_RED`, `S_MAX_RED`, `V_MIN_RED`, `V_MAX_RED`.

5. Execute `containers_identification.py` para iniciar o processamento da imagem e a comunicação serial com o ESP32.

## Utilização

1. **Identificação de Cores:**
   Execute `thresholds-identifier.py` para ajustar os valores HSV das cores.
    <p align="center">
      <img src="media/thresholds-identifier.jpeg" />
    </p>

2. **Interface de Seleção:**
   Utilize a interface para selecionar a quantidade e tipo de containers.
    <p align="center">
      <img src="media/interface.png" />
    </p>

3. **Operação Automática:**
   Após a seleção, o sistema operará automaticamente para mover e posicionar os containers conforme especificado.
    <p align="center">
      <img src="media/ponte-rolante.jpeg" />
    </p>

## Contribuição

1. Faça um fork do projeto.
2. Crie uma branch para sua feature (`git checkout -b feature/nova-feature`).
3. Commit suas mudanças (`git commit -am 'Adiciona nova feature'`).
4. Faça um push para a branch (`git push origin feature/nova-feature`).
5. Abra um Pull Request.

## Desenvolvedores

- **Thiago Felippe Neitzke Lahass** - *Desenvolvedor Principal* - [Thiago Lahass](https://github.com/ThiagoLahass)
- **Gabriel Gatti da Silva** - [Gabriel Gatti](https://github.com/gabrielgatti7)
- **Bruno Santos Fernandes** - [Bruno Santos](https://github.com/BrunoSantosFF)

## Agradecimentos

Agradecemos a todos os contribuidores e apoiadores deste projeto.

<p align="center">
  <img src="media/logo.png" />
</p>