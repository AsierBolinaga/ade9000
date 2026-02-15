#ifndef CONFIG_H
#define CONFIG_H

// ADE9000 SPI Configuration
#define ADE9000_CS_PIN      10
#define ADE9000_SPI_SPEED   2000000  // 2 MHz
#define ADE9000_RESET_PIN   9

// Serial Communication
#define SERIAL_BAUD         115200

// Sampling Configuration
#define SAMPLE_PERIOD_MS    1000  // Sample every 1 second

// ADE9000 Register Addresses (partial list - key registers)
#define ADE9000_REG_AIGAIN      0x0000
#define ADE9000_REG_AVGAIN      0x0002
#define ADE9000_REG_BIGAIN      0x0004
#define ADE9000_REG_BVGAIN      0x0006
#define ADE9000_REG_AWATTOS     0x0010
#define ADE9000_REG_BWATTOS     0x0012
#define ADE9000_REG_AVAROS      0x0014
#define ADE9000_REG_BVAROS      0x0016
#define ADE9000_REG_IRMS        0x0020
#define ADE9000_REG_VRMS        0x0022

// Data Packet Configuration
#define PACKET_HEADER       0xAA55
#define PACKET_FOOTER       0x55AA

#endif // CONFIG_H
