#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// === NRF24L01 SETUP ===
RF24 radio(9, 8); 

const byte address[5] = {0xE0, 0xE0, 0xF1, 0xF1, 0xE0};

void setup() {
  Serial.begin(9600);
  
  radio.begin();
  
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(0x76);                
  radio.setDataRate(RF24_250KBPS);
  radio.setPayloadSize(32);
  
  radio.openReadingPipe(1, address);
  radio.startListening();
  
  Serial.println("nRF24L01 Ready");
  delay(1000);
}

void loop() {
  if (radio.available()) {
    char receivedData[32] = "";
    radio.read(&receivedData, sizeof(receivedData));
    
    String dataString = String(receivedData);
    dataString.trim();
    
    if (dataString.length() > 0) {
      Serial.println(dataString);
      delay(10);
    }
  }
}