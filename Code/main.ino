// Blynk configuration
#define BLYNK_TEMPLATE_ID "TMPL35RQv0CVl"
#define BLYNK_TEMPLATE_NAME "SpO2"
#define BLYNK_AUTH_TOKEN "Y5xEbEVGZN05Ve6nhtUI3SkP2cPs7yS6"

#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <MAX30100_PulseOximeter.h>
#include <BlynkSimpleEsp8266.h>
#include "Adafruit_GFX.h"
#include "OakOLED.h"

// WiFi credentials
char ssid[] = "Your_SSID";     // Replace with your WiFi SSID
char pass[] = "Your_PASSWORD"; // Replace with your WiFi password

// DS18B20 configuration
#define ONE_WIRE_BUS D4 // GPIO2 for DS18B20
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// MAX30100 configuration
PulseOximeter pox;

// OLED display configuration
OakOLED oled;

// Timers for periodic updates
uint32_t lastTempRequest = 0;
uint32_t lastPulseOximeterReport = 0;

// Virtual pins for Blynk
#define VIRTUAL_PIN_TEMP V0
#define VIRTUAL_PIN_HEART_RATE V1
#define VIRTUAL_PIN_SPO2 V2

// Beat detection callback
void onBeatDetected() {
    Serial.println("Beat detected!");
}

void setup() {
    // Initialize Serial Monitor
    Serial.begin(115200);
    Serial.println("Starting...");

    // Initialize OLED display
    oled.begin();
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(1);
    oled.setCursor(0, 0);
    oled.println("Initializing...");
    oled.display();

    // Initialize Blynk
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    // Initialize DS18B20 sensor
    sensors.begin();

    // Initialize MAX30100 Pulse Oximeter
    if (!pox.begin()) {
        Serial.println("Failed to initialize MAX30100. Check wiring!");
        while (1);
    }
    pox.setOnBeatDetectedCallback(onBeatDetected);

    Serial.println("Sensors initialized.");
}

void loop() {
    // Run Blynk
    Blynk.run();

    // Update DS18B20 temperature every 2 seconds
    if (millis() - lastTempRequest > 2000) {
        lastTempRequest = millis();
        sensors.requestTemperatures(); // Request temperature
        float temperature = sensors.getTempCByIndex(0) + 4; // Adjust for calibration

        if (temperature != DEVICE_DISCONNECTED_C) {
            Serial.print("Temperature: ");
            Serial.print(temperature);
            Serial.println(" °C");

            // Send temperature to Blynk
            Blynk.virtualWrite(VIRTUAL_PIN_TEMP, temperature);

            // Display temperature on OLED
            oled.clearDisplay();
            oled.setTextSize(1);
            oled.setTextColor(1);
            oled.setCursor(0, 0);
            oled.println("Temp:");
            oled.setTextSize(2);
            oled.setCursor(0, 16);
            oled.println(temperature);
            oled.println("C");
            oled.display();
        } else {
            Serial.println("DS18B20: Error or not connected.");
        }
    }

    // Update MAX30100 data every second
    if (millis() - lastPulseOximeterReport > 1000) {
        lastPulseOximeterReport = millis();
        float heartRate = pox.getHeartRate();
        float spo2 = pox.getSpO2();

        if (heartRate > 30 && heartRate < 250 && spo2 > 70 && spo2 <= 100) {
            Serial.print("Heart rate: ");
            Serial.print(heartRate);
            Serial.print(" bpm / SpO2: ");
            Serial.print(spo2);
            Serial.println(" %");

            // Send heart rate and SpO2 to Blynk
            Blynk.virtualWrite(VIRTUAL_PIN_HEART_RATE, heartRate);
            Blynk.virtualWrite(VIRTUAL_PIN_SPO2, spo2);

            // Display Heart Rate and SpO2 on OLED
            oled.clearDisplay();
            oled.setTextSize(1);
            oled.setTextColor(1);
            oled.setCursor(0, 0);
            oled.println("Heart BPM:");
            oled.setTextSize(2);
            oled.setCursor(0, 16);
            oled.println(heartRate);
            oled.setTextSize(1);
            oled.setCursor(0, 30);
            oled.println("SpO2:");
            oled.setTextSize(2);
            oled.setCursor(0, 45);
            oled.println(spo2);
            oled.println("%");
            oled.display();
        } else {
            Serial.println("MAX30100: Waiting for valid readings...");
        }
    }

    // Continuously update MAX30100
    pox.update();
}
