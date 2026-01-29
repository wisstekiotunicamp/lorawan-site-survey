// =======================================================================
//                    CONFIGURAÇÕES DAS BIBLIOTECAS
// =======================================================================
#include <Arduino.h>

// =======================================================================
//                    CONFIGURAÇÕES DO PROJETO
// =======================================================================

// 1. PINOS DE COMUNICAÇÃO COM O MÓDULO LORA
// Define os pinos de comunicação UART com o módulo LoRa conforme a montagem do seu circuito.
#define LORA_TX 23 // Fio Verde: Pino de Transmissão (TX) do LoRa
#define LORA_RX 22 // Fio Laranja: Pino de Recepção (RX) do LoRa


// 2. CREDENCIAIS ABP (Activation By Personalization)
// ⮞⮞⮞ SUBSTITUA OS VALORES ABAIXO PELOS SEUS (COPIADOS DA TTN) ⮜✜✜
// IMPORTANTE: As chaves da TTN devem conter os dois-pontos (:)
const char* DEVADDR = "00:00:00:00";
const char* APPSKEY = "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00";
const char* NWKSKEY = "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00";

// =======================================================================

// Inicializa a porta serial para comunicação com o módulo LoRa
HardwareSerial LoRaSerial(1);

/**
 * @brief Envia um comando AT para o módulo LoRa, imprime a comunicação
 *        no Monitor Serial e verifica a resposta.
 * @param comando O comando AT a ser enviado (sem \r\n).
 * @param timeout O tempo máximo de espera pela resposta em milissegundos.
 * @return true se o módulo responder "OK", false para "ERROR" ou timeout.
 **/

void enviaComandoLoRa(const String& comando, unsigned long timeout = 3000) {
  // Limpa qualquer dado antigo no buffer de recepção
  while (LoRaSerial.available()) LoRaSerial.read();
  
  // Envia o comando para o módulo
  LoRaSerial.print(comando + "\r\n");

  // Imprime o comando enviado no Monitor Serial para depuração
  Serial.print("[COMANDO - ESP32] -> ");
  Serial.println(comando);

  unsigned long start = millis();
  String resposta = "";

  // Aguarda por uma resposta completa
  while (millis() - start < timeout) {
    while (LoRaSerial.available()) {
      char c = LoRaSerial.read();
      resposta += c;  // Armazena a resposta recebida
    }
  }

  // Se houver resposta, imprime o prefixo [RESPOSTA] -> 
  if (resposta.length() > 0) {
    Serial.print("[RESPOSTA - LoRa] -> ");
    Serial.print(resposta);
  } 
    // Caso não haja resposta, exibe uma mensagem de erro
  else {
    Serial.println("[RESPOSTA] -> Nenhum retorno recebido");
  }

  // Imprime uma linha em branco para separar os comandos/respostas
  // Serial.println();
}

// =======================================================================

void setup() {
  // Inicia o Monitor Serial para depuração
  Serial.begin(115200);
  delay(2000); // Aguarda o Monitor Serial estabilizar

  // Inicia a comunicação serial com o módulo LoRa
  Serial.println("\n[SETUP] Inicializando comunicação com o módulo Radioenge...");
  LoRaSerial.begin(9600, SERIAL_8N1, LORA_RX, LORA_TX);
  delay(1000);

  // --- CONFIGURAÇÃO DO MÓDULO LORAWAN ---
  // A seguir, configuramos o módulo para operar no modo ABP na rede TTN (The Things Network).
  Serial.println("[SETUP] Configurando o módulo para operar na TTN via ABP...\n");
  
  enviaComandoLoRa("AT+VER=?");                                 // Informa a versão do firmware
  enviaComandoLoRa("AT+DEUI=?");                                // Solicita o DevEUI (Endereço único do dispositivo)
  enviaComandoLoRa("AT");                                       // Testa a comunicação com o módulo
  enviaComandoLoRa("AT+NJM=0");                                 // Define o modo de ativação (ABP)
  enviaComandoLoRa("AT+CLASS=A");                               // Define a classe A para LoRaWAN
  enviaComandoLoRa("AT+ADR=0");                                 // Desativa Adaptive Data Rate (útil para testes)
  enviaComandoLoRa("AT+DR=5");                                  // Define o Data Rate 5 (SF7) para maior largura de banda
  enviaComandoLoRa("AT+DADDR=" + String(DEVADDR));              // Configura o Device Address
  enviaComandoLoRa("AT+NWKSKEY=" + String(NWKSKEY));            // Configura a Network Session Key
  enviaComandoLoRa("AT+APPSKEY=" + String(APPSKEY));            // Configura a Application Session Key
  enviaComandoLoRa("AT+CHMASK=0100:0000:0000:0000:0000:0000");  // Habilita canais 8-15 (AU915 - TTN Brasil)

  Serial.println("\n[SETUP] Configuração finalizada. Módulo pronto.\n");
  Serial.println("--- Iniciando o loop() de operação ---\n");
  delay(2000);
}

// =======================================================================

// 3. INTERVALO DE ENVIO
// Intervalo entre os envios de dados em milissegundos.
// Mínimo recomendado: 60000 (60 segundos) para não violar a política da TTN.
const unsigned long intervalo = 60000; 

void loop() {
  static unsigned long ultimaMensagem = 0;

  // Verifica se já passou o tempo para um novo envio
  if (millis() - ultimaMensagem > intervalo) {
    ultimaMensagem = millis();

    // --- PREPARAÇÃO E ENVIO DO PAYLOAD ---

    // Porta serial em uso para comunicação com LoRa
    String porta = "1";
    // Payload em HEX
    String payloadHex = "01020304"; // Dados de exemplo para envio

    // Exibe o payload a ser enviado
    Serial.println("[MSG] Enviando dados: " + porta + ":" + payloadHex);
    enviaComandoLoRa("AT+SENDB=" + porta + ":" + payloadHex, 5000); // Envia o payload via LoRa

    Serial.println("======================================\n");
  }

  // Verifica se o módulo enviou alguma mensagem não solicitada (ex: downlink)
  if (LoRaSerial.available()) {
    String msg = LoRaSerial.readString();
    Serial.print("[LoRa] <- (Mensagem não solicitada): ");
    Serial.println(msg);
  }
}
