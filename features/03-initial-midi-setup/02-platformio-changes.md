# platformio.ini Changes

## File Location

**Full Path:** `/Users/caillou/repos/caillou/mechanical-midi-piano/platformio.ini`

## Overview

This is a single-line change that enables USB MIDI functionality on the Teensy 4.1.

## Current Content

```ini
; PlatformIO Configuration for Mechanical MIDI Piano
; Target: Teensy 4.1 with Adafruit I2C Solenoid Driver

[env:teensy41]
platform = teensy
board = teensy41
framework = arduino

; Serial monitor baud rate
monitor_speed = 115200

; Build flags
build_flags =
    -D ARDUINO_TEENSY41
    -D USB_SERIAL
    -Wno-unused-variable

; Library dependencies
lib_deps =
    adafruit/Adafruit MCP23017 Arduino Library@^2.3.0
    adafruit/Adafruit BusIO@^1.14.0

; Upload settings
upload_protocol = teensy-cli

; Debug build (optional - uncomment for debugging)
; build_type = debug
```

## Change Required

**Line 15:** Change `-D USB_SERIAL` to `-D USB_MIDI_SERIAL`

### Before (Line 15)
```ini
    -D USB_SERIAL
```

### After (Line 15)
```ini
    -D USB_MIDI_SERIAL
```

## New Content (Complete File)

```ini
; PlatformIO Configuration for Mechanical MIDI Piano
; Target: Teensy 4.1 with Adafruit I2C Solenoid Driver

[env:teensy41]
platform = teensy
board = teensy41
framework = arduino

; Serial monitor baud rate
monitor_speed = 115200

; Build flags
build_flags =
    -D ARDUINO_TEENSY41
    -D USB_MIDI_SERIAL
    -Wno-unused-variable

; Library dependencies
lib_deps =
    adafruit/Adafruit MCP23017 Arduino Library@^2.3.0
    adafruit/Adafruit BusIO@^1.14.0

; Upload settings
upload_protocol = teensy-cli

; Debug build (optional - uncomment for debugging)
; build_type = debug
```

## Technical Explanation

### USB Type Definitions

The Teensy build system uses preprocessor definitions to configure USB functionality. Available options include:

| Definition | USB Device Type | Description |
|------------|-----------------|-------------|
| `USB_SERIAL` | CDC Serial | Serial communication only |
| `USB_MIDI` | MIDI | MIDI only (no serial) |
| `USB_MIDI_SERIAL` | Composite MIDI + Serial | Both MIDI and Serial |
| `USB_MIDI4_SERIAL` | 4-port MIDI + Serial | Multiple MIDI ports |
| `USB_MIDI16_SERIAL` | 16-port MIDI + Serial | Many MIDI ports |

### Why USB_MIDI_SERIAL?

1. **MIDI Support:** Enables the `usbMIDI` object in code
2. **Serial Debugging:** Retains `Serial` object for debugging
3. **No Additional Libraries:** Teensy core includes all MIDI functionality
4. **Composite Device:** Computer sees both MIDI device AND serial port

### What Changes on the Computer

**Before (USB_SERIAL):**
- Appears as: USB Serial device (COM port / /dev/ttyACM)

**After (USB_MIDI_SERIAL):**
- Appears as: USB MIDI device ("Teensy MIDI") AND USB Serial device
- MIDI device visible in DAWs, MIDI monitoring software
- Serial port still available in Arduino Serial Monitor, PlatformIO

## No lib_deps Changes Required

The Teensy core library (included automatically by PlatformIO) provides:
- `usbMIDI` object
- All MIDI callback functions
- USB descriptors for MIDI device

The existing library dependencies are still required:
- `Adafruit MCP23017 Arduino Library` - for I2C GPIO expander
- `Adafruit BusIO` - I2C communication utilities

## Verification After Change

After uploading with this change:

### On macOS
1. Open "Audio MIDI Setup"
2. Window > Show MIDI Studio
3. Look for "Teensy MIDI" device

### On Windows
1. Open Device Manager
2. Look under "Sound, video and game controllers"
3. Should see "Teensy MIDI"

### On Linux
```bash
aconnect -l
# or
cat /proc/asound/cards
```

### Serial Still Works
1. Open PlatformIO Serial Monitor
2. Should see startup messages
3. Type 'h' for help menu
