# main.cpp Restructure

## File Location

**Full Path:** `/Users/caillou/repos/caillou/mechanical-midi-piano/src/main.cpp`

## Overview

This document details every change to main.cpp, organized by section. The file will be significantly simplified: test code is removed and MIDI handling is added.

---

## Section 1: File Header Comment

### Current (Lines 1-32)
```cpp
/**
 * @file main.cpp
 * @brief Solenoid Driver Test Program for Mechanical MIDI Piano
 *
 * This program tests the Adafruit I2C to 8 Channel Solenoid Driver
 * with a Teensy 4.1 microcontroller.
 *
 * Hardware:
 *   - Teensy 4.1
 *   - Adafruit I2C Solenoid Driver (Product ID 6318)
 *   - I2C: SDA=Pin 18, SCL=Pin 19 (Wire)
 *   - Default I2C Address: 0x20
 *
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
 *   'x' - Emergency stop (all off)
 *   'h' - Show help menu
 *
 * @author Mechanical MIDI Piano Project
 * @date 2025-01-18
 * @version 1.0.0
 */
```

### New
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

---

## Section 2: Configuration Constants

### Keep These Constants (Lines 47-58)
```cpp
/** I2C bus speed in Hz (400kHz recommended for MCP23017) */
constexpr uint32_t I2C_CLOCK_SPEED = 400000;

/** Default I2C address for MCP23017 (A0=A1=A2=0) */
constexpr uint8_t MCP23017_DEFAULT_ADDRESS = 0x20;

/** I2C scan address range start (excludes reserved addresses 0x00-0x07) */
constexpr uint8_t I2C_SCAN_START_ADDR = 0x08;

/** I2C scan address range end (excludes reserved addresses 0x78-0x7F) */
constexpr uint8_t I2C_SCAN_END_ADDR = 0x78;
```

### Keep These Constants (Lines 66-67)
```cpp
/** Number of solenoid channels on the driver board */
constexpr uint8_t NUM_CHANNELS = 8;
```

### Modify These Constants (Lines 77-83)

**Current:**
```cpp
/**
 * Maximum solenoid on-time in milliseconds
 * Prevents coil overheating - adjust based on solenoid specs
 */
constexpr uint32_t MAX_ON_TIME_MS = 5000;

/**
 * Minimum off-time between activations in milliseconds
 * Allows coil cooling between activations
 */
constexpr uint32_t MIN_OFF_TIME_MS = 50;
```

**New (adjusted for musical performance):**
```cpp
/**
 * Maximum solenoid on-time in milliseconds
 * Prevents coil overheating - 2 seconds is plenty for piano notes
 */
constexpr uint32_t MAX_ON_TIME_MS = 2000;

/**
 * Minimum off-time between activations in milliseconds
 * 15ms allows fast trills while providing some cooling
 */
constexpr uint32_t MIN_OFF_TIME_MS = 15;
```

### Remove These Constants (Lines 88-94)
```cpp
// DELETE ENTIRE BLOCK:
/**
 * Test activation duration in milliseconds
 * Short duration for safe testing
 */
constexpr uint32_t TEST_ACTIVATION_MS = 100;

/**
 * Delay between sequential channel tests in milliseconds
 */
constexpr uint32_t TEST_DELAY_MS = 200;
```

### Add New MIDI Constants (After NUM_CHANNELS)

Insert after the NUM_CHANNELS constant:
```cpp
/**
 * @defgroup MIDIConfig MIDI Configuration
 * @{
 */

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

/** @} */
```

---

## Section 3: Function Prototypes

### Current (Lines 126-151)
```cpp
// Initialization
void initSerial();
void initI2C();
bool initMCP23017();

// I2C Utilities
void scanI2CBus();

// Solenoid Control
void toggleChannel(uint8_t channel);
bool setChannel(uint8_t channel, bool state);
bool setAllChannels(uint8_t states);
bool activateChannel(uint8_t channel, uint32_t duration);
void deactivateAllChannels();

// Test Functions
void runAllTests();
void testSequentialChannels();
void testAllChannelsSimultaneous();
void testCommunication();

// Utility Functions
void printSeparator();
void printHelp();
void handleSerialInput();
```

### New
```cpp
// Initialization
void initSerial();
void initI2C();
bool initMCP23017();

// MIDI Handlers
int8_t noteToChannel(uint8_t note);
void handleNoteOn(byte channel, byte note, byte velocity);
void handleNoteOff(byte channel, byte note, byte velocity);

// Solenoid Control
void deactivateAllChannels();

// Utility Functions
void printSeparator();
void printHelp();
void printStatus();
void handleSerialInput();
```

---

## Section 4: Setup Function

### Current (Lines 163-205)
```cpp
void setup()
{
    // Initialize LED pin for status indication
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // LED on during setup

    // Initialize serial communication
    initSerial();

    // Print startup banner
    printSeparator();
    Serial.println(F("MECHANICAL MIDI PIANO - SOLENOID DRIVER TEST"));
    Serial.println(F("Teensy 4.1 + Adafruit I2C Solenoid Driver"));
    printSeparator();
    Serial.println();

    // Initialize I2C bus
    initI2C();

    // Scan for I2C devices
    scanI2CBus();

    // Initialize MCP23017
    if (initMCP23017())
    {
        Serial.println(F("[OK] MCP23017 initialized successfully"));
        // Run initial tests
        Serial.println();
        Serial.println(F("Running initial tests..."));
        runAllTests();
    }
    else
    {
        Serial.println(F("[ERROR] Failed to initialize MCP23017!"));
        Serial.println(F("Check wiring and I2C address."));
    }

    // Print help menu
    Serial.println();
    printHelp();

    digitalWrite(LED_PIN, LOW); // LED off after setup
}
```

### New
```cpp
void setup()
{
    // Initialize LED pin for status indication
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // LED on during setup

    // Initialize serial communication
    initSerial();

    // Print startup banner
    printSeparator();
    Serial.println(F("MECHANICAL MIDI PIANO - USB MIDI CONTROLLER"));
    Serial.println(F("Teensy 4.1 + Adafruit I2C Solenoid Driver"));
    printSeparator();
    Serial.println();

    // Initialize I2C bus
    initI2C();

    // Initialize MCP23017
    if (initMCP23017())
    {
        Serial.println(F("[OK] MCP23017 initialized successfully"));
    }
    else
    {
        Serial.println(F("[ERROR] Failed to initialize MCP23017!"));
        Serial.println(F("Check wiring and I2C address."));
    }

    // Register MIDI callbacks
    usbMIDI.setHandleNoteOn(handleNoteOn);
    usbMIDI.setHandleNoteOff(handleNoteOff);
    Serial.println(F("[OK] MIDI handlers registered"));
    Serial.print(F("  Listening for notes "));
    Serial.print(MIDI_NOTE_LOW);
    Serial.print(F("-"));
    Serial.print(MIDI_NOTE_HIGH);
    Serial.println(F(" (C4-G4)"));

    // Print help menu
    Serial.println();
    printHelp();

    digitalWrite(LED_PIN, LOW); // LED off after setup
}
```

---

## Section 5: Main Loop

### Current (Lines 216-229)
```cpp
void loop()
{
    // Handle incoming serial commands
    handleSerialInput();

    // SolenoidDriver safety update - handles auto-shutoff for max on-time
    if (solenoidDriver.isInitialized())
    {
        solenoidDriver.update();
    }

    // Small delay to prevent tight loop
    delay(1);
}
```

### New
```cpp
void loop()
{
    // Process all pending MIDI messages
    // This calls handleNoteOn/handleNoteOff callbacks as needed
    while (usbMIDI.read()) { }

    // SolenoidDriver safety update - handles auto-shutoff for max on-time
    if (solenoidDriver.isInitialized())
    {
        solenoidDriver.update();
    }

    // Handle incoming serial commands (emergency stop, status, help)
    handleSerialInput();
}
```

**Note:** The `delay(1)` is removed to improve MIDI latency.

---

## Section 6: Initialization Functions (KEEP AS-IS)

Keep the following functions unchanged:
- `initSerial()` (Lines 241-253)
- `initI2C()` (Lines 261-275)
- `initMCP23017()` (Lines 285-314)

---

## Section 7: I2C Utility Functions (DELETE)

### Delete scanI2CBus() (Lines 327-361)

Remove the entire function:
```cpp
// DELETE THIS ENTIRE FUNCTION
void scanI2CBus()
{
    // ... all 35 lines ...
}
```

**Reason:** This was a diagnostic function for initial hardware testing. The SolenoidDriver library has its own `scanI2C()` method if needed.

---

## Section 8: Solenoid Control Functions (MOSTLY DELETE)

### Delete toggleChannel() (Lines 376-387)
```cpp
// DELETE THIS ENTIRE FUNCTION
void toggleChannel(uint8_t channel)
{
    // ... all 12 lines ...
}
```

### Delete setChannel() (Lines 399-424)
```cpp
// DELETE THIS ENTIRE FUNCTION
bool setChannel(uint8_t channel, bool state)
{
    // ... all 26 lines ...
}
```

### Delete setAllChannels() (Lines 434-452)
```cpp
// DELETE THIS ENTIRE FUNCTION
bool setAllChannels(uint8_t states)
{
    // ... all 19 lines ...
}
```

### Delete activateChannel() (Lines 463-483)
```cpp
// DELETE THIS ENTIRE FUNCTION
bool activateChannel(uint8_t channel, uint32_t duration)
{
    // ... all 21 lines ...
}
```

### Keep deactivateAllChannels() (Lines 492-501)

Keep this function unchanged - it's the emergency stop:
```cpp
void deactivateAllChannels()
{
    if (solenoidDriver.isInitialized())
    {
        solenoidDriver.emergencyStop();
        solenoidDriver.resetAllStats();
    }

    Serial.println(F("[OK] All channels deactivated"));
}
```

---

## Section 9: Test Functions (DELETE ALL)

### Delete runAllTests() (Lines 510-540)
```cpp
// DELETE THIS ENTIRE FUNCTION
void runAllTests()
{
    // ... all 31 lines ...
}
```

### Delete testCommunication() (Lines 547-565)
```cpp
// DELETE THIS ENTIRE FUNCTION
void testCommunication()
{
    // ... all 19 lines ...
}
```

### Delete testSequentialChannels() (Lines 573-607)
```cpp
// DELETE THIS ENTIRE FUNCTION
void testSequentialChannels()
{
    // ... all 35 lines ...
}
```

### Delete testAllChannelsSimultaneous() (Lines 614-636)
```cpp
// DELETE THIS ENTIRE FUNCTION
void testAllChannelsSimultaneous()
{
    // ... all 23 lines ...
}
```

---

## Section 10: New MIDI Handler Functions (ADD)

Add these new functions after `deactivateAllChannels()`:

```cpp
// =============================================================================
// MIDI HANDLER FUNCTIONS
// =============================================================================

/**
 * @brief Convert MIDI note number to solenoid channel
 *
 * @param note MIDI note number (0-127)
 * @return Channel number (0-7) if note is in range, -1 otherwise
 *
 * Maps notes 60-67 (C4 through G4) to channels 0-7.
 */
int8_t noteToChannel(uint8_t note)
{
    if (note < MIDI_NOTE_LOW || note > MIDI_NOTE_HIGH)
    {
        return -1;
    }
    return note - MIDI_NOTE_LOW;
}

/**
 * @brief Handle MIDI Note On messages
 *
 * @param channel MIDI channel (1-16, ignored - we respond to all channels)
 * @param note MIDI note number (0-127)
 * @param velocity Note velocity (0-127, 0 treated as note-off)
 *
 * Called automatically by usbMIDI when a Note On message is received.
 * Velocity 0 is treated as Note Off per MIDI specification.
 */
void handleNoteOn(byte channel, byte note, byte velocity)
{
    // Velocity 0 is equivalent to Note Off
    if (velocity == 0)
    {
        handleNoteOff(channel, note, velocity);
        return;
    }

    int8_t ch = noteToChannel(note);
    if (ch < 0)
    {
        return;  // Note not in our range
    }

    if (!solenoidDriver.isInitialized())
    {
        return;
    }

    // Turn on the solenoid
    SolenoidError err = solenoidDriver.on(ch);
    if (err != SolenoidError::OK)
    {
        Serial.print(F("[MIDI] Note "));
        Serial.print(note);
        Serial.print(F(" ON failed: "));
        Serial.println(SolenoidDriver::getErrorString(err));
    }
}

/**
 * @brief Handle MIDI Note Off messages
 *
 * @param channel MIDI channel (1-16, ignored)
 * @param note MIDI note number (0-127)
 * @param velocity Release velocity (0-127, ignored)
 *
 * Called automatically by usbMIDI when a Note Off message is received.
 */
void handleNoteOff(byte channel, byte note, byte velocity)
{
    (void)velocity;  // Unused parameter

    int8_t ch = noteToChannel(note);
    if (ch < 0)
    {
        return;  // Note not in our range
    }

    if (!solenoidDriver.isInitialized())
    {
        return;
    }

    // Turn off the solenoid
    SolenoidError err = solenoidDriver.off(ch);
    if (err != SolenoidError::OK)
    {
        Serial.print(F("[MIDI] Note "));
        Serial.print(note);
        Serial.print(F(" OFF failed: "));
        Serial.println(SolenoidDriver::getErrorString(err));
    }
}
```

---

## Section 11: Utility Functions

### Keep printSeparator() (Lines 645-648)
```cpp
void printSeparator()
{
    Serial.println(F("============================================================"));
}
```

### Replace printHelp() (Lines 653-664)

**Current:**
```cpp
void printHelp()
{
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

**New:**
```cpp
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

### Add printStatus() (NEW FUNCTION)

Add this new function after `printHelp()`:

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

### Replace handleSerialInput() (Lines 669-732)

**Current:**
```cpp
void handleSerialInput()
{
    if (Serial.available() > 0)
    {
        char cmd = Serial.read();

        // Flush any remaining characters
        while (Serial.available())
        {
            Serial.read();
        }

        Serial.println();

        switch (cmd)
        {
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
        case '7':
            toggleChannel(cmd - '0');
            break;

        case 'x':
        case 'X':
            Serial.println(F("EMERGENCY STOP"));
            deactivateAllChannels();
            break;

        case 'h':
        case 'H':
        case '?':
            printHelp();
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

**New:**
```cpp
void handleSerialInput()
{
    if (Serial.available() > 0)
    {
        char cmd = Serial.read();

        // Flush any remaining characters
        while (Serial.available())
        {
            Serial.read();
        }

        Serial.println();

        switch (cmd)
        {
        case 'x':
        case 'X':
            Serial.println(F("EMERGENCY STOP"));
            deactivateAllChannels();
            break;

        case 's':
        case 'S':
            printStatus();
            break;

        case 'h':
        case 'H':
        case '?':
            printHelp();
            break;

        case '\r':
        case '\n':
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

## Complete New main.cpp

For reference, here is the complete restructured file:

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

#include <Arduino.h>
#include <Wire.h>
#include "SolenoidDriver.h"

// =============================================================================
// CONFIGURATION CONSTANTS
// =============================================================================

/**
 * @defgroup I2CConfig I2C Configuration
 * @{
 */

/** I2C bus speed in Hz (400kHz recommended for MCP23017) */
constexpr uint32_t I2C_CLOCK_SPEED = 400000;

/** Default I2C address for MCP23017 (A0=A1=A2=0) */
constexpr uint8_t MCP23017_DEFAULT_ADDRESS = 0x20;

/** @} */

/**
 * @defgroup SolenoidConfig Solenoid Control Configuration
 * @{
 */

/** Number of solenoid channels on the driver board */
constexpr uint8_t NUM_CHANNELS = 8;

/** @note Timing calculations use unsigned 32-bit subtraction which is safe
 *  across millis() overflow (wraps every ~49.7 days) due to C/C++ unsigned
 *  arithmetic guarantees. */

/**
 * Maximum solenoid on-time in milliseconds
 * Prevents coil overheating - 2 seconds is plenty for piano notes
 */
constexpr uint32_t MAX_ON_TIME_MS = 2000;

/**
 * Minimum off-time between activations in milliseconds
 * 15ms allows fast trills while providing some cooling
 */
constexpr uint32_t MIN_OFF_TIME_MS = 15;

/** @} */

/**
 * @defgroup MIDIConfig MIDI Configuration
 * @{
 */

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

/** @} */

/**
 * @defgroup Pins Pin Definitions
 * @{
 */

/** Built-in LED pin on Teensy 4.1 */
constexpr uint8_t LED_PIN = LED_BUILTIN;

/** @} */

// =============================================================================
// GLOBAL OBJECTS
// =============================================================================

/** SolenoidDriver instance for MCP23017 control */
SolenoidDriver solenoidDriver;

// =============================================================================
// FUNCTION PROTOTYPES
// =============================================================================

// Initialization
void initSerial();
void initI2C();
bool initMCP23017();

// MIDI Handlers
int8_t noteToChannel(uint8_t note);
void handleNoteOn(byte channel, byte note, byte velocity);
void handleNoteOff(byte channel, byte note, byte velocity);

// Solenoid Control
void deactivateAllChannels();

// Utility Functions
void printSeparator();
void printHelp();
void printStatus();
void handleSerialInput();

// =============================================================================
// SETUP
// =============================================================================

/**
 * @brief Arduino setup function - runs once at startup
 *
 * Initializes serial communication, I2C bus, MCP23017, and MIDI handlers.
 */
void setup()
{
    // Initialize LED pin for status indication
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // LED on during setup

    // Initialize serial communication
    initSerial();

    // Print startup banner
    printSeparator();
    Serial.println(F("MECHANICAL MIDI PIANO - USB MIDI CONTROLLER"));
    Serial.println(F("Teensy 4.1 + Adafruit I2C Solenoid Driver"));
    printSeparator();
    Serial.println();

    // Initialize I2C bus
    initI2C();

    // Initialize MCP23017
    if (initMCP23017())
    {
        Serial.println(F("[OK] MCP23017 initialized successfully"));
    }
    else
    {
        Serial.println(F("[ERROR] Failed to initialize MCP23017!"));
        Serial.println(F("Check wiring and I2C address."));
    }

    // Register MIDI callbacks
    usbMIDI.setHandleNoteOn(handleNoteOn);
    usbMIDI.setHandleNoteOff(handleNoteOff);
    Serial.println(F("[OK] MIDI handlers registered"));
    Serial.print(F("  Listening for notes "));
    Serial.print(MIDI_NOTE_LOW);
    Serial.print(F("-"));
    Serial.print(MIDI_NOTE_HIGH);
    Serial.println(F(" (C4-G4)"));

    // Print help menu
    Serial.println();
    printHelp();

    digitalWrite(LED_PIN, LOW); // LED off after setup
}

// =============================================================================
// MAIN LOOP
// =============================================================================

/**
 * @brief Arduino main loop - runs continuously
 *
 * Processes MIDI messages and monitors solenoid safety.
 */
void loop()
{
    // Process all pending MIDI messages
    // This calls handleNoteOn/handleNoteOff callbacks as needed
    while (usbMIDI.read()) { }

    // SolenoidDriver safety update - handles auto-shutoff for max on-time
    if (solenoidDriver.isInitialized())
    {
        solenoidDriver.update();
    }

    // Handle incoming serial commands (emergency stop, status, help)
    handleSerialInput();
}

// =============================================================================
// INITIALIZATION FUNCTIONS
// =============================================================================

/**
 * @brief Initialize serial communication
 *
 * Waits for serial port to be ready (up to 3 seconds).
 * Uses 115200 baud rate as configured in platformio.ini.
 */
void initSerial()
{
    Serial.begin(115200);

    // Wait for serial port to connect (with timeout for standalone operation)
    uint32_t startTime = millis();
    while (!Serial && (millis() - startTime < 3000))
    {
        delay(10);
    }

    delay(100); // Additional stabilization delay
}

/**
 * @brief Initialize I2C bus (Wire)
 *
 * Configures the primary I2C bus with the specified clock speed.
 * Uses Teensy 4.1 default I2C pins: SDA=18, SCL=19.
 */
void initI2C()
{
    Serial.println(F("Initializing I2C bus..."));
    Serial.print(F("  SDA Pin: 18, SCL Pin: 19"));
    Serial.print(F(", Speed: "));
    Serial.print(I2C_CLOCK_SPEED / 1000);
    Serial.println(F(" kHz"));

    Wire.begin();
    Wire.setClock(I2C_CLOCK_SPEED);

    delay(100); // Allow bus to stabilize

    Serial.println(F("[OK] I2C bus initialized"));
}

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

// =============================================================================
// MIDI HANDLER FUNCTIONS
// =============================================================================

/**
 * @brief Convert MIDI note number to solenoid channel
 *
 * @param note MIDI note number (0-127)
 * @return Channel number (0-7) if note is in range, -1 otherwise
 *
 * Maps notes 60-67 (C4 through G4) to channels 0-7.
 */
int8_t noteToChannel(uint8_t note)
{
    if (note < MIDI_NOTE_LOW || note > MIDI_NOTE_HIGH)
    {
        return -1;
    }
    return note - MIDI_NOTE_LOW;
}

/**
 * @brief Handle MIDI Note On messages
 *
 * @param channel MIDI channel (1-16, ignored - we respond to all channels)
 * @param note MIDI note number (0-127)
 * @param velocity Note velocity (0-127, 0 treated as note-off)
 *
 * Called automatically by usbMIDI when a Note On message is received.
 * Velocity 0 is treated as Note Off per MIDI specification.
 */
void handleNoteOn(byte channel, byte note, byte velocity)
{
    // Velocity 0 is equivalent to Note Off
    if (velocity == 0)
    {
        handleNoteOff(channel, note, velocity);
        return;
    }

    int8_t ch = noteToChannel(note);
    if (ch < 0)
    {
        return;  // Note not in our range
    }

    if (!solenoidDriver.isInitialized())
    {
        return;
    }

    // Turn on the solenoid
    SolenoidError err = solenoidDriver.on(ch);
    if (err != SolenoidError::OK)
    {
        Serial.print(F("[MIDI] Note "));
        Serial.print(note);
        Serial.print(F(" ON failed: "));
        Serial.println(SolenoidDriver::getErrorString(err));
    }
}

/**
 * @brief Handle MIDI Note Off messages
 *
 * @param channel MIDI channel (1-16, ignored)
 * @param note MIDI note number (0-127)
 * @param velocity Release velocity (0-127, ignored)
 *
 * Called automatically by usbMIDI when a Note Off message is received.
 */
void handleNoteOff(byte channel, byte note, byte velocity)
{
    (void)velocity;  // Unused parameter

    int8_t ch = noteToChannel(note);
    if (ch < 0)
    {
        return;  // Note not in our range
    }

    if (!solenoidDriver.isInitialized())
    {
        return;
    }

    // Turn off the solenoid
    SolenoidError err = solenoidDriver.off(ch);
    if (err != SolenoidError::OK)
    {
        Serial.print(F("[MIDI] Note "));
        Serial.print(note);
        Serial.print(F(" OFF failed: "));
        Serial.println(SolenoidDriver::getErrorString(err));
    }
}

// =============================================================================
// SOLENOID CONTROL FUNCTIONS
// =============================================================================

/**
 * @brief Immediately deactivate all solenoid channels
 *
 * Emergency stop function - uses SolenoidDriver's emergencyStop()
 * which bypasses safety checks for immediate shutoff.
 * Also resets duty cycle stats to provide a clean slate.
 */
void deactivateAllChannels()
{
    if (solenoidDriver.isInitialized())
    {
        solenoidDriver.emergencyStop();
        solenoidDriver.resetAllStats();
    }

    Serial.println(F("[OK] All channels deactivated"));
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * @brief Print a visual separator line
 */
void printSeparator()
{
    Serial.println(F("============================================================"));
}

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

/**
 * @brief Handle incoming serial commands
 */
void handleSerialInput()
{
    if (Serial.available() > 0)
    {
        char cmd = Serial.read();

        // Flush any remaining characters
        while (Serial.available())
        {
            Serial.read();
        }

        Serial.println();

        switch (cmd)
        {
        case 'x':
        case 'X':
            Serial.println(F("EMERGENCY STOP"));
            deactivateAllChannels();
            break;

        case 's':
        case 'S':
            printStatus();
            break;

        case 'h':
        case 'H':
        case '?':
            printHelp();
            break;

        case '\r':
        case '\n':
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

## Summary of Changes

| Category | Removed | Added | Modified |
|----------|---------|-------|----------|
| Constants | 2 (TEST_*) | 2 (MIDI_NOTE_*) | 2 (timing) |
| Functions | 9 (test/control) | 4 (MIDI/status) | 4 (setup/loop/help/serial) |
| Lines of code | ~300 | ~120 | ~50 |

The file goes from 733 lines to approximately 400 lines.
