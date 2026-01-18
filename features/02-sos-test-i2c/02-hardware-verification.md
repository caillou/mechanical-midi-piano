# Hardware Verification Guide - SOS Test

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Hardware Configuration](#hardware-configuration)
3. [Pre-Test Checklist](#pre-test-checklist)
4. [Connection Verification](#connection-verification)
5. [Power Supply Verification](#power-supply-verification)
6. [I2C Bus Verification](#i2c-bus-verification)
7. [Solenoid Verification](#solenoid-verification)
8. [Safety Verification](#safety-verification)
9. [Troubleshooting](#troubleshooting)

---

## Prerequisites

### Required Hardware

| Component | Specification | Status |
|-----------|---------------|--------|
| Teensy 4.1 | PJRC Teensy 4.1 with headers | [ ] Ready |
| Adafruit Solenoid Driver | Product ID 6318, default address 0x20 | [ ] Ready |
| Test Solenoid | 12V or 24V DC solenoid | [ ] Ready |
| DC Power Supply | Matches solenoid voltage (12V or 24V), 2A+ | [ ] Ready |
| USB Cable | USB Micro-B for Teensy | [ ] Ready |
| Jumper Wires | 4+ female-female or male-male dupont | [ ] Ready |
| Multimeter | For voltage and continuity testing | [ ] Ready |

### Required Software

| Component | Version | Status |
|-----------|---------|--------|
| PlatformIO | 6.1+ | [ ] Installed |
| Serial Monitor | PlatformIO or Arduino IDE | [ ] Available |
| Existing Test Program | Current main.cpp uploaded | [ ] Verified |

### Knowledge Prerequisites

- [ ] Familiar with existing solenoid driver test program
- [ ] Understand I2C communication basics
- [ ] Know how to use multimeter for voltage testing
- [ ] Reviewed safety procedures for solenoid operation

---

## Hardware Configuration

### I2C Connection Diagram

```
TEENSY 4.1                      ADAFRUIT SOLENOID DRIVER (6318)
═══════════                     ═══════════════════════════════

Pin 18 (SDA) ───────────────────► SDA
Pin 19 (SCL) ───────────────────► SCL
3.3V ───────────────────────────► Vcc
GND ────────────────────────────► GND

                                  V+ ◄────────── DC Supply (+)
                                  GND ◄───────── DC Supply (-)

                                  Channel 0 ────► Solenoid Terminal 1
                                  GND Terminal ──► Solenoid Terminal 2
```

### Detailed Pin Mapping

| Signal | Teensy 4.1 Pin | Driver Board Pin | Wire Color (Suggested) |
|--------|----------------|------------------|------------------------|
| SDA | Pin 18 | SDA | Blue |
| SCL | Pin 19 | SCL | Yellow |
| Logic Power | 3.3V | Vcc | Red |
| Ground | GND | GND | Black |

### MCP23017 Configuration

| Setting | Value | Description |
|---------|-------|-------------|
| I2C Address | 0x20 | Default (A0=A1=A2=LOW) |
| Port A | Outputs | Solenoid channels 0-7 |
| Port B | Inputs | Unused (default) |
| I2C Speed | 400 kHz | Fast mode |

### Address Jumper Settings (Default)

```
┌────────────────────────────────┐
│ Vcc Gnd SDA SCL A0 A1 A2       │
│  ●   ●   ●   ●  ○  ○  ○        │  ← No jumpers = Address 0x20
│                 │  │  │        │
│                 0  0  0        │  ← All LOW (not bridged)
└────────────────────────────────┘
```

---

## Pre-Test Checklist

### Before Connecting Power

| # | Check Item | Method | Status |
|---|------------|--------|--------|
| 1 | DC power supply is OFF | Visual check | [ ] |
| 2 | USB cable is disconnected | Visual check | [ ] |
| 3 | No exposed conductors | Visual inspection | [ ] |
| 4 | Components on non-conductive surface | Visual check | [ ] |
| 5 | I2C connections (4 wires) verified | Trace each wire | [ ] |
| 6 | Solenoid connected to Channel 0 | Check terminal | [ ] |
| 7 | Solenoid ground connected to GND terminal | Check terminal | [ ] |

### I2C Wiring Verification

| Connection | From | To | Verified |
|------------|------|-----|----------|
| SDA | Teensy Pin 18 | Driver SDA | [ ] |
| SCL | Teensy Pin 19 | Driver SCL | [ ] |
| Power | Teensy 3.3V | Driver Vcc | [ ] |
| Ground | Teensy GND | Driver GND | [ ] |

### Critical Warnings

```
╔══════════════════════════════════════════════════════════════════════════╗
║  WARNING: VOLTAGE LEVELS                                                  ║
║                                                                           ║
║  1. NEVER connect 5V to driver Vcc - use 3.3V ONLY                       ║
║  2. Teensy 4.1 I/O is NOT 5V tolerant                                    ║
║  3. Keep DC solenoid power separate from logic power                     ║
║                                                                           ║
╚══════════════════════════════════════════════════════════════════════════╝
```

---

## Connection Verification

### Step 1: Continuity Test (Power OFF)

Using multimeter in continuity mode:

| Test | Probe 1 | Probe 2 | Expected | Result |
|------|---------|---------|----------|--------|
| GND Path | Teensy GND | Driver GND | Beep/Short | [ ] Pass |
| SDA Path | Teensy Pin 18 | Driver SDA | Beep/Short | [ ] Pass |
| SCL Path | Teensy Pin 19 | Driver SCL | Beep/Short | [ ] Pass |
| 3.3V Path | Teensy 3.3V | Driver Vcc | Beep/Short | [ ] Pass |
| No Short 1 | SDA | GND | No Beep | [ ] Pass |
| No Short 2 | SCL | GND | No Beep | [ ] Pass |
| No Short 3 | 3.3V | GND | No Beep | [ ] Pass |
| No Short 4 | SDA | SCL | No Beep | [ ] Pass |

### Step 2: Solenoid Connection Check

| Test | Probe 1 | Probe 2 | Expected | Result |
|------|---------|---------|----------|--------|
| Solenoid Coil | Terminal 1 | Terminal 2 | Low resistance (10-100 ohm) | [ ] Pass |
| Channel 0 Path | Driver Ch 0 | Solenoid terminal | Beep/Short | [ ] Pass |
| GND Path | Driver GND terminal | Solenoid terminal | Beep/Short | [ ] Pass |

---

## Power Supply Verification

### Step 3: USB Power Only (DC Supply OFF)

Connect USB to Teensy, leave DC supply OFF.

| Measurement | Test Points | Expected | Acceptable | Result |
|-------------|-------------|----------|------------|--------|
| Teensy 3.3V | Teensy 3.3V to GND | 3.3V | 3.2V - 3.4V | [ ] ___V |
| Driver Vcc | Driver Vcc to GND | 3.3V | 3.2V - 3.4V | [ ] ___V |
| SDA Idle | Driver SDA to GND | 3.3V | 3.0V - 3.4V | [ ] ___V |
| SCL Idle | Driver SCL to GND | 3.3V | 3.0V - 3.4V | [ ] ___V |
| Driver V+ | Driver V+ to GND | 0V | 0V | [ ] ___V |

### Step 4: Add DC Power

Turn on DC power supply.

| Measurement | Test Points | Expected | Acceptable | Result |
|-------------|-------------|----------|------------|--------|
| DC Supply | V+ terminal to GND | 12V or 24V | +/- 5% | [ ] ___V |
| No Smoke | Visual | None | None | [ ] Pass |
| No Heat | Touch test | Cool/Warm | Not hot | [ ] Pass |

### Power Supply Requirements

| Parameter | 12V System | 24V System | Your Setup |
|-----------|------------|------------|------------|
| Voltage | 12V DC | 24V DC | ___V |
| Current | 2A minimum | 2A minimum | ___A |
| Regulation | Switching or Linear | Switching or Linear | _______ |

---

## I2C Bus Verification

### Step 5: Run I2C Scanner

1. Open serial monitor (115200 baud)
2. Press 's' to run I2C scan

**Expected Output:**
```
Scanning I2C bus...
  [FOUND] Device at address 0x20 (MCP23017 - Solenoid Driver)

Scan complete. 1 device(s) found.
```

| Verification | Expected | Result |
|--------------|----------|--------|
| Device found at 0x20 | Yes | [ ] |
| Only 1 device found | Yes (or more if multiple boards) | [ ] |
| No communication errors | No error messages | [ ] |

### I2C Scanner Troubleshooting

| Output | Diagnosis | Solution |
|--------|-----------|----------|
| 0 devices found | No communication | Check wiring, verify 3.3V at Vcc |
| Device at wrong address | Address jumpers set | Check A0/A1/A2 jumpers |
| Multiple unexpected devices | Other I2C devices present | Verify which devices are connected |
| Scan hangs | I2C bus stuck | Reset Teensy, check for shorts |

---

## Solenoid Verification

### Step 6: Test Solenoid Directly (Optional)

Before testing through the driver, verify solenoid works:

1. Disconnect solenoid from driver
2. Apply DC voltage directly to solenoid terminals
3. Solenoid should click/actuate

| Test | Expected | Result |
|------|----------|--------|
| Solenoid actuates with direct DC | Click/movement | [ ] Pass |
| Solenoid releases when power removed | Returns to rest | [ ] Pass |

### Step 7: Test Through Driver

1. Reconnect solenoid to Channel 0
2. In serial monitor, press '0' to toggle Channel 0

**Expected Output:**
```
Toggling channel 0 -> ON
```

| Test | Expected | Result |
|------|----------|--------|
| Solenoid clicks when '0' pressed (ON) | Audible click | [ ] Pass |
| Solenoid releases when '0' pressed again (OFF) | Returns to rest | [ ] Pass |
| No errors in serial output | No error messages | [ ] Pass |

### Channel 0 Voltage Verification

| State | Measurement | Expected | Result |
|-------|-------------|----------|--------|
| Channel 0 OFF | Channel 0 terminal to GND | ~0V | [ ] ___V |
| Channel 0 ON | Channel 0 terminal to GND | ~V+ (12V/24V) | [ ] ___V |

---

## Safety Verification

### Step 8: Emergency Stop Test

1. Press '0' to turn Channel 0 ON
2. Immediately press 'x' for emergency stop

**Expected Output:**
```
Toggling channel 0 -> ON

EMERGENCY STOP
[OK] All channels deactivated
```

| Test | Expected | Result |
|------|----------|--------|
| Emergency stop deactivates channel | Solenoid releases immediately | [ ] Pass |
| All channels show OFF | channelStates = 0x00 | [ ] Pass |

### Step 9: Safety Timeout Test (Optional)

Test that channels auto-shutoff after 5 seconds:

1. Press '0' to turn Channel 0 ON
2. Wait 5+ seconds
3. Watch for auto-shutoff message

**Expected Output:**
```
[SAFETY] Channel 0 auto-shutoff (max on-time exceeded)
```

| Test | Expected | Result |
|------|----------|--------|
| Auto-shutoff after 5 seconds | Solenoid releases, message appears | [ ] Pass |

---

## Final Hardware Verification Checklist

Complete this checklist before proceeding to code implementation.

### Electrical Verification

| # | Item | Status |
|---|------|--------|
| 1 | All continuity tests pass | [ ] |
| 2 | 3.3V measured at driver Vcc | [ ] |
| 3 | Correct DC voltage at V+ terminal | [ ] |
| 4 | I2C lines idle at ~3.3V | [ ] |

### Communication Verification

| # | Item | Status |
|---|------|--------|
| 5 | I2C scanner finds device at 0x20 | [ ] |
| 6 | No I2C communication errors | [ ] |
| 7 | Serial monitor working at 115200 baud | [ ] |

### Solenoid Verification

| # | Item | Status |
|---|------|--------|
| 8 | Solenoid actuates when Channel 0 toggled ON | [ ] |
| 9 | Solenoid releases when Channel 0 toggled OFF | [ ] |
| 10 | Emergency stop ('x') works correctly | [ ] |

### Safety Verification

| # | Item | Status |
|---|------|--------|
| 11 | No overheating of any components | [ ] |
| 12 | Solenoid power within rated limits | [ ] |
| 13 | Auto-shutoff timeout functional (optional) | [ ] |

---

## Troubleshooting

### Problem: No I2C Devices Found

| Possible Cause | Diagnostic | Solution |
|----------------|------------|----------|
| No power to driver | Measure Vcc | Connect 3.3V |
| SDA/SCL swapped | Trace wires | Correct wiring |
| Wrong I2C pins | Verify pins | Use Pin 18 (SDA), Pin 19 (SCL) |
| Damaged MCP23017 | Try different board | Replace board |

### Problem: Wrong I2C Address

| Possible Cause | Diagnostic | Solution |
|----------------|------------|----------|
| Address jumpers set | Check A0/A1/A2 | Remove jumpers for 0x20 |
| Multiple boards | I2C scan shows multiple | Use correct address in code |

### Problem: Solenoid Doesn't Actuate

| Possible Cause | Diagnostic | Solution |
|----------------|------------|----------|
| DC power off | Check V+ voltage | Turn on DC supply |
| Wrong voltage | Measure V+ | Use correct supply voltage |
| Solenoid failed | Test directly with DC | Replace solenoid |
| Wrong channel | Check connections | Connect to Channel 0 |
| MOSFET failed | Test other channels | Use different channel or replace board |

### Problem: Solenoid Stays On

| Possible Cause | Diagnostic | Solution |
|----------------|------------|----------|
| Code stuck | Press 'x' for emergency stop | Use emergency stop |
| MOSFET shorted | Measure channel voltage | Replace board |
| Software bug | Check serial output | Debug code |

### Problem: I2C Errors During Operation

| Possible Cause | Diagnostic | Solution |
|----------------|------------|----------|
| Long cables | Measure cable length | Use <30cm cables |
| EMI from solenoid | Check if errors correlate with clicks | Add capacitors, separate power |
| Weak pull-ups | Check SDA/SCL idle voltage | Add 4.7k pull-ups to 3.3V |
| Bus speed too high | Try lower speed | Reduce to 100kHz |

---

## Ready for Code Implementation

Once all verification steps pass:

- [ ] All hardware verification items checked
- [ ] No communication errors
- [ ] Solenoid responds to Channel 0 toggle
- [ ] Emergency stop functional

**Proceed to**: [03-code-implementation.md](./03-code-implementation.md)

---

## Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2026-01-18 | - | Initial creation |
