  Summary

  1. Implementation Specification

  Board Details:
  ┌────────────────┬────────────────────────────────────────┐
  │   Component    │             Specification              │
  ├────────────────┼────────────────────────────────────────┤
  │ GPIO Expander  │ MCP23017                               │
  ├────────────────┼────────────────────────────────────────┤
  │ Output MOSFETs │ AO3406 (30V, 3.6A peak per channel)    │
  ├────────────────┼────────────────────────────────────────┤
  │ I2C Address    │ 0x20 (default), configurable 0x20-0x27 │
  ├────────────────┼────────────────────────────────────────┤
  │ Channels       │ 8 solenoid outputs on Port A           │
  ├────────────────┼────────────────────────────────────────┤
  │ Protection     │ Built-in flyback diodes                │
  └────────────────┴────────────────────────────────────────┘
  Key Architecture:
  - SolenoidDriver class with channel control, MIDI mapping, and safety features
  - Automatic timeout protection (max on-time to prevent overheating)
  - Scalable to 8 boards (64 channels) via I2C address jumpers
  - For 88-key piano: use multiple I2C buses (Wire + Wire1)

  ---
  2. Hardware Connection Manual

  Wiring Summary:
  ┌──────────────┬─────────────────┐
  │  Teensy 4.1  │ Solenoid Driver │
  ├──────────────┼─────────────────┤
  │ Pin 18 (SDA) │ SDA             │
  ├──────────────┼─────────────────┤
  │ Pin 19 (SCL) │ SCL             │
  ├──────────────┼─────────────────┤
  │ 3.3V         │ Vcc             │
  ├──────────────┼─────────────────┤
  │ GND          │ GND             │
  └──────────────┴─────────────────┘
  Power Architecture:
  - Logic power: 3.3V from Teensy USB
  - Solenoid power: Separate 3-24V DC supply to V+/GND terminals
  - Common ground between all components

  Critical Safety Notes:
  - Teensy 4.1 is NOT 5V tolerant - always use 3.3V
  - Connect grounds first before any other wiring
  - Keep solenoid power OFF until wiring verified

  ---
  3. Test Program

  Required platformio.ini update:
  lib_deps =
      adafruit/Adafruit MCP23017 Arduino Library@^2.3.0

  Test program features:
  - I2C bus scan to detect devices
  - MCP23017 initialization and verification
  - Sequential activation of all 8 channels (100ms each)
  - All-channels simultaneous test
  - Interactive serial commands: r (re-run cycle), a (all channels), s (scan)