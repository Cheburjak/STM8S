#include "config.h"
#include "naming.h"
#include "iostm8s105k4.h"

#define _countof(ARR) (sizeof(ARR) / sizeof(ARR[0]))

#define LED0 PRB(LED1_PORT, ODR, LED1_BIT)
#define LED1 PRB(LED2_PORT, ODR, LED2_BIT)
#define LED2 PRB(LED3_PORT, ODR, LED3_BIT)
#define LED3 PRB(LED4_PORT, ODR, LED4_BIT)
#define LED4 PRB(LED5_PORT, ODR, LED5_BIT)

static unsigned char prog1[] = {1, 2, 4, 8, 16, 32, 64, 128};
static unsigned char prog2[] = {128, 64, 32, 16, 8, 4, 2, 1};
static unsigned char prog3[] = {1, 3, 7, 15, 31, 63, 127, 255, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80, 0};
static unsigned char prog4[] = {255, 127, 63, 31, 15, 7, 3, 1, 3, 7, 15, 31, 63, 127, 255, 0};
static unsigned char prog5[] = {
    1, 2, 4, 8, 16, 32, 64, 128,
    128, 64, 32, 16, 8, 4, 2, 1,
    1, 3, 7, 15, 31, 63, 127, 255,
    255, 127, 63, 31, 15, 7, 3, 1};

static struct
{
  unsigned char *arr;
  unsigned char size;
} progs[] = {
    {prog1, _countof(prog1)},
    {prog2, _countof(prog2)},
    {prog3, _countof(prog3)},
    {prog4, _countof(prog4)},
    {prog5, _countof(prog5)},
};

static_assert(_countof(progs) == LEDS_PC, "CHANGE LEDS_PC at config.h");

static unsigned char idx = 0;
unsigned char led_prog = 0;
static unsigned char led_state = 0;
unsigned char led_speed = SPDMIN;

// CONFIG LED PINS
void LEDS_Init()
{
  PRB(LED1_PORT, DDR, LED1_BIT) = 1;
  PRB(LED2_PORT, DDR, LED2_BIT) = 1;
  PRB(LED3_PORT, DDR, LED3_BIT) = 1;
  PRB(LED4_PORT, DDR, LED4_BIT) = 1;
  PRB(LED5_PORT, DDR, LED5_BIT) = 1;

  PCRB(LED1_PORT, CR1, LED1_BIT) = 1;
  PCRB(LED2_PORT, CR1, LED2_BIT) = 1;
  PCRB(LED3_PORT, CR1, LED3_BIT) = 1;
  PCRB(LED4_PORT, CR1, LED4_BIT) = 1;
  PCRB(LED5_PORT, CR1, LED5_BIT) = 1;
}

void LEDS_Write()
{
  LED0 = (led_state & 1 << 0) && 1;
  LED1 = (led_state & 1 << 1) && 1;
  LED2 = (led_state & 1 << 2) && 1;
  LED3 = (led_state & 1 << 3) && 1;
  LED4 = (led_state & 1 << 4) && 1;
}

void LEDS_Clear()
{
  led_state = 0;
}

void LEDS_Next()
{
  led_state = progs[led_prog].arr[idx];
  idx = (idx + 1) % progs[led_prog].size;
}

void LEDS_SpeedUp()
{
  led_speed += 2;
  led_speed = led_speed > SPDMAX ? SPDMAX : led_speed;
}

void LEDS_SpeedDown()
{
  led_speed -= 2;
  led_speed = led_speed < SPDMIN ? SPDMIN : led_speed;
}

void LEDS_NextP()
{
  led_prog = (led_prog + 1) % _countof(progs);
  idx = 0;
}