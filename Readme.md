# Mechanical Midi Piano

A Teensy 4.1-based controller for a mechanical MIDI piano using I2C solenoid drivers.

## Prerequisites

- **PlatformIO CLI** or **VS Code with PlatformIO extension**
- **Teensy 4.1** board
- **USB cable** (Micro-USB)

## Building the Code

```bash
pio run
```

This compiles the firmware for the Teensy 4.1 target.

## Uploading to Teensy

```bash
pio run --target upload
```

If the upload fails, press the **bootloader button** on the Teensy 4.1 to enter programming mode, then retry the upload command.

## Connecting for Debugging (Serial Monitor)

```bash
pio device monitor
```

Or with explicit baud rate:

```bash
pio device monitor -b 115200
```

**Baud rate:** 115200

**Alternative tools:** Any serial terminal works (e.g., `screen /dev/ttyACM0 115200`, `minicom`, Arduino Serial Monitor).

## Available Serial Commands

| Command | Description |
|---------|-------------|
| `0`-`7` | Toggle individual solenoid channel |
| `a` | Activate all channels for 100ms |
| `x` | Emergency stop (all channels off) |
| `s` | Run I2C bus scanner |
| `r` | Re-run all diagnostic tests |
| `h` | Show help menu |
