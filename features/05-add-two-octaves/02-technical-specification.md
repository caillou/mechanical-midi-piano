# Technical Specification: 5-Board MCP23017 Expansion

This document provides complete code changes required for the expansion. Each change includes exact line numbers, before/after code blocks, and rationale.

## File: `src/main.cpp`

### Change 1: Update Board Count Constant

**Location:** Line 56
**Rationale:** Increase from 2 to 5 boards

**Before:**
```cpp
/** Number of MCP23017 boards */
constexpr uint8_t MCP23017_BOARD_COUNT = 2;
```

**After:**
```cpp
/** Number of MCP23017 boards */
constexpr uint8_t MCP23017_BOARD_COUNT = 5;
```

---

### Change 2: Update Board Addresses Array

**Location:** Lines 58-62
**Rationale:** Add addresses for 3 new boards (0x22, 0x23, 0x24)

**Before:**
```cpp
/** I2C addresses for MCP23017 boards
 *  Board 0: 0x20 (A0=A1=A2=0) - Channels 0-7
 *  Board 1: 0x21 (A0=1, A1=A2=0) - Channels 8-11
 */
constexpr uint8_t MCP23017_ADDRESSES[MCP23017_BOARD_COUNT] = {0x20, 0x21};
```

**After:**
```cpp
/** I2C addresses for MCP23017 boards
 *  Board 0: 0x20 (A2=0, A1=0, A0=0) - Channels 0-7   (MIDI 24-31)
 *  Board 1: 0x21 (A2=0, A1=0, A0=1) - Channels 8-15  (MIDI 32-35, 12-15 unused)
 *  Board 2: 0x22 (A2=0, A1=1, A0=0) - Channels 16-23 (MIDI 48-55)
 *  Board 3: 0x23 (A2=0, A1=1, A0=1) - Channels 24-31 (MIDI 56-63)
 *  Board 4: 0x24 (A2=1, A1=0, A0=0) - Channels 32-39 (MIDI 64-71)
 */
constexpr uint8_t MCP23017_ADDRESSES[MCP23017_BOARD_COUNT] = {0x20, 0x21, 0x22, 0x23, 0x24};
```

---

### Change 3: Update Channel Count

**Location:** Line 72
**Rationale:** Increase from 12 to 40 channels (5 boards x 8 channels)

**Before:**
```cpp
/** Total number of solenoid channels across all boards */
constexpr uint8_t NUM_CHANNELS = 12;
```

**After:**
```cpp
/** Total number of solenoid channels across all boards */
constexpr uint8_t NUM_CHANNELS = 40;
```

---

### Change 4: Replace MIDI Configuration Constants

**Location:** Lines 92-109
**Rationale:** Replace single-range constants with dual-range configuration to support non-contiguous MIDI mapping

**Before:**
```cpp
/**
 * @defgroup MIDIConfig MIDI Configuration
 * @{
 */

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

/** @} */
```

**After:**
```cpp
/**
 * @defgroup MIDIConfig MIDI Configuration
 * @{
 */

/**
 * MIDI Range 1: Notes 24-35 (C1-B1) -> Channels 0-11
 * This is the original octave, mapped linearly.
 */
constexpr uint8_t MIDI_RANGE1_LOW = 24;
constexpr uint8_t MIDI_RANGE1_HIGH = 35;
constexpr uint8_t MIDI_RANGE1_CHANNEL_BASE = 0;

/**
 * MIDI Range 2: Notes 48-71 (C3-B4) -> Channels 16-39
 * Two octaves added with expansion boards.
 * Note: Channels 12-15 are unused (gap in channel allocation).
 */
constexpr uint8_t MIDI_RANGE2_LOW = 48;
constexpr uint8_t MIDI_RANGE2_HIGH = 71;
constexpr uint8_t MIDI_RANGE2_CHANNEL_BASE = 16;

/**
 * Gap in MIDI mapping: Notes 36-47 (C2-B2) are not mapped.
 * These notes will be silently ignored.
 */

/** @} */
```

---

### Change 5: Add Reverse Mapping Function

**Location:** After line 148, before the `// =============================================================================` separator
**Rationale:** Need reverse mapping (channel to note) for status display since linear formula no longer works

**Before:** (function prototypes section, after line 148)
```cpp
// Utility Functions
void printSeparator();
void printHelp();
void printStatus();
void handleSerialInput();
```

**After:**
```cpp
// Utility Functions
void printSeparator();
void printHelp();
void printStatus();
void handleSerialInput();
int8_t channelToNote(uint8_t channel);
```

---

### Change 6: Rewrite noteToChannel() Function

**Location:** Lines 333-348
**Rationale:** Replace simple linear mapping with dual-range logic

**Before:**
```cpp
/**
 * @brief Convert MIDI note number to solenoid channel
 *
 * @param note MIDI note number (0-127)
 * @return Channel number (0-11) if note is in range, -1 otherwise
 *
 * Maps notes 24-35 (C1 through B1) to channels 0-11.
 */
int8_t noteToChannel(uint8_t note)
{
    if (note < MIDI_NOTE_LOW || note > MIDI_NOTE_HIGH)
    {
        return -1;
    }
    return note - MIDI_NOTE_LOW;
}
```

**After:**
```cpp
/**
 * @brief Convert MIDI note number to solenoid channel
 *
 * @param note MIDI note number (0-127)
 * @return Channel number (0-39) if note is in a mapped range, -1 otherwise
 *
 * Mapping:
 *   - Notes 24-35 (C1-B1)  -> Channels 0-11  (Range 1)
 *   - Notes 36-47 (C2-B2)  -> -1 (unmapped gap)
 *   - Notes 48-71 (C3-B4)  -> Channels 16-39 (Range 2)
 */
int8_t noteToChannel(uint8_t note)
{
    // Range 1: Notes 24-35 -> Channels 0-11
    if (note >= MIDI_RANGE1_LOW && note <= MIDI_RANGE1_HIGH)
    {
        return note - MIDI_RANGE1_LOW + MIDI_RANGE1_CHANNEL_BASE;
    }

    // Range 2: Notes 48-71 -> Channels 16-39
    if (note >= MIDI_RANGE2_LOW && note <= MIDI_RANGE2_HIGH)
    {
        return note - MIDI_RANGE2_LOW + MIDI_RANGE2_CHANNEL_BASE;
    }

    // Note not in any mapped range (including gap 36-47)
    return -1;
}
```

---

### Change 7: Add channelToNote() Function

**Location:** After the noteToChannel() function (insert after the new noteToChannel implementation)
**Rationale:** Needed for printStatus() to display correct MIDI notes for each channel

**Insert this new function:**
```cpp
/**
 * @brief Convert solenoid channel to MIDI note number
 *
 * @param channel Channel number (0-39)
 * @return MIDI note number if channel is mapped, -1 otherwise
 *
 * Reverse mapping for status display:
 *   - Channels 0-11  -> Notes 24-35 (C1-B1)
 *   - Channels 12-15 -> -1 (unused channels on board 0x21)
 *   - Channels 16-39 -> Notes 48-71 (C3-B4)
 */
int8_t channelToNote(uint8_t channel)
{
    // Range 1: Channels 0-11 -> Notes 24-35
    if (channel <= 11)
    {
        return channel + MIDI_RANGE1_LOW;
    }

    // Unused channels 12-15 on board 0x21
    if (channel <= 15)
    {
        return -1;
    }

    // Range 2: Channels 16-39 -> Notes 48-71
    if (channel <= 39)
    {
        return channel - MIDI_RANGE2_CHANNEL_BASE + MIDI_RANGE2_LOW;
    }

    // Channel out of range
    return -1;
}
```

---

### Change 8: Update Setup MIDI Message

**Location:** Lines 193-198
**Rationale:** Display both MIDI ranges in startup message

**Before:**
```cpp
    Serial.println(F("[OK] MIDI handlers registered"));
    Serial.print(F("  Listening for notes "));
    Serial.print(MIDI_NOTE_LOW);
    Serial.print(F("-"));
    Serial.print(MIDI_NOTE_HIGH);
    Serial.println(F(" (C1-B1)"));
```

**After:**
```cpp
    Serial.println(F("[OK] MIDI handlers registered"));
    Serial.print(F("  Range 1: Notes "));
    Serial.print(MIDI_RANGE1_LOW);
    Serial.print(F("-"));
    Serial.print(MIDI_RANGE1_HIGH);
    Serial.println(F(" (C1-B1) -> Ch 0-11"));
    Serial.print(F("  Range 2: Notes "));
    Serial.print(MIDI_RANGE2_LOW);
    Serial.print(F("-"));
    Serial.print(MIDI_RANGE2_HIGH);
    Serial.println(F(" (C3-B4) -> Ch 16-39"));
```

---

### Change 9: Update printHelp() Function

**Location:** Line 473
**Rationale:** Update displayed MIDI note range to show both ranges

**Before:**
```cpp
    Serial.println(F("MIDI: Listening for notes 24-35 (C1-B1) on all channels"));
```

**After:**
```cpp
    Serial.println(F("MIDI: Listening for notes 24-35 (C1-B1) and 48-71 (C3-B4)"));
```

---

### Change 10: Rewrite printStatus() Function

**Location:** Lines 478-510
**Rationale:** Use channelToNote() for correct display; skip unmapped channels; show board boundaries

**Before:**
```cpp
/**
 * @brief Print current driver status
 */
void printStatus()
{
    printSeparator();
    Serial.println(F("STATUS"));
    printSeparator();

    Serial.print(F("Driver initialized: "));
    Serial.println(solenoidDriver.isInitialized() ? F("Yes") : F("No"));

    if (solenoidDriver.isInitialized())
    {
        Serial.print(F("Boards: "));
        Serial.println(solenoidDriver.getBoardCount());
        Serial.print(F("Channels: "));
        Serial.println(solenoidDriver.getChannelCount());

        Serial.println(F("Channel states:"));
        for (uint8_t i = 0; i < NUM_CHANNELS; i++)
        {
            Serial.print(F("  Ch "));
            Serial.print(i);
            Serial.print(F(" (Note "));
            Serial.print(MIDI_NOTE_LOW + i);
            Serial.print(F("): "));
            Serial.println(solenoidDriver.isOn(i) ? F("ON") : F("off"));
        }
    }

    printSeparator();
}
```

**After:**
```cpp
/**
 * @brief Print current driver status
 */
void printStatus()
{
    printSeparator();
    Serial.println(F("STATUS"));
    printSeparator();

    Serial.print(F("Driver initialized: "));
    Serial.println(solenoidDriver.isInitialized() ? F("Yes") : F("No"));

    if (solenoidDriver.isInitialized())
    {
        Serial.print(F("Boards: "));
        Serial.println(solenoidDriver.getBoardCount());
        Serial.print(F("Channels: "));
        Serial.print(solenoidDriver.getChannelCount());
        Serial.println(F(" (36 mapped, 4 unused)"));

        Serial.println(F("Channel states:"));
        for (uint8_t i = 0; i < NUM_CHANNELS; i++)
        {
            // Print board header at board boundaries
            if (i % 8 == 0)
            {
                Serial.print(F("  --- Board "));
                Serial.print(i / 8);
                Serial.print(F(" (0x"));
                Serial.print(MCP23017_ADDRESSES[i / 8], HEX);
                Serial.println(F(") ---"));
            }

            int8_t note = channelToNote(i);
            Serial.print(F("  Ch "));
            if (i < 10) Serial.print(F(" "));  // Padding for alignment
            Serial.print(i);

            if (note >= 0)
            {
                Serial.print(F(" (Note "));
                if (note < 100) Serial.print(F(" "));  // Padding
                Serial.print(note);
                Serial.print(F("): "));
                Serial.println(solenoidDriver.isOn(i) ? F("ON") : F("off"));
            }
            else
            {
                Serial.println(F(" (unused)"));
            }
        }
    }

    printSeparator();
}
```

---

### Change 11: Update Header Documentation

**Location:** Lines 1-37
**Rationale:** Update hardware description and mapping tables to reflect new configuration

**Before:**
```cpp
/**
 * @file main.cpp
 * @brief USB MIDI Solenoid Controller for Mechanical MIDI Piano
 *
 * This program implements a USB MIDI instrument using the Adafruit I2C
 * Solenoid Driver with a Teensy 4.1 microcontroller.
 *
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
 *
 * Serial Commands (for debugging):
 *   'x' - Emergency stop (all off)
 *   's' - Print status
 *   'h' - Show help menu
 *
 * @author Mechanical MIDI Piano Project
 * @date 2025-01-18
 * @version 2.0.0
 */
```

**After:**
```cpp
/**
 * @file main.cpp
 * @brief USB MIDI Solenoid Controller for Mechanical MIDI Piano
 *
 * This program implements a USB MIDI instrument using the Adafruit I2C
 * Solenoid Driver with a Teensy 4.1 microcontroller.
 *
 * Hardware:
 *   - Teensy 4.1
 *   - 5x Adafruit I2C Solenoid Driver (Product ID 6318)
 *   - I2C: SDA=Pin 18, SCL=Pin 19 (Wire) @ 400kHz
 *   - Board 0: 0x20 (A2=0,A1=0,A0=0) - Channels 0-7
 *   - Board 1: 0x21 (A2=0,A1=0,A0=1) - Channels 8-15 (12-15 unused)
 *   - Board 2: 0x22 (A2=0,A1=1,A0=0) - Channels 16-23
 *   - Board 3: 0x23 (A2=0,A1=1,A0=1) - Channels 24-31
 *   - Board 4: 0x24 (A2=1,A1=0,A0=0) - Channels 32-39
 *
 * MIDI Mapping (36 channels across 5 boards):
 *   Range 1 - Notes 24-35 (C1-B1) -> Channels 0-11:
 *     - Notes 24-31 -> Board 0x20, Channels 0-7
 *     - Notes 32-35 -> Board 0x21, Channels 8-11
 *   Gap - Notes 36-47 (C2-B2) -> Not mapped
 *   Range 2 - Notes 48-71 (C3-B4) -> Channels 16-39:
 *     - Notes 48-55 -> Board 0x22, Channels 16-23
 *     - Notes 56-63 -> Board 0x23, Channels 24-31
 *     - Notes 64-71 -> Board 0x24, Channels 32-39
 *
 * Serial Commands (for debugging):
 *   'x' - Emergency stop (all off)
 *   's' - Print status
 *   'h' - Show help menu
 *
 * @author Mechanical MIDI Piano Project
 * @date 2025-01-18
 * @version 3.0.0
 */
```

---

## Summary of All Changes

| Change # | Lines | Description |
|----------|-------|-------------|
| 1 | 56 | `MCP23017_BOARD_COUNT = 2` -> `5` |
| 2 | 58-62 | Add addresses 0x22, 0x23, 0x24 to array |
| 3 | 72 | `NUM_CHANNELS = 12` -> `40` |
| 4 | 92-109 | Replace MIDI_NOTE_LOW/HIGH with dual-range constants |
| 5 | ~148 | Add `channelToNote()` function prototype |
| 6 | 333-348 | Rewrite `noteToChannel()` for dual ranges |
| 7 | After 348 | Add new `channelToNote()` function |
| 8 | 193-198 | Update setup message to show both ranges |
| 9 | 473 | Update `printHelp()` MIDI range display |
| 10 | 478-510 | Rewrite `printStatus()` with reverse mapping |
| 11 | 1-37 | Update header documentation |

## Complete Modified File Structure

After all changes, the MIDI configuration section (lines ~92-120) will look like:

```cpp
/**
 * @defgroup MIDIConfig MIDI Configuration
 * @{
 */

/**
 * MIDI Range 1: Notes 24-35 (C1-B1) -> Channels 0-11
 * This is the original octave, mapped linearly.
 */
constexpr uint8_t MIDI_RANGE1_LOW = 24;
constexpr uint8_t MIDI_RANGE1_HIGH = 35;
constexpr uint8_t MIDI_RANGE1_CHANNEL_BASE = 0;

/**
 * MIDI Range 2: Notes 48-71 (C3-B4) -> Channels 16-39
 * Two octaves added with expansion boards.
 * Note: Channels 12-15 are unused (gap in channel allocation).
 */
constexpr uint8_t MIDI_RANGE2_LOW = 48;
constexpr uint8_t MIDI_RANGE2_HIGH = 71;
constexpr uint8_t MIDI_RANGE2_CHANNEL_BASE = 16;

/**
 * Gap in MIDI mapping: Notes 36-47 (C2-B2) are not mapped.
 * These notes will be silently ignored.
 */

/** @} */
```

## Verification Checklist

After implementation, verify:

- [ ] Code compiles without warnings
- [ ] `MCP23017_BOARD_COUNT` is 5
- [ ] `MCP23017_ADDRESSES` contains {0x20, 0x21, 0x22, 0x23, 0x24}
- [ ] `NUM_CHANNELS` is 40
- [ ] `noteToChannel(24)` returns 0
- [ ] `noteToChannel(35)` returns 11
- [ ] `noteToChannel(36)` returns -1
- [ ] `noteToChannel(47)` returns -1
- [ ] `noteToChannel(48)` returns 16
- [ ] `noteToChannel(71)` returns 39
- [ ] `channelToNote(0)` returns 24
- [ ] `channelToNote(11)` returns 35
- [ ] `channelToNote(12)` returns -1
- [ ] `channelToNote(15)` returns -1
- [ ] `channelToNote(16)` returns 48
- [ ] `channelToNote(39)` returns 71
- [ ] Status display shows all 40 channels with correct note mapping
- [ ] Help menu shows both MIDI ranges
