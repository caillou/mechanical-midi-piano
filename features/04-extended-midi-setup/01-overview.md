# Extended MIDI Setup - Implementation Overview

## Document Information

| Field | Value |
|-------|-------|
| Feature ID | 04-extended-midi-setup |
| Version | 1.0 |
| Created | 2025-02-03 |
| Status | Planning |

---

## Executive Summary

This document outlines the implementation plan for extending the mechanical MIDI piano from 8 notes (single MCP23017 board) to 12 notes (two MCP23017 boards). The extension expands the MIDI note range from C4-G4 (notes 60-67) to C1-B1 (notes 24-35), providing a full chromatic octave in the bass register.

---

## Current State

### Hardware Configuration
- **Microcontroller**: Teensy 4.1
- **I2C Bus**: Wire (SDA=Pin 18, SCL=Pin 19) at 400kHz
- **GPIO Expander**: 1x MCP23017 at address 0x20 (Adafruit I2C Solenoid Driver)
- **Channels**: 8 solenoid channels (0-7)

### Software Configuration
- **MIDI Note Range**: 60-67 (C4 to G4)
- **Channel Mapping**: Note 60 -> Channel 0, Note 61 -> Channel 1, etc.
- **Configuration File**: `src/main.cpp`

### Current MIDI Mapping
| MIDI Note | Note Name | Solenoid Channel |
|-----------|-----------|------------------|
| 60 | C4 | 0 |
| 61 | C#4 | 1 |
| 62 | D4 | 2 |
| 63 | D#4 | 3 |
| 64 | E4 | 4 |
| 65 | F4 | 5 |
| 66 | F#4 | 6 |
| 67 | G4 | 7 |

---

## Target State

### Hardware Configuration
- **Microcontroller**: Teensy 4.1 (unchanged)
- **I2C Bus**: Wire (SDA=Pin 18, SCL=Pin 19) at 400kHz (unchanged)
- **GPIO Expanders**:
  - Board 0: MCP23017 at address 0x20 (existing Adafruit I2C Solenoid Driver)
  - Board 1: MCP23017 at address 0x21 (new board)
- **Channels**: 12 solenoid channels (0-11)

### Software Configuration
- **MIDI Note Range**: 24-35 (C1 to B1)
- **Channel Mapping**: Note 24 -> Channel 0, Note 25 -> Channel 1, etc.
- **Configuration File**: `src/main.cpp` (only file that changes)

### Target MIDI Mapping
| MIDI Note | Note Name | Global Channel | Board Address | Local Channel |
|-----------|-----------|----------------|---------------|---------------|
| 24 | C1 | 0 | 0x20 | 0 |
| 25 | C#1 | 1 | 0x20 | 1 |
| 26 | D1 | 2 | 0x20 | 2 |
| 27 | D#1 | 3 | 0x20 | 3 |
| 28 | E1 | 4 | 0x20 | 4 |
| 29 | F1 | 5 | 0x20 | 5 |
| 30 | F#1 | 6 | 0x20 | 6 |
| 31 | G1 | 7 | 0x20 | 7 |
| 32 | G#1 | 8 | 0x21 | 0 |
| 33 | A1 | 9 | 0x21 | 1 |
| 34 | A#1 | 10 | 0x21 | 2 |
| 35 | B1 | 11 | 0x21 | 3 |

---

## Scope

### In Scope
- Modifications to `src/main.cpp` only
- Configuration constant changes
- Initialization function updates
- Documentation updates within `main.cpp`

### Out of Scope
- SolenoidDriver library changes (already supports multiple boards)
- Hardware wiring changes (documented in hardware checklist only)
- New library dependencies
- Changes to `platformio.ini`

---

## Implementation Phases

### Phase 1: Code Preparation (15 minutes)
**Objective**: Prepare the codebase for multi-board support.

**Tasks**:
1. Update file header documentation
2. Change `MCP23017_DEFAULT_ADDRESS` from single value to array
3. Update `MIDI_NOTE_LOW` constant
4. Update `MIDI_NOTE_HIGH` constant
5. Update `NUM_CHANNELS` constant

**Verification**: Code compiles successfully

### Phase 2: Initialization Update (15 minutes)
**Objective**: Update MCP23017 initialization to support two boards.

**Tasks**:
1. Modify `initMCP23017()` to use array-based initialization
2. Update initialization log messages
3. Ensure error handling works for multi-board setup

**Verification**: Code compiles and initializes both boards (with hardware connected)

### Phase 3: UI/Display Updates (10 minutes)
**Objective**: Update user-facing messages and status displays.

**Tasks**:
1. Update MIDI range display in setup messages
2. Update `printHelp()` function
3. Update `printStatus()` function for 12 channels

**Verification**: Serial output shows correct information

### Phase 4: Integration Testing (20 minutes)
**Objective**: Verify complete system functionality.

**Tasks**:
1. Hardware verification with I2C scanner
2. Test each of the 12 MIDI notes
3. Verify emergency stop works for all channels
4. Performance testing

**Verification**: All 12 notes trigger correct solenoids

---

## File Changes Summary

### Files Modified
| File | Changes |
|------|---------|
| `src/main.cpp` | Constants, init function, display functions |

### Files Unchanged
| File | Reason |
|------|--------|
| `lib/SolenoidDriver/SolenoidDriver.h` | Already supports multiple boards |
| `lib/SolenoidDriver/SolenoidDriver.cpp` | Already supports multiple boards |
| `lib/SolenoidDriver/SolenoidConfig.h` | No changes needed |
| `lib/SolenoidDriver/SolenoidChannel.h` | No changes needed |
| `lib/SolenoidDriver/SolenoidChannel.cpp` | No changes needed |
| `platformio.ini` | No changes needed |

---

## Constants Change Summary

| Constant | Current Value | New Value | Location (Line) |
|----------|---------------|-----------|-----------------|
| `MIDI_NOTE_LOW` | 60 | 24 | Line 90 |
| `MIDI_NOTE_HIGH` | 67 | 35 | Line 96 |
| `NUM_CHANNELS` | 8 | 12 | Line 61 |
| `MCP23017_DEFAULT_ADDRESS` | `0x20` | `{0x20, 0x21}` | Line 51 |

---

## Risk Assessment

### Low Risk
- **Compile errors**: Mitigated by incremental changes and testing
- **I2C address conflicts**: Mitigated by using standard addresses 0x20 and 0x21

### Medium Risk
- **Hardware not detected**: Mitigated by I2C scanner verification step
- **Incorrect wiring**: Mitigated by hardware checklist and verification procedures

### Mitigation Strategies
1. Make changes incrementally
2. Compile after each phase
3. Test with hardware after Phase 2
4. Use I2C scanner to verify hardware before software testing

---

## Dependencies

### Hardware Requirements
- Second MCP23017 board (Adafruit I2C Solenoid Driver recommended)
- Address jumpers set to 0x21 (A0=1, A1=0, A2=0)
- I2C wiring (shared SDA/SCL bus)
- 4 additional solenoids for channels 8-11

### Software Requirements
- PlatformIO CLI or IDE
- Teensy 4.1 board support
- No additional libraries required

---

## Related Documentation

| Document | Description |
|----------|-------------|
| `02-technical-specification.md` | Detailed code changes with before/after snippets |
| `03-testing-plan.md` | Comprehensive testing procedures |
| `04-hardware-checklist.md` | Hardware setup and verification steps |

---

## Assumptions

1. The second MCP23017 board is physically identical to the first (Adafruit I2C Solenoid Driver)
2. The second board has its address pins configured for 0x21 (A0=HIGH, A1=LOW, A2=LOW)
3. Both boards share the same I2C bus (Wire)
4. The SolenoidDriver library version supports `begin(TwoWire&, const uint8_t[], uint8_t)` method
5. Channels 8-11 on the second board (local channels 0-3) will be wired to solenoids
6. Channels 4-7 on the second board will remain unused (available for future expansion)

---

## Rollback Plan

If issues arise during implementation:

1. **Revert code changes**:
   ```bash
   git checkout src/main.cpp
   ```

2. **Hardware rollback**:
   - Disconnect second MCP23017 board
   - System will function with original 8-note configuration

3. **Partial rollback** (keep 12 channels but different note range):
   - Only modify `MIDI_NOTE_LOW` and `MIDI_NOTE_HIGH` to desired range
   - The channel count and board configuration remain unchanged
