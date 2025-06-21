# SEL0373 - Projetos em IoT

Este repositório foi criado para a disciplina **SEL0373 - IoT**. O projeto tem como objetivo **habilitar a entrada de animais em uma balança** para pesagem e enviar os dados de peso via **comunicação MQTT**. A liberação ou não dos animais é realizada por meio de **comunicação MQTT**, permitindo um controle sem fio sobre quais animais são permitidos ou negados. Para a implementação, foram utilizados **dois microcontroladores ESP32**: um dedicado à **aquisição de dados** e outro à **comunicação MQTT**, com ambos se comunicando entre si por meio da **rede CAN**.

## ESP32 para Aquisição de Dados

Este microcontrolador **ESP32** é responsável pela aquisição e envio de dados para três módulos principais:

- **RFID**
- **HX711 (Módulo de Célula de Carga)**
- **Servo Motor**

O processo ocorre da seguinte forma:
1. O **ESP32** lê o ID do **RFID** e verifica se o ID é permitido (os IDs permitidos são enviados previamente via MQTT).
2. Se o ID for válido, o **servo motor** é acionado para abrir a porta, permitindo a entrada do animal.
3. Após a entrada, o **servo motor** é acionado novamente para fechar a porta.
4. Em seguida, é realizada a **pesagem** utilizando a **célula de carga**, e o valor obtido é convertido digitalmente via o módulo **HX711**.
5. O valor do peso é então transmitido via **rede CAN** para o outro **ESP32**, que, por sua vez, envia os dados para o servidor via **MQTT**.

O projeto final dessa ESP32 pode ser visualizado em [Projeto 1](proj_final_1).

## ESP32 para Comunicação MQTT

Este **ESP32** é responsável pela comunicação MQTT, e suas principais funções incluem:

1. Receber os novos IDs permitidos ou negados via **MQTT** e enviá-los para o outro **ESP32** através da **rede CAN**.
2. Receber os dados de peso de um ID via **rede CAN** e enviá-los para o servidor via **MQTT**.

O projeto final dessa ESP32 pode ser visualizado em [Projeto 2](proj_final_2).

## Repositório do Código Web

O código web do projeto está disponível no seguinte repositório:  
[Repositório Web](https://github.com/GUUDIN/sel0373-projeto)





