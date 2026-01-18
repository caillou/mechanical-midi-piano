# Mechanical MIDI Piano - Initial Setup Implementation Plan

## Executive Summary

This document outlines the complete implementation plan for integrating the Adafruit I2C to 8 Channel Solenoid Driver (Product ID 6318) with a Teensy 4.1 microcontroller. This forms the foundational hardware control layer for a mechanical MIDI piano project capable of driving up to 88 solenoid-actuated piano keys.

**Project Scope**: Establish reliable I2C communication between Teensy 4.1 and one or more MCP23017-based solenoid driver boards, implement safety-constrained solenoid control, and prepare the architecture for MIDI integration.

**Target Outcome**: A verified, safe, and scalable solenoid control system with comprehensive test coverage and clear API for future MIDI integration.

---

## Project Goals

### Primary Goals

1. **Hardware Verification**: Confirm electrical connections and I2C communication between Teensy 4.1 and Adafruit Solenoid Driver board
2. **Safe Solenoid Control**: Implement software-enforced safety limits to prevent solenoid damage from overheating
3. **Scalable Architecture**: Design for 88-key expansion using multiple I2C buses and driver boards
4. **Documented API**: Create a clean, well-documented SolenoidDriver library for future MIDI integration

### Secondary Goals

1. **Diagnostic Tools**: Build comprehensive I2C scanning and testing utilities
2. **Error Handling**: Implement robust error detection and recovery mechanisms
3. **Performance Baseline**: Establish timing benchmarks for solenoid response latency

---

## System Architecture Overview

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              TEENSY 4.1                                      │
│                                                                              │
│   ┌─────────────┐    ┌─────────────┐    ┌─────────────┐                     │
│   │   Wire      │    │   Wire1     │    │   Wire2     │                     │
│   │ (Primary)   │    │ (Secondary) │    │ (Tertiary)  │                     │
│   │ SDA=Pin 18  │    │ SDA=Pin 17  │    │ SDA=Pin 25  │                     │
│   │ SCL=Pin 19  │    │ SCL=Pin 16  │    │ SCL=Pin 24  │                     │
│   └──────┬──────┘    └──────┬──────┘    └─────────────┘                     │
│          │                  │                                                │
└──────────┼──────────────────┼────────────────────────────────────────────────┘
           │                  │
           │ I2C Bus 0        │ I2C Bus 1
           │ (400kHz)         │ (400kHz)
           │                  │
     ┌─────┴─────┐      ┌─────┴─────┐
     │           │      │           │
┌────┴────┐ ┌────┴────┐ ┌────┴────┐
│ Driver  │ │ Driver  │ │ Driver  │    ... up to 8 per bus
│  0x20   │ │  0x21   │ │  0x20   │
│ Ch 0-7  │ │ Ch 8-15 │ │ Ch 64-71│
└────┬────┘ └────┬────┘ └────┬────┘
     │           │           │
     ▼           ▼           ▼
  Solenoids   Solenoids   Solenoids
   (Keys)      (Keys)      (Keys)
```

### 88-Key Scaling Strategy

| I2C Bus | Address Range | Driver Boards | Channels | Piano Keys |
|---------|---------------|---------------|----------|------------|
| Wire    | 0x20 - 0x27   | 8 boards      | 0-63     | Keys 1-64  |
| Wire1   | 0x20 - 0x22   | 3 boards      | 64-87    | Keys 65-88 |
| **Total** | -           | **11 boards** | **88**   | **Full piano** |

---

## Phase Breakdown

### Phase 1: Hardware Setup and Verification (Days 1-2)

**Objective**: Physically assemble and verify all electrical connections.

| Task | Description | Duration | Deliverable |
|------|-------------|----------|-------------|
| 1.1 | Gather all components | 2 hours | Component checklist signed off |
| 1.2 | Wire Teensy to driver board (I2C + power) | 1 hour | Wiring complete |
| 1.3 | Connect test solenoid with separate power | 1 hour | Solenoid connected |
| 1.4 | Visual inspection and continuity testing | 1 hour | Verification checklist |
| 1.5 | Power-on test (no code) | 30 min | No smoke, correct voltages |

**Success Criteria**:
- [ ] 3.3V measured at driver board Vcc pin
- [ ] I2C lines show ~3.3V with no load (pulled high)
- [ ] Solenoid power supply delivers correct voltage (12V or 24V as specified)
- [ ] All ground connections verified with continuity tester

**Detailed Documentation**: See [02-hardware-setup.md](./02-hardware-setup.md)

---

### Phase 2: Basic I2C Communication Test (Days 2-3)

**Objective**: Establish and verify I2C communication with the MCP23017.

| Task | Description | Duration | Deliverable |
|------|-------------|----------|-------------|
| 2.1 | Update platformio.ini with MCP23017 library | 30 min | Build succeeds |
| 2.2 | Implement I2C bus scanner | 1 hour | Scanner code |
| 2.3 | Verify device at address 0x20 | 30 min | Address confirmed |
| 2.4 | Test register read/write | 2 hours | Register I/O working |
| 2.5 | Implement communication error detection | 2 hours | Error handling code |

**Success Criteria**:
- [ ] I2C scanner detects device at 0x20
- [ ] Can read MCP23017 device ID/registers
- [ ] Can write to IODIRA register and read back same value
- [ ] Error detection triggers on I2C failure (simulated by disconnecting SDA)

**Key Registers to Verify**:

| Register | Address | Purpose | Expected Default |
|----------|---------|---------|------------------|
| IODIRA   | 0x00    | Port A direction | 0xFF (all inputs) |
| IODIRB   | 0x01    | Port B direction | 0xFF (all inputs) |
| GPIOA    | 0x12    | Port A I/O | 0x00 |
| GPIOB    | 0x13    | Port B I/O | 0x00 |

**Detailed Documentation**: See [03-software-implementation.md](./03-software-implementation.md)

---

### Phase 3: Solenoid Control Implementation (Days 3-4)

**Objective**: Implement basic solenoid on/off control with proper timing.

| Task | Description | Duration | Deliverable |
|------|-------------|----------|-------------|
| 3.1 | Configure Port A as outputs | 30 min | Outputs configured |
| 3.2 | Implement single channel control | 1 hour | digitalWrite working |
| 3.3 | Implement multi-channel control | 1 hour | writeGPIOA working |
| 3.4 | Sequential channel test (cycling) | 1 hour | All 8 channels verified |
| 3.5 | Simultaneous activation test | 1 hour | All channels at once |

**Success Criteria**:
- [ ] Individual solenoid activates on command
- [ ] Solenoid deactivates on command
- [ ] All 8 channels cycle correctly in sequence
- [ ] All 8 channels activate simultaneously
- [ ] No cross-talk between channels

**Test Parameters**:
- Activation duration: 100ms
- Off duration between activations: 200ms
- Cycle repeat count: 3

---

### Phase 4: Safety Features and Error Handling (Days 4-5)

**Objective**: Implement safety constraints and robust error handling.

| Task | Description | Duration | Deliverable |
|------|-------------|----------|-------------|
| 4.1 | Implement maximum on-time limit | 2 hours | Auto-shutoff at 5000ms |
| 4.2 | Implement minimum off-time enforcement | 2 hours | 50ms cooldown |
| 4.3 | Add duty cycle monitoring | 2 hours | 50% duty cycle limit |
| 4.4 | Implement I2C timeout handling | 2 hours | Timeout recovery |
| 4.5 | Add emergency stop (all off) | 1 hour | E-stop function |
| 4.6 | Create diagnostic reporting | 2 hours | Status logging |

**Success Criteria**:
- [ ] Solenoid automatically turns off after 5000ms continuous activation
- [ ] Rapid re-activation blocked if <50ms since last off
- [ ] Duty cycle warning generated at >50%
- [ ] I2C communication loss triggers all-off state
- [ ] Emergency stop disables all channels within 1ms

**Safety Parameters Summary**:

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| MAX_ON_TIME_MS | 5000 | Prevent solenoid coil overheating |
| MIN_OFF_TIME_MS | 50 | Allow coil cooling between activations |
| MAX_DUTY_CYCLE | 0.5 (50%) | Manufacturer thermal limits |
| I2C_TIMEOUT_MS | 100 | Detect communication loss quickly |
| TEST_ACTIVATION_MS | 100 | Safe test duration |

**Detailed Documentation**: See [04-api-design.md](./04-api-design.md)

---

### Phase 5: MIDI Integration Preparation (Days 5-6)

**Objective**: Prepare the architecture for receiving MIDI note-on/note-off messages.

| Task | Description | Duration | Deliverable |
|------|-------------|----------|-------------|
| 5.1 | Define MIDI note to channel mapping | 2 hours | Mapping table |
| 5.2 | Design velocity-to-duration curves | 2 hours | Duration algorithm |
| 5.3 | Implement note queue/scheduler | 3 hours | Scheduler code |
| 5.4 | Add USB MIDI library integration | 2 hours | MIDI receive working |
| 5.5 | Create MIDI-to-solenoid bridge | 3 hours | Bridge implementation |

**Success Criteria**:
- [ ] MIDI note 60 (Middle C) activates corresponding solenoid
- [ ] Note velocity affects solenoid activation duration
- [ ] Note-off message deactivates solenoid
- [ ] Polyphonic playback (multiple simultaneous notes) works

**MIDI Mapping Preview**:

| MIDI Note | Note Name | Channel | Board | Address |
|-----------|-----------|---------|-------|---------|
| 21 | A0 | 0 | 0 | 0x20 |
| 22 | A#0 | 1 | 0 | 0x20 |
| ... | ... | ... | ... | ... |
| 60 | C4 | 39 | 4 | 0x24 |
| ... | ... | ... | ... | ... |
| 108 | C8 | 87 | 10 | 0x22 (Wire1) |

---

## Dependencies and Prerequisites

### Hardware Requirements

| Component | Quantity | Part Number | Source | Est. Cost |
|-----------|----------|-------------|--------|-----------|
| Teensy 4.1 | 1 | DEV-16771 | PJRC/SparkFun | $31.50 |
| Adafruit I2C Solenoid Driver | 1+ | 6318 | Adafruit | $9.95 |
| DC Solenoid (test) | 1+ | Various | - | $5-15 |
| DC Power Supply (12V/24V) | 1 | - | - | $15-30 |
| Breadboard + Jumper Wires | 1 kit | - | - | $10 |
| Multimeter | 1 | - | - | - |

### Software Requirements

| Dependency | Version | Purpose |
|------------|---------|---------|
| PlatformIO Core | 6.1+ | Build system |
| Arduino Framework (Teensy) | Latest | Core framework |
| Adafruit MCP23017 Library | ^2.3.0 | I2C GPIO expander |
| Adafruit BusIO | ^1.14.0 | I2C abstraction (dependency) |

### Knowledge Prerequisites

- Basic electronics (voltage, current, I2C protocol)
- Arduino/C++ programming
- PlatformIO usage
- Serial monitor debugging

---

## Success Criteria Summary

### Phase Gate Checklist

| Phase | Gate Criteria | Sign-off |
|-------|---------------|----------|
| 1 | All wiring verified, voltages correct | [ ] |
| 2 | I2C scanner finds 0x20, registers R/W | [ ] |
| 3 | All 8 channels cycle correctly | [ ] |
| 4 | Safety auto-shutoff verified | [ ] |
| 5 | Single MIDI note triggers solenoid | [ ] |

### Final Acceptance Criteria

1. **Reliability**: 1000 consecutive solenoid activations with zero I2C errors
2. **Safety**: Auto-shutoff triggers 100% of time when on-time exceeds limit
3. **Latency**: Note-on to solenoid activation < 5ms
4. **Documentation**: All code commented, API documented

---

## Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| Solenoid overheating | Medium | High | Software duty cycle limits, thermal monitoring |
| I2C communication loss | Low | High | Watchdog timer, fail-safe (all off) |
| Power supply noise | Medium | Medium | Separate solenoid power, decoupling capacitors |
| 3.3V/5V level mismatch | Low | High | Use only 3.3V for I2C, verify with multimeter |
| Mechanical solenoid failure | Medium | Low | Individual channel monitoring, spare solenoids |

---

## Timeline Summary

```
Week 1
├── Day 1-2: Phase 1 (Hardware Setup)
├── Day 2-3: Phase 2 (I2C Communication)
├── Day 3-4: Phase 3 (Solenoid Control)
├── Day 4-5: Phase 4 (Safety Features)
└── Day 5-6: Phase 5 (MIDI Preparation)

Week 2
└── Integration testing and refinement
```

---

## Document Index

| Document | Description |
|----------|-------------|
| [01-overview.md](./01-overview.md) | This document - main implementation plan |
| [02-hardware-setup.md](./02-hardware-setup.md) | Detailed hardware wiring and verification |
| [03-software-implementation.md](./03-software-implementation.md) | Code implementation and testing |
| [04-api-design.md](./04-api-design.md) | SolenoidDriver library API design |

---

## Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2025-01-18 | - | Initial plan creation |
