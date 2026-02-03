# Extended MIDI Setup - Technical Specification

## Document Information

| Field | Value |
|-------|-------|
| Feature ID | 04-extended-midi-setup |
| Version | 1.0 |
| Created | 2025-02-03 |
| Target File | `src/main.cpp` |

---

## Overview

This document provides the exact code changes required to extend the MIDI piano from 8 to 12 channels. All changes are confined to `src/main.cpp`. The SolenoidDriver library already supports multiple boards via its `begin(TwoWire&, const uint8_t[], uint8_t)` method.

---

## Change 1: Update File Header Documentation

### Location
`src/main.cpp`, Lines 1-32 (file header comment block)

### Purpose
Update documentation to reflect new hardware configuration and MIDI mapping.

### Before (Lines 7-22)
```cpp
 * Hardware:
 *   - Teensy 4.1
 *   - Adafruit I2C Solenoid Driver (Product ID 6318)
 *   - I2C: SDA=Pin 18, SCL=Pin 19 (Wire)
 *   - Default I2C Address: 0x20
 *
 * MIDI Mapping:
 *   - Note 60 (C4)  -> Solenoid Channel 0
 *   - Note 61 (C#4) -> Solenoid Channel 1
 *   - Note 62 (D4)  -> Solenoid Channel 2
 *   - Note 63 (D#4) -> Solenoid Channel 3
 *   - Note 64 (E4)  -> Solenoid Channel 4
 *   - Note 65 (F4)  -> Solenoid Channel 5
 *   - Note 66 (F#4) -> Solenoid Channel 6
 *   - Note 67 (G4)  -> Solenoid Channel 7
```

### After
```cpp
 * Hardware:
 *   - Teensy 4.1
 *   - 2x Adafruit I2C Solenoid Driver (Product ID 6318)
 *   - I2C: SDA=Pin 18, SCL=Pin 19 (Wire)
 *   - Board 0 Address: 0x20 (channels 0-7)
 *   - Board 1 Address: 0x21 (channels 8-11)
 *
 * MIDI Mapping (12 channels across 2 boards):
 *   - Note 24 (C1)  -> Board 0x20, Channel 0
 *   - Note 25 (C#1) -> Board 0x20, Channel 1
 *   - Note 26 (D1)  -> Board 0x20, Channel 2
 *   - Note 27 (D#1) -> Board 0x20, Channel 3
 *   - Note 28 (E1)  -> Board 0x20, Channel 4
 *   - Note 29 (F1)  -> Board 0x20, Channel 5
 *   - Note 30 (F#1) -> Board 0x20, Channel 6
 *   - Note 31 (G1)  -> Board 0x20, Channel 7
 *   - Note 32 (G#1) -> Board 0x21, Channel 0
 *   - Note 33 (A1)  -> Board 0x21, Channel 1
 *   - Note 34 (A#1) -> Board 0x21, Channel 2
 *   - Note 35 (B1)  -> Board 0x21, Channel 3
```

---

## Change 2: Update I2C Address Configuration

### Location
`src/main.cpp`, Lines 47-53

### Purpose
Change from single address constant to array of addresses for multi-board support.

### Before (Lines 47-53)
```cpp
/**
 * @defgroup I2CConfig I2C Configuration
 * @{
 */

/** I2C bus speed in Hz (400kHz recommended for MCP23017) */
constexpr uint32_t I2C_CLOCK_SPEED = 400000;

/** Default I2C address for MCP23017 (A0=A1=A2=0) */
constexpr uint8_t MCP23017_DEFAULT_ADDRESS = 0x20;

/** @} */
```

### After
```cpp
/**
 * @defgroup I2CConfig I2C Configuration
 * @{
 */

/** I2C bus speed in Hz (400kHz recommended for MCP23017) */
constexpr uint32_t I2C_CLOCK_SPEED = 400000;

/** Number of MCP23017 boards */
constexpr uint8_t MCP23017_BOARD_COUNT = 2;

/** I2C addresses for MCP23017 boards
 *  Board 0: 0x20 (A0=A1=A2=0) - Channels 0-7
 *  Board 1: 0x21 (A0=1, A1=A2=0) - Channels 8-11
 */
constexpr uint8_t MCP23017_ADDRESSES[MCP23017_BOARD_COUNT] = {0x20, 0x21};

/** @} */
```

### Rationale
- Using `constexpr` array maintains compile-time evaluation
- Separate `MCP23017_BOARD_COUNT` constant provides clarity and enables loop iteration
- Comments document the physical address pin configuration for each board

---

## Change 3: Update NUM_CHANNELS Constant

### Location
`src/main.cpp`, Line 61

### Purpose
Increase total channel count from 8 to 12.

### Before (Line 61)
```cpp
/** Number of solenoid channels on the driver board */
constexpr uint8_t NUM_CHANNELS = 8;
```

### After
```cpp
/** Total number of solenoid channels across all boards */
constexpr uint8_t NUM_CHANNELS = 12;
```

### Rationale
- 12 channels = 8 channels on board 0x20 + 4 channels on board 0x21
- The comment clarifies this is now a multi-board total

---

## Change 4: Update MIDI Note Range Constants

### Location
`src/main.cpp`, Lines 87-97

### Purpose
Change MIDI note range from C4-G4 (60-67) to C1-B1 (24-35).

### Before (Lines 87-97)
```cpp
/**
 * Lowest MIDI note that triggers a solenoid (C4 = Middle C)
 * Maps to solenoid channel 0
 */
constexpr uint8_t MIDI_NOTE_LOW = 60;

/**
 * Highest MIDI note that triggers a solenoid (G4)
 * Maps to solenoid channel 7
 */
constexpr uint8_t MIDI_NOTE_HIGH = 67;
```

### After
```cpp
/**
 * Lowest MIDI note that triggers a solenoid (C1)
 * Maps to solenoid channel 0 on board 0x20
 */
constexpr uint8_t MIDI_NOTE_LOW = 24;

/**
 * Highest MIDI note that triggers a solenoid (B1)
 * Maps to solenoid channel 3 on board 0x21 (global channel 11)
 */
constexpr uint8_t MIDI_NOTE_HIGH = 35;
```

### Rationale
- MIDI note 24 = C1 (two octaves below middle C)
- MIDI note 35 = B1 (completing the chromatic octave)
- Range of 12 notes matches NUM_CHANNELS = 12

### Note-to-Channel Mapping Verification
The existing `noteToChannel()` function (lines 318-325) uses this formula:
```cpp
return note - MIDI_NOTE_LOW;
```

With the new values:
- Note 24: `24 - 24 = 0` (Channel 0)
- Note 35: `35 - 24 = 11` (Channel 11)

This correctly maps notes 24-35 to channels 0-11. **No changes needed to `noteToChannel()`**.

---

## Change 5: Update initMCP23017() Function

### Location
`src/main.cpp`, Lines 275-304

### Purpose
Modify initialization to use array of addresses instead of single address.

### Before (Lines 275-304)
```cpp
/**
 * @brief Initialize MCP23017 GPIO expander via SolenoidDriver
 *
 * @return true if initialization successful, false otherwise
 *
 * Configures the SolenoidDriver with appropriate settings and initializes
 * the MCP23017 at the default address.
 */
bool initMCP23017()
{
    Serial.println();
    Serial.print(F("Initializing MCP23017 at address 0x"));
    Serial.print(MCP23017_DEFAULT_ADDRESS, HEX);
    Serial.println(F("..."));

    // Configure the SolenoidDriver before initialization
    SolenoidConfig config;
    config.maxOnTimeMs = MAX_ON_TIME_MS;
    config.minOffTimeMs = MIN_OFF_TIME_MS;
    config.i2cClockHz = I2C_CLOCK_SPEED;
    config.safetyEnabled = true;
    config.debugEnabled = false;
    config.maxDutyCycle = 0.75f; // 75% maximum duty cycle for solenoid protection
    solenoidDriver.setConfig(config);

    // Initialize with SolenoidDriver library
    if (!solenoidDriver.begin(Wire, MCP23017_DEFAULT_ADDRESS))
    {
        SolenoidError err = solenoidDriver.getLastError();
        Serial.print(F("[ERROR] SolenoidDriver init failed: "));
        Serial.println(SolenoidDriver::getErrorString(err));
        return false;
    }

    Serial.println(F("  SolenoidDriver initialized, all channels OFF"));

    return true;
}
```

### After
```cpp
/**
 * @brief Initialize MCP23017 GPIO expanders via SolenoidDriver
 *
 * @return true if initialization successful, false otherwise
 *
 * Configures the SolenoidDriver with appropriate settings and initializes
 * all MCP23017 boards at their configured addresses.
 */
bool initMCP23017()
{
    Serial.println();
    Serial.print(F("Initializing "));
    Serial.print(MCP23017_BOARD_COUNT);
    Serial.println(F(" MCP23017 board(s)..."));

    for (uint8_t i = 0; i < MCP23017_BOARD_COUNT; i++)
    {
        Serial.print(F("  Board "));
        Serial.print(i);
        Serial.print(F(": address 0x"));
        Serial.println(MCP23017_ADDRESSES[i], HEX);
    }

    // Configure the SolenoidDriver before initialization
    SolenoidConfig config;
    config.maxOnTimeMs = MAX_ON_TIME_MS;
    config.minOffTimeMs = MIN_OFF_TIME_MS;
    config.i2cClockHz = I2C_CLOCK_SPEED;
    config.safetyEnabled = true;
    config.debugEnabled = false;
    config.maxDutyCycle = 0.75f; // 75% maximum duty cycle for solenoid protection
    solenoidDriver.setConfig(config);

    // Initialize with SolenoidDriver library (multi-board variant)
    if (!solenoidDriver.begin(Wire, MCP23017_ADDRESSES, MCP23017_BOARD_COUNT))
    {
        SolenoidError err = solenoidDriver.getLastError();
        Serial.print(F("[ERROR] SolenoidDriver init failed: "));
        Serial.println(SolenoidDriver::getErrorString(err));
        return false;
    }

    Serial.print(F("  SolenoidDriver initialized: "));
    Serial.print(solenoidDriver.getBoardCount());
    Serial.print(F(" board(s), "));
    Serial.print(solenoidDriver.getChannelCount());
    Serial.println(F(" channel(s), all OFF"));

    return true;
}
```

### Key Changes
1. **Print board count** instead of single address in initial message
2. **Loop through addresses** to print each board's configuration
3. **Use array-based begin()**: `solenoidDriver.begin(Wire, MCP23017_ADDRESSES, MCP23017_BOARD_COUNT)`
4. **Enhanced success message** showing actual board and channel counts from driver

### SolenoidDriver API Reference
The `begin()` method signature used (from `SolenoidDriver.h`, lines 147-169):
```cpp
/**
 * @brief Initialize with multiple driver boards
 *
 * @param wire Reference to TwoWire instance
 * @param addresses Array of I2C addresses
 * @param count Number of boards (1-8)
 * @return true if all boards initialized successfully
 * @return false if any board failed (check getLastError())
 */
bool begin(TwoWire& wire, const uint8_t addresses[], uint8_t count);
```

---

## Change 6: Update Setup MIDI Range Display

### Location
`src/main.cpp`, Lines 183-187

### Purpose
Update the MIDI range display message in setup() to reflect new note names.

### Before (Lines 183-187)
```cpp
    Serial.println(F("[OK] MIDI handlers registered"));
    Serial.print(F("  Listening for notes "));
    Serial.print(MIDI_NOTE_LOW);
    Serial.print(F("-"));
    Serial.print(MIDI_NOTE_HIGH);
    Serial.println(F(" (C4-G4)"));
```

### After
```cpp
    Serial.println(F("[OK] MIDI handlers registered"));
    Serial.print(F("  Listening for notes "));
    Serial.print(MIDI_NOTE_LOW);
    Serial.print(F("-"));
    Serial.print(MIDI_NOTE_HIGH);
    Serial.println(F(" (C1-B1)"));
```

### Rationale
- Updates the human-readable note range from "C4-G4" to "C1-B1"
- The numeric values (24-35) are printed dynamically from constants

---

## Change 7: Update printHelp() Function

### Location
`src/main.cpp`, Lines 443-453

### Purpose
Update help text to show correct MIDI note range.

### Before (Lines 443-453)
```cpp
/**
 * @brief Print the help menu
 */
void printHelp()
{
    Serial.println(F("SERIAL COMMANDS:"));
    Serial.println(F("  'x' - Emergency stop (all solenoids off)"));
    Serial.println(F("  's' - Print status"));
    Serial.println(F("  'h' - Show this help menu"));
    Serial.println();
    Serial.println(F("MIDI: Listening for notes 60-67 (C4-G4) on all channels"));
    Serial.println();
    Serial.println(F("Ready for MIDI input..."));
}
```

### After
```cpp
/**
 * @brief Print the help menu
 */
void printHelp()
{
    Serial.println(F("SERIAL COMMANDS:"));
    Serial.println(F("  'x' - Emergency stop (all solenoids off)"));
    Serial.println(F("  's' - Print status"));
    Serial.println(F("  'h' - Show this help menu"));
    Serial.println();
    Serial.println(F("MIDI: Listening for notes 24-35 (C1-B1) on all channels"));
    Serial.println();
    Serial.println(F("Ready for MIDI input..."));
}
```

---

## Change 8: Update noteToChannel() Comment

### Location
`src/main.cpp`, Lines 310-317

### Purpose
Update function documentation to reflect new note range.

### Before (Lines 310-317)
```cpp
/**
 * @brief Convert MIDI note number to solenoid channel
 *
 * @param note MIDI note number (0-127)
 * @return Channel number (0-7) if note is in range, -1 otherwise
 *
 * Maps notes 60-67 (C4 through G4) to channels 0-7.
 */
```

### After
```cpp
/**
 * @brief Convert MIDI note number to solenoid channel
 *
 * @param note MIDI note number (0-127)
 * @return Channel number (0-11) if note is in range, -1 otherwise
 *
 * Maps notes 24-35 (C1 through B1) to channels 0-11.
 */
```

---

## Functions That Require NO Changes

The following functions work correctly without modification:

### noteToChannel() (Lines 318-325)
```cpp
int8_t noteToChannel(uint8_t note)
{
    if (note < MIDI_NOTE_LOW || note > MIDI_NOTE_HIGH)
    {
        return -1;
    }
    return note - MIDI_NOTE_LOW;
}
```
**Why no changes**: The formula `note - MIDI_NOTE_LOW` works for any note range as long as the constants are updated.

### handleNoteOn() (Lines 337-368)
**Why no changes**: Uses `noteToChannel()` and `solenoidDriver.on(ch)`. The SolenoidDriver handles global channel to board/local channel conversion internally.

### handleNoteOff() (Lines 379-404)
**Why no changes**: Uses `noteToChannel()` and `solenoidDriver.off(ch)`. Same reasoning as handleNoteOn.

### printStatus() (Lines 458-487)
**Why no changes**: The loop uses `NUM_CHANNELS` which will be updated, so it will correctly iterate through all 12 channels:
```cpp
for (uint8_t i = 0; i < NUM_CHANNELS; i++)
{
    // ... prints channel status
}
```

### deactivateAllChannels() (Lines 417-426)
**Why no changes**: Uses `solenoidDriver.emergencyStop()` which turns off all channels on all initialized boards.

### loop() (Lines 205-219)
**Why no changes**: Calls `usbMIDI.read()` and `solenoidDriver.update()` which handle all channels automatically.

---

## Complete Modified Code Summary

### Lines to Modify (by line number in original file)

| Original Line(s) | Change Type | Description |
|------------------|-------------|-------------|
| 7-22 | Comment update | Hardware and MIDI mapping documentation |
| 51 | Replace | Single address -> board count constant |
| 51 | Add | New `MCP23017_ADDRESSES` array constant |
| 61 | Modify | `NUM_CHANNELS = 8` -> `NUM_CHANNELS = 12` |
| 90 | Modify | `MIDI_NOTE_LOW = 60` -> `MIDI_NOTE_LOW = 24` |
| 96 | Modify | `MIDI_NOTE_HIGH = 67` -> `MIDI_NOTE_HIGH = 35` |
| 187 | Modify | Note range display "C4-G4" -> "C1-B1" |
| 275-304 | Rewrite | `initMCP23017()` function |
| 314-317 | Comment update | `noteToChannel()` documentation |
| 450 | Modify | Help text note range |

---

## Compilation Verification

After making all changes, verify compilation:

```bash
cd /Users/caillou/repos/caillou/mechanical-midi-piano
pio run
```

Expected output should show successful compilation with no errors or warnings related to these changes.

---

## Static Analysis Verification

Run static analysis to check for issues:

```bash
pio check
```

Verify no new warnings are introduced by the changes.
