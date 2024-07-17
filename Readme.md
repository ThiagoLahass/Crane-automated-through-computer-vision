# Carrinho Seguidor de Linha com Controle Proporcional

Este projeto descreve um carrinho seguidor de linha que utiliza três sensores para seguir a linha e um LDR para detectar a presença de um container no "bagageiro". O controle do carrinho é proporcional, garantindo um movimento suave e preciso.

## Componentes Utilizados

- 1 x Microcontrolador (Arduino)
- 3 x Sensores de Linha (TCRT5000)
- 1 x LDR (Light Dependent Resistor)
- 2 x Motores DC
- 1 x Ponte H (ex. L298N)
- 1 x Chassi do Carrinho
- Rodas, bateria e outros componentes de suporte

## Funcionamento

### Sensores de Linha

Os três sensores de linha são posicionados na frente do carrinho. Eles detectam a presença da linha no chão e enviam sinais ao microcontrolador, que ajusta a direção e a velocidade dos motores para manter o carrinho na trajetória correta.

### LDR (Bagageiro)

O LDR é usado como um sensor de carga no "bagageiro" do carrinho. Quando um container é colocado sobre o carrinho, o LDR detecta a mudança na intensidade da luz, permitindo que o microcontrolador registre a presença do container.

### Controle Proporcional

O controle proporcional ajusta a velocidade dos motores com base na posição do carrinho em relação à linha. Se o carrinho estiver desviando muito da linha, a correção será maior. Isso permite um movimento mais suave e preciso, reduzindo oscilações e melhorando a eficiência do seguimento da linha.

### Esquemático do circuito 

![Esquemático do Carrinho](/Schematic_carrinho-seguidor_2024-07-17.png)
