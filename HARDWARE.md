# Hardware Setup Guide

## ADE9000 to i.MXRT1062 Wiring

### SPI Connection

The ADE9000 communicates via SPI interface. Connect as follows:

```
ADE9000          i.MXRT1062 (Teensy 4.x)
--------         -----------------------
VDD     -------> 3.3V
GND     -------> GND
CS      -------> Pin 10 (or configured CS pin)
MOSI    -------> Pin 11 (MOSI)
MISO    -------> Pin 12 (MISO)
SCLK    -------> Pin 13 (SCK)
RESET   -------> Pin 9 (or configured reset pin)
PM0     -------> 3.3V (SPI mode)
PM1     -------> GND  (SPI mode)
```

### Power Supply

- **VDD**: 3.3V (regulated, stable supply)
- **Current consumption**: ~50mA typical
- **Decoupling**: Add 0.1µF and 10µF capacitors near VDD pins

### Analog Inputs

Connect your voltage and current sensors to the ADE9000 analog inputs according to the ADE9000 datasheet:

- **Voltage inputs**: IAP, IAN (Current channel A)
- **Current inputs**: VAP, VAN (Voltage channel A)
- Additional channels for 3-phase systems

## Teensy 4.0/4.1 Pinout Reference

```
        USB
         |
   ┌─────────────┐
   │  Teensy 4.x │
   │             │
   │          13 │──── SCLK (to ADE9000)
   │          12 │──── MISO (to ADE9000)
   │          11 │──── MOSI (to ADE9000)
   │          10 │──── CS (to ADE9000)
   │           9 │──── RESET (to ADE9000)
   │             │
   │         GND │──── GND (to ADE9000)
   │        3.3V │──── VDD (to ADE9000)
   │             │
   └─────────────┘
```

## Troubleshooting

### Communication Issues

1. **Check SPI connections**: Verify all SPI pins are correctly connected
2. **Check power supply**: Ensure stable 3.3V supply
3. **Check PM0/PM1**: Must be configured for SPI mode
4. **Verify ground**: Common ground between Teensy and ADE9000

### No Data Reading

1. **Reset sequence**: Verify RESET pin is working
2. **SPI speed**: Try reducing SPI_SPEED in config.h
3. **Pull-up resistors**: MISO may need pull-up on some boards

### Incorrect Readings

1. **Calibration needed**: Use setVoltageGain() and setCurrentGain()
2. **Sensor connection**: Verify voltage and current sensors are properly connected
3. **Reference voltage**: Check ADE9000 reference voltage stability

## Safety Notes

⚠️ **WARNING**: The ADE9000 may be connected to high voltage circuits. Always:
- Use proper isolation transformers for voltage sensing
- Use current transformers (CT) for current sensing
- Never connect high voltage directly to the ADE9000
- Follow all electrical safety regulations in your region
- Consult a qualified electrician for AC mains connections
