#include "HardwareSerial.h"

HardwareSerial LoRaSerial(1);

#define LORA_RX_PIN 23  // Recebe dados do módulo
#define LORA_TX_PIN 22  // Envia dados para o módulo
#define BAUD_RATE 9600

void setup() {
  Serial.begin(115200);
  LoRaSerial.begin(BAUD_RATE, SERIAL_8N1, LORA_RX_PIN, LORA_TX_PIN);
  delay(1000);

  // Limpa buffers
  while (LoRaSerial.available()) LoRaSerial.read();

  // Envia comando AT para obter o DevEUI
  Serial.println("Enviando comando AT+DEUI=?");
  LoRaSerial.println("AT+DEUI=?");
  delay(200);  // Dá tempo para resposta

  // Lê a resposta completa
  String response = "";
  unsigned long startTime = millis();
  while (millis() - startTime < 1000) {  // Espera até 1 segundo
    while (LoRaSerial.available()) {
      char c = LoRaSerial.read();
      response += c;
    }
  }

  Serial.println("Resposta recebida:");
  Serial.println(response);
}

void loop() {
  // Não faz nada
}
