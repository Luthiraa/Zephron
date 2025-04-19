#include <stdlib.h>
#include <stdio.h>

#define KEYS       ((volatile int *)0xFF200050)
#define ADDR_JP1   ((volatile int *)0xFF200060)

int main(void)
{
    *(ADDR_JP1 + 4) = 0x01;
    
    while (1) {
        if (((~(*KEYS)) & 0x1) != 0) {
            *ADDR_JP1 = *ADDR_JP1 | 0x01;
            
            while (((*ADDR_JP1) & 0x02) == 0) {
            }
            
            *ADDR_JP1 = *ADDR_JP1 & (~0x01);
            
            while (((*ADDR_JP1) & 0x02) != 0) {
            }
            
            while (((~(*KEYS)) & 0x1) != 0) {
            }
        }
    }
    
    return 0;
}