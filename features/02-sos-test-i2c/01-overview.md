# SOS Morse Code Solenoid Test - Implementation Plan

## Executive Summary

This document outlines the implementation plan for adding SOS morse code functionality to the Mechanical MIDI Piano solenoid driver test program. This feature serves as a visual and audible verification that the I2C communication and solenoid control are functioning correctly.

**Project Scope**: Modify the existing `src/main.cpp` to add SOS morse code pattern playback on Channel 0 (GPA0) of the MCP23017 at address 0x20.

**Target Outcome**: A working SOS test command that produces the distinctive morse code pattern (... --- ...) on a single solenoid, verifiable both visually (solenoid actuation) and audibly (clicking sounds).

---

## Technical Overview

### What is SOS Morse Code?

| Letter | Pattern | Description |
|--------|---------|-------------|
| S | . . . | Three short signals (dits) |
| O | - - - | Three long signals (dahs) |
| S | . . . | Three short signals (dits) |

### Timing Specifications

Based on 100ms base unit timing:

| Element | Duration | Calculation |
|---------|----------|-------------|
| Dit (dot) | 100ms | 1 unit |
| Dah (dash) | 300ms | 3 units |
| Intra-character gap | 100ms | 1 unit (between dits/dahs within a letter) |
| Inter-character gap | 300ms | 3 units (between letters) |
| Word gap | 700ms | 7 units (between SOS repetitions) |

### Complete SOS Timing Breakdown

```
S       gap    O       gap    S
._._.   (3u)  -_-_-   (3u)  ._._.

Detail:
S: dit(1) + gap(1) + dit(1) + gap(1) + dit(1) = 5 units
Gap between S and O: 3 units (letter gap minus element gap already counted)
O: dah(3) + gap(1) + dah(3) + gap(1) + dah(3) = 11 units
Gap between O and S: 3 units
S: dit(1) + gap(1) + dit(1) + gap(1) + dit(1) = 5 units

Total: 5 + 3 + 11 + 3 + 5 = 27 units = 2700ms active portion
Add word gap before next cycle: 7 units = 700ms

Total cycle time: ~3400ms
```

### Duty Cycle Analysis

| Metric | Value | Limit | Status |
|--------|-------|-------|--------|
| Total ON time per cycle | ~1500ms | - | - |
| Total cycle time | ~3400ms | - | - |
| Duty cycle | ~44% | 50% max | SAFE |
| Maximum single ON time | 300ms (dah) | 5000ms max | SAFE |
| Minimum OFF time | 100ms (gaps) | 50ms min | SAFE |

---

## System Architecture

### Hardware Configuration

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              TEENSY 4.1                                      │
│                                                                              │
│   ┌─────────────┐                                                           │
│   │   Wire      │                                                           │
│   │ (Primary)   │                                                           │
│   │ SDA=Pin 18  │                                                           │
│   │ SCL=Pin 19  │                                                           │
│   └──────┬──────┘                                                           │
│          │                                                                   │
└──────────┼───────────────────────────────────────────────────────────────────┘
           │
           │ I2C Bus (400kHz)
           │
     ┌─────┴─────┐
     │           │
┌────┴──────────────┐
│ Adafruit 6318     │
│ MCP23017 @ 0x20   │
│                   │
│ Channel 0 (GPA0) ─┼──► Solenoid (SOS Test)
│ Channels 1-7     ─┼──► (Available for other tests)
│                   │
│ V+ ───────────────┼──► 12V/24V DC Supply
│ GND ──────────────┼──► Common Ground
└───────────────────┘
```

### Software Components

```
src/main.cpp
│
├── Configuration Constants
│   ├── Existing: I2C, Timing, Safety
│   └── NEW: SOS Timing Constants
│
├── Global Variables
│   ├── Existing: mcp, mcpInitialized, channelStates, etc.
│   └── NEW: sosRunning (continuous mode flag)
│
├── Function Prototypes
│   └── NEW: SOS function declarations
│
├── setup()
│   └── (No changes required)
│
├── loop()
│   └── NEW: Check sosRunning flag, call SOS sequence
│
├── SOS Functions (NEW)
│   ├── playDit() - 100ms pulse
│   ├── playDah() - 300ms pulse
│   ├── playS() - Three dits
│   ├── playO() - Three dahs
│   ├── playSOS() - Complete S-O-S sequence
│   └── stopSOS() - Emergency stop for SOS
│
├── handleSerialInput()
│   └── NEW: Add 'o' and 'c' command handlers
│
└── printHelp()
    └── NEW: Add SOS commands to help menu
```

---

## Implementation Phases

### Phase 1: Add SOS Timing Constants (15 minutes)

**Objective**: Define all morse code timing constants in the configuration section.

| Task | Description | Location |
|------|-------------|----------|
| 1.1 | Add DIT duration constant (100ms) | After line ~78 (TEST_DELAY_MS) |
| 1.2 | Add DAH duration constant (300ms) | Same location |
| 1.3 | Add element gap constant (100ms) | Same location |
| 1.4 | Add letter gap constant (300ms) | Same location |
| 1.5 | Add word gap constant (700ms) | Same location |
| 1.6 | Add SOS channel constant (0) | Same location |

**Deliverable**: New constants added to configuration section.

---

### Phase 2: Add Global State Variable (5 minutes)

**Objective**: Add flag for continuous SOS mode.

| Task | Description | Location |
|------|-------------|----------|
| 2.1 | Add `sosRunning` boolean flag | After line ~143 (channelOffTime) |

**Deliverable**: New global variable added.

---

### Phase 3: Add Function Prototypes (5 minutes)

**Objective**: Declare all new SOS-related functions.

| Task | Description | Location |
|------|-------------|----------|
| 3.1 | Add SOS function prototypes | After line ~174 (handleSerialInput prototype) |

**Deliverable**: Function prototypes added.

---

### Phase 4: Implement SOS Functions (30 minutes)

**Objective**: Implement all morse code playback functions.

| Task | Description | Location |
|------|-------------|----------|
| 4.1 | Implement `playDit()` | After testAllChannelsSimultaneous() |
| 4.2 | Implement `playDah()` | Same section |
| 4.3 | Implement `playS()` | Same section |
| 4.4 | Implement `playO()` | Same section |
| 4.5 | Implement `playSOS()` | Same section |
| 4.6 | Implement `stopSOS()` | Same section |

**Deliverable**: Complete SOS playback implementation.

---

### Phase 5: Update Main Loop (10 minutes)

**Objective**: Add continuous SOS mode support to main loop.

| Task | Description | Location |
|------|-------------|----------|
| 5.1 | Add SOS continuous mode check | After line ~256 (safety timeout block) |
| 5.2 | Call playSOS() when sosRunning is true | Same location |

**Deliverable**: Loop handles continuous mode.

---

### Phase 6: Update Serial Command Handler (15 minutes)

**Objective**: Add new serial commands for SOS control.

| Task | Description | Location |
|------|-------------|----------|
| 6.1 | Add 'o' command case for single SOS | In handleSerialInput switch |
| 6.2 | Add 'c' command case for continuous toggle | Same location |
| 6.3 | Modify 'x' handler to also stop SOS | Emergency stop case |

**Deliverable**: New commands functional.

---

### Phase 7: Update Help Menu (5 minutes)

**Objective**: Document new commands in help output.

| Task | Description | Location |
|------|-------------|----------|
| 7.1 | Add 'o' command description | In printHelp() |
| 7.2 | Add 'c' command description | Same location |
| 7.3 | Update 'x' description to mention SOS | Same location |

**Deliverable**: Help menu updated.

---

### Phase 8: Testing and Verification (30 minutes)

**Objective**: Verify all functionality works correctly.

| Task | Description | Verification |
|------|-------------|--------------|
| 8.1 | Build project | No compiler errors |
| 8.2 | Upload to Teensy | Upload succeeds |
| 8.3 | Test 'o' command | Single SOS plays |
| 8.4 | Test 'c' command | Continuous mode toggles |
| 8.5 | Test 'x' command | Stops SOS immediately |
| 8.6 | Verify timing | Measure with stopwatch |
| 8.7 | Verify duty cycle | No safety warnings |

**Deliverable**: All tests pass.

---

## Serial Commands Summary

### Existing Commands (Unchanged)

| Command | Action |
|---------|--------|
| 'r' | Re-run all tests |
| 'a' | Activate all channels for 100ms |
| 's' | Run I2C scanner |
| '0'-'7' | Toggle individual channel |
| 'h' | Show help menu |

### New Commands

| Command | Action | Description |
|---------|--------|-------------|
| 'o' | Single SOS | Play SOS pattern once on Channel 0 |
| 'c' | Continuous SOS | Toggle continuous SOS mode |
| 'x' | Emergency Stop | All channels off + stop SOS (enhanced) |

---

## Safety Requirements

### Timing Constraints Met

| Constraint | Required | Implemented | Status |
|------------|----------|-------------|--------|
| Max ON time | < 5000ms | 300ms (dah) | PASS |
| Min OFF time | > 50ms | 100ms (gap) | PASS |
| Max duty cycle | < 50% | ~44% | PASS |

### Emergency Stop Behavior

The 'x' command MUST:
1. Immediately set Channel 0 LOW
2. Set `sosRunning = false`
3. Call existing `deactivateAllChannels()`
4. Print confirmation message

### Fail-Safe Design

- SOS functions check `mcpInitialized` before any operation
- Each element checks for emergency stop between pulses
- Continuous mode can be interrupted at any time
- Main loop safety timeout still applies (5 second max)

---

## Success Criteria

### Functional Requirements

- [ ] 'o' command plays complete SOS pattern once
- [ ] 'c' command toggles continuous SOS mode
- [ ] 'x' command stops SOS immediately
- [ ] Pattern timing matches specification
- [ ] Solenoid clicks audibly for each element
- [ ] No safety timeout warnings during normal operation

### Technical Requirements

- [ ] Code compiles without errors or warnings
- [ ] All existing tests still pass
- [ ] No memory leaks (static allocation only)
- [ ] Consistent coding style with existing code

### Verification Metrics

| Metric | Expected | Acceptable Range |
|--------|----------|------------------|
| Dit duration | 100ms | 90-110ms |
| Dah duration | 300ms | 280-320ms |
| Element gap | 100ms | 90-110ms |
| Letter gap | 300ms | 280-320ms |
| Full cycle | ~3400ms | 3200-3600ms |

---

## Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| Timing drift | Low | Low | Use blocking delays (acceptable for test) |
| Solenoid overheating | Low | Medium | Duty cycle well under limit |
| Emergency stop fails | Very Low | High | Test thoroughly before deployment |
| I2C communication error | Low | Medium | Existing error handling applies |

---

## Timeline Summary

| Phase | Duration | Cumulative |
|-------|----------|------------|
| Phase 1: Constants | 15 min | 15 min |
| Phase 2: Global State | 5 min | 20 min |
| Phase 3: Prototypes | 5 min | 25 min |
| Phase 4: SOS Functions | 30 min | 55 min |
| Phase 5: Main Loop | 10 min | 65 min |
| Phase 6: Serial Handler | 15 min | 80 min |
| Phase 7: Help Menu | 5 min | 85 min |
| Phase 8: Testing | 30 min | 115 min |
| **Total** | **~2 hours** | - |

---

## Document Index

| Document | Description |
|----------|-------------|
| [01-overview.md](./01-overview.md) | This document - main implementation plan |
| [02-hardware-verification.md](./02-hardware-verification.md) | Hardware setup checklist and verification |
| [03-code-implementation.md](./03-code-implementation.md) | Detailed code changes with line numbers |
| [04-testing-procedures.md](./04-testing-procedures.md) | Testing checklist and expected outputs |

---

## Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2026-01-18 | - | Initial plan creation |
