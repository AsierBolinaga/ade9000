#include <Arduino.h>
#include "ade9000.h"
#include "config.h"

// Create ADE9000 instance
ADE9000 meter(ADE9000_CS_PIN, ADE9000_RESET_PIN);

// Timing variables
unsigned long lastSampleTime = 0;

void setup() {
    // Initialize serial communication
    Serial.begin(SERIAL_BAUD);
    while (!Serial && millis() < 3000) {
        ; // Wait for serial port to connect (max 3 seconds)
    }
    
    Serial.println("ADE9000 Data Extractor for i.MXRT1062");
    Serial.println("======================================");
    
    // Initialize ADE9000
    Serial.print("Initializing ADE9000...");
    if (meter.begin()) {
        Serial.println("OK");
    } else {
        Serial.println("FAILED");
        while (1) {
            delay(1000);
        }
    }
    
    // Optional: Set calibration gains
    // meter.setVoltageGain(1.0);
    // meter.setCurrentGain(1.0);
    
    Serial.println("Starting data acquisition...");
    Serial.println();
}

void loop() {
    unsigned long currentTime = millis();
    
    // Check if it's time to sample
    if (currentTime - lastSampleTime >= SAMPLE_PERIOD_MS) {
        lastSampleTime = currentTime;
        
        // Read electrical parameters
        float voltage = meter.readVoltageRMS();
        float current = meter.readCurrentRMS();
        float activePower = meter.readActivePower();
        float reactivePower = meter.readReactivePower();
        float apparentPower = meter.readApparentPower();
        float powerFactor = meter.readPowerFactor();
        float frequency = meter.readFrequency();
        
        // Format and send data
        Serial.println("--- Energy Meter Data ---");
        Serial.print("Timestamp: ");
        Serial.print(currentTime);
        Serial.println(" ms");
        
        Serial.print("Voltage RMS: ");
        Serial.print(voltage);
        Serial.println(" V");
        
        Serial.print("Current RMS: ");
        Serial.print(current);
        Serial.println(" A");
        
        Serial.print("Active Power: ");
        Serial.print(activePower);
        Serial.println(" W");
        
        Serial.print("Reactive Power: ");
        Serial.print(reactivePower);
        Serial.println(" VAR");
        
        Serial.print("Apparent Power: ");
        Serial.print(apparentPower);
        Serial.println(" VA");
        
        Serial.print("Power Factor: ");
        Serial.println(powerFactor);
        
        Serial.print("Frequency: ");
        Serial.print(frequency);
        Serial.println(" Hz");
        
        Serial.println();
        
        // Alternative: Send as JSON format
        // Serial.print("{");
        // Serial.print("\"timestamp\":");
        // Serial.print(currentTime);
        // Serial.print(",\"voltage\":");
        // Serial.print(voltage);
        // Serial.print(",\"current\":");
        // Serial.print(current);
        // Serial.print(",\"power\":");
        // Serial.print(activePower);
        // Serial.print(",\"pf\":");
        // Serial.print(powerFactor);
        // Serial.println("}");
    }
}
