# Extended MIDI Setup - Hardware Checklist

## Document Information

| Field | Value |
|-------|-------|
| Feature ID | 04-extended-midi-setup |
| Version | 1.0 |
| Created | 2025-02-03 |
| Prerequisites | Second MCP23017 board available |

---

## Overview

This checklist guides hardware setup for the extended 12-channel MIDI piano configuration using two MCP23017-based boards on the I2C bus.

---

## Bill of Materials

### Required Components

| Qty | Item | Description | Notes |
|-----|------|-------------|-------|
| 1 | Teensy 4.1 | Microcontroller | Existing |
| 1 | MCP23017 Board 0 | Adafruit I2C Solenoid Driver (PID 6318) | Existing, Address 0x20 |
| 1 | MCP23017 Board 1 | Adafruit I2C Solenoid Driver (PID 6318) or compatible | New, Address 0x21 |
| 4 | Solenoids | For channels 8-11 | Match existing solenoids |
| 1 | Power Supply | Appropriate for solenoid voltage/current | May need upgrade |
| - | Hookup Wire | 22-26 AWG for I2C and power | As needed |

### Optional Components

| Qty | Item | Description | Notes |
|-----|------|-------------|-------|
| 2 | I2C Pull-up Resistors | 4.7k ohm | If not on boards |
| 4 | Flyback Diodes | 1N4001 or similar | If not built into solenoid driver |
| 1 | Logic Analyzer | For I2C debugging | Optional |

---

## Pre-Installation Verification

### Existing Hardware Check

- [ ] **Teensy 4.1 functioning**
  - USB connection working
  - Serial communication at 115200 baud
  - Current firmware uploads successfully

- [ ] **Board 0 (0x20) functioning**
  - All 8 channels (0-7) responsive
  - I2C communication stable
  - Address pins in default position (all LOW)

- [ ] **I2C Bus healthy**
  - SDA on Teensy Pin 18
  - SCL on Teensy Pin 19
  - Pull-up resistors present (check board or add 4.7k to 3.3V)

### New Hardware Inspection

- [ ] **Board 1 (0x21) inspection**
  - No visible damage
  - Solder joints clean
  - Address jumper pads accessible

---

## Board 1 Address Configuration

### MCP23017 I2C Address Calculation

The MCP23017 I2C address is determined by pins A0, A1, A2:

```
Address = 0x20 + (A2 << 2) + (A1 << 1) + A0

Board 0: A2=0, A1=0, A0=0 -> 0x20 + 0 = 0x20
Board 1: A2=0, A1=0, A0=1 -> 0x20 + 1 = 0x21
```

### Adafruit I2C Solenoid Driver Address Setup

The Adafruit board has solder jumpers for address configuration:

```
┌─────────────────────────────────────────┐
│  Adafruit I2C Solenoid Driver           │
│                                         │
│   A0  A1  A2                            │
│   [○] [○] [○]  <- Solder jumpers        │
│                                         │
│   Default: All open (LOW) = 0x20        │
│   For 0x21: Close A0 jumper only        │
└─────────────────────────────────────────┘
```

### Address Configuration Steps for Board 1

- [ ] **Step 1**: Locate A0 jumper pad on Board 1
  - Usually labeled "A0" or "ADDR0"
  - May be on top or bottom of board

- [ ] **Step 2**: Apply solder bridge to A0 pad
  - Clean pad with isopropyl alcohol
  - Apply flux if needed
  - Create solder bridge across the jumper

- [ ] **Step 3**: Verify A1 and A2 remain OPEN
  - No solder on A1 jumper
  - No solder on A2 jumper

- [ ] **Step 4**: Visual verification
  - A0: BRIDGED (connected)
  - A1: OPEN (not connected)
  - A2: OPEN (not connected)

### Address Verification Table

| Board | A2 | A1 | A0 | Address | Status |
|-------|----|----|----|---------| ------|
| 0 | Open | Open | Open | 0x20 | Existing |
| 1 | Open | Open | **Bridged** | 0x21 | Configure |

---

## I2C Wiring

### I2C Bus Topology

```
                    ┌─────────┐
                    │ Teensy  │
                    │   4.1   │
                    │         │
        3.3V ───────┤ 3.3V    │
        GND  ───────┤ GND     │
                    │         │
        SDA  ───────┤ Pin 18  │
        SCL  ───────┤ Pin 19  │
                    └─────────┘
                         │
           ┌─────────────┴─────────────┐
           │                           │
           │ I2C Bus (shared)          │
           │                           │
    ┌──────┴──────┐             ┌──────┴──────┐
    │  Board 0    │             │  Board 1    │
    │  (0x20)     │             │  (0x21)     │
    │             │             │             │
    │ SDA ────────│─────────────│──── SDA     │
    │ SCL ────────│─────────────│──── SCL     │
    │ VCC ────────│─────────────│──── VCC     │
    │ GND ────────│─────────────│──── GND     │
    │             │             │             │
    │ Ch 0-7      │             │ Ch 0-3      │
    │ (Solenoids  │             │ (Solenoids  │
    │  0-7)       │             │  8-11)      │
    └─────────────┘             └─────────────┘
```

### Wiring Checklist

#### Power Connections

- [ ] **3.3V or 5V Logic Power**
  - Connect VCC on Board 0 to appropriate voltage
  - Connect VCC on Board 1 to same voltage source
  - Verify both boards share common ground

- [ ] **Solenoid Power**
  - Verify solenoid voltage matches power supply
  - Ensure adequate current capacity for 12 solenoids
  - Consider inrush current when all solenoids activate

#### I2C Connections

- [ ] **SDA (Data Line)**
  - Teensy Pin 18 -> Board 0 SDA
  - Board 0 SDA -> Board 1 SDA (daisy-chain or star)
  - All connections solid and secure

- [ ] **SCL (Clock Line)**
  - Teensy Pin 19 -> Board 0 SCL
  - Board 0 SCL -> Board 1 SCL (daisy-chain or star)
  - All connections solid and secure

- [ ] **Ground (Common Reference)**
  - All GND pins connected together
  - Teensy GND -> Board 0 GND -> Board 1 GND
  - Solenoid power supply GND connected to common ground

#### Pull-up Resistors

- [ ] **SDA Pull-up**
  - 4.7k ohm resistor from SDA to 3.3V
  - May be built into boards (check schematic)
  - Only ONE set of pull-ups needed for entire bus

- [ ] **SCL Pull-up**
  - 4.7k ohm resistor from SCL to 3.3V
  - May be built into boards (check schematic)
  - Only ONE set of pull-ups needed for entire bus

---

## Solenoid Wiring

### Channel to Board Mapping

```
MIDI Note -> Global Channel -> Board -> Local Channel -> Physical Output

24 (C1)  -> 0  -> Board 0 (0x20) -> Local 0 -> Solenoid 0
25 (C#1) -> 1  -> Board 0 (0x20) -> Local 1 -> Solenoid 1
26 (D1)  -> 2  -> Board 0 (0x20) -> Local 2 -> Solenoid 2
27 (D#1) -> 3  -> Board 0 (0x20) -> Local 3 -> Solenoid 3
28 (E1)  -> 4  -> Board 0 (0x20) -> Local 4 -> Solenoid 4
29 (F1)  -> 5  -> Board 0 (0x20) -> Local 5 -> Solenoid 5
30 (F#1) -> 6  -> Board 0 (0x20) -> Local 6 -> Solenoid 6
31 (G1)  -> 7  -> Board 0 (0x20) -> Local 7 -> Solenoid 7
32 (G#1) -> 8  -> Board 1 (0x21) -> Local 0 -> Solenoid 8
33 (A1)  -> 9  -> Board 1 (0x21) -> Local 1 -> Solenoid 9
34 (A#1) -> 10 -> Board 1 (0x21) -> Local 2 -> Solenoid 10
35 (B1)  -> 11 -> Board 1 (0x21) -> Local 3 -> Solenoid 11
```

### Board 1 Solenoid Connections

- [ ] **Channel 8 (Local 0)**
  - Connect solenoid 8 to Board 1, Output 0
  - Verify polarity if applicable
  - Flyback diode in place

- [ ] **Channel 9 (Local 1)**
  - Connect solenoid 9 to Board 1, Output 1
  - Verify polarity if applicable
  - Flyback diode in place

- [ ] **Channel 10 (Local 2)**
  - Connect solenoid 10 to Board 1, Output 2
  - Verify polarity if applicable
  - Flyback diode in place

- [ ] **Channel 11 (Local 3)**
  - Connect solenoid 11 to Board 1, Output 3
  - Verify polarity if applicable
  - Flyback diode in place

### Unused Channels on Board 1

Board 1 has 8 channels but only 4 are used (channels 8-11, local 0-3). Channels 4-7 on Board 1 (local channels 4-7) are unused and available for future expansion.

- [ ] **Unused outputs (Local 4-7) left disconnected**
  - No solenoids connected to channels 4-7 on Board 1
  - These can be used for future expansion (notes 36-39)

---

## Power Supply Considerations

### Current Requirements

| Component | Current Draw (typical) | Notes |
|-----------|----------------------|-------|
| Teensy 4.1 | 100mA | 3.3V logic |
| MCP23017 x2 | 1mA each | Negligible |
| Solenoid (each) | 200-500mA | Depends on solenoid type |
| **12 Solenoids (peak)** | **2.4-6A** | All on simultaneously |

### Power Supply Checklist

- [ ] **Logic Power (3.3V)**
  - Source: USB or external
  - Current: 200mA minimum
  - Stable voltage

- [ ] **Solenoid Power**
  - Voltage: Match solenoid rating (commonly 12V or 24V)
  - Current: Peak demand + 20% headroom
  - Example: 12 solenoids @ 400mA = 4.8A peak, use 6A supply

- [ ] **Power Supply Adequacy**
  - Current supply >= 6A for 12 solenoids (adjust based on actual solenoid specs)
  - Voltage matches solenoid rating
  - Supply can handle inrush current

---

## Pre-Power-On Checklist

### Visual Inspection

- [ ] All solder joints clean and shiny
- [ ] No solder bridges between adjacent pins
- [ ] No loose wires or components
- [ ] Address jumper on Board 1 correctly configured (A0 bridged)
- [ ] All connections secure

### Continuity Checks (with multimeter)

- [ ] SDA continuity: Teensy Pin 18 -> Board 0 SDA -> Board 1 SDA
- [ ] SCL continuity: Teensy Pin 19 -> Board 0 SCL -> Board 1 SCL
- [ ] Ground continuity: All GND points connected
- [ ] No shorts between SDA and SCL
- [ ] No shorts between SDA/SCL and GND
- [ ] No shorts between SDA/SCL and VCC

### Resistance Checks

- [ ] SDA pull-up: ~4.7k ohm between SDA and 3.3V
- [ ] SCL pull-up: ~4.7k ohm between SCL and 3.3V
- [ ] Each solenoid resistance within spec (typically 10-100 ohms)

---

## Initial Power-On Procedure

### Step 1: Power Logic Only

1. [ ] Disconnect solenoid power supply
2. [ ] Connect USB to Teensy
3. [ ] Verify Teensy powers on (LED behavior)
4. [ ] Check for any overheating components

### Step 2: I2C Verification

1. [ ] Open serial terminal (115200 baud)
2. [ ] Reset Teensy
3. [ ] Verify startup messages show:
   - "Initializing 2 MCP23017 board(s)..."
   - "Board 0: address 0x20"
   - "Board 1: address 0x21"
   - "SolenoidDriver initialized: 2 board(s)"
4. [ ] If errors, check I2C wiring and addresses

### Step 3: Board Detection Test

1. [ ] Type 's' in serial terminal
2. [ ] Verify status shows:
   - "Boards: 2"
   - "Channels: 16" (or similar indicating both boards)
   - All 12 channels listed (Ch 0-11)

### Step 4: Connect Solenoid Power

1. [ ] Turn off/disconnect USB temporarily
2. [ ] Connect solenoid power supply (OFF)
3. [ ] Reconnect USB
4. [ ] Turn on solenoid power supply
5. [ ] Verify no unexpected solenoid activation
6. [ ] All solenoids should remain off until MIDI input

---

## Functional Verification

### Individual Channel Test

For each channel (0-11), verify operation:

| Channel | MIDI Note | Board | Test Command | Result |
|---------|-----------|-------|--------------|--------|
| 0 | 24 | 0x20 | sendmidi on 24 100 | [ ] Pass |
| 1 | 25 | 0x20 | sendmidi on 25 100 | [ ] Pass |
| 2 | 26 | 0x20 | sendmidi on 26 100 | [ ] Pass |
| 3 | 27 | 0x20 | sendmidi on 27 100 | [ ] Pass |
| 4 | 28 | 0x20 | sendmidi on 28 100 | [ ] Pass |
| 5 | 29 | 0x20 | sendmidi on 29 100 | [ ] Pass |
| 6 | 30 | 0x20 | sendmidi on 30 100 | [ ] Pass |
| 7 | 31 | 0x20 | sendmidi on 31 100 | [ ] Pass |
| 8 | 32 | 0x21 | sendmidi on 32 100 | [ ] Pass |
| 9 | 33 | 0x21 | sendmidi on 33 100 | [ ] Pass |
| 10 | 34 | 0x21 | sendmidi on 34 100 | [ ] Pass |
| 11 | 35 | 0x21 | sendmidi on 35 100 | [ ] Pass |

### Emergency Stop Test

1. [ ] Activate multiple solenoids via MIDI
2. [ ] Type 'x' in serial terminal
3. [ ] Verify ALL solenoids immediately turn off
4. [ ] Verify "All channels deactivated" message

---

## Troubleshooting Reference

### Board 1 Not Detected

| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| Only Board 0 found | Wrong address | Verify A0 jumper bridged |
| I2C error | Bad connection | Check SDA/SCL wiring to Board 1 |
| I2C error | Missing power | Verify VCC/GND to Board 1 |
| Intermittent | Loose connection | Resolder or secure connections |

### Address Conflict

| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| Strange behavior | Both boards at 0x20 | Verify Board 1 A0 jumper |
| Only 8 channels work | Address overlap | Check both board addresses |

### I2C Bus Issues

| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| Random errors | Missing pull-ups | Add 4.7k pull-ups |
| Errors at speed | Bus too long | Shorten wires, reduce speed |
| Errors under load | Insufficient current | Check power supply |

---

## Hardware Setup Sign-Off

### Checklist Completion

| Item | Verified | Initial | Date |
|------|----------|---------|------|
| Board 1 address configured (0x21) | [ ] | | |
| I2C wiring complete | [ ] | | |
| Solenoid wiring complete | [ ] | | |
| Power supply adequate | [ ] | | |
| Pre-power-on checks passed | [ ] | | |
| I2C verification passed | [ ] | | |
| All 12 channels functional | [ ] | | |
| Emergency stop tested | [ ] | | |

### Approval

| Role | Name | Signature | Date |
|------|------|-----------|------|
| Installer | | | |
| Reviewer | | | |

---

## Appendix: Pin Reference

### Teensy 4.1 I2C Pins

| Function | Pin | Notes |
|----------|-----|-------|
| SDA (Wire) | 18 | Primary I2C data |
| SCL (Wire) | 19 | Primary I2C clock |
| SDA1 (Wire1) | 17 | Secondary I2C (unused) |
| SCL1 (Wire1) | 16 | Secondary I2C (unused) |

### MCP23017 Pin Functions

| Pin | Function | Notes |
|-----|----------|-------|
| GPA0-GPA7 | Port A GPIO | Used for solenoid control |
| GPB0-GPB7 | Port B GPIO | Available for expansion |
| A0, A1, A2 | Address select | Set I2C address |
| SDA | I2C Data | Connect to bus |
| SCL | I2C Clock | Connect to bus |
| VDD | Logic power | 3.3V or 5V |
| VSS | Ground | Common ground |
| RESET | Active low reset | Usually pulled high |

### Adafruit I2C Solenoid Driver Output Mapping

| Output Label | MCP23017 Pin | Global Channel (Board 0) | Global Channel (Board 1) |
|--------------|--------------|--------------------------|--------------------------|
| OUT0 | GPA0 | 0 | 8 |
| OUT1 | GPA1 | 1 | 9 |
| OUT2 | GPA2 | 2 | 10 |
| OUT3 | GPA3 | 3 | 11 |
| OUT4 | GPA4 | 4 | (unused) |
| OUT5 | GPA5 | 5 | (unused) |
| OUT6 | GPA6 | 6 | (unused) |
| OUT7 | GPA7 | 7 | (unused) |
