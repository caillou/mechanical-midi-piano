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

/** I2C scan address range start (excludes reserved addresses 0x00-0x07) */
constexpr uint8_t I2C_SCAN_START_ADDR = 0x08;

/** I2C scan address range end (excludes reserved addresses 0x78-0x7F) */
constexpr uint8_t I2C_SCAN_END_ADDR = 0x78;

/** @} */

/**
 * @defgroup SolenoidConfig Solenoid Control Configuration
 * @{
 */

/** Number of solenoid channels on the driver board */
constexpr uint8_t NUM_CHANNELS = 8;

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

/**
 * Test activation duration in milliseconds
 * Short duration for safe testing
 */
constexpr uint32_t TEST_ACTIVATION_MS = 100;

/**
 * Delay between sequential channel tests in milliseconds
 */
constexpr uint32_t TEST_DELAY_MS = 200;

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

/** Flag indicating if MCP23017 was initialized successfully */
bool mcpInitialized = false;

/** Current channel states (bitmask) */
uint8_t channelStates = 0x00;



// =============================================================================
// FUNCTION PROTOTYPES
// =============================================================================

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


// =============================================================================
// SETUP
// =============================================================================

/**
 * @brief Arduino setup function - runs once at startup
 *
 * Initializes serial communication, I2C bus, and MCP23017.
 * Runs initial diagnostic tests.
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
        mcpInitialized = true;

        // Run initial tests
        Serial.println();
        Serial.println(F("Running initial tests..."));
        runAllTests();
    }
    else
    {
        Serial.println(F("[ERROR] Failed to initialize MCP23017!"));
        Serial.println(F("Check wiring and I2C address."));
        mcpInitialized = false;
    }

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
 * Handles serial commands and monitors solenoid safety timeouts.
 */
void loop()
{
    // Handle incoming serial commands
    handleSerialInput();

    // SolenoidDriver safety update - handles auto-shutoff for max on-time
    if (mcpInitialized)
    {
        solenoidDriver.update();
    }

    // Small delay to prevent tight loop
    delay(1);
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
    channelStates = 0x00;

    return true;
}

// =============================================================================
// I2C UTILITY FUNCTIONS
// =============================================================================

/**
 * @brief Scan the I2C bus and report all found devices
 *
 * Scans addresses 0x08-0x77 and prints each found device.
 */
void scanI2CBus()
{
    Serial.println();
    Serial.println(F("Scanning I2C bus..."));

    uint8_t deviceCount = 0;

    for (uint8_t address = I2C_SCAN_START_ADDR; address < I2C_SCAN_END_ADDR; address++)
    {
        Wire.beginTransmission(address);
        uint8_t error = Wire.endTransmission();

        if (error == 0)
        {
            Serial.print(F("  [FOUND] Device at address 0x"));
            if (address < 16)
                Serial.print(F("0"));
            Serial.print(address, HEX);

            // Identify known devices
            if (address >= 0x20 && address <= 0x27)
            {
                Serial.print(F(" (MCP23017 - Solenoid Driver)"));
            }

            Serial.println();
            deviceCount++;
        }
    }

    Serial.println();
    Serial.print(F("Scan complete. "));
    Serial.print(deviceCount);
    Serial.println(F(" device(s) found."));
}


// =============================================================================
// SOLENOID CONTROL FUNCTIONS
// =============================================================================

/**
 * @brief Toggle a solenoid channel on or off
 *
 * @param channel Channel number (0-7)
 *
 * Reads the current state and sets the channel to the opposite state.
 * Prints the toggle action to Serial for debugging.
 */
void toggleChannel(uint8_t channel)
{
    bool currentState = (channelStates >> channel) & 0x01;
    bool newState = !currentState;

    Serial.print(F("Toggling channel "));
    Serial.print(channel);
    Serial.print(F(" -> "));
    Serial.println(newState ? F("ON") : F("OFF"));

    setChannel(channel, newState);
}

/**
 * @brief Set the state of a single solenoid channel
 *
 * @param channel Channel number (0-7)
 * @param state true=ON, false=OFF
 * @return true if operation successful, false otherwise
 *
 * Uses SolenoidDriver with built-in safety checks for maximum on-time
 * and minimum off-time.
 */
bool setChannel(uint8_t channel, bool state)
{
    if (channel >= NUM_CHANNELS)
    {
        Serial.print(F("[ERROR] Invalid channel: "));
        Serial.println(channel);
        return false;
    }

    if (!mcpInitialized)
    {
        Serial.println(F("[ERROR] MCP23017 not initialized"));
        return false;
    }

    // Use SolenoidDriver for control with built-in safety
    SolenoidError err = solenoidDriver.set(channel, state);
    if (err != SolenoidError::OK)
    {
        Serial.print(F("[ERROR] setChannel failed: "));
        Serial.println(SolenoidDriver::getErrorString(err));
        return false;
    }

    // Update local state tracking
    if (state)
    {
        channelStates |= (1 << channel);
    }
    else
    {
        channelStates &= ~(1 << channel);
    }

    return true;
}

/**
 * @brief Set all channel states at once using a bitmask
 *
 * @param states Bitmask of channel states (bit 0 = channel 0, etc.)
 * @return true if operation successful, false otherwise
 *
 * Uses SolenoidDriver's setBoardChannels for efficient single I2C transaction.
 */
bool setAllChannels(uint8_t states)
{
    if (!mcpInitialized)
    {
        Serial.println(F("[ERROR] MCP23017 not initialized"));
        return false;
    }

    // Use SolenoidDriver to set all channels at once
    SolenoidError err = solenoidDriver.setBoardChannels(0, states);
    if (err != SolenoidError::OK)
    {
        Serial.print(F("[ERROR] setAllChannels failed: "));
        Serial.println(SolenoidDriver::getErrorString(err));
        return false;
    }

    // Update local state tracking
    channelStates = states;

    return true;
}

/**
 * @brief Activate a channel for a specified duration, then deactivate
 *
 * @param channel Channel number (0-7)
 * @param duration Activation duration in milliseconds
 * @return true if operation successful, false otherwise
 *
 * This is a blocking function - it waits for the duration to complete.
 */
bool activateChannel(uint8_t channel, uint32_t duration)
{
    // Enforce maximum on-time
    if (duration > MAX_ON_TIME_MS)
    {
        Serial.print(F("[WARNING] Duration clamped to max: "));
        Serial.println(MAX_ON_TIME_MS);
        duration = MAX_ON_TIME_MS;
    }

    if (!setChannel(channel, true))
    {
        return false;
    }

    delay(duration);

    setChannel(channel, false);

    return true;
}

/**
 * @brief Immediately deactivate all solenoid channels
 *
 * Emergency stop function - uses SolenoidDriver's emergencyStop()
 * which bypasses safety checks for immediate shutoff.
 * Also resets duty cycle stats to provide a clean slate.
 */
void deactivateAllChannels()
{
    if (mcpInitialized)
    {
        solenoidDriver.emergencyStop();
        solenoidDriver.resetAllStats();
    }

    // Reset local state tracking
    channelStates = 0x00;

    Serial.println(F("[OK] All channels deactivated"));
}

// =============================================================================
// TEST FUNCTIONS
// =============================================================================

/**
 * @brief Run all diagnostic tests
 */
void runAllTests()
{
    if (!mcpInitialized)
    {
        Serial.println(F("[ERROR] Cannot run tests - MCP23017 not initialized"));
        return;
    }

    printSeparator();
    Serial.println(F("RUNNING ALL TESTS"));
    printSeparator();

    // Test 1: Communication verification
    Serial.println(F("\n--- Test 1: Communication Verification ---"));
    testCommunication();

    // Wait for cooldown period before next test
    delay(MIN_OFF_TIME_MS + 10);  // minOffTimeMs + margin to ensure cooldown expires

    // Test 2: Sequential channel test
    Serial.println(F("\n--- Test 2: Sequential Channel Test ---"));
    testSequentialChannels();

    // Test 3: Simultaneous channel test
    Serial.println(F("\n--- Test 3: All Channels Simultaneous ---"));
    testAllChannelsSimultaneous();

    printSeparator();
    Serial.println(F("ALL TESTS COMPLETE"));
    printSeparator();
}

/**
 * @brief Test communication by pulsing channel 0
 *
 * Uses SolenoidDriver's pulse function for a simple communication test.
 */
void testCommunication()
{
    Serial.println(F("Testing communication with pulse test..."));

    // Use pulse to test communication - 50ms pulse on channel 0
    SolenoidError err = solenoidDriver.pulse(0, 50);
    if (err == SolenoidError::OK)
    {
        Serial.println(F("  [OK] Communication verified (pulse test passed)"));
    }
    else
    {
        Serial.print(F("  [ERROR] Communication test failed: "));
        Serial.println(SolenoidDriver::getErrorString(err));
    }

    // Ensure all channels are off after test
    solenoidDriver.allOff();
    channelStates = 0x00;
}

/**
 * @brief Test each channel sequentially
 *
 * Activates each channel one at a time for TEST_ACTIVATION_MS,
 * then waits TEST_DELAY_MS before the next channel.
 */
void testSequentialChannels()
{
    Serial.println(F("Testing channels sequentially..."));
    Serial.print(F("  Activation time: "));
    Serial.print(TEST_ACTIVATION_MS);
    Serial.print(F("ms, Delay: "));
    Serial.print(TEST_DELAY_MS);
    Serial.println(F("ms"));
    Serial.println();

    for (uint8_t channel = 0; channel < NUM_CHANNELS; channel++)
    {
        Serial.print(F("  Channel "));
        Serial.print(channel);
        Serial.print(F(": ON..."));

        digitalWrite(LED_PIN, HIGH);

        if (activateChannel(channel, TEST_ACTIVATION_MS))
        {
            Serial.println(F(" OFF [OK]"));
        }
        else
        {
            Serial.println(F(" [FAILED]"));
        }

        digitalWrite(LED_PIN, LOW);

        // Wait between channels (accounts for min off time)
        delay(TEST_DELAY_MS);
    }

    Serial.println(F("  Sequential test complete."));
}

/**
 * @brief Test all channels activated simultaneously
 *
 * Activates all 8 channels at once for TEST_ACTIVATION_MS.
 */
void testAllChannelsSimultaneous()
{
    Serial.println(F("Activating all channels simultaneously..."));
    Serial.print(F("  Duration: "));
    Serial.print(TEST_ACTIVATION_MS);
    Serial.println(F("ms"));

    digitalWrite(LED_PIN, HIGH);

    // All channels on
    setAllChannels(0xFF);
    Serial.println(F("  All channels ON"));

    delay(TEST_ACTIVATION_MS);

    // All channels off
    setAllChannels(0x00);
    Serial.println(F("  All channels OFF"));

    digitalWrite(LED_PIN, LOW);

    Serial.println(F("  Simultaneous test complete."));
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
    Serial.println(F("  'r' - Re-run all tests"));
    Serial.println(F("  'a' - Activate all channels for 100ms"));
    Serial.println(F("  's' - Run I2C scanner"));
    Serial.println(F("  '0'-'7' - Toggle individual channel"));
    Serial.println(F("  'x' - Emergency stop (all off)"));
    Serial.println(F("  'h' - Show this help menu"));
    Serial.println();
    Serial.println(F("Waiting for commands..."));
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
