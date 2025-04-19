#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   240
#define BUFFER_WIDTH    512    // must match your hardware settings
#define NUM_GAUGES      5
#define MAX_VALUE       180    // gauge counter stops at 180
#define INCREMENT_DELAY 2      // number of frames to wait between increments

// Hardware addresses: pixel controller and switches.
volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
#define SWITCHES ((volatile long *)0xFF200040)

// Declare two frame buffers (front and back)
short int Buffer1[SCREEN_HEIGHT][BUFFER_WIDTH];
short int Buffer2[SCREEN_HEIGHT][BUFFER_WIDTH];

volatile int pixel_buffer_start; // Global pointer to the current pixel buffer

// Array to hold the continuous gauge counters (one per switch)
int gauge_values[NUM_GAUGES] = {0, 0, 0, 0, 0};
// Array to hold per-gauge delay counters
int delay_counters[NUM_GAUGES] = {0, 0, 0, 0, 0};

// Function prototypes
void vSync();
void clear_screen();
void plot_pixel(int x, int y, short int color);
void draw_gauge(int centerX, int centerY, int radius, int draw_value);
void draw_char(int x, int y, char c, short color);
void draw_string(int x, int y, const char *str, short color);

// A simple 5x7 font for digits 0-9.
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
    // Set front buffer to Buffer1, wait for vSync, then clear it.
    *(pixel_ctrl_ptr + 1) = (int)&Buffer1;
    vSync();
    pixel_buffer_start = *pixel_ctrl_ptr;  // Now front buffer is Buffer1
    clear_screen();

    // Set back buffer to Buffer2 and clear it.
    *(pixel_ctrl_ptr + 1) = (int)&Buffer2;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    clear_screen();

    // Gauge positions (evenly spaced across the screen)
    int centersX[NUM_GAUGES] = {40, 100, 160, 220, 280};
    int centerY = 120;  // common vertical position for all gauges
    int radius  = 30;   // gauge radius

    char textBuffer[12];

    while (1)
    {
        // Read the switch register; assume lower 5 bits control the gauges.
        unsigned int sw_value = (unsigned int)(*SWITCHES);
        for (int i = 0; i < NUM_GAUGES; i++)
        {
            if (sw_value & (1 << i))
            {
                // Increase delay counter if switch is on.
                delay_counters[i]++;
                if (delay_counters[i] >= INCREMENT_DELAY)
                {
                    // Increment gauge counter as long as it hasn't reached MAX_VALUE.
                    if (gauge_values[i] < MAX_VALUE)
                        gauge_values[i]++;
                    delay_counters[i] = 0;
                }
            }
            else
            {
                // If the switch is off, reset the delay counter.
                delay_counters[i] = 0;
            }
        }

        // Clear the back buffer.
        clear_screen();

        // For each gauge, draw gauge number on top, the gauge (pie fill), and the counter below.
        for (int i = 0; i < NUM_GAUGES; i++)
        {
            // Draw gauge number (centered above the gauge)
            sprintf(textBuffer, "%d", i + 1);
            int textWidth = 6; // approximate width per digit
            int textX_top = centersX[i] - textWidth;
            int textY_top = centerY - radius - 10; // above the gauge
            draw_string(textX_top, textY_top, textBuffer, 0xFFFF); // white

            // Draw the gauge (pie chart) using the current counter value.
            draw_gauge(centersX[i], centerY, radius, gauge_values[i]);

            // Draw the continuous counter value below the gauge.
            sprintf(textBuffer, "%d", gauge_values[i]);
            int textX_val = centersX[i] - textWidth;
            int textY_val = centerY + radius + 2; // below the gauge
            draw_string(textX_val, textY_val, textBuffer, 0xFFFF); // white
        }

        // Wait for vSync (swap buffers) and update pixel_buffer_start.
        vSync();
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    }

    return 0;
}

/*
 * vSync()
 *
 * Requests a buffer swap by writing to the pixel controller and waits until
 * the swap is complete.
 */
void vSync()
{
    *pixel_ctrl_ptr = 1;  // Initiate buffer swap
    while ((*(pixel_ctrl_ptr + 3) & 0x01) != 0)
        ;  // Wait until swap complete (bit 0 cleared)
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
 * Uses the double-buffer addressing style with BUFFER_WIDTH = 512.
 */
void plot_pixel(int x, int y, short int color)
{
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
        return;
    volatile short int *pixel_addr = (volatile short int *)(pixel_buffer_start + (y << 10) + (x << 1));
    *pixel_addr = color;
}

/*
 * draw_gauge()
 *
 * Draws a circular gauge (pie chart) at (centerX, centerY) with the given radius.
 * The background is drawn in light gray and a red pie slice fills the gauge from 180°
 * (pointing left) clockwise by an angle proportional to draw_value (saturating at MAX_VALUE).
 * A white outline is drawn around the circle.
 */
void draw_gauge(int centerX, int centerY, int radius, int draw_value)
{
    // Define colors (RGB565).
    short int gauge_bg      = 0x7BEF;  // Light gray background
    short int gauge_fill    = 0xF800;  // Red fill for active portion
    short int gauge_outline = 0xFFFF;  // White outline

    // Draw the full circle background.
    for (int dy = -radius; dy <= radius; dy++)
        for (int dx = -radius; dx <= radius; dx++)
            if (dx * dx + dy * dy <= radius * radius)
                plot_pixel(centerX + dx, centerY + dy, gauge_bg);

    // Calculate fill angles: from 180° to (180° + draw_value).
    double startAngle = 180.0;
    double endAngle   = 180.0 + (double)draw_value;

    // Draw the pie slice.
    for (int dy = -radius; dy <= radius; dy++)
    {
        for (int dx = -radius; dx <= radius; dx++)
        {
            if (dx * dx + dy * dy <= radius * radius)
            {
                double angle = atan2(-dy, -dx) * 180.0 / M_PI;
                if (angle < 0)
                    angle += 360.0;
                if (angle >= startAngle && angle <= endAngle)
                    plot_pixel(centerX + dx, centerY + dy, gauge_fill);
            }
        }
    }

    // Draw the circular outline.
    for (int dx = -radius; dx <= radius; dx++)
    {
        int dy = (int)(sqrt(radius * radius - dx * dx) + 0.5);
        plot_pixel(centerX + dx, centerY + dy, gauge_outline);
        plot_pixel(centerX + dx, centerY - dy, gauge_outline);
    }
}

/*
 * draw_char()
 *
 * Draws a single character (digits 0-9 only) at (x, y) using a 5x7 font.
 */
void draw_char(int x, int y, char c, short color)
{
    if (c < '0' || c > '9') return; // Only digits supported.
    int digit = c - '0';
    for (int row = 0; row < 7; row++)
    {
        unsigned char row_data = font_digits[digit][row];
        for (int col = 0; col < 5; col++)
            if (row_data & (1 << (4 - col)))
                plot_pixel(x + col, y + row, color);
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
        x += 6;
        str++;
    }
}
