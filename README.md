# RTOS Product Storage Simulator

A FreeRTOS-based C++ simulation of a robotic warehouse system with product storage, retrieval, and expiration management using task scheduling and digital I/O control.

## Features

* Real-time multitasking using FreeRTOS
* Simulated 3-axis movement (X, Y, Z)
* 3x3 product storage grid
* Product expiration tracking
* Emergency stop and system reset
* User interaction via keyboard and hardware switches
* LED indicators for system states

## Requirements

* Windows OS
* Visual Studio (2017 or newer recommended)
* FreeRTOS library
* Hardware simulator or mock for digital I/O functions:

  * `readDigitalU8()`
  * `writeDigitalU8()`
  * `createDigitalInput()` / `createDigitalOutput()`

## Building

1. Open `labwork1.sln` in Visual Studio.
2. Restore or configure any missing include directories (especially for FreeRTOS and interface functions).
3. Build and run using **x86 Debug** configuration.

## Controls

| Input Key | Action                           |
| --------- | -------------------------------- |
| 1         | Store a product (enter ID + exp) |
| 2         | Retrieve a product by ID         |
| 3         | List all stored products         |
| 4         | Search for a product by ID       |
| 5         | List expired products            |
| w/a/s/d   | Calibrate Z/X movement           |
| t         | (Hidden) Emergency shutdown      |

## File Structure

```
labwork1/
├── labwork1.cpp       # Main program and tasks
├── product.c/.h       # Unused (abstraction placeholder)
├── resource.h         # Resource header (default)
├── labwork1.rc        # Resource script (default)
├── *.vcxproj, .sln    # Visual Studio project files
├── T1 Enunciado.pdf   # Assignment brief (Portuguese)
├── Task Diagram.pdf   # System diagram
```

## License

This project was created for academic purposes. License it as needed (e.g., MIT License).

---

**Repo:** [https://github.com/antoniopalves/RTOS-Product-Storage-Simulator](https://github.com/antoniopalves/RTOS-Product-Storage-Simulator)
