# Robot Arm Controller with VGA Dashboard and PID Control

This repository contains C code for an FPGA-based robot arm controller featuring:

- **PS/2 Keyboard Input**: Five servos (four segments + claw) controlled via PS/2 scancodes.  
- **VGA Dashboard**: Double-buffered 320×240 graphics display showing timer values and a rendered arm.  
- **PID Controller**: Implements a PID loop reading a process variable over AXI and driving an output bus.  
- **AXI GPIO Integration**: All I/O (LEDs, JP1 pins, PS/2 data, VGA controller, sensor & output buses) accessed via AXI reads/writes.  

## Features

- Real-time keyboard-based angle adjustment with auto‑acceleration when holding keys.  
- On-screen display of each servo’s angle (0–180°).  
- Graphical rendering of a four‑segment arm plus an open/close claw.  
- PID loop to maintain a target sensor value, with parameters configurable in code.  
- LED and JP1 pin output reflect key presses.  
- Double‑buffered VGA for tear‑free animation.  

## Hardware Mapping

| Peripheral            | Base Address    | Notes                                  |
|-----------------------|-----------------|----------------------------------------|
| PS/2 Keyboard         | `0xFF200100`    | Scancode register and status bit       |
| Onboard LEDs          | `0xFF200000`    | 10‑bit LED bus                         |
| JP1 Pins              | `0xFF200060`    | 6‑bit output bus                       |
| VGA Pixel Controller  | `0xFF203020`    | Buffer swap control & status           |
| PID Sensor Input      | `0xFF200080`    | 8‑bit process variable                 |
| PID Output            | `0xFF200000`    | Shares LED bus for demonstration       |

_☞ Adjust addresses in `#define` if your platform differs._

## Building

1. Import into your FPGA toolchain (Quartus, Vivado, etc.) and assign the peripheral base addresses.  
2. Compile with your soft‑CPU SDK (e.g. Nios II or MicroBlaze):  
   ```bash
   nios2-bsp-generate --app csrc robot_arm_controller
   make
