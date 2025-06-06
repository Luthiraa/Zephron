# Zephron: Robot Arm Controller with VGA Dashboard and PS/2 Input
![zephron](zephron.jpg)

## Overview

This repository contains C code for an NIOS V-based robot arm controller featuring:

- **AXI GPIO Integration**: All I/O (LEDs, JP1 pins, PS/2 data, VGA controller, sensor & output buses) accessed via AXI reads/writes  
- **PS/2 Keyboard Input**: Five servos (four segments + claw) controlled via PS/2 scancodes  
- **VGA Dashboard**: Double-buffered 320×240 graphics display showing timer values and a rendered arm  

## Electrical Specifications

- **5× Servo Motors** (standard hobby servos; draw up to 12 V under load)  
- **3D‑Printed Mechanical Robot Arm Structure**  
- **Arduino Uno** (I²C bridge between DE1‑SoC and PWM driver)  
- **PCA9685 16‑Channel 12‑Bit PWM Servo Motor Driver (I²C)**  
- **Breadboard** (for prototyping power and signal distribution)  
- **DE1‑SoC Development Board**  
- **Jumper Wires** (male‑to‑male and male‑to‑female)  
- **12 V Power Supply Adapter** (must be rated for the combined stall‑current of all servos)  

> **Note:** Our servos can draw up to 12 V at stall. Be sure to use a power supply rated for that voltage **and** capable of delivering sufficient current. Running at the upper limit increases available torque but can also lead to stalling under heavy loads, which degrades positional accuracy.

## Embedded Software Design

Memory-mapped I/O communication is implemented using AXI protocol semantics

| Peripheral            | Base Address    | Notes                                  |
|-----------------------|-----------------|----------------------------------------|
| PS/2 Keyboard         | `0xFF200100`    | Scancode register and status bit       |
| Onboard LEDs          | `0xFF200000`    | 10‑bit LED bus                         |
| JP1 Pins              | `0xFF200060`    | 6‑bit output bus                       |
| VGA Pixel Controller  | `0xFF203020`    | Buffer swap control & status           |
| Sensor Input          | `0xFF200080`    | 8‑bit value from robotic arm sensors   |

### AXI Protocol

Peripherals like LEDs, PS/2, and sensor interfaces are accessed via base addresses such as:

```c
#define LED_BASE         0xFF200000
#define JP1_BASE         0xFF200060
#define PS2_BASE         0xFF200100
#define SENSOR_BASE      0xFF200080
```
  - Read and write functions wrap AXI access for clarity:
    ```c
    static inline uint32_t axi_read(uint32_t base);
    static inline void axi_write(uint32_t base, uint32_t val);
    ```
  - These are used to read sensor values and write control output, e.g.:
    ```c
    float measurement = (float)(axi_read(PID_SENSOR_BASE) & 0xFF);
    axi_write(PID_OUTPUT_BASE, (uint32_t)control);
    ```

- **VGA Controller**
  - The VGA system displays real-time feedback and arm position using a double-buffered frame approach.
  - Framebuffers:
    ```c
    short int Buffer1[SCREEN_HEIGHT][BUFFER_WIDTH];
    short int Buffer2[SCREEN_HEIGHT][BUFFER_WIDTH];
    ```
  - Frame buffers are assigned and managed through:
    ```c
    *(pixel_ctrl_ptr + 1) = (int)&Buffer1;
    pixel_buffer_start = *pixel_ctrl_ptr;
    ```
  - VGA functions include:
    - `plot_pixel(int x, int y, short int color)`
    - `draw_char()`, `draw_string()`, `draw_robot_arm()`
    - These update the buffer for VGA to read from.
  - `vSync()` ensures buffer swap aligns with screen refresh:
    ```c
    void vSync() {
        *pixel_ctrl_ptr = 1;
        while ((*(pixel_ctrl_ptr + 3) & 0x01) != 0);
    }
    ```

- **Communication Protocol**
  - PS2 keyboard inputs are processed through memory-mapped PS2 interface at `PS2_BASE`.
  - The code reads keyboard scancodes and updates `key_pressed[]` accordingly:
    ```c
    int ps2_data = axi_read(PS2_BASE);
    if (ps2_data & 0x8000) {
        unsigned char code = ps2_data & 0xFF;
        update_key_state(code, release_flag ? false : true);
    }
    ```
  - Key states control robotic arm segments and LED output through direct register writes:
    ```c
    axi_write(LED_BASE, led_value);
    axi_write(JP1_BASE, led_value);
    ```
  - User feedback is provided through `printf()` statements on key events:
    ```c
    if (pressed)
        printf("A pressed, LED bit 0 ON\n");
    ```

- **PID Control**
  - A software PID controller computes output based on sensor input:
    ```c
    typedef struct {
        float Kp, Ki, Kd, prev_error, integral, setpoint;
    } PID;
    ```
  - It adjusts control based on:
    - Proportional error
    - Integrated error
    - Derivative of error
  - Output is written back via AXI to `PID_OUTPUT_BASE`.

- **Arduino as I2C Intermediary**
  - **Reason for Arduino**:  
    The DE1-SoC cannot directly communicate with the I2C-based PWM servo driver. So the Arduino acts as a **bridge**, receiving timer values via digital pins , converting them into I2C commands, and sending them to the PWM servo controller

  - **Architecture Insight**:  
    The overall system is a **hybrid embedded architecture**:
    - The **DE1-SoC** handles real-time user input, PID computation, and VGA output.
    - The **Arduino** handles low-level hardware actuation via I2C.


## Building
Create a directory with all your code (cpp files and header files) and copy over the MAKEFILE and gmake.bat file

**Note the MAKFILE is already configured to work for the math.h library**

Open a powershell prompt in the directory that the project/file is located and run the following commands sequentially

**Compile & Debug**:  
   ```bash
   # init de1-soc by downloading nios V onto fpga board
   .\gmake.bat DE1-SOC
   # start gdb server
   .\gmake.bat GDB_SERVER
   # compile program
   .\gmake.bat COMPILE
   # load program onto nios V processor
   .\gmake.bat GDB_CLIENT
   # In the GDB prompt:
   continue
   # or simply:
   c
   # to get a cli interface for terminal output: 
   .\gmake.bat TERMINAL

