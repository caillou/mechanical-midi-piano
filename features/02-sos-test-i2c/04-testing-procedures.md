# Testing Procedures - SOS Solenoid Test

## Table of Contents

1. [Pre-Test Requirements](#pre-test-requirements)
2. [Test Environment Setup](#test-environment-setup)
3. [Functional Tests](#functional-tests)
4. [Timing Verification](#timing-verification)
5. [Safety Tests](#safety-tests)
6. [Integration Tests](#integration-tests)
7. [Expected Serial Outputs](#expected-serial-outputs)
8. [Test Sign-Off Checklist](#test-sign-off-checklist)
9. [Known Issues and Limitations](#known-issues-and-limitations)

---

## Pre-Test Requirements

### Hardware Requirements

| Item | Requirement | Verified |
|------|-------------|----------|
| Teensy 4.1 | Powered via USB | [ ] |
| Adafruit Solenoid Driver | Connected at 0x20 | [ ] |
| Test Solenoid | Connected to Channel 0 | [ ] |
| DC Power Supply | On, correct voltage | [ ] |
| Serial Monitor | Open at 115200 baud | [ ] |

### Software Requirements

| Item | Requirement | Verified |
|------|-------------|----------|
| Code changes implemented | All 8 steps from 03-code-implementation.md | [ ] |
| Project builds | No compilation errors | [ ] |
| Firmware uploaded | Successfully uploaded to Teensy | [ ] |

### Pre-Test Verification

Before running tests, verify:

1. **Serial Monitor Output**:
```
============================================================
MECHANICAL MIDI PIANO - SOLENOID DRIVER TEST
Teensy 4.1 + Adafruit I2C Solenoid Driver
============================================================

Initializing I2C bus...
  SDA Pin: 18, SCL Pin: 19, Speed: 400 kHz
[OK] I2C bus initialized

Scanning I2C bus...
  [FOUND] Device at address 0x20 (MCP23017 - Solenoid Driver)

Scan complete. 1 device(s) found.

Initializing MCP23017 at address 0x20...
  Configuring Port A as outputs (solenoid channels)...
  Port A configured, all channels OFF
  Verifying configuration...
Testing register read/write...
  GPIOA read: 0x00
  [OK] Write/read verified (0xAA)
[OK] MCP23017 initialized successfully
```

2. **Basic Solenoid Function**:
   - Press '0' to toggle Channel 0
   - Verify solenoid clicks ON
   - Press '0' again to toggle OFF
   - Verify solenoid releases

---

## Test Environment Setup

### Serial Monitor Configuration

| Setting | Value |
|---------|-------|
| Baud Rate | 115200 |
| Line Ending | None or Newline |
| Timestamp | Optional (recommended) |

### Timing Measurement Tools

For timing verification, use one of:
- Smartphone stopwatch app
- Physical stopwatch
- Oscilloscope (most accurate)
- Logic analyzer with timing display

### Test Recording

For each test, record:
- Date/Time
- Test name
- Pass/Fail status
- Any observations or anomalies
- Serial output (copy/paste or screenshot)

---

## Functional Tests

### Test F1: Help Menu Verification

**Purpose**: Verify new SOS commands appear in help menu.

**Steps**:
1. Open serial monitor
2. Press 'h' to display help menu

**Expected Output**:
```
SERIAL COMMANDS:
  'r' - Re-run all tests
  'a' - Activate all channels for 100ms
  's' - Run I2C scanner
  '0'-'7' - Toggle individual channel
  'o' - Run single SOS sequence (Channel 0)
  'c' - Toggle continuous SOS mode
  'x' - Emergency stop (all off + stop SOS)
  'h' - Show this help menu

Waiting for commands...
```

**Pass Criteria**:
- [ ] 'o' command listed with correct description
- [ ] 'c' command listed with correct description
- [ ] 'x' command description updated to mention SOS

| Result | Notes |
|--------|-------|
| [ ] PASS / [ ] FAIL | |

---

### Test F2: Single SOS Sequence

**Purpose**: Verify single SOS sequence plays correctly.

**Steps**:
1. Press 'o' in serial monitor
2. Observe solenoid and LED behavior
3. Wait for completion

**Expected Serial Output**:
```
Running single SOS sequence...
Playing SOS: ... --- ...
SOS complete.
```

**Expected Physical Behavior**:

| Element | Duration | Count | Observation |
|---------|----------|-------|-------------|
| Dit (short click) | ~100ms | 3 | S: . . . |
| Gap after S | ~200ms | 1 | Pause |
| Dah (long activation) | ~300ms | 3 | O: - - - |
| Gap after O | ~200ms | 1 | Pause |
| Dit (short click) | ~100ms | 3 | S: . . . |

**Pass Criteria**:
- [ ] Serial output matches expected
- [ ] Three short clicks heard (S)
- [ ] Three longer activations heard (O)
- [ ] Three short clicks heard (S)
- [ ] LED blinks with each element
- [ ] Solenoid returns to OFF state after completion

| Result | Notes |
|--------|-------|
| [ ] PASS / [ ] FAIL | |

---

### Test F3: Continuous SOS Mode Start

**Purpose**: Verify continuous mode starts correctly.

**Steps**:
1. Press 'c' in serial monitor
2. Observe solenoid repeating SOS pattern
3. Let it run for at least 2 complete cycles

**Expected Serial Output**:
```
Starting continuous SOS mode...
Press 'x' or 'c' to stop.
Playing SOS: ... --- ...
SOS complete.
Playing SOS: ... --- ...
SOS complete.
...
```

**Pass Criteria**:
- [ ] Mode starts with confirmation message
- [ ] SOS pattern repeats continuously
- [ ] Approximately 700ms gap between repetitions
- [ ] Pattern remains consistent across repetitions

| Result | Notes |
|--------|-------|
| [ ] PASS / [ ] FAIL | |

---

### Test F4: Continuous SOS Mode Stop with 'c'

**Purpose**: Verify 'c' toggles continuous mode off.

**Steps**:
1. Start continuous mode with 'c'
2. Wait for at least one complete SOS
3. Press 'c' again to stop

**Expected Serial Output**:
```
Starting continuous SOS mode...
Press 'x' or 'c' to stop.
Playing SOS: ... --- ...
SOS complete.
Playing SOS: ... --- ...
Stopping continuous SOS mode...
[OK] SOS stopped
```

**Pass Criteria**:
- [ ] Stop message appears
- [ ] SOS playback stops (may complete current element)
- [ ] Solenoid returns to OFF state
- [ ] LED turns off

| Result | Notes |
|--------|-------|
| [ ] PASS / [ ] FAIL | |

---

### Test F5: Continuous SOS Mode Stop with 'x'

**Purpose**: Verify emergency stop halts continuous mode.

**Steps**:
1. Start continuous mode with 'c'
2. Wait for SOS to be mid-pattern
3. Press 'x' for emergency stop

**Expected Serial Output**:
```
Starting continuous SOS mode...
Press 'x' or 'c' to stop.
Playing SOS: ... --- ...
EMERGENCY STOP
[OK] SOS stopped
[OK] All channels deactivated
```

**Pass Criteria**:
- [ ] Emergency stop message appears immediately
- [ ] SOS stops immediately (may be mid-element)
- [ ] "[OK] SOS stopped" confirmation appears
- [ ] "[OK] All channels deactivated" confirmation appears
- [ ] Solenoid releases immediately
- [ ] LED turns off

| Result | Notes |
|--------|-------|
| [ ] PASS / [ ] FAIL | |

---

### Test F6: Repeated Single SOS

**Purpose**: Verify multiple single SOS sequences work correctly.

**Steps**:
1. Press 'o' and wait for completion
2. Press 'o' again immediately after completion
3. Repeat 3 more times (5 total)

**Pass Criteria**:
- [ ] Each SOS sequence completes successfully
- [ ] No error messages
- [ ] Timing remains consistent
- [ ] No stuck channels between sequences

| Result | Notes |
|--------|-------|
| [ ] PASS / [ ] FAIL | |

---

### Test F7: SOS During Running (Guard Check)

**Purpose**: Verify 'o' command is blocked when SOS already running.

**Steps**:
1. Start continuous mode with 'c'
2. While running, press 'o'

**Expected Serial Output**:
```
Starting continuous SOS mode...
Press 'x' or 'c' to stop.
Playing SOS: ... --- ...
[INFO] SOS already running
```

**Pass Criteria**:
- [ ] "[INFO] SOS already running" message appears
- [ ] Continuous mode continues uninterrupted

| Result | Notes |
|--------|-------|
| [ ] PASS / [ ] FAIL | |

---

## Timing Verification

### Test T1: Dit Duration

**Purpose**: Verify dit (dot) is approximately 100ms.

**Method**:
1. Use stopwatch or oscilloscope
2. Run single SOS and measure individual dit durations
3. Measure at least 3 dits

**Measurement Points**:
- Dit 1 (first S): _____ ms
- Dit 2 (first S): _____ ms
- Dit 3 (first S): _____ ms
- Average: _____ ms

**Pass Criteria**:
- [ ] Each dit is between 90-110ms
- [ ] Average is approximately 100ms

| Result | Notes |
|--------|-------|
| [ ] PASS / [ ] FAIL | |

---

### Test T2: Dah Duration

**Purpose**: Verify dah (dash) is approximately 300ms.

**Method**:
1. Use stopwatch or oscilloscope
2. Run single SOS and measure individual dah durations
3. Measure at least 3 dahs

**Measurement Points**:
- Dah 1 (O): _____ ms
- Dah 2 (O): _____ ms
- Dah 3 (O): _____ ms
- Average: _____ ms

**Pass Criteria**:
- [ ] Each dah is between 280-320ms
- [ ] Average is approximately 300ms

| Result | Notes |
|--------|-------|
| [ ] PASS / [ ] FAIL | |

---

### Test T3: Complete SOS Cycle Time

**Purpose**: Verify total SOS sequence time.

**Method**:
1. Use stopwatch
2. Start timing when first dit begins
3. Stop timing when last dit ends

**Measurement Points**:
- Cycle 1: _____ ms
- Cycle 2: _____ ms
- Cycle 3: _____ ms
- Average: _____ ms

**Expected Timing Breakdown**:
```
S:   dit(100) + gap(100) + dit(100) + gap(100) + dit(100) = 500ms
Gap: letter gap additional = 200ms
O:   dah(300) + gap(100) + dah(300) + gap(100) + dah(300) = 1100ms
Gap: letter gap additional = 200ms
S:   dit(100) + gap(100) + dit(100) + gap(100) + dit(100) = 500ms
                                                           ------
Total active portion:                                       2500ms
```

**Pass Criteria**:
- [ ] Total cycle is between 2300-2700ms
- [ ] Consistent across multiple measurements

| Result | Notes |
|--------|-------|
| [ ] PASS / [ ] FAIL | |

---

### Test T4: Word Gap (Continuous Mode)

**Purpose**: Verify gap between SOS repetitions in continuous mode.

**Method**:
1. Start continuous mode
2. Measure time from end of one SOS to start of next

**Measurement Points**:
- Gap 1: _____ ms
- Gap 2: _____ ms
- Gap 3: _____ ms
- Average: _____ ms

**Pass Criteria**:
- [ ] Gap is between 650-750ms (approximately 700ms)

| Result | Notes |
|--------|-------|
| [ ] PASS / [ ] FAIL | |

---

## Safety Tests

### Test S1: No Safety Timeout During SOS

**Purpose**: Verify SOS pattern doesn't trigger safety timeout (5000ms max on-time).

**Steps**:
1. Run single SOS with 'o'
2. Observe serial output

**Pass Criteria**:
- [ ] No "[SAFETY] Channel X auto-shutoff" messages
- [ ] Pattern completes normally

| Result | Notes |
|--------|-------|
| [ ] PASS / [ ] FAIL | |

---

### Test S2: Continuous Mode Extended Run

**Purpose**: Verify continuous mode can run for extended period without safety issues.

**Steps**:
1. Start continuous mode with 'c'
2. Let run for at least 1 minute (approximately 18 SOS cycles)
3. Observe for any safety warnings or anomalies
4. Stop with 'c' or 'x'

**Observations**:
- Duration run: _____ minutes
- Number of cycles: _____
- Any warnings: _____

**Pass Criteria**:
- [ ] No safety timeout warnings
- [ ] Consistent timing throughout
- [ ] Solenoid not overheating (check by touch - should be warm, not hot)
- [ ] Clean stop when commanded

| Result | Notes |
|--------|-------|
| [ ] PASS / [ ] FAIL | |

---

### Test S3: Emergency Stop Responsiveness

**Purpose**: Verify emergency stop responds quickly during any phase of SOS.

**Steps**:
1. Start continuous mode
2. Press 'x' during different phases:
   - During a dit
   - During a dah
   - During a gap

**Observations**:
- Response time during dit: [ ] Immediate / [ ] Delayed
- Response time during dah: [ ] Immediate / [ ] Delayed
- Response time during gap: [ ] Immediate / [ ] Delayed

**Pass Criteria**:
- [ ] Response within 100ms for all phases
- [ ] Solenoid releases immediately

| Result | Notes |
|--------|-------|
| [ ] PASS / [ ] FAIL | |

---

### Test S4: Channel State After SOS

**Purpose**: Verify channel states are clean after SOS completion.

**Steps**:
1. Run single SOS with 'o'
2. After completion, toggle Channel 0 with '0'

**Expected Behavior**:
```
Running single SOS sequence...
Playing SOS: ... --- ...
SOS complete.

Toggling channel 0 -> ON
```

**Pass Criteria**:
- [ ] Channel 0 can be toggled normally after SOS
- [ ] No unexpected state or errors

| Result | Notes |
|--------|-------|
| [ ] PASS / [ ] FAIL | |

---

## Integration Tests

### Test I1: SOS with Other Channel Operations

**Purpose**: Verify SOS doesn't interfere with other channel operations.

**Steps**:
1. Turn on Channel 7 with '7'
2. Run single SOS with 'o'
3. Verify Channel 7 is still on after SOS
4. Turn off Channel 7 with '7'

**Pass Criteria**:
- [ ] Channel 7 remains on during SOS
- [ ] Channel 7 can be toggled after SOS

| Result | Notes |
|--------|-------|
| [ ] PASS / [ ] FAIL | |

---

### Test I2: I2C Scanner During SOS (Not Recommended)

**Purpose**: Verify behavior if I2C scan attempted during SOS.

**Steps**:
1. Start continuous SOS with 'c'
2. Press 's' to run I2C scanner

**Expected Behavior**:
- I2C scan may interleave with SOS output
- SOS should continue after scan

**Pass Criteria**:
- [ ] No system crash or hang
- [ ] SOS continues (may pause briefly)

| Result | Notes |
|--------|-------|
| [ ] PASS / [ ] FAIL | |

---

### Test I3: All Tests Cycle After SOS

**Purpose**: Verify existing tests still work after SOS operations.

**Steps**:
1. Run single SOS with 'o'
2. Press 'r' to re-run all tests
3. Verify all tests pass

**Pass Criteria**:
- [ ] Communication test passes
- [ ] Sequential channel test passes
- [ ] Simultaneous channel test passes

| Result | Notes |
|--------|-------|
| [ ] PASS / [ ] FAIL | |

---

## Expected Serial Outputs

### Complete Startup with SOS

```
============================================================
MECHANICAL MIDI PIANO - SOLENOID DRIVER TEST
Teensy 4.1 + Adafruit I2C Solenoid Driver
============================================================

Initializing I2C bus...
  SDA Pin: 18, SCL Pin: 19, Speed: 400 kHz
[OK] I2C bus initialized

Scanning I2C bus...
  [FOUND] Device at address 0x20 (MCP23017 - Solenoid Driver)

Scan complete. 1 device(s) found.

Initializing MCP23017 at address 0x20...
  Configuring Port A as outputs (solenoid channels)...
  Port A configured, all channels OFF
  Verifying configuration...
Testing register read/write...
  GPIOA read: 0x00
  [OK] Write/read verified (0xAA)
[OK] MCP23017 initialized successfully

Running initial tests...
============================================================
RUNNING ALL TESTS
============================================================

--- Test 1: Communication Verification ---
Testing register read/write...
  GPIOA read: 0x00
  [OK] Write/read verified (0xAA)

--- Test 2: Sequential Channel Test ---
Testing channels sequentially...
  Activation time: 100ms, Delay: 200ms

  Channel 0: ON... OFF [OK]
  Channel 1: ON... OFF [OK]
  Channel 2: ON... OFF [OK]
  Channel 3: ON... OFF [OK]
  Channel 4: ON... OFF [OK]
  Channel 5: ON... OFF [OK]
  Channel 6: ON... OFF [OK]
  Channel 7: ON... OFF [OK]
  Sequential test complete.

--- Test 3: All Channels Simultaneous ---
Activating all channels simultaneously...
  Duration: 100ms
  All channels ON
  All channels OFF
  Simultaneous test complete.
============================================================
ALL TESTS COMPLETE
============================================================

SERIAL COMMANDS:
  'r' - Re-run all tests
  'a' - Activate all channels for 100ms
  's' - Run I2C scanner
  '0'-'7' - Toggle individual channel
  'o' - Run single SOS sequence (Channel 0)
  'c' - Toggle continuous SOS mode
  'x' - Emergency stop (all off + stop SOS)
  'h' - Show this help menu

Waiting for commands...
```

### Single SOS Output

```
Running single SOS sequence...
Playing SOS: ... --- ...
SOS complete.
```

### Continuous SOS Start/Stop

```
Starting continuous SOS mode...
Press 'x' or 'c' to stop.
Playing SOS: ... --- ...
SOS complete.
Playing SOS: ... --- ...
SOS complete.
Stopping continuous SOS mode...
[OK] SOS stopped
```

### Emergency Stop During SOS

```
Starting continuous SOS mode...
Press 'x' or 'c' to stop.
Playing SOS: ... --- ...
EMERGENCY STOP
[OK] SOS stopped
[OK] All channels deactivated
```

---

## Test Sign-Off Checklist

### Functional Tests

| Test ID | Description | Result | Tester | Date |
|---------|-------------|--------|--------|------|
| F1 | Help Menu Verification | [ ] PASS / [ ] FAIL | | |
| F2 | Single SOS Sequence | [ ] PASS / [ ] FAIL | | |
| F3 | Continuous SOS Mode Start | [ ] PASS / [ ] FAIL | | |
| F4 | Continuous Mode Stop with 'c' | [ ] PASS / [ ] FAIL | | |
| F5 | Continuous Mode Stop with 'x' | [ ] PASS / [ ] FAIL | | |
| F6 | Repeated Single SOS | [ ] PASS / [ ] FAIL | | |
| F7 | SOS During Running Guard | [ ] PASS / [ ] FAIL | | |

### Timing Tests

| Test ID | Description | Result | Tester | Date |
|---------|-------------|--------|--------|------|
| T1 | Dit Duration (~100ms) | [ ] PASS / [ ] FAIL | | |
| T2 | Dah Duration (~300ms) | [ ] PASS / [ ] FAIL | | |
| T3 | Complete SOS Cycle (~2500ms) | [ ] PASS / [ ] FAIL | | |
| T4 | Word Gap (~700ms) | [ ] PASS / [ ] FAIL | | |

### Safety Tests

| Test ID | Description | Result | Tester | Date |
|---------|-------------|--------|--------|------|
| S1 | No Safety Timeout | [ ] PASS / [ ] FAIL | | |
| S2 | Extended Run (1+ min) | [ ] PASS / [ ] FAIL | | |
| S3 | Emergency Stop Responsiveness | [ ] PASS / [ ] FAIL | | |
| S4 | Channel State After SOS | [ ] PASS / [ ] FAIL | | |

### Integration Tests

| Test ID | Description | Result | Tester | Date |
|---------|-------------|--------|--------|------|
| I1 | SOS with Other Channels | [ ] PASS / [ ] FAIL | | |
| I2 | I2C Scanner During SOS | [ ] PASS / [ ] FAIL | | |
| I3 | All Tests After SOS | [ ] PASS / [ ] FAIL | | |

### Final Sign-Off

| Criteria | Status |
|----------|--------|
| All functional tests pass | [ ] |
| All safety tests pass | [ ] |
| Timing within acceptable range | [ ] |
| No regression in existing functionality | [ ] |

**Testing Complete**: [ ] YES / [ ] NO

**Tested By**: _______________________

**Date**: _______________________

**Notes**:

---

## Known Issues and Limitations

### Timing Accuracy

- Blocking delays are used, so timing may drift slightly under heavy serial I/O
- Oscilloscope measurements will be more accurate than stopwatch
- Acceptable tolerance is +/- 10% for all timing elements

### Responsiveness

- Emergency stop may take up to 100ms to take effect (waiting for current delay to complete)
- This is expected behavior with blocking implementation
- For sub-millisecond response, non-blocking implementation would be needed

### Resource Usage

- SOS uses Channel 0 exclusively
- Other channels remain available during SOS
- No additional memory allocation (static variables only)

### Serial Buffer

- Sending many commands during SOS may fill serial buffer
- Commands are processed between SOS repetitions in continuous mode
- Emergency stop takes priority

---

## Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2026-01-18 | - | Initial creation |
