# RTOS Product Storage Simulator

A FreeRTOS-based C++ simulation of a robotic warehouse system with product storage, retrieval, and expiration management using task scheduling and digital I/O control. Made for Real Time Systems course.

## Features

* Real-time simulation of a 3-axis robotic arm in a 3x3 storage grid
* Storage and retrieval of products with unique IDs and expiration timers
* Automatic removal of expired products
* Emergency stop, resume, and full reset via hardware switches
* Manual calibration of X and Z axes
* Menu-driven interface via keyboard input
* LED blinking indicators for expired products and emergency stop
* Cooperative task handling and synchronization using FreeRTOS
* Queue and semaphore-based task communication
* System reset and calibration verification before shutdown
* Support for digital I/O operations simulating real-world control logic

## Skills & Concepts Practiced

* **RTOS Programming:** Task creation, scheduling, suspension, and termination using FreeRTOS
* **Concurrency Management:** Use of semaphores and queues for thread-safe task interaction
* **Digital I/O Simulation:** Bitwise operations for controlling and reading virtual hardware ports
* **State Machines & Event Handling:** Switch-based state transitions for safety and workflow
* **Resource-Constrained Design:** Efficient handling of a 3x3 matrix with limited product space
* **Embedded Systems Concepts:** Movement abstraction (motor control, calibration, safety protocols)
* **User Interaction Design:** Menu system with key input parsing and debounce handling
* **Error Handling & Safety:** Emergency logic, input validation, and system reset flows

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

This project was created for academic purposes.

---

**Repo:** [https://github.com/antoniopalves/RTOS-Product-Storage-Simulator](https://github.com/antoniopalves/RTOS-Product-Storage-Simulator)
