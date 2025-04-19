#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

// VGA settings
#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   240
#define BUFFER_WIDTH    512   

#define DELAY_COUNT 1


volatile int *ps2_ptr = (int *)0xFF200100;
volatile int *onboard_led_ptr = (int *)0xFF200000;
volatile int *jp1_ptr = (int *)0xFF200060;
volatile int *pixel_ctrl_ptr = (int *)0xFF203020;

// Frame buffers for VGA
short int Buffer1[SCREEN_HEIGHT][BUFFER_WIDTH];
short int Buffer2[SCREEN_HEIGHT][BUFFER_WIDTH];
volatile int pixel_buffer_start; 

bool key_pressed[10] = { false, false, false, false, false, false, false, false, false, false };

// Function prototypes
void update_key_state(unsigned char code, bool pressed);
void delay_ms(int ms);
void vSync();
void clear_screen();
void plot_pixel(int x, int y, short int color);
void draw_char(int x, int y, char c, short color);
void draw_string(int x, int y, const char *str, short color);
void draw_circle(int cx, int cy, int radius, short color);
void draw_line_thick(int x0, int y0, int x1, int y1, short color, int thickness);
void draw_robot_arm(int timers[5]);

void update_key_state(unsigned char code, bool pressed)
{
    switch(code)
    {
        case 0x1C: // 'A'
            key_pressed[0] = pressed;
            if (pressed)
                printf("A pressed, LED bit 0 ON\n");
            else
                printf("A released, LED bit 0 OFF\n");
            break;
        case 0x1A: // 'Z'
            key_pressed[1] = pressed;
            if (pressed)
                printf("Z pressed, LED bit 1 ON\n");
            else
                printf("Z released, LED bit 1 OFF\n");
            break;
        case 0x1B: // 'S'
            key_pressed[2] = pressed;
            if (pressed)
                printf("S pressed, LED bit 2 ON\n");
            else
                printf("S released, LED bit 2 OFF\n");
            break;
        case 0x22: // 'X'
            key_pressed[3] = pressed;
            if (pressed)
                printf("X pressed, LED bit 3 ON\n");
            else
                printf("X released, LED bit 3 OFF\n");
            break;
        case 0x23: // 'D'
            key_pressed[4] = pressed;
            if (pressed)
                printf("D pressed, LED bit 4 ON\n");
            else
                printf("D released, LED bit 4 OFF\n");
            break;
        case 0x21: // 'C'
            key_pressed[5] = pressed;
            if (pressed)
                printf("C pressed, LED bit 5 ON\n");
            else
                printf("C released, LED bit 5 OFF\n");
            break;
        case 0x2B: // 'F'
            key_pressed[6] = pressed;
            if (pressed)
                printf("F pressed, LED bit 6 ON\n");
            else
                printf("F released, LED bit 6 OFF\n");
            break;
        case 0x2A: // 'V'
            key_pressed[7] = pressed;
            if (pressed)
                printf("V pressed, LED bit 7 ON\n");
            else
                printf("V released, LED bit 7 OFF\n");
            break;
        case 0x34: // 'G'
            key_pressed[8] = pressed;
            if (pressed)
                printf("G pressed, LED bit 8 ON\n");
            else
                printf("G released, LED bit 8 OFF\n");
            break;
        case 0x32: // 'B'
            key_pressed[9] = pressed;
            if (pressed)
                printf("B pressed, LED bit 9 ON\n");
            else
                printf("B released, LED bit 9 OFF\n");
            break;
        default:
            break;
    }
}


void delay_ms(int ms)
{
    volatile int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < DELAY_COUNT; j++)
            ; 
}


void vSync()
{
    *pixel_ctrl_ptr = 1; 
    while ((*(pixel_ctrl_ptr + 3) & 0x01) != 0)
        ; 
}

void clear_screen()
{
    int x, y;
    volatile short int *pixel_ptr = (volatile short int *)pixel_buffer_start;
    for (y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (x = 0; x < SCREEN_WIDTH; x++)
        {
            pixel_ptr[y * BUFFER_WIDTH + x] = 0;
        }
    }
}


void plot_pixel(int x, int y, short int color)
{
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
        return;
    volatile short int *pixel_addr = 
        (volatile short int *)(pixel_buffer_start + (y << 10) + (x << 1));
    *pixel_addr = color;
}


static const unsigned char font_digits[10][7] = {
    {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, 
    {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}, 
    {0x0E, 0x11, 0x01, 0x06, 0x08, 0x10, 0x1F}, 
    {0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E}, 
    {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}, 
    {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E}, 
    {0x0E, 0x10, 0x1E, 0x11, 0x11, 0x11, 0x0E}, 
    {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}, 
    {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}, 
    {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x11, 0x0E}  
};


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


void draw_string(int x, int y, const char *str, short color)
{
    while (*str)
    {
        draw_char(x, y, *str, color);
        x += 6;
        str++;
    }
}


void draw_circle(int cx, int cy, int radius, short color)
{
    for (int dy = -radius; dy <= radius; dy++)
    {
        for (int dx = -radius; dx <= radius; dx++)
        {
            if (dx*dx + dy*dy <= radius*radius)
            {
                plot_pixel(cx + dx, cy + dy, color);
            }
        }
    }
}


void draw_line_thick(int x0, int y0, int x1, int y1, short color, int thickness)
{
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    int cx = x0, cy = y0;

    while (true)
    {
        draw_circle(cx, cy, thickness, color);
        if (cx == x1 && cy == y1)
            break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; cx += sx; }
        if (e2 < dx)  { err += dx; cy += sy; }
    }
}


void draw_robot_arm(int timers[5])
{
    // Base position near the bottom center of the screen
    int base_x = SCREEN_WIDTH / 2;
    int base_y = SCREEN_HEIGHT - 20;

    // Draw base (a rectangle for clarity)
    for (int y = base_y; y < SCREEN_HEIGHT; y++)
    {
        for (int x = base_x - 30; x <= base_x + 30; x++)
        {
            plot_pixel(x, y, 0x7BEF); // grayish color
        }
    }
    // Draw a red pivot circle at the base
    draw_circle(base_x, base_y, 8, 0xF800);

    // Segment lengths for the arm
    int seg_lengths[4] = {50, 40, 30, 20};

    // Start orientation: if timer[0] == 90, arm points straight up
    double cumulative_angle = -90.0 + (timers[0] - 90);
    double x0 = base_x;
    double y0 = base_y, x1, y1;
    short segment_color = 0x07E0;  // greenish segment
    short joint_color   = 0xFFE0;  // yellow joints

    for (int i = 0; i < 4; i++)
    {
        double rad = cumulative_angle * M_PI / 180.0;
        x1 = x0 + seg_lengths[i] * cos(rad);
        y1 = y0 + seg_lengths[i] * sin(rad);
        draw_line_thick((int)x0, (int)y0, (int)x1, (int)y1, segment_color, 3);
        draw_circle((int)x1, (int)y1, 5, joint_color);
        x0 = x1;
        y0 = y1;
        if (i < 3)
            cumulative_angle += (timers[i + 1] - 90);
    }


    double claw_angle_deg = cumulative_angle;
    double max_spread = 40.0;
    double spread = (timers[4] / 180.0) * max_spread;
    double left_angle  = claw_angle_deg + spread;
    double right_angle = claw_angle_deg - spread;
    int claw_length = 15;
    int claw_x1 = (int)(x0 + claw_length * cos(left_angle * M_PI / 180.0));
    int claw_y1 = (int)(y0 + claw_length * sin(left_angle * M_PI / 180.0));
    int claw_x2 = (int)(x0 + claw_length * cos(right_angle * M_PI / 180.0));
    int claw_y2 = (int)(y0 + claw_length * sin(right_angle * M_PI / 180.0));
    short claw_color = 0x001F; // blue for claw fingers
    draw_line_thick((int)x0, (int)y0, claw_x1, claw_y1, claw_color, 2);
    draw_line_thick((int)x0, (int)y0, claw_x2, claw_y2, claw_color, 2);
    draw_circle(claw_x1, claw_y1, 3, 0xFFFF);
    draw_circle(claw_x2, claw_y2, 3, 0xFFFF);
}

int main(void)
{

    *onboard_led_ptr = 0;
    *(jp1_ptr + 1) = 0x3F;
    *jp1_ptr = 0;

    *(pixel_ctrl_ptr + 1) = (int)&Buffer1;  
    vSync();
    pixel_buffer_start = *pixel_ctrl_ptr; 
    clear_screen();

    *(pixel_ctrl_ptr + 1) = (int)&Buffer2;     
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    clear_screen();

    // Timer text settings
    int textX = 10;
    int timer_y_positions[5] = {20, 50, 80, 110, 140};


    int timers[5] = {90, 90, 90, 90, 90};


    int holdCounterIncrease[5] = {0, 0, 0, 0, 0};
    int holdCounterDecrease[5] = {0, 0, 0, 0, 0};


    int normalIncrement      = 1;
    int acceleratedIncrement = 10;
    int normalDecrement      = 1;
    int acceleratedDecrement = 10;
    const int accelerationThreshold = 20;

    bool release_flag = false;
    int ps2_data;
    unsigned char code;
    int led_value = 0;

    while (1)
    {
        ps2_data = *ps2_ptr;
        if (ps2_data & 0x8000)  
        {
            code = ps2_data & 0xFF;
            if (code == 0xF0)
            {
                release_flag = true;
            }
            else
            {
                if (release_flag)
                {
                    update_key_state(code, false);
                    release_flag = false;
                }
                else
                {
                    update_key_state(code, true);   
                }
            }
        }

        // --- Update LED output based on key states 
        led_value = 0;
        for (int i = 0; i < 10; i++)
        {
            if (key_pressed[i])
                led_value |= (1 << i);
        }
        *onboard_led_ptr = led_value;
        *jp1_ptr = led_value;

        // --- Update timers for the robot arm using key states ---
        //  For each timer i, key index 2*i is increase, 2*i+1 is decrease
        for (int i = 0; i < 5; i++)
        {
            if (key_pressed[2*i])
            {
                holdCounterIncrease[i]++;
                int inc = (holdCounterIncrease[i] > accelerationThreshold) 
                            ? acceleratedIncrement 
                            : normalIncrement;
                timers[i] += inc;
                if (timers[i] > 180) timers[i] = 180;
            }
            else
            {
                holdCounterIncrease[i] = 0;
            }

            if (key_pressed[2*i + 1])
            {
                holdCounterDecrease[i]++;
                int dec = (holdCounterDecrease[i] > accelerationThreshold) 
                            ? acceleratedDecrement 
                            : normalDecrement;
                timers[i] -= dec;
                if (timers[i] < 0) timers[i] = 0;
            }
            else
            {
                holdCounterDecrease[i] = 0;
            }
        }

        clear_screen();
        for (int i = 0; i < 5; i++)
        {
            char textBuffer[16];
            sprintf(textBuffer, "T%d: %d", i+1, timers[i]);
            draw_string(textX, timer_y_positions[i], textBuffer, 0xFFFF);
        }
        draw_robot_arm(timers);

        vSync();
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);
        delay_ms(10);
    }
    
    return 0;
}


