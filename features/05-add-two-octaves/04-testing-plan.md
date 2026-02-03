# Testing Plan: 5-Board MCP23017 Expansion

This document provides comprehensive test cases for verifying the expansion implementation.

## Test Environment Setup

### Required Equipment
- Teensy 4.1 with USB cable
- 5x Adafruit I2C Solenoid Driver boards (MCP23017-based)
- Test solenoids or LEDs (for visual verification)
- MIDI controller or MIDI software (e.g., MIDI-OX, DAW)
- Serial terminal (Arduino Serial Monitor, PlatformIO monitor, or similar)
- Multimeter (for continuity and voltage checks)
- Logic analyzer (optional, for I2C debugging)

### Software Setup
```bash
# Build the firmware
cd /Users/caillou/repos/caillou/mechanical-midi-piano
pio run

# Upload to Teensy
pio run -t upload

# Monitor serial output
pio device monitor -b 115200
```

## Phase 1: Pre-Integration Software Tests

These tests can be run before hardware is connected by examining the code.

### Test 1.1: Constant Values Verification

**Objective:** Verify all configuration constants are correct

**Procedure:**
1. Open `src/main.cpp`
2. Verify the following values:

| Constant | Expected Value | Actual Value | Pass/Fail |
|----------|---------------|--------------|-----------|
| `MCP23017_BOARD_COUNT` | 5 | | |
| `MCP23017_ADDRESSES[0]` | 0x20 | | |
| `MCP23017_ADDRESSES[1]` | 0x21 | | |
| `MCP23017_ADDRESSES[2]` | 0x22 | | |
| `MCP23017_ADDRESSES[3]` | 0x23 | | |
| `MCP23017_ADDRESSES[4]` | 0x24 | | |
| `NUM_CHANNELS` | 40 | | |
| `MIDI_RANGE1_LOW` | 24 | | |
| `MIDI_RANGE1_HIGH` | 35 | | |
| `MIDI_RANGE1_CHANNEL_BASE` | 0 | | |
| `MIDI_RANGE2_LOW` | 48 | | |
| `MIDI_RANGE2_HIGH` | 71 | | |
| `MIDI_RANGE2_CHANNEL_BASE` | 16 | | |

### Test 1.2: Compilation Test

**Objective:** Verify code compiles without errors or warnings

**Procedure:**
```bash
pio run 2>&1 | tee build_output.txt
```

**Expected Result:**
- No compilation errors
- No warnings (or only expected warnings)
- Binary size within Teensy 4.1 limits

**Pass Criteria:**
- [ ] Compilation succeeds
- [ ] No errors
- [ ] Warnings documented and acceptable

### Test 1.3: noteToChannel() Logic Verification

**Objective:** Verify MIDI note to channel mapping logic

**Test Cases:**

| Test | Input Note | Expected Channel | Description |
|------|------------|------------------|-------------|
| 1.3.1 | 23 | -1 | Below range 1 |
| 1.3.2 | 24 | 0 | Range 1 start (C1) |
| 1.3.3 | 25 | 1 | Range 1 |
| 1.3.4 | 35 | 11 | Range 1 end (B1) |
| 1.3.5 | 36 | -1 | Gap start (C2) |
| 1.3.6 | 47 | -1 | Gap end (B2) |
| 1.3.7 | 48 | 16 | Range 2 start (C3) |
| 1.3.8 | 49 | 17 | Range 2 |
| 1.3.9 | 60 | 28 | Middle C (C4) |
| 1.3.10 | 71 | 39 | Range 2 end (B4) |
| 1.3.11 | 72 | -1 | Above range 2 |
| 1.3.12 | 0 | -1 | Minimum MIDI |
| 1.3.13 | 127 | -1 | Maximum MIDI |

**Verification Method:** Add temporary test code or use debugger:
```cpp
// Add to setup() temporarily for verification
void testNoteToChannel() {
    struct TestCase { uint8_t note; int8_t expected; };
    TestCase tests[] = {
        {23, -1}, {24, 0}, {25, 1}, {35, 11},
        {36, -1}, {47, -1}, {48, 16}, {49, 17},
        {60, 28}, {71, 39}, {72, -1}, {0, -1}, {127, -1}
    };

    Serial.println(F("Testing noteToChannel():"));
    bool allPass = true;
    for (auto& t : tests) {
        int8_t result = noteToChannel(t.note);
        bool pass = (result == t.expected);
        if (!pass) allPass = false;
        Serial.print(F("  Note "));
        Serial.print(t.note);
        Serial.print(F(" -> Ch "));
        Serial.print(result);
        Serial.print(F(" (expected "));
        Serial.print(t.expected);
        Serial.println(pass ? F(") PASS") : F(") FAIL"));
    }
    Serial.println(allPass ? F("ALL TESTS PASSED") : F("SOME TESTS FAILED"));
}
```

### Test 1.4: channelToNote() Logic Verification

**Objective:** Verify channel to MIDI note reverse mapping

**Test Cases:**

| Test | Input Channel | Expected Note | Description |
|------|---------------|---------------|-------------|
| 1.4.1 | 0 | 24 | First channel (C1) |
| 1.4.2 | 11 | 35 | Last of range 1 (B1) |
| 1.4.3 | 12 | -1 | Unused channel |
| 1.4.4 | 15 | -1 | Unused channel |
| 1.4.5 | 16 | 48 | First of range 2 (C3) |
| 1.4.6 | 28 | 60 | Middle C channel (C4) |
| 1.4.7 | 39 | 71 | Last channel (B4) |
| 1.4.8 | 40 | -1 | Beyond range |

## Phase 2: Hardware Integration Tests

### Test 2.1: I2C Bus Scan

**Objective:** Verify all 5 boards are detected on the I2C bus

**Procedure:**
1. Connect all 5 boards with correct address jumpers
2. Upload firmware
3. Open serial monitor

**Expected Output:**
```
Initializing 5 MCP23017 board(s)...
  Board 0: address 0x20
  Board 1: address 0x21
  Board 2: address 0x22
  Board 3: address 0x23
  Board 4: address 0x24
[OK] MCP23017 initialized successfully
  SolenoidDriver initialized: 5 board(s), 40 channel(s), all OFF
```

**Failure Modes:**
| Message | Cause | Action |
|---------|-------|--------|
| "Failed to initialize MCP23017" | Board not responding | Check address/wiring |
| Only N boards found | Missing board | Check specific board address |

### Test 2.2: Individual Board Verification

**Objective:** Verify each board is at the correct address

**Procedure:** For each board (0-4):
1. Disconnect all other boards
2. Connect only the board under test
3. Verify initialization message shows correct address
4. Reconnect next board

**Results Table:**

| Board | Address | Detected | Correct Address | Pass/Fail |
|-------|---------|----------|-----------------|-----------|
| 0 | 0x20 | | | |
| 1 | 0x21 | | | |
| 2 | 0x22 | | | |
| 3 | 0x23 | | | |
| 4 | 0x24 | | | |

### Test 2.3: Status Command Test

**Objective:** Verify 's' command displays all 40 channels correctly

**Procedure:**
1. With all boards connected, type 's' in serial monitor

**Expected Output (partial):**
```
============================================================
STATUS
============================================================
Driver initialized: Yes
Boards: 5
Channels: 40 (36 mapped, 4 unused)
Channel states:
  --- Board 0 (0x20) ---
  Ch  0 (Note  24): off
  Ch  1 (Note  25): off
  ...
  Ch  7 (Note  31): off
  --- Board 1 (0x21) ---
  Ch  8 (Note  32): off
  ...
  Ch 11 (Note  35): off
  Ch 12 (unused)
  Ch 13 (unused)
  Ch 14 (unused)
  Ch 15 (unused)
  --- Board 2 (0x22) ---
  Ch 16 (Note  48): off
  ...
  --- Board 3 (0x23) ---
  Ch 24 (Note  56): off
  ...
  --- Board 4 (0x24) ---
  Ch 32 (Note  64): off
  ...
  Ch 39 (Note  71): off
============================================================
```

**Verification Checklist:**
- [ ] Shows 5 boards
- [ ] Shows 40 channels
- [ ] Channels 0-11 show notes 24-35
- [ ] Channels 12-15 show "(unused)"
- [ ] Channels 16-23 show notes 48-55
- [ ] Channels 24-31 show notes 56-63
- [ ] Channels 32-39 show notes 64-71
- [ ] Board headers appear at correct positions

### Test 2.4: Help Command Test

**Objective:** Verify 'h' command shows updated MIDI range

**Procedure:**
1. Type 'h' in serial monitor

**Expected Output (contains):**
```
MIDI: Listening for notes 24-35 (C1-B1) and 48-71 (C3-B4)
```

## Phase 3: MIDI Functional Tests

### Test 3.1: Range 1 MIDI Notes (Existing Functionality)

**Objective:** Verify notes 24-35 still work correctly

**Procedure:**
1. Connect MIDI controller
2. For each note in range 24-35:
   - Send Note On
   - Verify correct channel activates (via status or physical observation)
   - Send Note Off
   - Verify channel deactivates

**Test Matrix:**

| Note | Name | Expected Ch | Board | Activated | Deactivated | Pass |
|------|------|-------------|-------|-----------|-------------|------|
| 24 | C1 | 0 | 0x20 | | | |
| 25 | C#1 | 1 | 0x20 | | | |
| 26 | D1 | 2 | 0x20 | | | |
| 27 | D#1 | 3 | 0x20 | | | |
| 28 | E1 | 4 | 0x20 | | | |
| 29 | F1 | 5 | 0x20 | | | |
| 30 | F#1 | 6 | 0x20 | | | |
| 31 | G1 | 7 | 0x20 | | | |
| 32 | G#1 | 8 | 0x21 | | | |
| 33 | A1 | 9 | 0x21 | | | |
| 34 | A#1 | 10 | 0x21 | | | |
| 35 | B1 | 11 | 0x21 | | | |

### Test 3.2: Gap Notes (36-47) Ignored

**Objective:** Verify notes in the gap are silently ignored

**Procedure:**
1. Send Note On for each note 36-47
2. Verify NO channel activates
3. Verify NO error messages in serial monitor

**Test Matrix:**

| Note | Name | Expected | No Activation | No Error | Pass |
|------|------|----------|---------------|----------|------|
| 36 | C2 | Ignored | | | |
| 37 | C#2 | Ignored | | | |
| 38 | D2 | Ignored | | | |
| 39 | D#2 | Ignored | | | |
| 40 | E2 | Ignored | | | |
| 41 | F2 | Ignored | | | |
| 42 | F#2 | Ignored | | | |
| 43 | G2 | Ignored | | | |
| 44 | G#2 | Ignored | | | |
| 45 | A2 | Ignored | | | |
| 46 | A#2 | Ignored | | | |
| 47 | B2 | Ignored | | | |

### Test 3.3: Range 2 MIDI Notes (New Functionality)

**Objective:** Verify notes 48-71 activate correct channels

**Procedure:**
Same as Test 3.1, for the new range.

**Test Matrix:**

| Note | Name | Expected Ch | Board | Activated | Deactivated | Pass |
|------|------|-------------|-------|-----------|-------------|------|
| 48 | C3 | 16 | 0x22 | | | |
| 49 | C#3 | 17 | 0x22 | | | |
| 50 | D3 | 18 | 0x22 | | | |
| 51 | D#3 | 19 | 0x22 | | | |
| 52 | E3 | 20 | 0x22 | | | |
| 53 | F3 | 21 | 0x22 | | | |
| 54 | F#3 | 22 | 0x22 | | | |
| 55 | G3 | 23 | 0x22 | | | |
| 56 | G#3 | 24 | 0x23 | | | |
| 57 | A3 | 25 | 0x23 | | | |
| 58 | A#3 | 26 | 0x23 | | | |
| 59 | B3 | 27 | 0x23 | | | |
| 60 | C4 | 28 | 0x23 | | | |
| 61 | C#4 | 29 | 0x23 | | | |
| 62 | D4 | 30 | 0x23 | | | |
| 63 | D#4 | 31 | 0x23 | | | |
| 64 | E4 | 32 | 0x24 | | | |
| 65 | F4 | 33 | 0x24 | | | |
| 66 | F#4 | 34 | 0x24 | | | |
| 67 | G4 | 35 | 0x24 | | | |
| 68 | G#4 | 36 | 0x24 | | | |
| 69 | A4 | 37 | 0x24 | | | |
| 70 | A#4 | 38 | 0x24 | | | |
| 71 | B4 | 39 | 0x24 | | | |

### Test 3.4: Polyphonic Test

**Objective:** Verify multiple simultaneous notes work across all boards

**Procedure:**
1. Play a chord spanning multiple boards (e.g., C1 + C3 + C4)
2. Verify all notes activate simultaneously
3. Release all notes
4. Verify all notes deactivate

**Test Chords:**

| Chord | Notes | Channels | Boards | Pass |
|-------|-------|----------|--------|------|
| Test 1 | 24, 48, 64 | 0, 16, 32 | 0, 2, 4 | |
| Test 2 | 24, 32, 48, 56, 64 | 0, 8, 16, 24, 32 | all 5 | |
| Test 3 | 35, 55, 71 | 11, 23, 39 | 1, 2, 4 | |

### Test 3.5: Boundary Notes Test

**Objective:** Verify notes at range boundaries work correctly

**Test Cases:**

| Note | Position | Expected |
|------|----------|----------|
| 23 | Before range 1 | Ignored |
| 24 | Start of range 1 | Ch 0 |
| 35 | End of range 1 | Ch 11 |
| 36 | Start of gap | Ignored |
| 47 | End of gap | Ignored |
| 48 | Start of range 2 | Ch 16 |
| 71 | End of range 2 | Ch 39 |
| 72 | After range 2 | Ignored |

## Phase 4: Safety and Edge Case Tests

### Test 4.1: Emergency Stop

**Objective:** Verify 'x' command stops all channels

**Procedure:**
1. Activate multiple notes across all boards
2. Press 'x'
3. Verify ALL channels turn off immediately

**Pass Criteria:**
- [ ] All 5 boards' outputs turn off
- [ ] Serial shows "[OK] All channels deactivated"
- [ ] Status shows all channels off

### Test 4.2: Velocity 0 = Note Off

**Objective:** Verify Note On with velocity 0 is treated as Note Off

**Procedure:**
1. Send Note On (note 48, velocity 100) - channel 16 should activate
2. Send Note On (note 48, velocity 0) - channel 16 should deactivate

### Test 4.3: Invalid Channel Access

**Objective:** Verify no crash when accessing channels 12-15 directly

**Note:** These channels exist but are not mapped to MIDI notes. The SolenoidDriver should handle them gracefully if accessed programmatically.

### Test 4.4: Long Duration Stability

**Objective:** Verify system remains stable over extended operation

**Procedure:**
1. Play random MIDI sequence for 30+ minutes
2. Monitor for:
   - I2C communication errors
   - Missed notes
   - Incorrect channel activation
   - Memory leaks (via Serial memory reports if available)

## Phase 5: Performance Tests

### Test 5.1: Latency Test

**Objective:** Measure MIDI-to-solenoid latency

**Procedure:**
1. Use oscilloscope or logic analyzer
2. Trigger MIDI note and measure time to GPIO change

**Expected:** < 5ms for typical operation

### Test 5.2: Maximum Polyphony

**Objective:** Test activating all 36 mapped channels simultaneously

**Procedure:**
1. Send Note On for all 36 mapped notes at once
2. Verify all activate
3. Check power supply stability
4. Send Note Off for all

### Test 5.3: Rapid Note Sequence

**Objective:** Test fast sequential notes

**Procedure:**
1. Play fast scale ascending 24 -> 35
2. Play fast scale ascending 48 -> 71
3. Verify all notes trigger correctly with no missed notes

## Debugging Commands Reference

### Serial Monitor Commands

| Command | Action | Use Case |
|---------|--------|----------|
| `x` | Emergency stop | Kill all outputs immediately |
| `s` | Print status | See all channel states |
| `h` | Show help | Display available commands |

### I2C Debugging

If I2C issues occur, add this scan code temporarily:

```cpp
void scanI2C() {
    Serial.println(F("Scanning I2C bus..."));
    for (uint8_t addr = 0x20; addr <= 0x27; addr++) {
        Wire.beginTransmission(addr);
        uint8_t result = Wire.endTransmission();
        if (result == 0) {
            Serial.print(F("  Found: 0x"));
            Serial.println(addr, HEX);
        }
    }
    Serial.println(F("Scan complete"));
}
```

### Logic Analyzer Setup

For I2C debugging with logic analyzer:
- Channel 0: SDA (Pin 18)
- Channel 1: SCL (Pin 19)
- Protocol: I2C
- Sample rate: 2MHz minimum (5MHz+ recommended)

## Test Results Summary

| Phase | Test | Result | Notes |
|-------|------|--------|-------|
| 1.1 | Constants | | |
| 1.2 | Compilation | | |
| 1.3 | noteToChannel() | | |
| 1.4 | channelToNote() | | |
| 2.1 | I2C Scan | | |
| 2.2 | Individual Boards | | |
| 2.3 | Status Command | | |
| 2.4 | Help Command | | |
| 3.1 | Range 1 Notes | | |
| 3.2 | Gap Notes | | |
| 3.3 | Range 2 Notes | | |
| 3.4 | Polyphony | | |
| 3.5 | Boundary Notes | | |
| 4.1 | Emergency Stop | | |
| 4.2 | Velocity 0 | | |
| 4.3 | Invalid Channel | | |
| 4.4 | Stability | | |
| 5.1 | Latency | | |
| 5.2 | Max Polyphony | | |
| 5.3 | Rapid Sequence | | |

**Overall Result:** ________

**Tested By:** ________________  **Date:** ________________

**Notes:**
