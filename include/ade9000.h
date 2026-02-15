#ifndef ADE9000_H
#define ADE9000_H

#include <stdint.h>
#include <stdbool.h>

// ADE9000 Class for data extraction
class ADE9000 {
public:
    ADE9000(uint8_t cs_pin, uint8_t reset_pin);
    
    // Initialization
    bool begin(void);
    void reset(void);
    
    // Register access
    uint16_t readRegister16(uint16_t address);
    uint32_t readRegister32(uint16_t address);
    void writeRegister16(uint16_t address, uint16_t value);
    void writeRegister32(uint16_t address, uint32_t value);
    
    // Data extraction methods
    float readVoltageRMS(void);
    float readCurrentRMS(void);
    float readActivePower(void);
    float readReactivePower(void);
    float readApparentPower(void);
    float readPowerFactor(void);
    float readFrequency(void);
    
    // Calibration
    void setVoltageGain(float gain);
    void setCurrentGain(float gain);
    
private:
    uint8_t _cs_pin;
    uint8_t _reset_pin;
    
    // SPI communication helpers
    void spiWrite(uint16_t address, const uint8_t* data, uint8_t length);
    void spiRead(uint16_t address, uint8_t* data, uint8_t length);
};

#endif // ADE9000_H
