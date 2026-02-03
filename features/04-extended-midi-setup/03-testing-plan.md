# Extended MIDI Setup - Testing Plan

## Document Information

| Field | Value |
|-------|-------|
| Feature ID | 04-extended-midi-setup |
| Version | 1.0 |
| Created | 2025-02-03 |
| Prerequisites | Hardware setup complete (see 04-hardware-checklist.md) |

---

## Testing Overview

This document provides comprehensive testing procedures for the extended MIDI setup. Tests are organized into phases that should be executed sequentially.

---

## Test Environment Setup

### Required Hardware
- Teensy 4.1 with USB cable
- MCP23017 Board 0 at address 0x20
- MCP23017 Board 1 at address 0x21
- 12 solenoids connected (or LEDs for initial testing)
- Computer with:
  - PlatformIO installed
  - Serial terminal (115200 baud)
  - MIDI software (e.g., MIDI-OX, Logic Pro, Ableton, or sendmidi CLI)

### Required Software
- PlatformIO CLI or IDE
- Serial terminal application
- MIDI controller or software MIDI sender

### Test MIDI Setup
For software MIDI testing, you can use the `sendmidi` command-line tool:
```bash
# Install on macOS
brew install sendmidi

# List available MIDI devices
sendmidi list

# Send a note (example)
sendmidi dev "Teensy MIDI" on 24 100
sendmidi dev "Teensy MIDI" off 24
```

---

## Phase 1: Compilation Tests

### Test 1.1: Clean Build
**Objective**: Verify code compiles without errors after changes.

**Procedure**:
```bash
cd /Users/caillou/repos/caillou/mechanical-midi-piano
pio run --target clean
pio run
```

**Expected Result**:
- Build completes with no errors
- No new warnings introduced

**Pass Criteria**:
- [ ] Exit code 0
- [ ] "SUCCESS" message displayed
- [ ] No compilation errors

### Test 1.2: Static Analysis
**Objective**: Verify no static analysis issues.

**Procedure**:
```bash
pio check
```

**Expected Result**:
- No critical or high-severity issues
- No new warnings related to changed code

**Pass Criteria**:
- [ ] No errors reported
- [ ] No new warnings in modified lines

---

## Phase 2: Hardware Detection Tests

### Test 2.1: I2C Bus Scan
**Objective**: Verify both MCP23017 boards are detected on I2C bus.

**Procedure**:
1. Connect hardware
2. Upload firmware: `pio run --target upload`
3. Open serial terminal at 115200 baud
4. Observe startup messages

**Expected Serial Output**:
```
============================================================
MECHANICAL MIDI PIANO - USB MIDI CONTROLLER
Teensy 4.1 + Adafruit I2C Solenoid Driver
============================================================

Initializing I2C bus...
  SDA Pin: 18, SCL Pin: 19, Speed: 400 kHz
[OK] I2C bus initialized

Initializing 2 MCP23017 board(s)...
  Board 0: address 0x20
  Board 1: address 0x21
  SolenoidDriver initialized: 2 board(s), 16 channel(s), all OFF
[OK] MCP23017 initialized successfully
```

**Pass Criteria**:
- [ ] Both boards (0x20 and 0x21) listed in output
- [ ] "2 board(s)" confirmed
- [ ] "16 channel(s)" displayed (8 per board, even though only 12 used)
- [ ] No error messages

### Test 2.2: Single Board Failure
**Objective**: Verify error handling when second board is missing.

**Procedure**:
1. Disconnect Board 1 (0x21) from I2C bus
2. Reset Teensy
3. Observe serial output

**Expected Serial Output**:
```
Initializing 2 MCP23017 board(s)...
  Board 0: address 0x20
  Board 1: address 0x21
[ERROR] SolenoidDriver init failed: I2C communication error
[ERROR] Failed to initialize MCP23017!
Check wiring and I2C address.
```

**Pass Criteria**:
- [ ] Error message clearly indicates failure
- [ ] System does not crash
- [ ] Reconnecting board and resetting recovers

### Test 2.3: Status Command
**Objective**: Verify status command shows all 12 channels.

**Procedure**:
1. Ensure both boards connected
2. Reset Teensy
3. Wait for initialization
4. Send 's' command via serial terminal

**Expected Serial Output**:
```
============================================================
STATUS
============================================================
Driver initialized: Yes
Boards: 2
Channels: 16
Channel states:
  Ch 0 (Note 24): off
  Ch 1 (Note 25): off
  Ch 2 (Note 26): off
  Ch 3 (Note 27): off
  Ch 4 (Note 28): off
  Ch 5 (Note 29): off
  Ch 6 (Note 30): off
  Ch 7 (Note 31): off
  Ch 8 (Note 32): off
  Ch 9 (Note 33): off
  Ch 10 (Note 34): off
  Ch 11 (Note 35): off
============================================================
```

**Pass Criteria**:
- [ ] All 12 channels (0-11) listed
- [ ] Correct MIDI note numbers shown (24-35)
- [ ] All channels initially "off"

---

## Phase 3: MIDI Note Tests

### Test 3.1: Board 0 Notes (MIDI 24-31)
**Objective**: Verify notes 24-31 trigger channels 0-7 on board 0x20.

**Procedure**:
For each note in the range 24-31, send Note On and Note Off:

```bash
# Test each note on Board 0
for note in 24 25 26 27 28 29 30 31; do
  echo "Testing note $note..."
  sendmidi dev "Teensy MIDI" on $note 100
  sleep 0.5
  sendmidi dev "Teensy MIDI" off $note
  sleep 0.2
done
```

**Verification for each note**:
| MIDI Note | Expected Channel | Board Address | Physical Output |
|-----------|------------------|---------------|-----------------|
| 24 | 0 | 0x20 | Solenoid/LED 0 activates |
| 25 | 1 | 0x20 | Solenoid/LED 1 activates |
| 26 | 2 | 0x20 | Solenoid/LED 2 activates |
| 27 | 3 | 0x20 | Solenoid/LED 3 activates |
| 28 | 4 | 0x20 | Solenoid/LED 4 activates |
| 29 | 5 | 0x20 | Solenoid/LED 5 activates |
| 30 | 6 | 0x20 | Solenoid/LED 6 activates |
| 31 | 7 | 0x20 | Solenoid/LED 7 activates |

**Pass Criteria**:
- [ ] Note 24 -> Channel 0 activates
- [ ] Note 25 -> Channel 1 activates
- [ ] Note 26 -> Channel 2 activates
- [ ] Note 27 -> Channel 3 activates
- [ ] Note 28 -> Channel 4 activates
- [ ] Note 29 -> Channel 5 activates
- [ ] Note 30 -> Channel 6 activates
- [ ] Note 31 -> Channel 7 activates
- [ ] All channels turn off when Note Off received

### Test 3.2: Board 1 Notes (MIDI 32-35)
**Objective**: Verify notes 32-35 trigger channels 8-11 on board 0x21.

**Procedure**:
```bash
# Test each note on Board 1
for note in 32 33 34 35; do
  echo "Testing note $note..."
  sendmidi dev "Teensy MIDI" on $note 100
  sleep 0.5
  sendmidi dev "Teensy MIDI" off $note
  sleep 0.2
done
```

**Verification for each note**:
| MIDI Note | Expected Channel | Board Address | Local Channel |
|-----------|------------------|---------------|---------------|
| 32 | 8 | 0x21 | 0 |
| 33 | 9 | 0x21 | 1 |
| 34 | 10 | 0x21 | 2 |
| 35 | 11 | 0x21 | 3 |

**Pass Criteria**:
- [ ] Note 32 -> Global Channel 8 (Board 0x21, Local 0) activates
- [ ] Note 33 -> Global Channel 9 (Board 0x21, Local 1) activates
- [ ] Note 34 -> Global Channel 10 (Board 0x21, Local 2) activates
- [ ] Note 35 -> Global Channel 11 (Board 0x21, Local 3) activates
- [ ] All channels turn off when Note Off received

### Test 3.3: Out-of-Range Notes
**Objective**: Verify notes outside range 24-35 are ignored.

**Procedure**:
```bash
# Test notes below range
sendmidi dev "Teensy MIDI" on 23 100  # Below range
sleep 0.5
sendmidi dev "Teensy MIDI" off 23

# Test notes above range
sendmidi dev "Teensy MIDI" on 36 100  # Above range
sleep 0.5
sendmidi dev "Teensy MIDI" off 36

# Test middle C (old range)
sendmidi dev "Teensy MIDI" on 60 100  # Old C4
sleep 0.5
sendmidi dev "Teensy MIDI" off 60
```

**Expected Result**:
- No solenoids/LEDs activate
- No error messages in serial output

**Pass Criteria**:
- [ ] Note 23 produces no output
- [ ] Note 36 produces no output
- [ ] Note 60 produces no output (no longer in range)
- [ ] No errors in serial monitor

### Test 3.4: Velocity Zero Handling
**Objective**: Verify velocity 0 is treated as Note Off.

**Procedure**:
```bash
# Send Note On with velocity 100
sendmidi dev "Teensy MIDI" on 24 100
sleep 0.5

# Send Note On with velocity 0 (should act as Note Off)
sendmidi dev "Teensy MIDI" on 24 0
```

**Expected Result**:
- Channel 0 turns on with first message
- Channel 0 turns off with velocity 0 message

**Pass Criteria**:
- [ ] Note On velocity 0 correctly turns channel off

---

## Phase 4: Multi-Note Tests

### Test 4.1: Simultaneous Notes (Same Board)
**Objective**: Verify multiple notes can be active simultaneously on Board 0.

**Procedure**:
```bash
# Turn on multiple notes on Board 0
sendmidi dev "Teensy MIDI" on 24 100
sendmidi dev "Teensy MIDI" on 26 100
sendmidi dev "Teensy MIDI" on 28 100
sleep 1

# Verify via serial status
# (Type 's' in serial terminal)

# Turn off all
sendmidi dev "Teensy MIDI" off 24
sendmidi dev "Teensy MIDI" off 26
sendmidi dev "Teensy MIDI" off 28
```

**Expected Status Output**:
```
  Ch 0 (Note 24): ON
  Ch 1 (Note 25): off
  Ch 2 (Note 26): ON
  Ch 3 (Note 27): off
  Ch 4 (Note 28): ON
  ...
```

**Pass Criteria**:
- [ ] Channels 0, 2, 4 show ON simultaneously
- [ ] All turn off correctly

### Test 4.2: Simultaneous Notes (Cross-Board)
**Objective**: Verify notes can be active on both boards simultaneously.

**Procedure**:
```bash
# Turn on notes across both boards
sendmidi dev "Teensy MIDI" on 24 100   # Board 0, Channel 0
sendmidi dev "Teensy MIDI" on 32 100   # Board 1, Channel 8
sendmidi dev "Teensy MIDI" on 31 100   # Board 0, Channel 7
sendmidi dev "Teensy MIDI" on 35 100   # Board 1, Channel 11
sleep 1

# Verify via serial status

# Turn off all
sendmidi dev "Teensy MIDI" off 24
sendmidi dev "Teensy MIDI" off 32
sendmidi dev "Teensy MIDI" off 31
sendmidi dev "Teensy MIDI" off 35
```

**Pass Criteria**:
- [ ] Channels 0, 7 (Board 0) and 8, 11 (Board 1) active simultaneously
- [ ] All turn off correctly
- [ ] No I2C errors

### Test 4.3: Full Chord Test
**Objective**: Verify all 12 channels can be activated simultaneously.

**Procedure**:
```bash
# Turn on all 12 notes
for note in 24 25 26 27 28 29 30 31 32 33 34 35; do
  sendmidi dev "Teensy MIDI" on $note 100
done
sleep 1

# Check status (type 's')

# Turn off all
for note in 24 25 26 27 28 29 30 31 32 33 34 35; do
  sendmidi dev "Teensy MIDI" off $note
done
```

**Pass Criteria**:
- [ ] All 12 channels show ON in status
- [ ] All channels turn off correctly
- [ ] No timing issues or errors

---

## Phase 5: Safety Feature Tests

### Test 5.1: Emergency Stop
**Objective**: Verify emergency stop turns off all 12 channels.

**Procedure**:
1. Turn on notes across both boards:
   ```bash
   sendmidi dev "Teensy MIDI" on 24 100
   sendmidi dev "Teensy MIDI" on 32 100
   sendmidi dev "Teensy MIDI" on 35 100
   ```
2. Send emergency stop via serial: type 'x'
3. Check status: type 's'

**Expected Serial Output**:
```
EMERGENCY STOP
[OK] All channels deactivated
```

**Pass Criteria**:
- [ ] All channels immediately turn off
- [ ] Status shows all channels "off"
- [ ] Both boards affected (0x20 and 0x21)

### Test 5.2: Max On-Time Safety
**Objective**: Verify auto-shutoff works for extended notes.

**Procedure**:
1. Send Note On without Note Off:
   ```bash
   sendmidi dev "Teensy MIDI" on 24 100
   ```
2. Wait for MAX_ON_TIME_MS (2000ms = 2 seconds)
3. Observe serial output

**Expected Result**:
- Channel automatically turns off after 2 seconds
- Safety timeout message may appear (if debug enabled)

**Pass Criteria**:
- [ ] Channel 0 auto-shutoff after ~2 seconds
- [ ] No damage to solenoid

### Test 5.3: Cooldown Enforcement
**Objective**: Verify minimum off-time is enforced.

**Procedure**:
```bash
# Rapid on/off cycling
for i in {1..20}; do
  sendmidi dev "Teensy MIDI" on 24 100
  sleep 0.005  # 5ms - less than MIN_OFF_TIME_MS (15ms)
  sendmidi dev "Teensy MIDI" off 24
  sleep 0.005
done
```

**Expected Result**:
- Some activations may be blocked by cooldown
- No errors should occur

**Pass Criteria**:
- [ ] System remains stable
- [ ] Cooldown protection engaged (visible in debug mode)

---

## Phase 6: Performance Tests

### Test 6.1: Rapid Note Sequences
**Objective**: Verify system handles rapid MIDI input.

**Procedure**:
```bash
# Play ascending scale quickly
for note in 24 25 26 27 28 29 30 31 32 33 34 35; do
  sendmidi dev "Teensy MIDI" on $note 100
  sleep 0.05
  sendmidi dev "Teensy MIDI" off $note
done
```

**Pass Criteria**:
- [ ] All notes trigger in sequence
- [ ] No dropped notes
- [ ] No I2C errors

### Test 6.2: Sustained Load Test
**Objective**: Verify stability over extended operation.

**Procedure**:
Run the rapid note sequence test in a loop for 5 minutes:
```bash
for i in {1..60}; do
  for note in 24 25 26 27 28 29 30 31 32 33 34 35; do
    sendmidi dev "Teensy MIDI" on $note 80
    sleep 0.05
    sendmidi dev "Teensy MIDI" off $note
  done
  sleep 0.5
done
```

**Pass Criteria**:
- [ ] No system hangs
- [ ] No I2C communication errors
- [ ] No memory leaks (check free memory if available)
- [ ] Duty cycle limits engaged appropriately

---

## Phase 7: Help and UI Tests

### Test 7.1: Help Command
**Objective**: Verify help text shows correct MIDI range.

**Procedure**:
1. Type 'h' in serial terminal

**Expected Output**:
```
SERIAL COMMANDS:
  'x' - Emergency stop (all solenoids off)
  's' - Print status
  'h' - Show this help menu

MIDI: Listening for notes 24-35 (C1-B1) on all channels

Ready for MIDI input...
```

**Pass Criteria**:
- [ ] Note range shows "24-35"
- [ ] Note names show "(C1-B1)"

### Test 7.2: Startup Banner
**Objective**: Verify startup shows correct MIDI range.

**Procedure**:
1. Reset Teensy
2. Observe startup messages

**Expected Output** (relevant section):
```
[OK] MIDI handlers registered
  Listening for notes 24-35 (C1-B1)
```

**Pass Criteria**:
- [ ] Correct note range displayed
- [ ] Correct note names displayed

---

## Test Results Summary

### Test Execution Log

| Test ID | Test Name | Date | Tester | Result | Notes |
|---------|-----------|------|--------|--------|-------|
| 1.1 | Clean Build | | | | |
| 1.2 | Static Analysis | | | | |
| 2.1 | I2C Bus Scan | | | | |
| 2.2 | Single Board Failure | | | | |
| 2.3 | Status Command | | | | |
| 3.1 | Board 0 Notes | | | | |
| 3.2 | Board 1 Notes | | | | |
| 3.3 | Out-of-Range Notes | | | | |
| 3.4 | Velocity Zero | | | | |
| 4.1 | Simultaneous (Same Board) | | | | |
| 4.2 | Simultaneous (Cross-Board) | | | | |
| 4.3 | Full Chord | | | | |
| 5.1 | Emergency Stop | | | | |
| 5.2 | Max On-Time Safety | | | | |
| 5.3 | Cooldown Enforcement | | | | |
| 6.1 | Rapid Note Sequences | | | | |
| 6.2 | Sustained Load | | | | |
| 7.1 | Help Command | | | | |
| 7.2 | Startup Banner | | | | |

### Sign-off

| Role | Name | Date | Signature |
|------|------|------|-----------|
| Developer | | | |
| Tester | | | |
| Reviewer | | | |

---

## Troubleshooting Guide

### Board Not Detected
**Symptom**: "I2C communication error" at startup

**Checklist**:
1. Verify power to board (3.3V or 5V depending on board)
2. Check SDA connection (Pin 18 on Teensy)
3. Check SCL connection (Pin 19 on Teensy)
4. Verify address jumpers:
   - Board 0: A0=LOW, A1=LOW, A2=LOW (0x20)
   - Board 1: A0=HIGH, A1=LOW, A2=LOW (0x21)
5. Check for I2C pull-up resistors (typically 4.7k to 3.3V)

### Wrong Channel Activates
**Symptom**: Note 32 triggers Channel 0 instead of Channel 8

**Checklist**:
1. Verify `MCP23017_ADDRESSES` array order: `{0x20, 0x21}`
2. Verify board address jumper settings
3. Check that boards are not swapped physically

### Notes in Old Range Not Working
**Symptom**: Note 60 (C4) doesn't work anymore

**Expected**: This is correct behavior. The note range changed from 60-67 to 24-35.

**Solution**: Use MIDI notes 24-35 instead of 60-67.

### Intermittent I2C Errors
**Symptom**: Occasional "I2C communication error"

**Checklist**:
1. Check I2C wiring length (keep under 50cm)
2. Verify pull-up resistors present and correct value
3. Consider reducing I2C speed if issues persist
4. Check for electrical noise from solenoids (add flyback diodes)
