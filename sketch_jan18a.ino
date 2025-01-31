#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <VL53L0X.h>
#include <BH1750.h>

// OLED screen settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1  // Reset pin, -1 if not used
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// VL53L0X sensor settings
VL53L0X sensor;

// BH1750 sensor settings
BH1750 lightMeter;

// 20-20-20 Rule Timer (2 minutes work and 20 seconds break)
unsigned long startTime = 0;
const unsigned long workDuration = 2 * 60 * 1000;  // 2 minutes (120,000 milliseconds)
const unsigned long breakDuration = 20 * 1000;     // 20 seconds (20,000 milliseconds)
bool onBreak = false;

void setup() {
  // Start serial communication for debugging
  Serial.begin(115200);
  Wire.begin();

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED initialization failed"));
    while (1);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Initialize VL53L0X sensor
  if (!sensor.init()) {
    Serial.println("VL53L0X initialization failed!");
    while (1);  // Stop execution if sensor fails
  }
  sensor.setTimeout(500);  // Set timeout for readings
  Serial.println("VL53L0X initialized successfully");

  // Initialize BH1750 sensor
  if (lightMeter.begin()) {
    Serial.println("BH1750 initialized successfully");
  } else {
    Serial.println("BH1750 initialization failed");
    while (1);  // Stop execution if sensor fails
  }

  startTime = millis(); // Start tracking time
}

void loop() {
  // Get distance reading from VL53L0X
  uint16_t distance = sensor.readRangeSingleMillimeters();
  distance = distance / 10;  // Convert to centimeters

  // Get light level from BH1750
  float lightLevel = lightMeter.readLightLevel();

  // Check if VL53L0X reading was successful
  display.clearDisplay();
  display.setCursor(0, 0);

  // Display Distance Status
  if (sensor.timeoutOccurred()) {
    display.println("VL53L0X Timeout");
  } else {
    display.print("Distance: ");
    display.print(distance);
    display.println(" cm");

    if (distance < 25) {
      display.println("Too Close!");
    } else if (distance >= 25 && distance <= 50) {
      display.println("OK");
    } else {
      display.println("Too Far");
    }
  }

  // Display Light Level Status
  display.setCursor(0, 35);
  display.print("Light: ");
  display.print(lightLevel);
  display.println(" lux");

  if (lightLevel < 100) {
    display.println("Low Light");
  } else if (lightLevel >= 100 && lightLevel <= 500) {
    display.println("OK");
  } else {
    display.println("Too Bright");
  }

  // Implement 20-20-20 Rule (2 minutes work, 20 seconds break)
  unsigned long elapsedTime = millis() - startTime;
  display.setCursor(0, 55);

  if (!onBreak && elapsedTime >= workDuration) {
    display.println("Take a Break!");
    onBreak = true;
    startTime = millis(); // Reset timer for break
  } else if (onBreak && elapsedTime >= breakDuration) {
    display.println("Back to Work!");
    onBreak = false;
    startTime = millis(); // Reset timer for work session
  }

  // Show the data on the OLED screen
  display.display();

  // Delay before the next reading
  delay(500);
}
