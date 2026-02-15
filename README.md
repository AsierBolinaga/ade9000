# ADE9000 Data Extractor

Accurate ADE9000 data extraction and transmission firmware for the i.MXRT1062 microcontroller.

## Overview

This firmware enables accurate energy metering data extraction from the Analog Devices ADE9000 energy metering IC using the NXP i.MXRT1062 microcontroller (ARM Cortex-M7). The system reads electrical parameters and transmits them via serial interface.

## Features

- **Real-time Energy Monitoring**: Continuous measurement of electrical parameters
- **ADE9000 Driver**: Complete SPI-based driver for ADE9000 communication
- **Data Extraction**: Reads voltage, current, power, power factor, and frequency
- **Serial Transmission**: Outputs data via USB serial interface
- **Configurable Sampling**: Adjustable sampling rate and data format

## Hardware Requirements

- **Microcontroller**: i.MXRT1062 (Teensy 4.0/4.1 or compatible board)
- **Energy Meter IC**: ADE9000 from Analog Devices
- **Interface**: SPI connection between i.MXRT1062 and ADE9000

### Pin Configuration

| ADE9000 Pin | i.MXRT1062 Pin | Description |
|-------------|----------------|-------------|
| CS          | Pin 10         | Chip Select |
| MOSI        | Pin 11         | SPI MOSI    |
| MISO        | Pin 12         | SPI MISO    |
| SCK         | Pin 13         | SPI Clock   |
| RESET       | Pin 9          | Reset       |

## Software Requirements

- [PlatformIO](https://platformio.org/) - Build system and IDE integration
- [Arduino Framework](https://www.arduino.cc/) - For Teensy support

## Building and Flashing

### Using PlatformIO

1. **Install PlatformIO**: Follow the [installation guide](https://platformio.org/install)

2. **Clone the repository**:
   ```bash
   git clone https://github.com/AsierBolinaga/ade9000.git
   cd ade9000
   ```

3. **Build the firmware**:
   ```bash
   pio run
   ```

4. **Upload to board**:
   ```bash
   pio run --target upload
   ```

5. **Monitor serial output**:
   ```bash
   pio device monitor
   ```

## Configuration

Edit `include/config.h` to adjust:

- **Pin assignments**: Change CS, RESET, and other pin numbers
- **SPI speed**: Adjust ADE9000_SPI_SPEED (default: 2 MHz)
- **Sampling rate**: Modify SAMPLE_PERIOD_MS (default: 1000 ms)
- **Serial baud rate**: Change SERIAL_BAUD (default: 115200)

## Usage

After uploading the firmware:

1. Open a serial terminal at 115200 baud
2. The device will initialize and begin outputting data
3. Data is printed every second (configurable) in human-readable format

### Sample Output

```
ADE9000 Data Extractor for i.MXRT1062
======================================
Initializing ADE9000...OK
Starting data acquisition...

--- Energy Meter Data ---
Timestamp: 1000 ms
Voltage RMS: 230.5 V
Current RMS: 2.45 A
Active Power: 564.2 W
Reactive Power: 12.3 VAR
Apparent Power: 564.3 VA
Power Factor: 0.999
Frequency: 60.0 Hz
```

## Project Structure

```
ade9000/
├── include/
│   ├── config.h          # Configuration parameters
│   └── ade9000.h         # ADE9000 driver header
├── src/
│   ├── main.cpp          # Main application
│   └── ade9000/
│       └── ade9000.cpp   # ADE9000 driver implementation
├── platformio.ini        # PlatformIO configuration
└── README.md            # This file
```

## Calibration

For accurate measurements, calibration is required:

1. Apply known voltage and current to the meter
2. Use `setVoltageGain()` and `setCurrentGain()` methods to adjust
3. The conversion formulas in the driver may need adjustment based on your hardware

## Extending the Firmware

### Adding JSON Output

Uncomment the JSON output section in `src/main.cpp` to send data in JSON format.

### Adding Network Transmission

Integrate Ethernet or WiFi libraries to transmit data over the network instead of serial.

### Multi-Channel Support

The ADE9000 supports multiple current channels. Extend the driver to read all channels.

## License

This project is open source. See LICENSE file for details.

## Author

Asier Bolinaga

## Acknowledgments

- Analog Devices for ADE9000 documentation
- NXP for i.MXRT1062 SDK
- PJRC for Teensy platform support
