# ForestPastures

ForestPastures is an open-source energy-autonomous livestock monitoring platform designed for remote and forest pasture environments. The project demonstrates how energy harvesting, low-power sensing and wireless communication can extend animal welfare monitoring beyond traditional farm buildings.

In the demonstrator, an energy-autonomous monitoring station is deployed at a feeding location within a forest pasture. Animal-borne collar devices collect activity data and automatically synchronize their records whenever animals visit the feeding area. The monitoring station then provides access to collected data for farm management, research and automation systems.

## Objectives

The project investigates how animal monitoring can be expanded from barns to more natural grazing environments while maintaining access to valuable behavioral data.

Collected information can be used to estimate:

- Animal activity levels
- Feeding frequency
- Presence at feeding stations
- Patterns of movement
- Long-term welfare indicators

By enabling data collection across wider grazing areas, the system supports flexible pasture management while encouraging more natural and diverse grazing conditions for livestock.

## Equipment

### TeleCollar

- ESP32 microcontroller
- ADXL313 accelerometer
- MicroSD card
- Battery power supply
- BLE scanning
- ESP-NOW wireless communication

### TeleBase

- ESP32 microcontroller
- MicroSD card
- BLE beacon transmitter
- ESP-NOW receiver
- WiFi connectivity
- Embedded FTP server

### Mechanical Components

- Animal collar enclosure
- Base station enclosure
- Charging accessories
- PCB mechanical models

CAD files are available in:

```text
/cad
```

## System Architecture

```text
                Animal Activity
                       │
                       ▼

           ┌─────────────────────┐
           │     TeleCollar      │
           │─────────────────────│
           │ ESP32               │
           │ ADXL313             │
           │ SD Card Storage     │
           │ Deep Sleep          │
           └──────────┬──────────┘
                      │
                      │ BLE proximity detection
                      │
                      ▼

           ┌─────────────────────┐
           │      TeleBase       │
           │─────────────────────│
           │ ESP32               │
           │ BLE Beacon          │
           │ ESP-NOW Receiver    │
           │ SD Card Storage     │
           │ FTP Server          │
           └──────────┬──────────┘
                      │
                      ▼

                Farm Network
                      │
                      ▼

             Research / Automation
```

## TeleCollar

The TeleCollar unit continuously monitors animal movement using an ADXL313 accelerometer.

### Features

- Motion-based activity monitoring
- Timestamped event logging
- Local SD card storage
- BLE-based base station discovery
- ESP-NOW data synchronization
- Automatic time synchronization
- Deep sleep power management

The collar remains in a low-power state whenever possible, significantly extending operational lifetime and making the system suitable for energy-constrained deployments.

## TeleBase

The TeleBase unit acts as a synchronization and data collection hub.

### Functions

- Advertises presence through BLE
- Detects nearby collars
- Receives activity logs through ESP-NOW
- Stores received records on SD card
- Synchronizes collar clocks
- Provides wireless access to collected data via FTP

Each collar is automatically identified by its MAC address and assigned a dedicated log file.

Example:

```text
/94B97E6612AC.txt
```

## Data Synchronization

1. Animal approaches the feeding station.
2. TeleCollar detects the TeleBase BLE beacon.
3. TeleCollar establishes an ESP-NOW connection.
4. Logged activity records are transferred.
5. TeleBase stores received records on its SD card.
6. TeleBase returns the current Unix timestamp.
7. TeleCollar synchronizes its internal clock.

## Remote Data Access

TeleBase includes an embedded FTP server that allows wireless access to collected datasets over a local WiFi network.

This enables:

- Downloading activity logs
- Archiving measurement data
- Integration with analysis tools
- Integration with farm automation systems

## Repository Structure

```text
ForestPastures
│
├── cad
│   ├── Enclosure models
│   ├── PCB references
│   ├── Charging accessories
│   └── STEP exports
│
├── TeleBase
│   ├── ESP-NOW receiver
│   ├── BLE beacon
│   ├── SD storage
│   └── FTP server
│
└── TeleCollar
    ├── Activity monitoring
    ├── ADXL313 interface
    ├── Power management
    ├── BLE scanning
    └── ESP-NOW transfer
```

## Research Motivation

Modern livestock production seeks to balance animal welfare, sustainable land use and operational efficiency. While automated monitoring solutions are common within barns, collecting equivalent information in remote grazing environments remains challenging.

ForestPastures explores how energy harvesting, local data storage and low-power wireless communication can provide a practical monitoring solution for forest pastures and other extensive grazing systems without requiring continuous network connectivity or cellular subscriptions.

## Project Status

Prototype and field-testing phase.

Current capabilities:

- Activity monitoring
- SD card logging
- Deep sleep operation
- BLE proximity detection
- ESP-NOW synchronization
- Automatic time synchronization
- FTP-based data access

-----
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="/img/logot/license.png">
  <source media="(prefers-color-scheme: light)" srcset="/img/logot/license_lightmode.png">
  <img alt="License logo" src="/img/logot/license.png">
</picture>
