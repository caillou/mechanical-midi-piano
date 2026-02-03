# Feature 05: Expand MIDI Piano from 12 to 36 Channels

## Executive Summary

This feature expands the mechanical MIDI piano from 2 MCP23017 boards (12 channels) to 5 MCP23017 boards (40 allocated channels, 36 used). The expansion adds support for MIDI notes spanning from octave 1 (C1-B1) through octave 4 (E4-B4), enabling a much wider range of playable notes.

**Key Changes:**
- Add 3 new MCP23017 boards at addresses 0x22, 0x23, 0x24
- Increase channel count from 12 to 40
- Implement non-contiguous MIDI note mapping with a gap at notes 36-47
- Update all status and debugging functions to handle the new mapping

## System Architecture

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│                              TEENSY 4.1                                         │
│                                                                                 │
│  ┌─────────────┐    ┌──────────────────────────────────────────────────────┐   │
│  │   USB MIDI  │    │                    I2C BUS (Wire)                    │   │
│  │   INPUT     │    │              SDA=Pin 18, SCL=Pin 19                  │   │
│  │             │    │                   400 kHz                             │   │
│  │ Notes:      │    └──────────────────────────────────────────────────────┘   │
│  │  24-35      │                           │                                    │
│  │  48-71      │                           │                                    │
│  └─────────────┘                           │                                    │
└────────────────────────────────────────────┼────────────────────────────────────┘
                                             │
     ┌───────────────────────────────────────┼───────────────────────────────────┐
     │                                       │                                   │
     │                        I2C Bus (4.7kΩ pull-ups)                          │
     │                                       │                                   │
     ├───────────┬───────────┬───────────┬───────────┬───────────┐              │
     │           │           │           │           │           │              │
     ▼           ▼           ▼           ▼           ▼           │              │
┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐      │              │
│MCP23017 │ │MCP23017 │ │MCP23017 │ │MCP23017 │ │MCP23017 │      │              │
│ Board 0 │ │ Board 1 │ │ Board 2 │ │ Board 3 │ │ Board 4 │      │              │
│  0x20   │ │  0x21   │ │  0x22   │ │  0x23   │ │  0x24   │      │              │
│ A2=0    │ │ A2=0    │ │ A2=0    │ │ A2=0    │ │ A2=1    │      │              │
│ A1=0    │ │ A1=0    │ │ A1=1    │ │ A1=1    │ │ A1=0    │      │              │
│ A0=0    │ │ A0=1    │ │ A0=0    │ │ A0=1    │ │ A0=0    │      │              │
├─────────┤ ├─────────┤ ├─────────┤ ├─────────┤ ├─────────┤      │              │
│ Ch 0-7  │ │ Ch 8-15 │ │Ch 16-23 │ │Ch 24-31 │ │Ch 32-39 │      │              │
│ (8 used)│ │ (4 used)│ │ (8 used)│ │ (8 used)│ │ (8 used)│      │              │
└────┬────┘ └────┬────┘ └────┬────┘ └────┬────┘ └────┬────┘      │              │
     │           │           │           │           │           │              │
     ▼           ▼           ▼           ▼           ▼           │              │
┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐      │              │
│Solenoids│ │Solenoids│ │Solenoids│ │Solenoids│ │Solenoids│      │              │
│  0-7    │ │  8-11   │ │ 16-23   │ │ 24-31   │ │ 32-39   │      │              │
│ Notes   │ │ Notes   │ │ Notes   │ │ Notes   │ │ Notes   │      │              │
│ 24-31   │ │ 32-35   │ │ 48-55   │ │ 56-63   │ │ 64-71   │      │              │
│ C1-G1   │ │G#1-B1   │ │ C3-G3   │ │G#3-D#4  │ │ E4-B4   │      │              │
└─────────┘ └─────────┘ └─────────┘ └─────────┘ └─────────┘      │              │
                                                                  │              │
     └────────────────────────────────────────────────────────────┘              │
```

## MIDI Note to Channel Mapping

### Complete Mapping Table

| MIDI Note | Note Name | Channel | Board | Board Addr | Local Pin | Status |
|-----------|-----------|---------|-------|------------|-----------|--------|
| 24 | C1 | 0 | 0 | 0x20 | 0 | Existing |
| 25 | C#1 | 1 | 0 | 0x20 | 1 | Existing |
| 26 | D1 | 2 | 0 | 0x20 | 2 | Existing |
| 27 | D#1 | 3 | 0 | 0x20 | 3 | Existing |
| 28 | E1 | 4 | 0 | 0x20 | 4 | Existing |
| 29 | F1 | 5 | 0 | 0x20 | 5 | Existing |
| 30 | F#1 | 6 | 0 | 0x20 | 6 | Existing |
| 31 | G1 | 7 | 0 | 0x20 | 7 | Existing |
| 32 | G#1 | 8 | 1 | 0x21 | 0 | Existing |
| 33 | A1 | 9 | 1 | 0x21 | 1 | Existing |
| 34 | A#1 | 10 | 1 | 0x21 | 2 | Existing |
| 35 | B1 | 11 | 1 | 0x21 | 3 | Existing |
| 36-47 | C2-B2 | - | - | - | - | **UNMAPPED (gap)** |
| 48 | C3 | 16 | 2 | 0x22 | 0 | **NEW** |
| 49 | C#3 | 17 | 2 | 0x22 | 1 | **NEW** |
| 50 | D3 | 18 | 2 | 0x22 | 2 | **NEW** |
| 51 | D#3 | 19 | 2 | 0x22 | 3 | **NEW** |
| 52 | E3 | 20 | 2 | 0x22 | 4 | **NEW** |
| 53 | F3 | 21 | 2 | 0x22 | 5 | **NEW** |
| 54 | F#3 | 22 | 2 | 0x22 | 6 | **NEW** |
| 55 | G3 | 23 | 2 | 0x22 | 7 | **NEW** |
| 56 | G#3 | 24 | 3 | 0x23 | 0 | **NEW** |
| 57 | A3 | 25 | 3 | 0x23 | 1 | **NEW** |
| 58 | A#3 | 26 | 3 | 0x23 | 2 | **NEW** |
| 59 | B3 | 27 | 3 | 0x23 | 3 | **NEW** |
| 60 | C4 | 28 | 3 | 0x23 | 4 | **NEW** |
| 61 | C#4 | 29 | 3 | 0x23 | 5 | **NEW** |
| 62 | D4 | 30 | 3 | 0x23 | 6 | **NEW** |
| 63 | D#4 | 31 | 3 | 0x23 | 7 | **NEW** |
| 64 | E4 | 32 | 4 | 0x24 | 0 | **NEW** |
| 65 | F4 | 33 | 4 | 0x24 | 1 | **NEW** |
| 66 | F#4 | 34 | 4 | 0x24 | 2 | **NEW** |
| 67 | G4 | 35 | 4 | 0x24 | 3 | **NEW** |
| 68 | G#4 | 36 | 4 | 0x24 | 4 | **NEW** |
| 69 | A4 | 37 | 4 | 0x24 | 5 | **NEW** |
| 70 | A#4 | 38 | 4 | 0x24 | 6 | **NEW** |
| 71 | B4 | 39 | 4 | 0x24 | 7 | **NEW** |

### Channel Utilization Summary

| Board | Address | Channels | Used | Notes Mapped | Status |
|-------|---------|----------|------|--------------|--------|
| 0 | 0x20 | 0-7 | 8/8 | 24-31 (C1-G1) | Existing |
| 1 | 0x21 | 8-15 | 4/8 | 32-35 (G#1-B1) | Existing (4 unused) |
| 2 | 0x22 | 16-23 | 8/8 | 48-55 (C3-G3) | **NEW** |
| 3 | 0x23 | 24-31 | 8/8 | 56-63 (G#3-D#4) | **NEW** |
| 4 | 0x24 | 32-39 | 8/8 | 64-71 (E4-B4) | **NEW** |

**Total: 40 channels allocated, 36 channels used, 4 channels unused (12-15 on board 0x21)**

### Mapping Visualization

```
MIDI Notes:   24 25 26 27 28 29 30 31 | 32 33 34 35 | 36-47 | 48 49 50 51 52 53 54 55 | 56 57 58 59 60 61 62 63 | 64 65 66 67 68 69 70 71
              C1 C# D  D# E  F  F# G  | G# A  A# B  | (gap) | C3 C# D  D# E  F  F# G  | G# A  A# B  C4 C# D  D# | E  F  F# G  G# A  A# B
              ─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
Channels:      0  1  2  3  4  5  6  7 |  8  9 10 11 | none  | 16 17 18 19 20 21 22 23 | 24 25 26 27 28 29 30 31 | 32 33 34 35 36 37 38 39
Board:         ──── Board 0 (0x20) ───│─ Board 1 ──│       │──── Board 2 (0x22) ─────│──── Board 3 (0x23) ─────│──── Board 4 (0x24) ────
                                      │   (0x21)   │       │                         │                         │
Unused:                               │  Ch 12-15  │       │                         │                         │
```

## Goals

1. **Maintain backward compatibility** - Existing MIDI notes 24-35 continue to work identically
2. **Add 24 new playable notes** - Notes 48-71 (C3-B4) mapped to new hardware
3. **No library changes** - SolenoidDriver library already supports up to 8 boards
4. **Clean code architecture** - Proper handling of non-contiguous MIDI ranges
5. **Full debugging support** - Status functions correctly display all channels and their MIDI mappings

## Non-Goals

1. **Filling the gap at notes 36-47** - These octave 2 notes remain unmapped (no physical keys)
2. **Using channels 12-15** - Board 0x21 pins 4-7 remain unused for this phase
3. **Changing timing parameters** - MAX_ON_TIME_MS, MIN_OFF_TIME_MS unchanged
4. **Library modifications** - SolenoidDriver remains unmodified
5. **Dynamic MIDI mapping** - Mapping remains compile-time constants

## Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| I2C bus issues with 5 devices | Low | High | Proper pull-up resistors, short wiring |
| Address conflicts | Low | High | Verify jumper settings before power-on |
| Power supply overload | Medium | High | Size power supply for worst case (all 36 on) |
| Software regression | Low | Medium | Comprehensive testing of existing functionality |

## Success Criteria

1. All 5 MCP23017 boards initialize successfully
2. MIDI notes 24-35 function identically to before
3. MIDI notes 48-71 correctly trigger channels 16-39
4. Notes 36-47 are silently ignored (no error)
5. Status command shows correct note-to-channel mapping for all channels
6. I2C bus remains stable under load

## Files to Modify

| File | Changes |
|------|---------|
| `src/main.cpp` | Configuration constants, MIDI mapping logic, status functions |

## Files NOT Modified

| File | Reason |
|------|--------|
| `lib/SolenoidDriver/SolenoidConfig.h` | Already supports 8 boards, 128 channels |
| `lib/SolenoidDriver/SolenoidDriver.h` | No changes needed |
| `lib/SolenoidDriver/SolenoidDriver.cpp` | No changes needed |
| `lib/SolenoidDriver/SolenoidChannel.h` | No changes needed |
| `lib/SolenoidDriver/SolenoidChannel.cpp` | No changes needed |

## Implementation Phases

1. **Phase 1: Configuration Constants** - Update board count, addresses, channel count
2. **Phase 2: MIDI Mapping Logic** - Rewrite noteToChannel() for dual ranges
3. **Phase 3: Reverse Mapping** - Add channelToNote() for status display
4. **Phase 4: Status Functions** - Update printStatus(), printHelp()
5. **Phase 5: Documentation** - Update header comments
6. **Phase 6: Hardware Integration** - Wire new boards, test
