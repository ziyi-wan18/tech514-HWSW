#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <AccelStepper.h>

// OLED display settings
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)

// Initialize Adafruit SSD1306 display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Adjusted stepper motor setup based on your connection
// Define stepper motor connections and motor interface type
#define motorPin1  1   // Adjust according to your setup
#define motorPin2  2   // Adjust according to your setup
#define motorPin3  3   // Adjust according to your setup
#define motorPin4  4   // Adjust according to your setup
AccelStepper stepper(AccelStepper::FULL4WIRE, motorPin1, motorPin2, motorPin3, motorPin4);

void setup() {
  Serial.begin(9600); // Start serial communication at 9600 bauds
  // Initialize the OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

  delay(2000); // Wait two seconds
  display.setCursor(0, 0);

  // Initialize the stepper motor
  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(500);
}

void loop() {

  display.clearDisplay();
  //display.drawRect(0, 15, 60, 40, WHITE);
  //display.display();
  //delay(2000);

  display.drawRect(0, 15, 60, 40, WHITE);
  Serial.println(F("draw"));
  display.display();
  Serial.println(F("display"));
  delay(2000);

  display.setTextSize(1);
  Serial.println(F("size"));
  display.setTextColor(SSD1306_WHITE); // Draw white text
  Serial.println(F("color"));
  display.setCursor(0,0);     // Start at top-left corner
  Serial.println(F("set"));
  display.println(F("Hello, World!"));
  Serial.println(F("text"));
  display.display(); // Actually draw everything on the display
  Serial.println(F("display"));
  delay(2000);

  // Rotate stepper motor 90 degrees (assuming 400 steps per revolution)
  int stepsPerRevolution = 400; // Change this based on your stepper's specification
  int degreesToRotate = 90;
  int stepsToMove = (stepsPerRevolution / 360) * degreesToRotate;
  stepper.move(stepsToMove);

  // Wait for the stepper to reach the target position
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }

  //delay(2000); // Wait for 2 seconds before the next loop iteration
}