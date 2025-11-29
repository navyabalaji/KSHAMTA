#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>


LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial nrfSerial(14, 15); // RX=A0 (pin 14), TX=A1 (pin 15)

const int modeButtonPin = 17;   
const int readerButtonPin = 16; 
bool STATE = true;              
bool PRINTER_INPUT_MODE = true; 
bool lastButtonState = HIGH;
bool lastReaderButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long lastReaderDebounceTime = 0;
const unsigned long debounceDelay = 300;

Servo servos[6];
int servoPins[6] = {7,6,5,4,3,2};


int brailleLetters[26][6] = {
  {1,0,0,0,0,0}, {1,1,0,0,0,0}, {1,0,0,1,0,0}, {1,0,0,1,1,0}, {1,0,0,0,1,0},
  {1,1,0,1,0,0}, {1,1,0,1,1,0}, {1,1,0,0,1,0}, {0,1,0,1,0,0}, {0,1,0,1,1,0},
  {1,0,1,0,0,0}, {1,1,1,0,0,0}, {1,0,1,1,0,0}, {1,0,1,1,1,0}, {1,0,1,0,1,0},
  {1,1,1,1,0,0}, {1,1,1,1,1,0}, {1,1,1,0,1,0}, {0,1,1,1,0,0}, {0,1,1,1,1,0},
  {1,0,1,0,0,1}, {1,1,1,0,0,1}, {0,1,0,1,1,1}, {1,0,1,1,0,1}, {1,0,1,1,1,1}, {1,0,1,0,1,1}
};


struct BrailleChar {
  char character;
  int pattern[6];
};

BrailleChar specialChars[] = {
  {' ', {0,0,0,0,0,0}},   
  {',', {0,1,0,0,0,0}},   
  {';', {0,1,1,0,0,0}},   
  {':', {0,1,0,0,1,0}},  
  {'.', {0,1,0,0,1,1}},   
  {'?', {0,1,1,0,1,1}},  
  {'!', {0,1,1,0,1,0}},   
  {'\'', {0,0,1,0,0,0}},  
  {'-', {0,0,1,0,0,1}},   
  {'#', {0,0,1,1,1,1}}    
};
const int numSpecialChars = sizeof(specialChars) / sizeof(BrailleChar);


int brailleNumbers[10][6] = {
  {0,1,0,1,1,0},  
  {1,0,0,0,0,0},  
  {1,1,0,0,0,0},  
  {1,0,0,1,0,0},  
  {1,0,0,1,1,0},  
  {1,0,0,0,1,0},  
  {1,1,0,1,0,0},  
  {1,1,0,1,1,0},  
  {1,1,0,0,1,0},  
  {0,1,0,1,0,0}   
};


String inputWord = "";

const int limitSwitchPins[] = {13,12,11,10,9,8};
const int numSwitches = 6;

bool switchPressed[6] = {false, false, false, false, false, false};


struct BrailleLetter {
  byte pattern;
  char letter;
};


BrailleLetter brailleMap[] = {
  // Letters
  {0b000001, 'A'}, {0b000011, 'B'}, {0b001001, 'C'}, {0b011001, 'D'},
  {0b010001, 'E'}, {0b001011, 'F'}, {0b011011, 'G'}, {0b010011, 'H'},
  {0b001010, 'I'}, {0b011010, 'J'}, {0b000101, 'K'}, {0b000111, 'L'},
  {0b001101, 'M'}, {0b011101, 'N'}, {0b010101, 'O'}, {0b001111, 'P'},
  {0b011111, 'Q'}, {0b010111, 'R'}, {0b001110, 'S'}, {0b011110, 'T'},
  {0b100101, 'U'}, {0b100111, 'V'}, {0b111010, 'W'}, {0b101101, 'X'},
  {0b111101, 'Y'}, {0b110101, 'Z'},
  // Punctuation and Special
  {0b000000, ' '},  
  {0b000010, ','},  
  {0b000110, ';'},  
  {0b010010, ':'},  
  {0b110010, '.'},  
  {0b110110, '?'},  
  {0b010110, '!'}, 
  {0b000100, '\''}, 
  {0b100100, '-'},  
  {0b111100, '#'}   
};
const int brailleMapSize = sizeof(brailleMap) / sizeof(BrailleLetter);


// === INITIALIZATION ===
void setup() {
  Serial.begin(9600);       
  nrfSerial.begin(9600);     // nRF communication
  
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Braille System");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  
  pinMode(modeButtonPin, INPUT_PULLUP);
  pinMode(readerButtonPin, INPUT_PULLUP);
  for (int i = 0; i < numSwitches; i++) pinMode(limitSwitchPins[i], INPUT_PULLUP);


  Serial.println("System starting...");
  resetServosToZero();
  Serial.println("Servos reset.");


  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Mode: PRINT-NRF");
  lcd.setCursor(0, 1);
  lcd.print("Ready!");
  
  Serial.println("Mode: PRINTER (nRF Input)");
  attachAllServos();
  detachAllServos();
}

void loop() {
  handleModeToggle();
  handlePrinterSubModeToggle();
  
  if (!STATE) {
    monitorLimitSwitches();
  }


  if (STATE) {
    printerMode();
  } else {
    readerMode();
  }
}


void handleModeToggle() {
  int reading = digitalRead(modeButtonPin);
  if (reading == LOW && lastButtonState == HIGH && (millis() - lastDebounceTime) > debounceDelay) {
    STATE = !STATE;
    lastDebounceTime = millis();
    
    clearSerialBuffer();
    clearNrfBuffer();
    
    if (!STATE) {
      for (int i = 0; i < numSwitches; i++) {
        switchPressed[i] = false;
      }
      resetServosToZero();
    }
    
    lcd.clear();
    lcd.setCursor(0, 0);
    if (STATE) {
      if (PRINTER_INPUT_MODE) {
        lcd.print("Mode: PRINT-NRF");
        Serial.println("Mode: PRINTER (nRF)");
      } else {
        lcd.print("Mode: PRINT-SER");
        Serial.println("Mode: PRINTER (Serial)");
      }
    } else {
      lcd.print("Mode: READER");
      Serial.println("Mode: READER");
    }
    lcd.setCursor(0, 1);
    lcd.print("Ready!");
  }
  lastButtonState = reading;
}

void handlePrinterSubModeToggle() {
  if (STATE) {
    int reading = digitalRead(readerButtonPin);
    if (reading == LOW && lastReaderButtonState == HIGH && (millis() - lastReaderDebounceTime) > debounceDelay) {
      PRINTER_INPUT_MODE = !PRINTER_INPUT_MODE;
      lastReaderDebounceTime = millis();
      
      clearSerialBuffer();
      clearNrfBuffer();
      
      lcd.clear();
      lcd.setCursor(0, 0);
      if (PRINTER_INPUT_MODE) {
        lcd.print("Mode: PRINT-NRF");
        Serial.println("Input: nRF");
      } else {
        lcd.print("Mode: PRINT-SER");
        Serial.println("Input: Serial Monitor");
      }
      lcd.setCursor(0, 1);
      lcd.print("Ready!");
    }
    lastReaderButtonState = reading;
  }
}


void clearSerialBuffer() {
  while (Serial.available() > 0) {
    Serial.read();
  }
}


void clearNrfBuffer() {
  while (nrfSerial.available() > 0) {
    nrfSerial.read();
  }
}


void monitorLimitSwitches() {
  for (int i = 0; i < numSwitches; i++) {
    if (digitalRead(limitSwitchPins[i]) == LOW) {
      switchPressed[i] = true;
    }
  }
}

void printerMode() {
  bool dataAvailable = false;
  
  if (PRINTER_INPUT_MODE) {
    if (nrfSerial.available()) {
      inputWord = nrfSerial.readStringUntil('\n');
      dataAvailable = true;
      Serial.print("nRF received: ");
      Serial.println(inputWord);
    }
    clearSerialBuffer();
    
  } else {
    if (Serial.available()) {
      inputWord = Serial.readStringUntil('\n');
      dataAvailable = true;
      Serial.print("Serial received: ");
      Serial.println(inputWord);
    }
    clearNrfBuffer();
  }
  
  if (dataAvailable) {
    processAndPrint();
  }
}


void processAndPrint() {
  inputWord.trim();
  
  if (inputWord.length() == 0) {
    return;
  }
  
  Serial.print("Printing: ");
  Serial.println(inputWord);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Printing:");
  lcd.setCursor(0, 1);
  if (inputWord.length() <= 16) {
    lcd.print(inputWord);
  } else {
    lcd.print(inputWord.substring(0, 16));
  }


  for (int i = 0; i < inputWord.length(); i++) {
    char ch = inputWord[i];
    bool charDisplayed = false;
    
    if (ch >= 'a' && ch <= 'z') {
      int index = ch - 'a';
      displayBraille(brailleLetters[index]);
      charDisplayed = true;
    }
    else if (ch >= 'A' && ch <= 'Z') {
      int index = ch - 'A';
      displayBraille(brailleLetters[index]);
      charDisplayed = true;
    }
    else if (ch >= '0' && ch <= '9') {
      displayBraille(specialChars[9].pattern);  // Number indicator
      delay(400);
      int digit = ch - '0';
      displayBraille(brailleNumbers[digit]);
      charDisplayed = true;
    }
    else {
      for (int j = 0; j < numSpecialChars; j++) {
        if (specialChars[j].character == ch) {
          displayBraille(specialChars[j].pattern);
          charDisplayed = true;
          break;
        }
      }
    }
    
    if (charDisplayed) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Char: ");
      lcd.print(ch);
      lcd.setCursor(0, 1);
      lcd.print("Progress: ");
      lcd.print(i + 1);
      lcd.print("/");
      lcd.print(inputWord.length());
      
      Serial.print("Displayed: ");
      Serial.println(ch);
      delay(800);
    }
  }


  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Completed!");
  lcd.setCursor(0, 1);
  if (PRINTER_INPUT_MODE) {
    lcd.print("Awaiting nRF...");
  } else {
    lcd.print("Enter new text");
  }
  
  Serial.println("Done!");
}


void displayBraille(int dots[6]) {
  attachAllServos();
  for (int i = 0; i < 3; i++) servos[i].write(dots[i] ? 120 : 180);
  for (int i = 3; i < 6; i++) servos[i].write(dots[i] ? 60 : 0);
  delay(2000);
  for (int i = 0; i < 3; i++) servos[i].write(180);
  for (int i = 3; i < 6; i++) servos[i].write(0);
  delay(800);
  detachAllServos();
}

void readerMode() {
  clearSerialBuffer();
  clearNrfBuffer();
  
  if (digitalRead(readerButtonPin) == LOW) {
    delay(40);
    if (digitalRead(readerButtonPin) == LOW) {
      byte pattern = 0;
      for (int i = 0; i < numSwitches; i++) {
        if (switchPressed[i]) pattern |= (1 << i);
      }


      char letter = '?';
      for (int i = 0; i < brailleMapSize; i++) {
        if (brailleMap[i].pattern == pattern) {
          letter = brailleMap[i].letter;
          break;
        }
      }


      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Detected: ");
      lcd.print(letter);
      lcd.setCursor(0, 1);
      lcd.print("Pattern: ");
      for (int i = 0; i < numSwitches; i++)
        lcd.print((pattern & (1 << i)) ? '1' : '0');


      Serial.print("Dots: ");
      for (int i = 0; i < numSwitches; i++)
        Serial.print((pattern & (1 << i)) ? '1' : '0');
      Serial.print(" -> ");
      Serial.println(letter);
      
      delay(3000);
      Serial.println("Resetting...");
      resetServosToZero();
      
      for (int i = 0; i < numSwitches; i++) {
        switchPressed[i] = false;
      }


      while (digitalRead(readerButtonPin) == LOW) delay(10);
      delay(100);
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Mode: READER");
      lcd.setCursor(0, 1);
      lcd.print("Ready!");
    }
  }
}


void resetServosToZero() {
  attachAllServos();
  for (int i = 0; i < 3; i++) servos[i].write(180);
  for (int i = 3; i < 6; i++) servos[i].write(0);
  delay(500);
  detachAllServos();
}


void liftServosTo60() {
  attachAllServos();
  for (int pos = 0; pos <= 60; pos++) {
    for (int i = 3; i < 6; i++) servos[i].write(pos);
    delay(10);
  }
  
  for (int pos = 180; pos >= 120; pos--) {
    for (int i = 0; i < 3; i++) servos[i].write(pos);
    delay(10);
  }


  detachAllServos();
}


void attachAllServos() {
  for (int i = 0; i < 6; i++) servos[i].attach(servoPins[i]);
}


void detachAllServos() {
  for (int i = 0; i < 6; i++) servos[i].detach();
}