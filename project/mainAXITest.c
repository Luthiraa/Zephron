#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

// VGA settings
#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   240
#define BUFFER_WIDTH    512   

#define DELAY_COUNT 1

volatile int *ps2_ptr            = (int *)0xFF200100;
volatile int *onboard_led_ptr    = (int *)0xFF200000;
volatile int *jp1_ptr            = (int *)0xFF200060;
volatile int *pixel_ctrl_ptr     = (int *)0xFF203020;

// Frame buffers for VGA
short int Buffer1[SCREEN_HEIGHT][BUFFER_WIDTH];
short int Buffer2[SCREEN_HEIGHT][BUFFER_WIDTH];
volatile int pixel_buffer_start;

bool key_pressed[10] = { false };

// AXI protocol and PID definitions
#define LED_BASE         0xFF200000
#define JP1_BASE         0xFF200060
#define PS2_BASE         0xFF200100
#define PIXEL_CTRL_BASE  0xFF203020
#define PID_SENSOR_BASE  0xFF200080
#define PID_OUTPUT_BASE  LED_BASE

static inline uint32_t axi_read(uint32_t base) { return (*(volatile uint32_t *)base); }
static inline void axi_write(uint32_t base, uint32_t val) { (*(volatile uint32_t *)(base + 4)) = val; }

typedef struct { float Kp, Ki, Kd, prev_error, integral, setpoint; } PID;
PID pid;

void pid_init(PID *p, float Kp, float Ki, float Kd, float sp) {
    p->Kp = Kp;
    p->Ki = Ki;
    p->Kd = Kd;
    p->setpoint = sp;
    p->prev_error = 0;
    p->integral = 0;
}

float pid_compute(PID *p, float measurement) {
    float error = p->setpoint - measurement;
    p->integral += error;
    float derivative = error - p->prev_error;
    p->prev_error = error;
    return p->Kp * error + p->Ki * p->integral + p->Kd * derivative;
}

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
    volatile short int *pixel_ptr = (volatile short int *)pixel_buffer_start;
    for (int y = 0; y < SCREEN_HEIGHT; y++)
        for (int x = 0; x < SCREEN_WIDTH; x++)
            pixel_ptr[y * BUFFER_WIDTH + x] = 0;
}

void plot_pixel(int x, int y, short int color)
{
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
        return;
    volatile short int *pixel_addr = 
        (volatile short int *)(pixel_buffer_start + (y << 10) + (x << 1));
    *pixel_addr = color;
}

void draw_char(int x, int y, char c, short color)
{
    if (c < '0' || c > '9')
        return;  
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
    int base_x = SCREEN_WIDTH / 2;
    int base_y = SCREEN_HEIGHT - 20;
    for (int y = base_y; y < SCREEN_HEIGHT; y++)
        for (int x = base_x - 30; x <= base_x + 30; x++)
            plot_pixel(x, y, 0x7BEF);
    draw_circle(base_x, base_y, 8, 0xF800);
    int seg_lengths[4] = {50,40,30,20};
    double angle = -90.0 + (timers[0] - 90);
    double x0 = base_x, y0 = base_y;
    for (int i = 0; i < 4; i++) {
        double rad = angle * M_PI / 180.0;
        double x1 = x0 + seg_lengths[i] * cos(rad);
        double y1 = y0 + seg_lengths[i] * sin(rad];
        draw_line_thick((int)x0, (int)y0, (int)x1, (int)y1, 0x07E0, 3);
        draw_circle((int)x1, (int)y1, 5, 0xFFE0);
        x0 = x1; y0 = y1;
        if (i < 3) angle += (timers[i+1] - 90);
    }
    double claw_angle = angle;
    double spread = (timers[4] / 180.0) * 40.0;
    double la = (claw_angle + spread) * M_PI / 180.0;
    double ra = (claw_angle - spread) * M_PI / 180.0;
    int c1x = (int)(x0 + 15 * cos(la));
    int c1y = (int)(y0 + 15 * sin(la));
    int c2x = (int)(x0 + 15 * cos(ra));
    int c2y = (int)(y0 + 15 * sin(ra));
    draw_line_thick((int)x0, (int)y0, c1x, c1y, 0x001F, 2);
    draw_line_thick((int)x0, (int)y0, c2x, c2y, 0x001F, 2);
    draw_circle(c1x, c1y, 3, 0xFFFF);
    draw_circle(c2x, c2y, 3, 0xFFFF);
}

int main(void) {
    pid_init(&pid, 1.0f, 0.01f, 0.1f, 90.0f);
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
    int textX = 10;
    int timer_y_positions[5] = {20,50,80,110,140};
    int timers[5] = {90,90,90,90,90};
    int holdCounterIncrease[5] = {0};
    int holdCounterDecrease[5] = {0};
    int normalIncrement = 1, acceleratedIncrement = 10;
    int normalDecrement = 1, acceleratedDecrement = 10;
    const int accelerationThreshold = 20;
    bool release_flag = false;
    int ps2_data;
    unsigned char code;
    int led_value;
    while (1) {
        ps2_data = axi_read(PS2_BASE);
        if (ps2_data & 0x8000) {
            code = ps2_data & 0xFF;
            if (code == 0xF0) release_flag = true;
            else { update_key_state(code, release_flag ? false : true); release_flag = false; }
        }
        led_value = 0;
        for (int i = 0; i < 10; i++) if (key_pressed[i]) led_value |= (1 << i);
        axi_write(LED_BASE, led_value);
        axi_write(JP1_BASE, led_value);
        for (int i = 0; i < 5; i++) {
            if (key_pressed[2*i]) { holdCounterIncrease[i]++; int inc = (holdCounterIncrease[i] > accelerationThreshold) ? acceleratedIncrement : normalIncrement; timers[i]+=inc; if (timers[i]>180) timers[i]=180; } else holdCounterIncrease[i]=0;
            if (key_pressed[2*i+1]) { holdCounterDecrease[i]++; int dec = (holdCounterDecrease[i] > accelerationThreshold) ? acceleratedDecrement : normalDecrement; timers[i]-=dec; if (timers[i]<0) timers[i]=0; } else holdCounterDecrease[i]=0;
        }
        float measurement = (float)(axi_read(PID_SENSOR_BASE) & 0xFF);
        float control = pid_compute(&pid, measurement);
        axi_write(PID_OUTPUT_BASE, (uint32_t)control);
        clear_screen();
        char textBuffer[16];
        for (int i = 0; i < 5; i++) { sprintf(textBuffer, "T%d: %d", i+1, timers[i]); draw_string(textX, timer_y_positions[i], textBuffer, 0xFFFF); }
        draw_robot_arm(timers);
        vSync();
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);
        delay_ms(10);
    }
    return 0;
}
