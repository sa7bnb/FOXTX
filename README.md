# Fox Transmitter - Amateur Radio Morse Code Transmitter

**Designed and developed by SA7BNB**

A compact, web-controlled Morse code transmitter perfect for fox hunting, amateur radio events, and educational purposes. Built using affordable components from AliExpress.

## Overview

The Fox Transmitter is a WiFi-controlled Morse code transmitter that operates on the 433MHz amateur radio band. It features a web-based interface for easy configuration and control, making it ideal for remote operation during fox hunting activities or as an automated beacon.

## Hardware Requirements

### Main Components
- **Wemos D1 Mini Pro** (ESP8266-based development board) - Available on AliExpress
- **CC1101 RF Module** (433MHz transceiver) - Available on AliExpress
- **LED** (5mm, any color) with 220-470Ω resistor
- **Breadboard or PCB** for connections
- **Jumper wires**

### Component Sources
Both the Wemos D1 Mini Pro and CC1101 module can be purchased inexpensively from AliExpress, making this an affordable project for amateur radio operators worldwide.

## Pinout and Wiring

### CC1101 to Wemos D1 Mini Pro Connections

| CC1101 Pin | Wemos D1 Mini Pro Pin | GPIO | Description |
|------------|----------------------|------|-------------|
| VCC        | 3.3V                 | -    | Power supply (3.3V only!) |
| GND        | GND                  | -    | Ground |
| CSN        | D8                   | 15   | Chip Select (SPI) |
| SCK        | D5                   | 14   | Serial Clock (SPI) |
| MOSI       | D7                   | 13   | Master Out Slave In (SPI) |
| MISO       | D6                   | 12   | Master In Slave Out (SPI) |
| GDO0       | D2                   | 4    | General Digital Output 0 |

### LED Connection

| LED | Wemos D1 Mini Pro | Description |
|-----|-------------------|-------------|
| Anode (+) | D1 via 220Ω resistor | Morse code indicator |
| Cathode (-) | GND | Ground |

### Important Notes
- **Use 3.3V only** for the CC1101 module - 5V will damage it
- Ensure solid connections for reliable SPI communication
- Keep wiring as short as possible to minimize interference

## Software Installation

### Prerequisites
- Arduino IDE (1.8.19 or later)
- ESP8266 Board Package
- Required Libraries:
  - `ELECHOUSE_CC1101_SRC_DRV` (CC1101 control)
  - `ESP8266WiFi` (Built-in)
  - `ESP8266WebServer` (Built-in)
  - `EEPROM` (Built-in)

### Installation Steps
1. Install the Arduino IDE and ESP8266 board package
2. Install the ELECHOUSE_CC1101_SRC_DRV library via Library Manager
3. Upload the Fox Transmitter code to your Wemos D1 Mini Pro
4. The device will create a WiFi access point on first boot

## Operation

### Initial Setup
1. Power on the Fox Transmitter
2. Connect to the WiFi network: **FOXTX**
3. Password: **FOXTX1234**
4. Open a web browser and navigate to: **http://192.168.4.1**

### Web Interface Features

#### Main Controls
- **Start/Stop Transmission**: Control automatic Morse code transmission
- **Test Transmission**: Send a single "TEST" message
- **Carrier Test**: Generate unmodulated carrier for frequency adjustment

#### Configuration Settings
- **Frequency**: Adjustable from 400-500 MHz (step: 0.000001 MHz)
- **Dit Length**: Morse code timing (50-1000 ms)
- **Interval**: Time between automatic transmissions (0.01+ minutes)
- **Message**: Custom text to transmit (max 100 characters, A-Z, 0-9, spaces)

#### Advanced Features
- **Real-time Frequency Adjustment**: Fine-tune frequency during carrier test
  - ±1 kHz, ±100 Hz, ±10 Hz, ±1 Hz steps
- **EEPROM Storage**: All settings automatically saved and restored
- **LED Indicator**: Visual feedback of transmission activity

### LED Status Indicators
- **Off**: Inactive/standby
- **Morse Pattern**: Flashes in sync with transmitted Morse code
- **Slow Blink (0.5s)**: Carrier test mode active

## Features

### Automatic Operation
- Programmable transmission intervals
- Persistent settings storage in EEPROM
- Automatic startup with saved configuration
- No manual intervention required after setup

### Web-Based Control
- Mobile-friendly responsive interface
- Real-time status updates
- No special software required - any web browser works
- Multiple device access to same transmitter

### Frequency Management
- Precise frequency control with 1 Hz resolution
- Wide frequency range (400-500 MHz)
- Live frequency adjustment during carrier test
- Maximum power output (12 dBm) for amateur radio use

### Morse Code Engine
- Complete A-Z, 0-9 character support
- Standard ITU timing ratios
- Adjustable speed control
- Professional morse code formatting

## Technical Specifications

### RF Performance
- **Frequency Range**: 400-500 MHz
- **Output Power**: 12 dBm (≈16 mW) maximum
- **Modulation**: OOK (On-Off Keying)
- **Frequency Resolution**: 1 Hz steps
- **Band**: Primarily 70cm amateur radio band (433 MHz)

### Power Requirements
- **Input Voltage**: 5V USB or 3.3-5V DC
- **Current Consumption**: ~100-200mA during transmission
- **Standby Current**: ~50mA

### Operating Specifications
- **Morse Speed Range**: 50-1000ms dit length
- **Message Length**: Up to 100 characters
- **Transmission Interval**: 0.01 minutes to unlimited
- **Memory**: Non-volatile EEPROM storage

## Usage Examples

### Fox Hunting Setup
1. Set frequency to agreed hunting frequency (e.g., 433.920 MHz)
2. Configure message: "FOX FOX DE [YOURCALL] K"
3. Set interval to 60 seconds
4. Deploy transmitter in field and start transmission

### Beacon Operation
1. Configure location-specific message
2. Set longer interval (5-10 minutes)
3. Use carrier test to precisely adjust frequency
4. Start automatic operation

### Educational Demonstrations
1. Use short dit length for clear demonstration
2. Set simple messages like "CQ CQ TEST"
3. Use test transmission for single demonstrations
4. LED provides visual morse code learning aid

## Troubleshooting

### Common Issues

#### No WiFi Access Point
- Check power connections
- Verify Wemos D1 Mini Pro is properly programmed
- Look for "FOXTX" network name

#### No RF Output
- Verify CC1101 wiring, especially power (3.3V only)
- Check SPI connections (CSN, SCK, MOSI, MISO)
- Ensure frequency is within amateur radio privileges

#### Settings Not Saved
- Check EEPROM functionality in serial monitor
- Use "Clear EEPROM" if corruption suspected
- Verify all settings are within valid ranges

#### Web Interface Issues
- Ensure connected to FOXTX network
- Try different web browser
- Check IP address: 192.168.4.1
- Manual page refresh if status seems outdated

### Serial Monitor Debugging
Connect via USB and monitor at 115200 baud for detailed status information including:
- EEPROM operations
- WiFi connection status
- Transmission activity
- Error messages

## Legal Considerations

### Amateur Radio License Required
Operation of this transmitter requires a valid amateur radio license in most countries. Check local regulations before use.

### Frequency Restrictions
- Ensure operation within amateur radio bands
- Respect local band plans and restrictions
- Consider other users before deploying automated transmissions
- Use minimum necessary power

### International Use
The Fox Transmitter is designed for global amateur radio use with English interface and standard amateur radio protocols.

## Development and Customization

### Source Code
The complete source code is available and can be modified for specific requirements:
- Custom frequency ranges
- Different modulation schemes
- Additional message protocols
- Extended web interface features

### Hardware Modifications
- External antenna connections
- Power amplifier additions
- Battery operation enhancements
- Weather-resistant enclosures

## License and Credits

**Designed and developed by SA7BNB**

This project is open source and available for amateur radio operators worldwide. The hardware uses affordable components readily available from AliExpress, making it accessible for educational and emergency communication purposes.

### Acknowledgments
- ESP8266 Arduino Core community
- ELECHOUSE CC1101 library developers
- Amateur radio community for testing and feedback


**73, SA7BNB**

*Amateur Radio - Experimenting with Electronics Since [Year]*
