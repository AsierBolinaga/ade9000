# Changelog

All notable changes to this project will be documented in this file.

## [1.0.0] - 2026-02-15

### Added
- Initial firmware implementation for ADE9000 data extraction
- i.MXRT1062 microcontroller support (Teensy 4.0/4.1)
- SPI-based ADE9000 driver
- Real-time electrical parameter monitoring
  - Voltage RMS measurement
  - Current RMS measurement
  - Active power calculation
  - Reactive power calculation
  - Apparent power calculation
  - Power factor calculation
  - Frequency measurement
- Serial data transmission at 115200 baud
- Configurable sampling rate
- PlatformIO build system support
- Comprehensive documentation
  - README with setup instructions
  - Hardware wiring guide
  - Configuration examples
- MIT License

### Features
- Read voltage, current, and power measurements from ADE9000
- Transmit data via USB serial interface
- Configurable sampling period (default: 1 second)
- Human-readable text output format
- Optional JSON output format (commented)
- Gain calibration support
