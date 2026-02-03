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

/** Number of MCP23017 boards */
constexpr uint8_t MCP23017_BOARD_COUNT = 5;

/** I2C addresses for MCP23017 boards
 *  Board 0: 0x20 (A2=0, A1=0, A0=0) - Channels 0-7   (MIDI 24-31)
 *  Board 1: 0x21 (A2=0, A1=0, A0=1) - Channels 8-15  (MIDI 32-35, 12-15 unused)
 *  Board 2: 0x22 (A2=0, A1=1, A0=0) - Channels 16-23 (MIDI 48-55)
 *  Board 3: 0x23 (A2=0, A1=1, A0=1) - Channels 24-31 (MIDI 56-63)
 *  Board 4: 0x24 (A2=1, A1=0, A0=0) - Channels 32-39 (MIDI 64-71)
 */
constexpr uint8_t MCP23017_ADDRESSES[MCP23017_BOARD_COUNT] = {0x20, 0x21, 0x22, 0x23, 0x24};

/** @} */

/**
 * @defgroup SolenoidConfig Solenoid Control Configuration
 * @{
 */

/** Total number of solenoid channels across all boards */
constexpr uint8_t NUM_CHANNELS = 40;

/** @note Timing calculations use unsigned 32-bit subtraction which is safe
 *  across millis() overflow (wraps every ~49.7 days) due to C/C++ unsigned
 *  arithmetic guarantees. */

/**
 * Maximum solenoid on-time in milliseconds
 * Prevents coil overheating - 5 seconds is plenty for piano notes
 */
constexpr uint32_t MAX_ON_TIME_MS = 5000;

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
int8_t channelToNote(uint8_t channel);

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

// =============================================================================
// MIDI HANDLER FUNCTIONS
// =============================================================================

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
    (void)channel;  // Responds to all MIDI channels

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
    (void)channel;   // Responds to all MIDI channels
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
    Serial.println(F("MIDI: Listening for notes 24-35 (C1-B1) and 48-71 (C3-B4)"));
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
