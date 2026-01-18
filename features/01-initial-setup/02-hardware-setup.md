# Hardware Setup Guide

## Table of Contents

1. [Components List](#components-list)
2. [Component Specifications](#component-specifications)
3. [Wiring Diagrams](#wiring-diagrams)
4. [Pin Mapping Tables](#pin-mapping-tables)
5. [Power Supply Specifications](#power-supply-specifications)
6. [I2C Address Configuration](#i2c-address-configuration)
7. [Assembly Instructions](#assembly-instructions)
8. [Safety Checklist](#safety-checklist)
9. [Verification Procedures](#verification-procedures)
10. [Troubleshooting](#troubleshooting)

---

## Components List

### Required Components

| Item | Description | Part Number | Quantity | Source | Notes |
|------|-------------|-------------|----------|--------|-------|
| Microcontroller | Teensy 4.1 | PJRC Teensy 4.1 | 1 | PJRC.com | Main controller |
| Solenoid Driver | Adafruit I2C 8-Ch Solenoid Driver | 6318 | 1+ | Adafruit | Per 8 channels |
| Test Solenoid | 12V or 24V DC Push/Pull Solenoid | Various | 1+ | - | For testing |
| Power Supply | 12V or 24V DC, 2A+ per 8 channels | Various | 1 | - | Match solenoid voltage |
| USB Cable | USB Micro-B | - | 1 | - | For Teensy programming |
| Breadboard | Full-size solderless | - | 1 | - | For prototyping |
| Jumper Wires | Male-Male dupont | - | 10+ | - | Various colors |
| Flyback Diode | 1N4001 or equivalent | 1N4001 | 1 per solenoid | - | If not built into solenoid |

### Recommended Test Equipment

| Item | Purpose | Notes |
|------|---------|-------|
| Digital Multimeter | Voltage/continuity testing | Essential |
| Logic Analyzer | I2C debugging (optional) | Saleae or similar |
| Oscilloscope | Signal verification (optional) | For advanced debugging |
| Bench Power Supply | Adjustable voltage testing | Recommended |

### Optional Components (for 88-key expansion)

| Item | Quantity | Notes |
|------|----------|-------|
| Additional Solenoid Driver Boards | 10 | Total 11 for 88 keys |
| TCA9548A I2C Multiplexer | 0-2 | Alternative to using Wire1 |
| Screw Terminal Blocks | 22+ | For solenoid connections |
| 12AWG Wire | 10m | For power distribution |

---

## Component Specifications

### Teensy 4.1

| Specification | Value |
|---------------|-------|
| Processor | ARM Cortex-M7 @ 600MHz |
| Flash | 8MB |
| RAM | 1MB |
| Operating Voltage | 3.3V |
| I/O Voltage | **3.3V (NOT 5V tolerant!)** |
| I2C Buses | 3 (Wire, Wire1, Wire2) |
| USB | Native USB Host/Device |

**Critical Warning**: Teensy 4.1 I/O pins are **NOT 5V tolerant**. Applying 5V to any GPIO will damage the microcontroller permanently.

### Adafruit I2C to 8 Channel Solenoid Driver (6318)

| Specification | Value |
|---------------|-------|
| GPIO Expander | MCP23017 |
| I2C Address | 0x20 (default), configurable 0x20-0x27 |
| Logic Voltage (Vcc) | 3.3V - 5V |
| Solenoid Voltage (V+) | 3V - 24V DC |
| MOSFET per Channel | AO3406 |
| Max Current per Channel | 3.6A peak, 2.7A continuous |
| RdsOn | 70 milliohm |
| Max Voltage (V+) | 30V DC |
| Channels | 8 (Port A of MCP23017) |
| Extra GPIO | 8 (Port B of MCP23017) |

### MCP23017 GPIO Expander

| Specification | Value |
|---------------|-------|
| Interface | I2C (100kHz, 400kHz, 1.7MHz) |
| GPIO Pins | 16 (2 ports of 8) |
| Operating Voltage | 1.8V - 5.5V |
| Sink/Source Current | 25mA per pin |
| Address Pins | A0, A1, A2 |
| Interrupt Pins | INTA, INTB |

### AO3406 MOSFET (per channel)

| Specification | Value |
|---------------|-------|
| Type | N-Channel |
| Vds (max) | 30V |
| Id (continuous) | 2.7A |
| Id (peak) | 3.6A |
| RdsOn | 70 milliohm @ Vgs=4.5V |
| Vgs (threshold) | 0.5V - 1.3V |
| Package | SOT-23 |

---

## Wiring Diagrams

### Basic Single-Board Setup

```
                                    ┌──────────────────────────────────────────┐
                                    │     Adafruit Solenoid Driver (6318)      │
                                    │                                          │
┌─────────────────────┐             │  ┌─────────────────────────────────────┐ │
│     TEENSY 4.1      │             │  │           MCP23017                  │ │
│                     │   I2C       │  │                                     │ │
│  Pin 18 (SDA) ──────┼─────────────┼──┼─── SDA                              │ │
│                     │             │  │                                     │ │
│  Pin 19 (SCL) ──────┼─────────────┼──┼─── SCL                              │ │
│                     │             │  │                                     │ │
│  3.3V ──────────────┼─────────────┼──┼─── Vcc                              │ │
│                     │             │  │                                     │ │
│  GND ───────────────┼─────────────┼──┼─── GND ──────────────┐              │ │
│                     │             │  │                      │              │ │
└─────────────────────┘             │  │  GPA0-7 ───┐         │              │ │
                                    │  │            │         │              │ │
                                    │  └────────────┼─────────┼──────────────┘ │
                                    │               │         │                │
                                    │      ┌────────┴────┐    │                │
                                    │      │   MOSFETs   │    │                │
                                    │      │  (AO3406)   │    │                │
                                    │      └────────┬────┘    │                │
                                    │               │         │                │
                                    │  V+ ──────────┼─────────┼───────┐        │
                                    │  (12-24V)     │         │       │        │
                                    │               │         │       │        │
                                    │  OUT0-7 ──────┴─────────┼───────┼────────┤
                                    │                         │       │        │
                                    │  GND Terminal ──────────┴───────┘        │
                                    │                                          │
                                    └──────────────────────────────────────────┘
                                                      │         │
                                                      │         │
                                              ┌───────┴───┐     │
                                              │  SOLENOID │     │
                                              │    (+)    │     │
                                              │           │     │
                                              │    (-)────┼─────┘
                                              └───────────┘

                     ┌─────────────────┐
                     │  POWER SUPPLY   │
                     │   (12V/24V)     │
                     │                 │
                     │  (+) ───────────┼───────────► V+ on driver board
                     │                 │
                     │  (-) ───────────┼───────────► GND Terminal on driver board
                     │                 │
                     └─────────────────┘
```

### Detailed Pin-by-Pin Wiring

```
TEENSY 4.1 PINOUT (relevant pins)
═══════════════════════════════════════════════════════════════════════

                    ┌─────────────────────────────────────────┐
                    │              TEENSY 4.1                  │
                    │                                          │
                    │  ┌─────────────────────────────────────┐ │
                    │  │             USB PORT                │ │
                    │  └─────────────────────────────────────┘ │
                    │                                          │
         GND ───────│● GND                         3.3V ●│─────── 3.3V OUT
                    │● 0                             GND ●│
                    │● 1                              23 ●│
                    │● 2                              22 ●│
                    │● 3                              21 ●│
                    │● 4                              20 ●│
                    │● 5                              19 ●│─────── SCL (Wire)
                    │● 6                              18 ●│─────── SDA (Wire)
                    │● 7                              17 ●│─────── SDA (Wire1)
                    │● 8                              16 ●│─────── SCL (Wire1)
                    │● 9                              15 ●│
                    │● 10                             14 ●│
                    │● 11                             13 ●│ LED
                    │● 12                           3.3V ●│
                    │● 3.3V                          GND ●│
                    │● 24 (Wire2 SCL)                    ●│ Vin
                    │● 25 (Wire2 SDA)                    ●│
                    │                                     │
                    └─────────────────────────────────────┘
```

### Solenoid Driver Board Pinout

```
ADAFRUIT I2C SOLENOID DRIVER (6318)
═══════════════════════════════════════════════════════════════════════

                    ┌──────────────────────────────────────────┐
                    │                                          │
    I2C + Power     │    ○ ○ ○ ○ ○ ○                           │
    Header          │   Vc Gn SD SC A0 A1 A2                   │
                    │   c  d  A  L                              │
                    │                                          │
                    │   ┌────────────────────────────────────┐ │
                    │   │         MCP23017 + MOSFETs          │ │
                    │   └────────────────────────────────────┘ │
                    │                                          │
                    │                                          │
    Solenoid        │   ═══╦═══╦═══╦═══╦═══╦═══╦═══╦═══╦═══╗  │
    Outputs         │      ║ 0 ║ 1 ║ 2 ║ 3 ║ 4 ║ 5 ║ 6 ║ 7 ║  │
    (Screw          │   GND╠═══╬═══╬═══╬═══╬═══╬═══╬═══╬═══╣  │
    Terminals)      │      ║   ║   ║   ║   ║   ║   ║   ║   ║  │
                    │   V+ ╠═══╬═══╬═══╬═══╬═══╬═══╬═══╬═══╣  │
                    │      ║   ║   ║   ║   ║   ║   ║   ║   ║  │
                    │   ═══╩═══╩═══╩═══╩═══╩═══╩═══╩═══╩═══╝  │
                    │                                          │
                    │       Extra GPIO Header (Port B)         │
                    │    ○ ○ ○ ○ ○ ○ ○ ○                       │
                    │   G8 G9 G10 G11 G12 G13 G14 G15          │
                    │                                          │
                    └──────────────────────────────────────────┘

Header Pin Labels:
  Vcc = Logic power (3.3V from Teensy)
  Gnd = Ground
  SDA = I2C Data
  SCL = I2C Clock
  A0, A1, A2 = Address select (active LOW with pull-ups)

Screw Terminal Labels:
  V+  = Solenoid power (12-24V DC)
  GND = Solenoid ground (common with DC supply -)
  0-7 = Solenoid output channels
```

### Multi-Board Daisy Chain Configuration

```
88-KEY PIANO WIRING (11 boards)
═══════════════════════════════════════════════════════════════════════

                    ┌─────────────────────┐
                    │     TEENSY 4.1      │
                    │                     │
                    │  Wire (Primary I2C) │
                    │  Pin 18 SDA ────────┼────┬────┬────┬────┬────┬────┬────┬───→
                    │  Pin 19 SCL ────────┼────┼────┼────┼────┼────┼────┼────┼───→
                    │                     │    │    │    │    │    │    │    │
                    │ Wire1 (Secondary)   │    │    │    │    │    │    │    │
                    │  Pin 17 SDA ────────┼────┼────┼────│────│────│────│────│───→
                    │  Pin 16 SCL ────────┼────┼────┼────│────│────│────│────│───→
                    │                     │    │    │    │    │    │    │    │
                    │  3.3V ──────────────┼──┬─┼──┬─┼──┬─┼──┬─┼──┬─┼──┬─┼──┬─┼───→
                    │  GND ───────────────┼──┼─┼──┼─┼──┼─┼──┼─┼──┼─┼──┼─┼──┼─┼───→
                    │                     │  │ │  │ │  │ │  │ │  │ │  │ │  │ │
                    └─────────────────────┘  │ │  │ │  │ │  │ │  │ │  │ │  │ │

Wire Bus (64 channels):                      │ │  │ │  │ │  │ │  │ │  │ │  │ │
                                             │ │  │ │  │ │  │ │  │ │  │ │  │ │
                                         ┌───┴─┴──┴─┴──┴─┴──┴─┴──┴─┴──┴─┴──┴─┴───┐
    Board 0 (0x20) Keys 1-8  ────────────┤ SDA SCL Vcc GND                       │
    Board 1 (0x21) Keys 9-16 ────────────┤ (All boards parallel on same bus)     │
    Board 2 (0x22) Keys 17-24 ───────────┤                                       │
    Board 3 (0x23) Keys 25-32 ───────────┤                                       │
    Board 4 (0x24) Keys 33-40 ───────────┤                                       │
    Board 5 (0x25) Keys 41-48 ───────────┤                                       │
    Board 6 (0x26) Keys 49-56 ───────────┤                                       │
    Board 7 (0x27) Keys 57-64 ───────────┤                                       │
                                         └───────────────────────────────────────┘

Wire1 Bus (24 channels):
                                         ┌───────────────────────────────────────┐
    Board 8  (0x20) Keys 65-72 ──────────┤ SDA SCL Vcc GND                       │
    Board 9  (0x21) Keys 73-80 ──────────┤ (All boards parallel on same bus)     │
    Board 10 (0x22) Keys 81-88 ──────────┤                                       │
                                         └───────────────────────────────────────┘
```

---

## Pin Mapping Tables

### Teensy 4.1 I2C Pin Mapping

| I2C Bus | Instance | SDA Pin | SCL Pin | Max Speed | Pull-ups |
|---------|----------|---------|---------|-----------|----------|
| Wire | Primary | **Pin 18** | **Pin 19** | 1 MHz | External 2.2k-4.7k recommended |
| Wire1 | Secondary | **Pin 17** | **Pin 16** | 1 MHz | External 2.2k-4.7k recommended |
| Wire2 | Tertiary | **Pin 25** | **Pin 24** | 1 MHz | External 2.2k-4.7k recommended |

**Note**: The Adafruit board includes on-board pull-up resistors, so external pull-ups are typically not needed for a single board. For long cable runs or multiple boards, additional pull-ups (4.7k ohm to 3.3V) may improve reliability.

### MCP23017 Register Map (BANK=0, default)

| Register | Address | Read/Write | Purpose |
|----------|---------|------------|---------|
| IODIRA | 0x00 | R/W | Port A direction (1=input, 0=output) |
| IODIRB | 0x01 | R/W | Port B direction |
| IPOLA | 0x02 | R/W | Port A input polarity |
| IPOLB | 0x03 | R/W | Port B input polarity |
| GPINTENA | 0x04 | R/W | Port A interrupt-on-change enable |
| GPINTENB | 0x05 | R/W | Port B interrupt-on-change enable |
| DEFVALA | 0x06 | R/W | Port A default compare value |
| DEFVALB | 0x07 | R/W | Port B default compare value |
| INTCONA | 0x08 | R/W | Port A interrupt control |
| INTCONB | 0x09 | R/W | Port B interrupt control |
| IOCON | 0x0A | R/W | Configuration register |
| IOCON | 0x0B | R/W | Configuration register (mirror) |
| GPPUA | 0x0C | R/W | Port A pull-up resistors |
| GPPUB | 0x0D | R/W | Port B pull-up resistors |
| INTFA | 0x0E | R | Port A interrupt flag |
| INTFB | 0x0F | R | Port B interrupt flag |
| INTCAPA | 0x10 | R | Port A interrupt capture |
| INTCAPB | 0x11 | R | Port B interrupt capture |
| **GPIOA** | **0x12** | **R/W** | **Port A data (solenoid control)** |
| **GPIOB** | **0x13** | **R/W** | **Port B data (extra GPIO)** |
| OLATA | 0x14 | R/W | Port A output latch |
| OLATB | 0x15 | R/W | Port B output latch |

### Solenoid Channel to MCP23017 Pin Mapping

| Channel | MCP23017 Port | Pin | Register Bit | Binary Mask |
|---------|---------------|-----|--------------|-------------|
| 0 | Port A | GPA0 | Bit 0 | 0x01 |
| 1 | Port A | GPA1 | Bit 1 | 0x02 |
| 2 | Port A | GPA2 | Bit 2 | 0x04 |
| 3 | Port A | GPA3 | Bit 3 | 0x08 |
| 4 | Port A | GPA4 | Bit 4 | 0x10 |
| 5 | Port A | GPA5 | Bit 5 | 0x20 |
| 6 | Port A | GPA6 | Bit 6 | 0x40 |
| 7 | Port A | GPA7 | Bit 7 | 0x80 |

### 88-Key Piano Channel Mapping

| MIDI Note | Note Name | Key # | Board # | I2C Bus | Address | Channel |
|-----------|-----------|-------|---------|---------|---------|---------|
| 21 | A0 | 1 | 0 | Wire | 0x20 | 0 |
| 22 | A#0/Bb0 | 2 | 0 | Wire | 0x20 | 1 |
| 23 | B0 | 3 | 0 | Wire | 0x20 | 2 |
| 24 | C1 | 4 | 0 | Wire | 0x20 | 3 |
| 25 | C#1/Db1 | 5 | 0 | Wire | 0x20 | 4 |
| 26 | D1 | 6 | 0 | Wire | 0x20 | 5 |
| 27 | D#1/Eb1 | 7 | 0 | Wire | 0x20 | 6 |
| 28 | E1 | 8 | 0 | Wire | 0x20 | 7 |
| 29 | F1 | 9 | 1 | Wire | 0x21 | 0 |
| ... | ... | ... | ... | ... | ... | ... |
| 60 | C4 (Middle C) | 40 | 4 | Wire | 0x24 | 7 |
| ... | ... | ... | ... | ... | ... | ... |
| 84 | C6 | 64 | 7 | Wire | 0x27 | 7 |
| 85 | C#6/Db6 | 65 | 8 | Wire1 | 0x20 | 0 |
| ... | ... | ... | ... | ... | ... | ... |
| 108 | C8 | 88 | 10 | Wire1 | 0x22 | 7 |

**Formula**:
```
key_number = midi_note - 20  // (1-88 for standard piano)
board_number = (key_number - 1) / 8
channel = (key_number - 1) % 8
i2c_bus = board_number < 8 ? Wire : Wire1
address = 0x20 + (board_number % 8)
```

---

## Power Supply Specifications

### Teensy 4.1 Power

| Parameter | Value | Notes |
|-----------|-------|-------|
| Input Voltage (USB) | 5V | Via USB connector |
| Input Voltage (Vin) | 3.6V - 5.5V | External supply |
| Operating Voltage | 3.3V | Internal regulator |
| Current Draw | ~100mA typical | Varies with clock speed |

### Solenoid Power Requirements

**Per-Solenoid Calculation**:

| Parameter | Typical 12V Solenoid | Typical 24V Solenoid |
|-----------|---------------------|---------------------|
| Coil Resistance | 10-30 ohm | 40-100 ohm |
| Operating Current | 0.4A - 1.2A | 0.24A - 0.6A |
| Power | 5W - 15W | 6W - 15W |

**Full Piano Power Budget**:

| Configuration | Max Simultaneous Keys | Current @ 12V | Current @ 24V | Recommended PSU |
|---------------|----------------------|---------------|---------------|-----------------|
| Single Board Test | 8 | 8A peak | 4A peak | 12V/10A or 24V/5A |
| 88-Key Piano | 10 (typical) | 10A typical | 5A typical | 12V/20A or 24V/10A |
| 88-Key Piano | 88 (theoretical) | 80A+ | 40A+ | Multiple PSUs |

**Important**: Piano playing rarely activates more than 10 keys simultaneously. Size power supply for typical use plus safety margin.

### Power Supply Selection Criteria

1. **Voltage**: Match solenoid rated voltage (12V or 24V typical)
2. **Current**:
   - Minimum: (max simultaneous solenoids) x (solenoid current) x 1.5 safety factor
   - For testing: 5A minimum
   - For full piano: 20A+ recommended
3. **Regulation**: Switching power supply acceptable, linear preferred for low noise
4. **Protection**: Over-current, over-voltage, short-circuit protection required

### Recommended Power Supplies

| Use Case | Voltage | Current | Example |
|----------|---------|---------|---------|
| Single Board Test | 12V | 5A | Mean Well RS-75-12 |
| 8-Board Setup | 12V | 20A | Mean Well SE-200-12 |
| Full 88-Key Piano | 24V | 10A | Mean Well SE-240-24 |

---

## I2C Address Configuration

### Address Jumper Settings

The MCP23017 base address is 0x20. Addresses 0x20-0x27 are set using the A0, A1, A2 jumper pads on the driver board.

| Address | A2 | A1 | A0 | Binary |
|---------|----|----|----| -------|
| **0x20** | 0 | 0 | 0 | 0100000 (default) |
| 0x21 | 0 | 0 | 1 | 0100001 |
| 0x22 | 0 | 1 | 0 | 0100010 |
| 0x23 | 0 | 1 | 1 | 0100011 |
| 0x24 | 1 | 0 | 0 | 0100100 |
| 0x25 | 1 | 0 | 1 | 0100101 |
| 0x26 | 1 | 1 | 0 | 0100110 |
| 0x27 | 1 | 1 | 1 | 0100111 |

**Jumper Configuration**:
- **0** = Jumper NOT bridged (pin pulled to GND via on-board resistor)
- **1** = Jumper BRIDGED (solder bridge or jumper wire to Vcc)

### Jumper Location on Board

```
┌──────────────────────────────┐
│  Vcc Gnd SDA SCL A0 A1 A2   │
│   ●   ●   ●   ●  ○  ○  ○    │  ← Header pins
│                  ┃  ┃  ┃    │
│                 [J][J][J]   │  ← Jumper pads (solder to set to 1)
│                             │
└──────────────────────────────┘

To set A0=1: Solder bridge the A0 jumper pads
```

---

## Assembly Instructions

### Step 1: Prepare Components

1. Unpack Teensy 4.1 and solder header pins if not pre-soldered
2. Unpack Adafruit Solenoid Driver board
3. Prepare breadboard and jumper wires
4. Verify you have the correct power supply voltage for your solenoid

### Step 2: Wire I2C Connections

**Connection Order** (power disconnected):

1. Connect Teensy GND to breadboard ground rail
2. Connect Teensy 3.3V to breadboard power rail (red)
3. Connect Solenoid Driver GND to breadboard ground rail
4. Connect Solenoid Driver Vcc to breadboard 3.3V rail
5. Connect Teensy Pin 18 (SDA) to Solenoid Driver SDA
6. Connect Teensy Pin 19 (SCL) to Solenoid Driver SCL

### Step 3: Wire Solenoid Power

**Warning**: Do not power the solenoid supply until I2C is verified working.

1. Connect DC power supply (-) to Solenoid Driver GND terminal
2. Connect DC power supply (+) to Solenoid Driver V+ terminal
3. **Do not turn on DC supply yet**

### Step 4: Connect Test Solenoid

1. Connect solenoid (-) or (GND) terminal to one of the numbered screw terminals (0-7)
2. The solenoid (+) is internally connected to V+ through the MOSFET when activated

**Solenoid Polarity**: Most DC solenoids are not polarity sensitive. If using a solenoid with a built-in diode, ensure correct polarity (diode stripe toward V+).

### Step 5: Verify Wiring

Complete the safety checklist below before powering on.

---

## Safety Checklist

### Pre-Power Checklist

| # | Check | Status |
|---|-------|--------|
| 1 | All components on non-conductive surface | [ ] |
| 2 | No loose wires or exposed conductors | [ ] |
| 3 | DC power supply is OFF | [ ] |
| 4 | USB cable is disconnected | [ ] |
| 5 | 3.3V (not 5V) connected to Solenoid Driver Vcc | [ ] |
| 6 | Ground connections verified with continuity tester | [ ] |
| 7 | No shorts between 3.3V and GND | [ ] |
| 8 | No shorts between V+ and GND | [ ] |
| 9 | Solenoid power supply voltage matches solenoid rating | [ ] |
| 10 | Flyback diode present (if solenoid requires external) | [ ] |

### First Power-On Procedure

1. **USB Power Only** (DC supply OFF):
   - Connect USB to Teensy
   - Verify Teensy LED blinks (if running blink code)
   - Measure 3.3V at Solenoid Driver Vcc pin
   - Measure I2C lines: should show ~3.3V (idle high)

2. **Add DC Power**:
   - Turn on DC power supply
   - Verify correct voltage at V+ terminal
   - Check for any heating or smoke (should be none)
   - Solenoids should NOT activate (GPIO not configured yet)

3. **Run Test Code**:
   - Upload I2C scanner
   - Verify device detected at expected address

---

## Verification Procedures

### Procedure 1: Continuity Test

| Test | Probe 1 | Probe 2 | Expected |
|------|---------|---------|----------|
| Ground continuity | Teensy GND | Driver GND | Beep (short) |
| No short to power | Teensy GND | Teensy 3.3V | No beep |
| No short to power | Driver GND | Driver Vcc | No beep |
| I2C SDA connection | Teensy Pin 18 | Driver SDA | Beep |
| I2C SCL connection | Teensy Pin 19 | Driver SCL | Beep |

### Procedure 2: Voltage Test (USB Power Only)

| Test Point | Expected Voltage | Acceptable Range |
|------------|------------------|------------------|
| Teensy 3.3V to GND | 3.3V | 3.2V - 3.4V |
| Driver Vcc to GND | 3.3V | 3.2V - 3.4V |
| SDA line (idle) | 3.3V | 3.0V - 3.4V |
| SCL line (idle) | 3.3V | 3.0V - 3.4V |
| Driver V+ (DC off) | 0V | 0V |

### Procedure 3: Voltage Test (DC Power On)

| Test Point | Expected Voltage | Notes |
|------------|------------------|-------|
| Driver V+ to GND | 12V or 24V | Match your supply |
| Solenoid output (off) | 0V | MOSFET not conducting |
| Solenoid output (on) | ~V+ | When activated by code |

### Procedure 4: I2C Scanner Test

Upload and run the I2C scanner code (see software implementation document).

**Expected Output**:
```
I2C Scanner
Scanning...
I2C device found at address 0x20
Scan complete. 1 device(s) found.
```

**Troubleshooting**:
- No devices found: Check wiring, ensure 3.3V power
- Wrong address: Check A0/A1/A2 jumpers
- Multiple devices: Normal if using multiple boards

---

## Troubleshooting

### Problem: I2C Device Not Detected

| Cause | Solution |
|-------|----------|
| No power to driver board | Verify 3.3V at Vcc pin |
| SDA/SCL swapped | Double-check pin connections |
| Wrong I2C pins used | Teensy: SDA=18, SCL=19 |
| Address jumpers wrong | Check A0/A1/A2 configuration |
| Damaged MCP23017 | Try different driver board |
| Pull-ups missing | Add 4.7k pull-ups to 3.3V |

### Problem: Solenoid Doesn't Activate

| Cause | Solution |
|-------|----------|
| DC power not connected | Check V+ connection |
| DC power supply off | Turn on supply |
| Wrong channel in code | Verify channel number (0-7) |
| GPIO not configured as output | Check IODIRA = 0x00 |
| Solenoid failed | Test solenoid directly with DC |
| MOSFET failed | Test different channel |

### Problem: Solenoid Stays On

| Cause | Solution |
|-------|----------|
| Software stuck | Add watchdog timer |
| MOSFET shorted | Replace board |
| Code bug | Check GPIO write values |

### Problem: I2C Communication Errors

| Cause | Solution |
|-------|----------|
| Cable too long | Use shorter cables (<30cm) |
| Noise interference | Add pull-ups, use shielded cable |
| Speed too high | Reduce I2C clock to 100kHz |
| Multiple devices loading bus | Add I2C buffer chip |

### Problem: Solenoid Overheating

| Cause | Solution |
|-------|----------|
| On time too long | Implement max on-time limit |
| Duty cycle too high | Implement duty cycle limiting |
| Insufficient cooling | Add cooling or reduce duty |
| Voltage too high | Verify correct supply voltage |

---

## Appendix: Component Datasheets

- **Teensy 4.1**: https://www.pjrc.com/store/teensy41.html
- **Adafruit I2C Solenoid Driver**: https://www.adafruit.com/product/6318
- **MCP23017 Datasheet**: https://ww1.microchip.com/downloads/en/devicedoc/20001952c.pdf
- **AO3406 Datasheet**: https://www.aosmd.com/pdfs/datasheet/AO3406.pdf
