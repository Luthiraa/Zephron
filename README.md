# Robot Arm Controller with VGA Dashboard and PID Control

This repository contains C code for an NIOS V based robot arm controller featuring:

- **AXI GPIO Integration**: All I/O (LEDs, JP1 pins, PS/2 data, VGA controller, sensor & output buses) accessed via AXI reads/writes
- - **PID Controller**: Implements a PID loop reading a process variable over AXI and driving an output bus
- **PS/2 Keyboard Input**: Five servos (four segments + claw) controlled via PS/2 scancodes
- **VGA Dashboard**: Double-buffered 320×240 graphics display showing timer values and a rendered arm

## Features
- Real-time keyboard-based angle adjustment with auto‑acceleration when holding keys.  
- On-screen display of each servo’s angle (0–180°).  
- Graphical rendering of a four‑segment arm plus an open/close claw
- PID loop to maintain a target sensor value, with parameters configurable in code  
- LED and JP1 pin output reflect key presses
- Double‑buffered VGA for tear‑free animation 

## Hardware Mapping

| Peripheral            | Base Address    | Notes                                  |
|-----------------------|-----------------|----------------------------------------|
| PS/2 Keyboard         | `0xFF200100`    | Scancode register and status bit       |
| Onboard LEDs          | `0xFF200000`    | 10‑bit LED bus                         |
| JP1 Pins              | `0xFF200060`    | 6‑bit output bus                       |
| VGA Pixel Controller  | `0xFF203020`    | Buffer swap control & status           |
| PID Sensor Input      | `0xFF200080`    | 8‑bit process variable                 |
| PID Output            | `0xFF200000`    | Shares LED bus for demonstration       |



## Building
Create a directory with all your code (cpp files and header files) and copy over the MAKEFILE and gmake.bat file
*Note the MAKFILE is already configured to work for the math.h library*
Open a powershell prompt in the directory that the project/file is located and run the following commands sequentially

**Compile & Debug**:  
   ```bash
   ./gmake.bat DE1-SOC
   ./gmake.bat GDB_SERVER
   ./gmake.bat GDB_CLIENT
   # In the GDB prompt:
   continue
   # or simply:
   c
   # to get a cli interface for terminal output: 
   ./gmake.bat TERMINAL

