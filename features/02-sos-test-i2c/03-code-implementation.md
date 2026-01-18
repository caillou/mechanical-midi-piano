# Code Implementation Guide - SOS Test

## Table of Contents

1. [Overview](#overview)
2. [File Structure](#file-structure)
3. [Implementation Steps](#implementation-steps)
4. [Complete Code Blocks](#complete-code-blocks)
5. [Build and Upload](#build-and-upload)
6. [Verification](#verification)

---

## Overview

This document provides exact code changes to implement SOS morse code functionality in `src/main.cpp`. All changes are additive - no existing functionality is removed or modified (except enhancing the emergency stop).

### Files to Modify

| File | Changes |
|------|---------|
| `src/main.cpp` | Add SOS constants, variables, functions, and commands |

### No Files to Create

This implementation modifies only the existing `src/main.cpp` file.

---

## File Structure

### Current main.cpp Structure (Relevant Sections)

```
Line 1-31:     File header documentation
Line 32-35:    Includes
Line 37-95:    Configuration Constants
Line 97-124:   MCP23017 Register Definitions
Line 126-143:  Global Objects and Variables
Line 145-175:  Function Prototypes
Line 177-227:  setup()
Line 229-260:  loop()
Line 262-345:  Initialization Functions
Line 347-403:  I2C Utility Functions
Line 405-580:  Solenoid Control Functions
Line 582-712:  Test Functions
Line 714-816:  Utility Functions (printSeparator, printHelp, handleSerialInput)
```

### After Implementation

```
Line 1-31:     File header documentation (UPDATED)
Line 32-35:    Includes
Line 37-95:    Configuration Constants (UPDATED - add SOS timing)
Line 97-124:   MCP23017 Register Definitions
Line 126-150:  Global Objects and Variables (UPDATED - add sosRunning)
Line 152-185:  Function Prototypes (UPDATED - add SOS functions)
Line 187-237:  setup()
Line 239-280:  loop() (UPDATED - add SOS continuous mode check)
Line 282-365:  Initialization Functions
Line 367-423:  I2C Utility Functions
Line 425-600:  Solenoid Control Functions
Line 602-732:  Test Functions
Line 734-850:  SOS Functions (NEW SECTION)
Line 852-970:  Utility Functions (UPDATED - printHelp, handleSerialInput)
```

---

## Implementation Steps

### Step 1: Update File Header Documentation

**Location**: Lines 14-27 of `src/main.cpp`

**Find this block:**
```cpp
 * Test Features:
 *   1. I2C bus scanner
 *   2. MCP23017 initialization and communication verification
 *   3. Sequential channel cycling
 *   4. All-channels simultaneous test
 *   5. Interactive serial commands
 *
 * Serial Commands:
 *   'r' - Re-run all tests
 *   'a' - Activate all channels simultaneously
 *   's' - Run I2C scanner only
 *   '0'-'7' - Toggle individual channel
 *   'h' - Show help menu
```

**Replace with:**
```cpp
 * Test Features:
 *   1. I2C bus scanner
 *   2. MCP23017 initialization and communication verification
 *   3. Sequential channel cycling
 *   4. All-channels simultaneous test
 *   5. Interactive serial commands
 *   6. SOS morse code solenoid test
 *
 * Serial Commands:
 *   'r' - Re-run all tests
 *   'a' - Activate all channels simultaneously
 *   's' - Run I2C scanner only
 *   '0'-'7' - Toggle individual channel
 *   'o' - Run single SOS sequence on Channel 0
 *   'c' - Toggle continuous SOS mode
 *   'x' - Emergency stop (all off + stop SOS)
 *   'h' - Show help menu
```

---

### Step 2: Add SOS Timing Constants

**Location**: After line 84 (after `constexpr uint32_t TEST_DELAY_MS = 200;`)

**Add this new block:**
```cpp
/**
 * @defgroup SOSConfig SOS Morse Code Configuration
 * @{
 */

/** SOS test channel (GPA0) */
constexpr uint8_t SOS_CHANNEL = 0;

/** Dit (dot) duration in milliseconds - 1 unit */
constexpr uint32_t SOS_DIT_MS = 100;

/** Dah (dash) duration in milliseconds - 3 units */
constexpr uint32_t SOS_DAH_MS = 300;

/** Gap between elements (dits/dahs) within a letter - 1 unit */
constexpr uint32_t SOS_ELEMENT_GAP_MS = 100;

/** Gap between letters - 3 units (actually 2 additional after element gap) */
constexpr uint32_t SOS_LETTER_GAP_MS = 300;

/** Gap between words/SOS repetitions - 7 units */
constexpr uint32_t SOS_WORD_GAP_MS = 700;

/** @} */
```

---

### Step 3: Add Global State Variable

**Location**: After line 143 (after `uint32_t channelOffTime[NUM_CHANNELS] = {0};`)

**Add this line:**
```cpp
/** Flag for continuous SOS mode */
volatile bool sosRunning = false;
```

---

### Step 4: Add Function Prototypes

**Location**: After line 174 (after `void handleSerialInput();`)

**Add these prototypes:**
```cpp
// SOS Morse Code Functions
void playDit();
void playDah();
void playS();
void playO();
void playSOS();
void stopSOS();
```

---

### Step 5: Update Main Loop for Continuous SOS Mode

**Location**: Inside `loop()` function, after the safety timeout block (around line 256), before the `delay(1);` line.

**Find this section:**
```cpp
            }
        }
    }

    // Small delay to prevent tight loop
    delay(1);
}
```

**Replace with:**
```cpp
            }
        }
    }

    // Continuous SOS mode
    if (sosRunning && mcpInitialized) {
        playSOS();
        if (sosRunning) {  // Check again in case stopped during playSOS
            delay(SOS_WORD_GAP_MS);  // Gap between repetitions
        }
    }

    // Small delay to prevent tight loop
    delay(1);
}
```

---

### Step 6: Add SOS Functions

**Location**: After the Test Functions section (after `testAllChannelsSimultaneous()`), before the Utility Functions section.

**Add this complete new section:**
```cpp
// =============================================================================
// SOS MORSE CODE FUNCTIONS
// =============================================================================

/**
 * @brief Play a single dit (dot) - short pulse
 *
 * Activates SOS_CHANNEL for SOS_DIT_MS (100ms).
 * This is a blocking function.
 */
void playDit() {
    if (!mcpInitialized || !sosRunning) return;

    digitalWrite(LED_PIN, HIGH);
    mcp.digitalWrite(SOS_CHANNEL, HIGH);
    delay(SOS_DIT_MS);
    mcp.digitalWrite(SOS_CHANNEL, LOW);
    digitalWrite(LED_PIN, LOW);
}

/**
 * @brief Play a single dah (dash) - long pulse
 *
 * Activates SOS_CHANNEL for SOS_DAH_MS (300ms).
 * This is a blocking function.
 */
void playDah() {
    if (!mcpInitialized || !sosRunning) return;

    digitalWrite(LED_PIN, HIGH);
    mcp.digitalWrite(SOS_CHANNEL, HIGH);
    delay(SOS_DAH_MS);
    mcp.digitalWrite(SOS_CHANNEL, LOW);
    digitalWrite(LED_PIN, LOW);
}

/**
 * @brief Play the letter 'S' in morse code (. . .)
 *
 * Three dits with element gaps between them.
 * Pattern: dit-gap-dit-gap-dit
 */
void playS() {
    if (!sosRunning) return;

    // First dit
    playDit();
    if (!sosRunning) return;
    delay(SOS_ELEMENT_GAP_MS);

    // Second dit
    playDit();
    if (!sosRunning) return;
    delay(SOS_ELEMENT_GAP_MS);

    // Third dit
    playDit();
}

/**
 * @brief Play the letter 'O' in morse code (- - -)
 *
 * Three dahs with element gaps between them.
 * Pattern: dah-gap-dah-gap-dah
 */
void playO() {
    if (!sosRunning) return;

    // First dah
    playDah();
    if (!sosRunning) return;
    delay(SOS_ELEMENT_GAP_MS);

    // Second dah
    playDah();
    if (!sosRunning) return;
    delay(SOS_ELEMENT_GAP_MS);

    // Third dah
    playDah();
}

/**
 * @brief Play complete SOS sequence
 *
 * Plays S-O-S with proper letter gaps.
 * Pattern: S (letter gap) O (letter gap) S
 *
 * Timing breakdown:
 *   S = dit(100) + gap(100) + dit(100) + gap(100) + dit(100) = 500ms
 *   Letter gap = 300ms (but 100ms of element gap consumed) = 200ms additional
 *   O = dah(300) + gap(100) + dah(300) + gap(100) + dah(300) = 1100ms
 *   Letter gap = 200ms additional
 *   S = 500ms
 *   Total active time: ~2700ms
 */
void playSOS() {
    if (!mcpInitialized) {
        Serial.println(F("[ERROR] Cannot play SOS - MCP23017 not initialized"));
        return;
    }

    Serial.println(F("Playing SOS: ... --- ..."));

    // Ensure channel starts OFF
    mcp.digitalWrite(SOS_CHANNEL, LOW);

    // Play 'S' (. . .)
    playS();
    if (!sosRunning) {
        mcp.digitalWrite(SOS_CHANNEL, LOW);
        return;
    }

    // Letter gap between S and O (300ms total, minus element gap already taken)
    delay(SOS_LETTER_GAP_MS - SOS_ELEMENT_GAP_MS);
    if (!sosRunning) {
        mcp.digitalWrite(SOS_CHANNEL, LOW);
        return;
    }

    // Play 'O' (- - -)
    playO();
    if (!sosRunning) {
        mcp.digitalWrite(SOS_CHANNEL, LOW);
        return;
    }

    // Letter gap between O and S
    delay(SOS_LETTER_GAP_MS - SOS_ELEMENT_GAP_MS);
    if (!sosRunning) {
        mcp.digitalWrite(SOS_CHANNEL, LOW);
        return;
    }

    // Play 'S' (. . .)
    playS();

    // Ensure channel ends OFF
    mcp.digitalWrite(SOS_CHANNEL, LOW);
    digitalWrite(LED_PIN, LOW);

    Serial.println(F("SOS complete."));
}

/**
 * @brief Stop SOS playback immediately
 *
 * Sets sosRunning flag to false and ensures channel is OFF.
 * Called by emergency stop and continuous mode toggle.
 */
void stopSOS() {
    sosRunning = false;
    if (mcpInitialized) {
        mcp.digitalWrite(SOS_CHANNEL, LOW);
    }
    digitalWrite(LED_PIN, LOW);
    Serial.println(F("[OK] SOS stopped"));
}
```

---

### Step 7: Update printHelp() Function

**Location**: Inside `printHelp()` function (around line 728-738)

**Find this block:**
```cpp
void printHelp() {
    Serial.println(F("SERIAL COMMANDS:"));
    Serial.println(F("  'r' - Re-run all tests"));
    Serial.println(F("  'a' - Activate all channels for 100ms"));
    Serial.println(F("  's' - Run I2C scanner"));
    Serial.println(F("  '0'-'7' - Toggle individual channel"));
    Serial.println(F("  'x' - Emergency stop (all off)"));
    Serial.println(F("  'h' - Show this help menu"));
    Serial.println();
    Serial.println(F("Waiting for commands..."));
}
```

**Replace with:**
```cpp
void printHelp() {
    Serial.println(F("SERIAL COMMANDS:"));
    Serial.println(F("  'r' - Re-run all tests"));
    Serial.println(F("  'a' - Activate all channels for 100ms"));
    Serial.println(F("  's' - Run I2C scanner"));
    Serial.println(F("  '0'-'7' - Toggle individual channel"));
    Serial.println(F("  'o' - Run single SOS sequence (Channel 0)"));
    Serial.println(F("  'c' - Toggle continuous SOS mode"));
    Serial.println(F("  'x' - Emergency stop (all off + stop SOS)"));
    Serial.println(F("  'h' - Show this help menu"));
    Serial.println();
    Serial.println(F("Waiting for commands..."));
}
```

---

### Step 8: Update handleSerialInput() Function

**Location**: Inside `handleSerialInput()` function, in the switch statement.

**Find the emergency stop case (around line 793-796):**
```cpp
            case 'x':
            case 'X':
                Serial.println(F("EMERGENCY STOP"));
                deactivateAllChannels();
                break;
```

**Replace with:**
```cpp
            case 'x':
            case 'X':
                Serial.println(F("EMERGENCY STOP"));
                stopSOS();  // Stop SOS first
                deactivateAllChannels();
                break;
```

**Find the default case (around line 810):**
```cpp
            default:
                Serial.print(F("Unknown command: '"));
                Serial.print(cmd);
                Serial.println(F("' (press 'h' for help)"));
                break;
```

**Add these new cases BEFORE the default case:**
```cpp
            case 'o':
            case 'O':
                if (!sosRunning) {
                    Serial.println(F("Running single SOS sequence..."));
                    sosRunning = true;  // Enable for this sequence
                    playSOS();
                    sosRunning = false;  // Disable after completion
                } else {
                    Serial.println(F("[INFO] SOS already running"));
                }
                break;

            case 'c':
            case 'C':
                if (sosRunning) {
                    Serial.println(F("Stopping continuous SOS mode..."));
                    stopSOS();
                } else {
                    Serial.println(F("Starting continuous SOS mode..."));
                    Serial.println(F("Press 'x' or 'c' to stop."));
                    sosRunning = true;
                }
                break;

            default:
                Serial.print(F("Unknown command: '"));
                Serial.print(cmd);
                Serial.println(F("' (press 'h' for help)"));
                break;
```

---

## Complete Code Blocks

### Complete SOS Configuration Constants

```cpp
/**
 * @defgroup SOSConfig SOS Morse Code Configuration
 * @{
 */

/** SOS test channel (GPA0) */
constexpr uint8_t SOS_CHANNEL = 0;

/** Dit (dot) duration in milliseconds - 1 unit */
constexpr uint32_t SOS_DIT_MS = 100;

/** Dah (dash) duration in milliseconds - 3 units */
constexpr uint32_t SOS_DAH_MS = 300;

/** Gap between elements (dits/dahs) within a letter - 1 unit */
constexpr uint32_t SOS_ELEMENT_GAP_MS = 100;

/** Gap between letters - 3 units (actually 2 additional after element gap) */
constexpr uint32_t SOS_LETTER_GAP_MS = 300;

/** Gap between words/SOS repetitions - 7 units */
constexpr uint32_t SOS_WORD_GAP_MS = 700;

/** @} */
```

### Complete SOS Functions Section

```cpp
// =============================================================================
// SOS MORSE CODE FUNCTIONS
// =============================================================================

/**
 * @brief Play a single dit (dot) - short pulse
 *
 * Activates SOS_CHANNEL for SOS_DIT_MS (100ms).
 * This is a blocking function.
 */
void playDit() {
    if (!mcpInitialized || !sosRunning) return;

    digitalWrite(LED_PIN, HIGH);
    mcp.digitalWrite(SOS_CHANNEL, HIGH);
    delay(SOS_DIT_MS);
    mcp.digitalWrite(SOS_CHANNEL, LOW);
    digitalWrite(LED_PIN, LOW);
}

/**
 * @brief Play a single dah (dash) - long pulse
 *
 * Activates SOS_CHANNEL for SOS_DAH_MS (300ms).
 * This is a blocking function.
 */
void playDah() {
    if (!mcpInitialized || !sosRunning) return;

    digitalWrite(LED_PIN, HIGH);
    mcp.digitalWrite(SOS_CHANNEL, HIGH);
    delay(SOS_DAH_MS);
    mcp.digitalWrite(SOS_CHANNEL, LOW);
    digitalWrite(LED_PIN, LOW);
}

/**
 * @brief Play the letter 'S' in morse code (. . .)
 *
 * Three dits with element gaps between them.
 * Pattern: dit-gap-dit-gap-dit
 */
void playS() {
    if (!sosRunning) return;

    // First dit
    playDit();
    if (!sosRunning) return;
    delay(SOS_ELEMENT_GAP_MS);

    // Second dit
    playDit();
    if (!sosRunning) return;
    delay(SOS_ELEMENT_GAP_MS);

    // Third dit
    playDit();
}

/**
 * @brief Play the letter 'O' in morse code (- - -)
 *
 * Three dahs with element gaps between them.
 * Pattern: dah-gap-dah-gap-dah
 */
void playO() {
    if (!sosRunning) return;

    // First dah
    playDah();
    if (!sosRunning) return;
    delay(SOS_ELEMENT_GAP_MS);

    // Second dah
    playDah();
    if (!sosRunning) return;
    delay(SOS_ELEMENT_GAP_MS);

    // Third dah
    playDah();
}

/**
 * @brief Play complete SOS sequence
 *
 * Plays S-O-S with proper letter gaps.
 * Pattern: S (letter gap) O (letter gap) S
 *
 * Timing breakdown:
 *   S = dit(100) + gap(100) + dit(100) + gap(100) + dit(100) = 500ms
 *   Letter gap = 300ms (but 100ms of element gap consumed) = 200ms additional
 *   O = dah(300) + gap(100) + dah(300) + gap(100) + dah(300) = 1100ms
 *   Letter gap = 200ms additional
 *   S = 500ms
 *   Total active time: ~2700ms
 */
void playSOS() {
    if (!mcpInitialized) {
        Serial.println(F("[ERROR] Cannot play SOS - MCP23017 not initialized"));
        return;
    }

    Serial.println(F("Playing SOS: ... --- ..."));

    // Ensure channel starts OFF
    mcp.digitalWrite(SOS_CHANNEL, LOW);

    // Play 'S' (. . .)
    playS();
    if (!sosRunning) {
        mcp.digitalWrite(SOS_CHANNEL, LOW);
        return;
    }

    // Letter gap between S and O (300ms total, minus element gap already taken)
    delay(SOS_LETTER_GAP_MS - SOS_ELEMENT_GAP_MS);
    if (!sosRunning) {
        mcp.digitalWrite(SOS_CHANNEL, LOW);
        return;
    }

    // Play 'O' (- - -)
    playO();
    if (!sosRunning) {
        mcp.digitalWrite(SOS_CHANNEL, LOW);
        return;
    }

    // Letter gap between O and S
    delay(SOS_LETTER_GAP_MS - SOS_ELEMENT_GAP_MS);
    if (!sosRunning) {
        mcp.digitalWrite(SOS_CHANNEL, LOW);
        return;
    }

    // Play 'S' (. . .)
    playS();

    // Ensure channel ends OFF
    mcp.digitalWrite(SOS_CHANNEL, LOW);
    digitalWrite(LED_PIN, LOW);

    Serial.println(F("SOS complete."));
}

/**
 * @brief Stop SOS playback immediately
 *
 * Sets sosRunning flag to false and ensures channel is OFF.
 * Called by emergency stop and continuous mode toggle.
 */
void stopSOS() {
    sosRunning = false;
    if (mcpInitialized) {
        mcp.digitalWrite(SOS_CHANNEL, LOW);
    }
    digitalWrite(LED_PIN, LOW);
    Serial.println(F("[OK] SOS stopped"));
}
```

### Complete Updated handleSerialInput() Function

```cpp
/**
 * @brief Handle incoming serial commands
 */
void handleSerialInput() {
    if (Serial.available() > 0) {
        char cmd = Serial.read();

        // Flush any remaining characters
        while (Serial.available()) {
            Serial.read();
        }

        Serial.println();

        switch (cmd) {
            case 'r':
            case 'R':
                Serial.println(F("Re-running all tests..."));
                runAllTests();
                break;

            case 'a':
            case 'A':
                Serial.println(F("Activating all channels..."));
                testAllChannelsSimultaneous();
                break;

            case 's':
            case 'S':
                scanI2CBus();
                break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7': {
                uint8_t channel = cmd - '0';
                bool currentState = (channelStates >> channel) & 0x01;
                bool newState = !currentState;

                Serial.print(F("Toggling channel "));
                Serial.print(channel);
                Serial.print(F(" -> "));
                Serial.println(newState ? F("ON") : F("OFF"));

                setChannel(channel, newState);
                break;
            }

            case 'o':
            case 'O':
                if (!sosRunning) {
                    Serial.println(F("Running single SOS sequence..."));
                    sosRunning = true;  // Enable for this sequence
                    playSOS();
                    sosRunning = false;  // Disable after completion
                } else {
                    Serial.println(F("[INFO] SOS already running"));
                }
                break;

            case 'c':
            case 'C':
                if (sosRunning) {
                    Serial.println(F("Stopping continuous SOS mode..."));
                    stopSOS();
                } else {
                    Serial.println(F("Starting continuous SOS mode..."));
                    Serial.println(F("Press 'x' or 'c' to stop."));
                    sosRunning = true;
                }
                break;

            case 'x':
            case 'X':
                Serial.println(F("EMERGENCY STOP"));
                stopSOS();  // Stop SOS first
                deactivateAllChannels();
                break;

            case 'h':
            case 'H':
            case '?':
                printHelp();
                break;

            case '\n':
            case '\r':
                // Ignore newlines
                break;

            default:
                Serial.print(F("Unknown command: '"));
                Serial.print(cmd);
                Serial.println(F("' (press 'h' for help)"));
                break;
        }
    }
}
```

---

## Build and Upload

### Build Project

```bash
cd /Users/caillou/repos/caillou/mechanical-midi-piano

# Clean build
pio run --target clean

# Build
pio run
```

**Expected Output:**
```
Processing teensy41 (platform: teensy; board: teensy41; framework: arduino)
--------------------------------------------------------------------------------
...
Compiling .pio/build/teensy41/src/main.cpp.o
Linking .pio/build/teensy41/firmware.elf
Building .pio/build/teensy41/firmware.hex
============ [SUCCESS] ============
```

### Check for Warnings

Ensure no warnings related to SOS functions:
- No unused variable warnings
- No type conversion warnings
- No missing prototype warnings

### Upload to Teensy

```bash
pio run --target upload
```

**Expected Output:**
```
Uploading .pio/build/teensy41/firmware.hex
...
============ [SUCCESS] ============
```

### Open Serial Monitor

```bash
pio device monitor --baud 115200
```

---

## Verification

### Test 1: Help Menu Shows New Commands

Press 'h' in serial monitor.

**Expected Output:**
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

### Test 2: Single SOS Sequence

Press 'o' in serial monitor.

**Expected Output:**
```
Running single SOS sequence...
Playing SOS: ... --- ...
SOS complete.
```

**Expected Solenoid Behavior:**
- 3 quick clicks (S: dit-dit-dit)
- Short pause
- 3 longer activations (O: dah-dah-dah)
- Short pause
- 3 quick clicks (S: dit-dit-dit)

### Test 3: Continuous SOS Mode

Press 'c' to start, then 'c' or 'x' to stop.

**Start Output:**
```
Starting continuous SOS mode...
Press 'x' or 'c' to stop.
Playing SOS: ... --- ...
SOS complete.
Playing SOS: ... --- ...
...
```

**Stop Output (with 'c'):**
```
Stopping continuous SOS mode...
[OK] SOS stopped
```

**Stop Output (with 'x'):**
```
EMERGENCY STOP
[OK] SOS stopped
[OK] All channels deactivated
```

### Test 4: Emergency Stop During SOS

1. Start continuous SOS with 'c'
2. Press 'x' during playback

**Expected Behavior:**
- SOS stops immediately
- Solenoid releases
- LED turns off
- Confirmation messages appear

---

## Troubleshooting

### Compilation Errors

| Error | Cause | Solution |
|-------|-------|----------|
| `'sosRunning' was not declared` | Missing global variable | Add `volatile bool sosRunning = false;` |
| `'playDit' was not declared` | Missing prototype | Add function prototypes |
| `'SOS_CHANNEL' was not declared` | Missing constants | Add SOS configuration constants |

### Runtime Errors

| Symptom | Cause | Solution |
|---------|-------|----------|
| SOS doesn't play | MCP23017 not initialized | Check I2C connection |
| SOS on wrong channel | SOS_CHANNEL not 0 | Verify `constexpr uint8_t SOS_CHANNEL = 0;` |
| Can't stop SOS | Emergency stop not updated | Verify `stopSOS()` called before `deactivateAllChannels()` |
| Timing wrong | Constants incorrect | Verify timing constant values |

### Testing Tips

1. **Use LED for debugging**: The built-in LED mirrors solenoid state
2. **Check serial output**: All operations print status messages
3. **Measure timing**: Use stopwatch to verify approximate timing
4. **Test emergency stop first**: Always verify 'x' works before extended testing

---

## Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2026-01-18 | - | Initial creation |
