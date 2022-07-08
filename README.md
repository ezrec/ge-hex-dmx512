# GE Hex LED+ (as sold by Lowes Hardware in 2022) control.

![Picture of GE LED+](https://github.com/ezrec/ge-hex-dmx512/raw/main/doc/GE_Hex_LED_package.jpg "GE Hex LED+")

## Wire-up:

Remove GE Hex LED control board from housing, and desolder and remove the
8051 controller (U2).

![Picture of GE control PCB](https://github.com/ezrec/ge-hex-dmx512/raw/main/doc/GE_Hex_LED_control_pcb.jpg "GE Hex LED+ Control PCB, modified")

### From ESP32 to 3v to 5v level converter:

- Pin `3.3V` to 3v to 5v level converter pins `VA` and `OE`
- Pin `GND` to 3v to 5v level converter pin `GND`
- Pin `PIN_D` to 3v to 5v level converter pin `A1`
- Pin `PIN_S` to 3v to 5v level converter pin `A2`

### From GE Hex LED control board to 3v to 5v level converter:

- Test point `S` to 3v to 5v level converter pin `B1`
- Test point `DOUT` to 3v to 5v level converter pin `B2`
- Test point `GND` to 3v to 5v level converter pin `GND`
- Test point `V5` to 3v to 5v level converter pin `VB`

NOTE: The GE Hex LED will not supply enough power for the ESP-32 wifi,
      make sure to power the ESP-32 separately via USB!

NOTE: If running more than 6 Hex LEDs, use the GE Hex Led power cable
      included in the set to 'jumper' the first Hex LED to the last one.

## Building Firmware

Required Arduino libraries:

- `ArtnetWifi`, v1.5.1
- `AutoConnect`, v1.4.1
- `FastLED`, v3.5.0

## Usage

- Flash firmware to ESP-32
- After power-on, connect to the "hex-dmx512" AP (no password), and configure the ESP-32 wifi to connect to your local Wifi network.
- Use any DMX-512 Ethernet controller to send DMX-512 packets to your lights!
