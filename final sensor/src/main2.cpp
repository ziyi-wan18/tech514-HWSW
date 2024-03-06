#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define SOIL_MOISTURE_SENSOR_PIN 36  // ESP32-S3 ADC pin connected to Soil Moisture Sensor
#define LIGHT_SENSOR_PIN 35          // ESP32-S3 ADC pin connected to Light Sensor
unsigned long previousMillis = 0;   // Save the last time the display was updated
const long interval = 1000;         // Update interval (milliseconds)

void setup() {
    Serial.begin(115200);
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Check if your display I2C address is 0x3C
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Loop forever
    }
    display.clearDisplay();
    display.setTextSize(1);      
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.println(F("Initializing..."));
    display.display();  // Show initial content

    analogReadResolution(12);  // Set ADC resolution to 12 bits
}

void loop() {
    unsigned long currentMillis = millis();
    
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        int soilMoistureValue = analogRead(SOIL_MOISTURE_SENSOR_PIN);
        int soilMoisturePercent = map(soilMoistureValue, 0, 4095, 0, 100); // Convert to percentage

        int lightLevelValue = analogRead(LIGHT_SENSOR_PIN);
        int lightIntensity = map(lightLevelValue, 0, 4095, 0, 100); // Convert to percentage

        display.clearDisplay();
        display.setCursor(0,0);
        display.setTextSize(1);
        
        display.print(F("Moisture: "));
        display.print(soilMoisturePercent);
        display.println(F("%"));
        
        display.print(F("Light:    "));
        display.print(lightIntensity);
        display.println(F("%"));

        display.display(); // Update display content
    }
}
