#include <Servo.h>
#include <Stepper.h>
#include <string.h>

// Create 5 servo objects
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;
Servo servo5;

// Define servo pins
int servoPin1 = 3;  // middle finger
int servoPin2 = 5;  // ring finger
int servoPin3 = 6;  // pinky finger
int servoPin4 = 9;  // index finger
int servoPin5 = 10; // thumb finger

// Stepper motor configuration
const int stepsPerRevolution = 2048;
const int stepsFor90Degrees = 512;
Stepper myStepper(stepsPerRevolution, A2, A4, A3, A5);

// Stepper control pins (for manual power control)
const int stepperPin1 = A2;
const int stepperPin2 = A4;
const int stepperPin3 = A3;
const int stepperPin4 = A5;

// Track current stepper position (in steps)
int currentStepperPosition = 0;

// Flag to indicate if hand is busy performing an action
bool handBusy = false;

// Hand signs (6 elements: 5 servo angles + stepper direction)
// Stepper: 0 = no movement, 1 = 90° CW, -1 = 90° CCW
// middle, ring, little, index, thumb
int sign_one[6] = {180, 180, 180, 0, 180, 1};
int sign_two[6] = {0, 180, 180, 0, 180, 0};
int sign_three[6] = {0, 180, 180, 0, 0, 0};
int sign_four[6] = {0, 0, 0, 0, 180, 1};
int sign_five[6] = {0, 0, 0, 0, 0, 0};
int middle_finger[6] = {0, 180, 180, 180, 0, -1};
int sign_a[6] = {180, 180, 180, 180, 60, 0};
int sign_b[6] = {0, 0, 0, 0, 180, 0};
int sign_c[6] = {120, 120, 120, 120, 180, -1};
int sign_d[6] = {180, 180, 180, 0, 180, -1};
int sign_e[6] = {120, 120, 120, 120, 180, 0};
int sign_f[6] = {75, 45, 0, 180, 180, -1};
int sign_g[6] = {180, 180, 180, 80, 120, -1};
int sign_h[6] = {80, 180, 180, 80, 120, 0};
int sign_i[6] = {180, 180, 0, 180, 180, 0};
int sign_j[6] = {180, 180, 0, 180, 180, -1};
int sign_k[6] = {180, 180, 180, 45, 90, 0};
int sign_l[6] = {180, 180, 180, 0, 90, 1};
int sign_m[6] = {0, 0, 0, 180, 180, 0};
int sign_n[6] = {0, 0, 180, 180, 180, 0};
int sign_o[6] = {90, 90, 90, 90, 90, 0};
int sign_p[6] = {180, 180, 180, 45, 90, -1};
int sign_q[6] = {180, 180, 180, 90, 90, -1};
int sign_r[6] = {180, 180, 180, 30, 30, 0};
int sign_s[6] = {180, 180, 180, 180, 180, 0};
int sign_t[6] = {0, 180, 180, 180, 90, 0};
int sign_u[6] = {180, 180, 180, 0, 0, 1};
int sign_v[6] = {180, 180, 180, 0, 0, -1};
int sign_w[6] = {180, 0, 0, 0, 180, 0};
int sign_x[6] = {180, 180, 180, 120, 180, 1};
int sign_y[6] = {180, 180, 0, 180, 0, 1};
int sign_z[6] = {180, 180, 180, 0, 180, -1};
int thumbs_up[6] = {180, 180, 180, 180, 0, 1};
int thumbs_down[6] = {180, 180, 180, 180, 0, -1};
int ok_sign[6] = {0, 0, 0, 90, 90, 0};
int peace[6] = {180, 180, 180, 0, 0, 0};
int rock_on[6] = {180, 180, 0, 0, 180, 1};
int call_me[6] = {180, 180, 0, 180, 0, -1};
int shaka[6] = {180, 180, 180, 180, 0, 0};
int rest[6] = {0, 0, 0, 0, 0, 0};


void returnToRest() {
  // Return stepper to home position (0 degrees)
  if (currentStepperPosition != 0) {
    Serial.print("Returning base to home (");
    Serial.print(-currentStepperPosition);
    Serial.println(" steps)...");
    myStepper.step(-currentStepperPosition);
    currentStepperPosition = 0;
    Serial.println("Base at home position");
  }
  
  delay(300);
  
  // Return all servos to rest position (0 degrees)
  Serial.println("Returning fingers to rest...");
  servo1.write(0);
  servo2.write(0);
  servo3.write(0);
  servo4.write(0);
  servo5.write(0);
  
  delay(500);
  Serial.println("Hand at rest position");
}


void performAction(int arr[6]) {
  // Set hand as busy
  handBusy = true;
  
  Serial.println("=== Starting Action ===");
  
  // Enable stepper motor coils
  pinMode(stepperPin1, OUTPUT);
  pinMode(stepperPin2, OUTPUT);
  pinMode(stepperPin3, OUTPUT);
  pinMode(stepperPin4, OUTPUT);
  
  // Attach servos for movement
  servo1.attach(servoPin1);
  servo2.attach(servoPin2);
  servo3.attach(servoPin3);
  servo4.attach(servoPin4);
  servo5.attach(servoPin5);
  
  delay(100);  // Allow servos to stabilize
  
  // FIRST: Control stepper (base rotation)
  int stepsToMove = 0;
  
  if (arr[5] == 1) {
    Serial.println("Rotating base 90° CW...");
    stepsToMove = stepsFor90Degrees;
    myStepper.step(stepsToMove);
    currentStepperPosition += stepsToMove;
    Serial.println("Base rotation complete");
  } 
  else if (arr[5] == -1) {
    Serial.println("Rotating base 90° CCW...");
    stepsToMove = -stepsFor90Degrees;
    myStepper.step(stepsToMove);
    currentStepperPosition += stepsToMove;
    Serial.println("Base rotation complete");
  }
  
  // Small delay between base rotation and finger movement
  delay(500);
  
  // SECOND: Control servos (finger bending)
  Serial.println("Moving fingers...");
  servo1.write(arr[0]);
  servo2.write(arr[1]);
  servo3.write(arr[2]);
  servo4.write(arr[3]);
  servo5.write(arr[4]);
  
  delay(2000);  // Hold position for 2 seconds
  Serial.println("Sign displayed");
  
  // THIRD: Return to rest position
  delay(500);  // Brief pause before returning
  returnToRest();
  
  // Turn off stepper coils to reduce power
  digitalWrite(stepperPin1, LOW);
  digitalWrite(stepperPin2, LOW);
  digitalWrite(stepperPin3, LOW);
  digitalWrite(stepperPin4, LOW);
  
  // Detach servos to stop PWM signals
  servo1.detach();
  servo2.detach();
  servo3.detach();
  servo4.detach();
  servo5.detach();
  
  delay(100);
  
  // Hand is ready for next command
  handBusy = false;
  Serial.println("=== Action Complete ===\n");
}


int* handsign(String s) {
  s.toLowerCase(); // Convert to lowercase for easier matching
  
  if (s == "1" || s == "one") {
    return sign_one;
  } else if (s == "2" || s == "two") {
    return sign_two;
  } else if (s == "3" || s == "three") {
    return sign_three;
  } else if (s == "4" || s == "four") {
    return sign_four;
  } else if (s == "5" || s == "five") {
    return sign_five;
  } else if (s.indexOf("fuck") >= 0 || s.indexOf("middle") >= 0) {
    return middle_finger;
  } else if (s == "0" || s == "rest" || s == "zero") {
    return rest;
  } else if (s == "a"){
    return sign_a;
  } else if (s == "b"){
    return sign_b;
  } else if (s == "c"){
    return sign_c;
  } else if (s == "d"){
    return sign_d;
  } else if (s == "e"){
    return sign_e;
  } else if (s == "f"){
    return sign_f;
  } else if (s == "g"){
    return sign_g;
  } else if (s == "h"){
    return sign_h;
  } else if (s == "i"){
    return sign_i;
  } else if (s == "j"){
    return sign_j;
  } else if (s == "k"){
    return sign_k;
  } else if (s == "l"){
    return sign_l;
  } else if (s == "m"){
    return sign_m;
  } else if (s == "n"){
    return sign_n;
  } else if (s == "o"){
    return sign_o;
  } else if (s == "p"){
    return sign_p;
  } else if (s == "q"){
    return sign_q;
  } else if (s == "r"){
    return sign_r;
  } else if (s == "s"){
    return sign_s;
  } else if (s == "t"){
    return sign_t;
  } else if (s == "u"){
    return sign_u;
  } else if (s == "v"){
    return sign_v;
  } else if (s == "w"){
    return sign_w;
  } else if (s == "x"){
    return sign_x;
  } else if (s == "y"){
    return sign_y;
  } else if (s == "z"){
    return sign_z;
  } else if (s.indexOf("thumbs up") >= 0 || s.indexOf("thumbsup") >= 0 || s.indexOf("good") >= 0) {
    return thumbs_up;
  } else if (s.indexOf("thumbs down") >= 0 || s.indexOf("thumbsdown") >= 0 || s.indexOf("bad") >= 0) {
    return thumbs_down;
  } else if (s.indexOf("ok") >= 0 || s.indexOf("okay") >= 0) {
    return ok_sign;
  } else if (s.indexOf("peace") >= 0 || s.indexOf("victory") >= 0) {
    return peace;
  } else if (s.indexOf("rock") >= 0) {
    return rock_on;
  } else if (s.indexOf("call") >= 0) {
    return call_me;
  } else if (s.indexOf("shaka") >= 0 || s.indexOf("hang loose") >= 0) {
    return shaka;
  } else if (s.indexOf("hello") >= 0 || s.indexOf("hi") >= 0 || s.indexOf("wave") >= 0) {
    return shaka;
  }
  
  return NULL;
}


void processCommand(String command) {
  command.trim();
  command.toLowerCase();
  
  if (command.length() == 0) {
    return;
  }
  
  Serial.print("Processing command: ");
  Serial.println(command);
  
  int* angles = handsign(command);
  
  if (angles != NULL) {
    performAction(angles);
  } 
  else {
    Serial.println("Unknown hand sign");
  }
}


void setup() {
  // Initialize Serial for UART communication with Arduino Nano
  // Uno RX (D0) <- Nano TX (D1)
  // Uno TX (D1) -> Nano RX (D0)
  Serial.begin(9600);
  
  delay(1000);  // Give time for serial to stabilize
  
  Serial.println("=== Arduino Uno + Servos + Stepper ===");
  
  // Configure stepper motor speed
  myStepper.setSpeed(10);
  
  // Initialize stepper pins and turn off coils immediately
  pinMode(stepperPin1, OUTPUT);
  pinMode(stepperPin2, OUTPUT);
  pinMode(stepperPin3, OUTPUT);
  pinMode(stepperPin4, OUTPUT);
  digitalWrite(stepperPin1, LOW);
  digitalWrite(stepperPin2, LOW);
  digitalWrite(stepperPin3, LOW);
  digitalWrite(stepperPin4, LOW);
  
  // DON'T attach servos here - they'll be attached only when needed
  
  // Initialize position tracking
  currentStepperPosition = 0;
  handBusy = false;
  
  Serial.println("\n=== System Ready ===");
  Serial.println("Waiting for commands from Arduino Nano (UART) or Serial Monitor...");
  Serial.println("Supported: a-z, 0-5, hello, thumbs up, thumbs down, ok, peace, rock, call me, shaka\n");
}


void loop() {
  // Only check for new commands if hand is not busy
  if (!handBusy) {
    // Check for UART data from Arduino Nano
    if (Serial.available() > 0) {
      String input = Serial.readStringUntil('\n');
      input.trim();
      
      // Check if this is a command from Nano (prefixed with "UART_CMD:")
      if (input.startsWith("UART_CMD:")) {
        // Extract the actual command
        String command = input.substring(9);  // Remove "UART_CMD:" prefix
        command.trim();
        
        Serial.print("\n>>> Received from Nano: ");
        Serial.println(command);
        
        // Process the command
        processCommand(command);
      }
      // Check if it's from ">>> Received from RPi:" (Nano's debug message)
      else if (input.startsWith(">>>")) {
        // Just echo it for debugging
        Serial.println(input);
      }
      // Otherwise treat as direct Serial Monitor input
      else if (input.length() > 0) {
        Serial.print("\n>>> Received from Serial Monitor: ");
        Serial.println(input);
        
        // Process the command
        processCommand(input);
      }
    }
  }
  // If hand is busy, ignore all incoming commands
}
