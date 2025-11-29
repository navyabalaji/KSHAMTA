#include <SPI.h>
#include <RF24.h>

// Create RF24 object - CE on pin 7, CSN on pin 8
RF24 radio(7, 8);

// Must match Raspberry Pi transmitter address
const byte address[5] = {0xE0, 0xE0, 0xF1, 0xF1, 0xE0};

void setup() {
  // Initialize Serial for UART communication with Arduino Uno
  // Nano TX (D1) -> Uno RX (D0)
  // Nano RX (D0) -> Uno TX (D1)
  Serial.begin(9600);
  
  delay(1000);  // Give time for serial to stabilize
  
  Serial.println("=== Nano + NRF24L01 Receiver ===");
  
  // Initialize radio
  if (!radio.begin()) {
    Serial.println("ERROR: Radio hardware not responding!");
    Serial.println("Check connections and power supply");
    while (1); // Hold program in infinite loop
  }
  
  // Configure radio to match Raspberry Pi settings
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(0x76);           // Channel 118
  radio.setDataRate(RF24_250KBPS);  // 250 Kbps
  radio.setPayloadSize(32);         // 32 bytes payload
  
  // Open reading pipe with the same address
  radio.openReadingPipe(1, address);
  
  // Start listening for transmissions
  radio.startListening();
  
  Serial.println("nRF24L01 initialized successfully");
  Serial.println("Waiting for voice commands from Raspberry Pi...");
  Serial.println("Will forward commands to Arduino Uno via UART\n");
  
  // Print radio details for debugging
  radio.printDetails();
  
  Serial.println("\n=== System Ready ===\n");
}

void loop() {
  // Check if data is available from nRF24L01
  if (radio.available()) {
    char receivedText[33] = "";  // 32 bytes + null terminator
    
    // Read the data from nRF
    radio.read(&receivedText, 32);
    
    // Ensure null termination
    receivedText[32] = '\0';
    
    // Debug: Print what was received from Raspberry Pi
    Serial.print(">>> Received from RPi: ");
    Serial.println(receivedText);
    
    // Forward the command to Arduino Uno via UART
    // Send with newline for easy parsing on Uno side
    Serial.print("UART_CMD:");
    Serial.println(receivedText);
    
    Serial.println(">>> Forwarded to Uno via UART\n");
    
    // Small delay to prevent flooding
    delay(100);
  }
}