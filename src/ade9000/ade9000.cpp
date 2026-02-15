#include <Arduino.h>
#include <SPI.h>
#include "ade9000.h"
#include "config.h"

ADE9000::ADE9000(uint8_t cs_pin, uint8_t reset_pin) {
    _cs_pin = cs_pin;
    _reset_pin = reset_pin;
}

bool ADE9000::begin(void) {
    // Initialize CS and Reset pins
    pinMode(_cs_pin, OUTPUT);
    digitalWrite(_cs_pin, HIGH);
    
    pinMode(_reset_pin, OUTPUT);
    digitalWrite(_reset_pin, HIGH);
    
    // Initialize SPI
    SPI.begin();
    
    // Reset the ADE9000
    reset();
    
    // Wait for device to be ready
    delay(100);
    
    // Configure ADE9000 for basic operation
    // In a real implementation, you would:
    // 1. Set up PGA gains
    // 2. Configure integration period
    // 3. Enable necessary interrupts
    // 4. Calibrate voltage and current channels
    
    return true;
}

void ADE9000::reset(void) {
    digitalWrite(_reset_pin, LOW);
    delay(10);
    digitalWrite(_reset_pin, HIGH);
    delay(100);
}

void ADE9000::spiWrite(uint16_t address, const uint8_t* data, uint8_t length) {
    digitalWrite(_cs_pin, LOW);
    
    // Send address (write bit = 0)
    SPI.transfer16((address << 4) & 0xFFF0);
    
    // Send data
    for (uint8_t i = 0; i < length; i++) {
        SPI.transfer(data[i]);
    }
    
    digitalWrite(_cs_pin, HIGH);
}

void ADE9000::spiRead(uint16_t address, uint8_t* data, uint8_t length) {
    digitalWrite(_cs_pin, LOW);
    
    // Send address (read bit = 1)
    SPI.transfer16(((address << 4) & 0xFFF0) | 0x0008);
    
    // Read data
    for (uint8_t i = 0; i < length; i++) {
        data[i] = SPI.transfer(0x00);
    }
    
    digitalWrite(_cs_pin, HIGH);
}

uint16_t ADE9000::readRegister16(uint16_t address) {
    uint8_t data[2];
    spiRead(address, data, 2);
    return ((uint16_t)data[0] << 8) | data[1];
}

uint32_t ADE9000::readRegister32(uint16_t address) {
    uint8_t data[4];
    spiRead(address, data, 4);
    return ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) | 
           ((uint32_t)data[2] << 8) | data[3];
}

void ADE9000::writeRegister16(uint16_t address, uint16_t value) {
    uint8_t data[2];
    data[0] = (value >> 8) & 0xFF;
    data[1] = value & 0xFF;
    spiWrite(address, data, 2);
}

void ADE9000::writeRegister32(uint16_t address, uint32_t value) {
    uint8_t data[4];
    data[0] = (value >> 24) & 0xFF;
    data[1] = (value >> 16) & 0xFF;
    data[2] = (value >> 8) & 0xFF;
    data[3] = value & 0xFF;
    spiWrite(address, data, 4);
}

float ADE9000::readVoltageRMS(void) {
    uint32_t vrms_raw = readRegister32(ADE9000_REG_VRMS);
    // Convert raw value to voltage (calibration needed)
    // This is a simplified conversion
    float voltage = (float)vrms_raw / 1000000.0f;
    return voltage;
}

float ADE9000::readCurrentRMS(void) {
    uint32_t irms_raw = readRegister32(ADE9000_REG_IRMS);
    // Convert raw value to current (calibration needed)
    float current = (float)irms_raw / 1000000.0f;
    return current;
}

float ADE9000::readActivePower(void) {
    uint32_t power_raw = readRegister32(ADE9000_REG_AWATTOS);
    // Convert raw value to watts (calibration needed)
    float power = (float)((int32_t)power_raw) / 1000.0f;
    return power;
}

float ADE9000::readReactivePower(void) {
    uint32_t var_raw = readRegister32(ADE9000_REG_AVAROS);
    // Convert raw value to VAR (calibration needed)
    float reactive_power = (float)((int32_t)var_raw) / 1000.0f;
    return reactive_power;
}

float ADE9000::readApparentPower(void) {
    float active = readActivePower();
    float reactive = readReactivePower();
    return sqrt(active * active + reactive * reactive);
}

float ADE9000::readPowerFactor(void) {
    float active = readActivePower();
    float apparent = readApparentPower();
    if (apparent > 0) {
        return active / apparent;
    }
    return 0.0f;
}

float ADE9000::readFrequency(void) {
    // Placeholder - would read from frequency register
    return 60.0f;
}

void ADE9000::setVoltageGain(float gain) {
    // Convert gain to register value
    uint16_t gain_val = (uint16_t)(gain * 1000.0f);
    writeRegister16(ADE9000_REG_AVGAIN, gain_val);
}

void ADE9000::setCurrentGain(float gain) {
    // Convert gain to register value
    uint16_t gain_val = (uint16_t)(gain * 1000.0f);
    writeRegister16(ADE9000_REG_AIGAIN, gain_val);
}
