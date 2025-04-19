

#include <stdlib.h>
#include <stdio.h>

#define SWITCHES   ((volatile int *)0xFF200040)  // Switch inputs (SW)
#define ADDR_JP1   ((volatile int *)0xFF200060)  // JP1 port base address

int main(void)
{
    // Configure JP1 direction register:
    // Set bit0 (D0) to output. Other bits are don't-care.
    *(ADDR_JP1 + 1) = 0x1;  // Set D0 as output
    *ADDR_JP1 = 0;          // Initialize output to low

    while (1) {
        // Read the state of the switches.
        int sw = *SWITCHES;
        if (sw & 0x1) {
            // If SW is HIGH, drive JP1 D0 HIGH.
            *ADDR_JP1 = 0x1;
        } else {
            // Otherwise, drive JP1 D0 LOW.
            *ADDR_JP1 = 0x0;
        }
    }

    return 0;
}
