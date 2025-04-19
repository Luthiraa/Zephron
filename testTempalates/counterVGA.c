#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   240
#define BUFFER_WIDTH    512   // must match your hardware settings

// VGA hardware: pixel controller is at this address.
volatile int *pixel_ctrl_ptr = (int *)0xFF203020;

// Declare two frame buffers (front and back)
short int Buffer1[SCREEN_HEIGHT][BUFFER_WIDTH];
short int Buffer2[SCREEN_HEIGHT][BUFFER_WIDTH];

volatile int pixel_buffer_start; // Current pixel buffer pointer

// Delay busy-loop constant: adjust this value so that one iteration of delay_ms(1)
// approximates 1 millisecond on your system.
#define DELAY_COUNT 1

// Function prototypes
void delay_ms(int ms);
void clear_screen();
void vSync();
void plot_pixel(int x, int y, short int color);
void draw_char(int x, int y, char c, short color);
void draw_string(int x, int y, const char *str, short color);

// A simple 5x7 font for digits 0–9.
static const unsigned char font_digits[10][7] = {
    {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, // '0'
    {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}, // '1'
    {0x0E, 0x11, 0x01, 0x06, 0x08, 0x10, 0x1F}, // '2'
    {0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E}, // '3'
    {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}, // '4'
    {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E}, // '5'
    {0x0E, 0x10, 0x1E, 0x11, 0x11, 0x11, 0x0E}, // '6'
    {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}, // '7'
    {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}, // '8'
    {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x11, 0x0E}  // '9'
};

int main(void)
{
    // Set up double buffering.
    *(pixel_ctrl_ptr + 1) = (int)&Buffer1;  // set front buffer to Buffer1
    vSync();
    pixel_buffer_start = *pixel_ctrl_ptr;     // now front buffer is Buffer1
    clear_screen();
    
    *(pixel_ctrl_ptr + 1) = (int)&Buffer2;     // set back buffer to Buffer2
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    clear_screen();

    // We'll display the counter centered on the screen.
    // For a 3-digit number (max width ~3 characters * 6 pixels per char = 18 pixels wide)
    int textX = (SCREEN_WIDTH - 18) / 2;
    int textY = (SCREEN_HEIGHT - 7) / 2;  // 7-pixel high font

    char textBuffer[16];

    // Count from 0 to 180 with a 10 ms delay per increment.
    for (int count = 0; count <= 180; count++)
    {
        // Convert count to string.
        sprintf(textBuffer, "%d", count);
        
        // Clear the back buffer.
        clear_screen();
        
        // Draw the number (in white, 0xFFFF) at the center.
        draw_string(textX, textY, textBuffer, 0xFFFF);
        
        // Swap buffers.
        vSync();
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);
        
        // Delay approximately 10 ms.
        delay_ms(10);
    }
    
    // Optionally, you can loop indefinitely here.
    while (1) {}

    return 0;
}

/*
 * delay_ms()
 *
 * Busy-waits for approximately ms milliseconds.
 * (You may need to adjust DELAY_COUNT for your system's speed.)
 */
void delay_ms(int ms)
{
    volatile int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < DELAY_COUNT; j++)
            ; // empty loop for delay
}

/*
 * vSync()
 *
 * Initiates a buffer swap by writing to the pixel controller and waits until
 * the swap is complete (bit 0 of the status register is cleared).
 */
void vSync()
{
    *pixel_ctrl_ptr = 1; // request swap
    while ((*(pixel_ctrl_ptr + 3) & 0x01) != 0)
        ; // wait until swap complete
}

/*
 * clear_screen()
 *
 * Clears the current (back) buffer by writing black (0) to every pixel.
 */
void clear_screen()
{
    int x, y;
    volatile short int *pixel_ptr = (volatile short int *)pixel_buffer_start;
    for (y = 0; y < SCREEN_HEIGHT; y++)
        for (x = 0; x < SCREEN_WIDTH; x++)
            pixel_ptr[y * BUFFER_WIDTH + x] = 0;
}

/*
 * plot_pixel()
 *
 * Sets the pixel at coordinate (x, y) in the current pixel buffer to the specified color.
 * Uses a double-buffer addressing style with a BUFFER_WIDTH of 512.
 */
void plot_pixel(int x, int y, short int color)
{
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
        return;
    volatile short int *pixel_addr = (volatile short int *)(pixel_buffer_start + (y << 10) + (x << 1));
    *pixel_addr = color;
}

/*
 * draw_char()
 *
 * Draws a single digit (0–9) at (x, y) using a 5×7 font.
 */
void draw_char(int x, int y, char c, short color)
{
    if (c < '0' || c > '9')
        return;  // Only digits supported.
    int digit = c - '0';
    for (int row = 0; row < 7; row++)
    {
        unsigned char row_data = font_digits[digit][row];
        for (int col = 0; col < 5; col++)
        {
            if (row_data & (1 << (4 - col)))
                plot_pixel(x + col, y + row, color);
        }
    }
}

/*
 * draw_string()
 *
 * Draws a null-terminated string starting at (x, y). Characters are spaced 6 pixels apart.
 */
void draw_string(int x, int y, const char *str, short color)
{
    while (*str)
    {
        draw_char(x, y, *str, color);
        x += 6;  // Advance 5 pixels plus 1 pixel space.
        str++;
    }
}
