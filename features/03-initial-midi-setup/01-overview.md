# Feature 03: Initial MIDI Setup

## Overview

This document describes the implementation plan for converting the Teensy 4.1 solenoid controller into a USB MIDI instrument. After implementation, the Teensy will:

1. Register as a USB MIDI device when connected to a computer
2. Respond to MIDI note-on/note-off messages
3. Map MIDI notes 60-67 (C4 through G4) to solenoid channels 0-7
4. Retain serial debugging capability via composite USB device

## Document Structure

| File | Purpose |
|------|---------|
| `01-overview.md` | This file - high-level summary and implementation phases |
| `02-platformio-changes.md` | Detailed platformio.ini modifications |
| `03-main-cpp-changes.md` | Complete main.cpp restructure with line-by-line changes |
| `04-testing-verification.md` | Testing procedures and verification steps |

## Implementation Phases

### Phase 1: Platform Configuration (5 minutes)

**File:** `/Users/caillou/repos/caillou/mechanical-midi-piano/platformio.ini`

Change the USB device type from serial-only to composite MIDI+Serial.

**Summary:**
- Change `-D USB_SERIAL` to `-D USB_MIDI_SERIAL`
- No additional library dependencies needed

**Details:** See `02-platformio-changes.md`

---

### Phase 2: Remove Test Code (15 minutes)

**File:** `/Users/caillou/repos/caillou/mechanical-midi-piano/src/main.cpp`

Remove all test-related functions and serial commands that are no longer needed.

**Functions to Remove:**
| Function | Lines | Reason |
|----------|-------|--------|
| `scanI2CBus()` | 327-361 | Test utility, not needed for MIDI operation |
| `toggleChannel()` | 376-387 | Replaced by MIDI control |
| `setChannel()` | 399-424 | Internal helper, replaced by direct SolenoidDriver calls |
| `setAllChannels()` | 434-452 | Test utility |
| `activateChannel()` | 463-483 | Test utility (blocking) |
| `runAllTests()` | 510-540 | Test suite |
| `testSequentialChannels()` | 573-607 | Test function |
| `testAllChannelsSimultaneous()` | 614-636 | Test function |
| `testCommunication()` | 547-565 | Test function |

**Constants to Remove:**
| Constant | Line | Reason |
|----------|------|--------|
| `TEST_ACTIVATION_MS` | 89 | Test-only |
| `TEST_DELAY_MS` | 94 | Test-only |

**Details:** See `03-main-cpp-changes.md`

---

### Phase 3: Add MIDI Infrastructure (20 minutes)

**File:** `/Users/caillou/repos/caillou/mechanical-midi-piano/src/main.cpp`

Add MIDI note mapping and callback handlers.

**New Constants:**
```cpp
constexpr uint8_t MIDI_NOTE_LOW = 60;   // C4 - maps to channel 0
constexpr uint8_t MIDI_NOTE_HIGH = 67;  // G4 - maps to channel 7
```

**New Functions:**
| Function | Purpose |
|----------|---------|
| `noteToChannel()` | Convert MIDI note number to solenoid channel |
| `handleNoteOn()` | MIDI note-on callback |
| `handleNoteOff()` | MIDI note-off callback |

**Details:** See `03-main-cpp-changes.md`

---

### Phase 4: Update Setup and Loop (10 minutes)

**File:** `/Users/caillou/repos/caillou/mechanical-midi-piano/src/main.cpp`

Modify `setup()` and `loop()` for MIDI operation.

**Setup Changes:**
- Remove: Call to `runAllTests()`
- Remove: Call to `scanI2CBus()`
- Add: MIDI callback registration

**Loop Changes:**
- Add: `usbMIDI.read()` call for processing MIDI messages
- Remove: `delay(1)` for better latency
- Keep: `solenoidDriver.update()` for safety monitoring
- Keep: `handleSerialInput()` for emergency stop

**Details:** See `03-main-cpp-changes.md`

---

### Phase 5: Simplify Serial Commands (5 minutes)

**File:** `/Users/caillou/repos/caillou/mechanical-midi-piano/src/main.cpp`

Reduce serial commands to essential debugging/safety features only.

**Commands to Keep:**
| Command | Action |
|---------|--------|
| `x` / `X` | Emergency stop - all solenoids off |
| `s` / `S` | Status - print driver state |
| `h` / `H` / `?` | Help menu |

**Commands to Remove:**
| Command | Reason |
|---------|--------|
| `r` / `R` | No more tests to run |
| `a` / `A` | Replaced by MIDI control |
| `0`-`7` | Replaced by MIDI control |

**Details:** See `03-main-cpp-changes.md`

---

### Phase 6: Tune Safety Parameters (Optional, 5 minutes)

**File:** `/Users/caillou/repos/caillou/mechanical-midi-piano/src/main.cpp`

Adjust safety timing for musical performance.

**Recommended Changes:**
| Parameter | Current | Recommended | Reason |
|-----------|---------|-------------|--------|
| `MIN_OFF_TIME_MS` | 50 | 15 | Allow faster trills and repeated notes |
| `MAX_ON_TIME_MS` | 5000 | 2000 | Piano notes don't sustain 5 seconds |

**Note:** These can be tuned later based on actual solenoid performance and heat generation.

**Details:** See `03-main-cpp-changes.md`

---

## Architecture Summary

### Before (Test Mode)
```
USB Serial <---> Teensy 4.1 <---> MCP23017 <---> Solenoids
                     |
              Serial Commands
              (0-7, r, a, x, h)
```

### After (MIDI Mode)
```
USB MIDI -----> Teensy 4.1 <---> MCP23017 <---> Solenoids
USB Serial <--------|
                    |
              MIDI Note On/Off
              (Notes 60-67)
              +
              Serial Commands
              (x, s, h only)
```

### MIDI Note Mapping

| MIDI Note | Note Name | Solenoid Channel |
|-----------|-----------|------------------|
| 60 | C4 (Middle C) | 0 |
| 61 | C#4 | 1 |
| 62 | D4 | 2 |
| 63 | D#4 | 3 |
| 64 | E4 | 4 |
| 65 | F4 | 5 |
| 66 | F#4 | 6 |
| 67 | G4 | 7 |

---

## Technical Decisions

### Why USB_MIDI_SERIAL instead of USB_MIDI?

The composite device (`USB_MIDI_SERIAL`) provides both MIDI and Serial over USB. This allows:
- MIDI control for normal operation
- Serial debugging for troubleshooting
- Emergency stop via serial console
- No hardware changes needed for debugging

### Why velocity 0 == note-off?

Some MIDI devices send note-on with velocity 0 instead of note-off messages. The implementation handles this in `handleNoteOn()`:
```cpp
if (velocity == 0) { handleNoteOff(channel, note, velocity); return; }
```

### Why no delay in loop?

The original `delay(1)` was removed because:
- MIDI timing is critical for musical performance
- `usbMIDI.read()` and `solenoidDriver.update()` are non-blocking
- Removing delay improves responsiveness

### Why keep solenoidDriver.update()?

The `update()` function is essential for safety:
- Enforces maximum on-time limits
- Auto-shutoff if note-off is missed
- Prevents solenoid overheating

---

## File Change Summary

| File | Changes |
|------|---------|
| `platformio.ini` | 1 line modified |
| `src/main.cpp` | ~300 lines removed, ~50 lines added |

---

## Implementation Checklist

- [ ] **Phase 1:** Modify platformio.ini
- [ ] **Phase 2:** Remove test functions from main.cpp
- [ ] **Phase 3:** Add MIDI constants and handlers
- [ ] **Phase 4:** Update setup() and loop()
- [ ] **Phase 5:** Simplify serial commands
- [ ] **Phase 6:** (Optional) Tune safety parameters
- [ ] **Verification:** Upload and test with MIDI software

---

## Next Steps

After completing this feature:
1. Test with a MIDI keyboard or DAW
2. Verify emergency stop still works
3. Monitor solenoid temperature during extended play
4. Consider adding MIDI channel filtering (currently responds to all channels)
5. Consider adding velocity-to-pulse-duration mapping for dynamics
