#include "config.h"
#include "naming.h"
#include "iostm8s105k4.h"
#include "delay.h"
#include "lcd.h"
#include "CQueue.h"
#include <stdbool.h>

#define ENLEFT PRB(ENCR_LEFT_PORT, IDR, ENCR_LEFT_BIT)
#define ENRGHT PRB(ENCR_RGHT_PORT, IDR, ENCR_RGHT_BIT)
#define ENPUSH PRB(ENCR_PUSH_PORT, IDR, ENCR_PUSH_BIT)

#define LED0 PRB(LED1_PORT, ODR, LED1_BIT)
#define LED1 PRB(LED2_PORT, ODR, LED2_BIT)
#define LED2 PRB(LED3_PORT, ODR, LED3_BIT)
#define LED3 PRB(LED4_PORT, ODR, LED4_BIT)
#define LED4 PRB(LED5_PORT, ODR, LED5_BIT)

unsigned char prog1[] = {1, 2, 4, 8, 16, 32, 64, 128};
unsigned char prog2[] = {128, 64, 32, 16, 8, 4, 2, 1};
unsigned char prog3[] = {1, 3, 7, 15, 31, 63, 127, 255, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80, 0};
unsigned char prog4[] = {255, 127, 63, 31, 15, 7, 3, 1, 3, 7, 15, 31, 63, 127, 255, 0};
unsigned char prog5[] = {
    1, 2, 4, 8, 16, 32, 64, 128,
    128, 64, 32, 16, 8, 4, 2, 1,
    1, 3, 7, 15, 31, 63, 127, 255,
    255, 127, 63, 31, 15, 7, 3, 1};

#define countof(x) (sizeof(x) / sizeof(x[0]))
struct
{
  unsigned char *arr;
  unsigned char size;
} progs[] = {
    {prog1, countof(prog1)},
    {prog2, countof(prog2)},
    {prog3, countof(prog3)},
    {prog4, countof(prog4)},
    {prog5, countof(prog5)},
};
unsigned char idx = 0;
unsigned char curr = 0;

enum
{
  EN_WAIT = 0,
  EN_FRST,
  EN_SCND
} enstate = EN_WAIT;

typedef enum
{
  EV_NONE = 0, // IDLE
  EV_LEFT,     // ENCODER LEFT
  EV_RGHT,     // ENCODER RIGHT
  EV_PUSH,     // ENCODER PUSH
  EV_TTCK,     // TIMER TICK FOR LEDS
  EV_LONG,     // ENCODER LONG PUSH
  EV_IACT      // INACTIVE
} event_t;

typedef enum
{
  WORK = 0,
  WAIT
} state_t;

// GLOBALS
#define SPDMAX 96
#define SPDMIN 3
#define GOAL 0xFF
#define PUSHTIME 0x64
#define IACTTIME 0x13E

CQueue evbuff;
state_t mstate = WAIT;
unsigned char ledstate = 0;
unsigned char step = SPDMIN;
int iact_counter = 0;

#pragma vector = EXTI3_vector
__interrupt void PinHundler(void)
{
  if (!ENPUSH)
  {
    CQueuePush(&evbuff, EV_PUSH);
    iact_counter = 0;

    enstate = EN_WAIT;
    return;
  }
  switch (enstate)
  {
  case EN_WAIT:
    if (!ENRGHT && ENLEFT)
      enstate = EN_FRST;
    else if (!ENLEFT && ENRGHT)
      enstate = EN_SCND;
    break;
  case EN_FRST:
    if (!ENLEFT && !ENRGHT)
    {
      CQueuePush(&evbuff, EV_RGHT);
      iact_counter = 0;

      enstate = EN_WAIT;
    }
    break;
  case EN_SCND:
    if (!ENLEFT && !ENRGHT)
    {
      CQueuePush(&evbuff, EV_LEFT);
      iact_counter = 0;

      enstate = EN_WAIT;
    }
    break;
  }
  return;
}

#pragma vector = TIM4_OVR_UIF_vector
__interrupt void Timer4Hundler(void)
{
  static int counter = 0;
  static int lcounter = 0;
  TIM4_SR &= ~(1 << 0);

  if (iact_counter++ >= IACTTIME)
  {
    iact_counter = 0;
    CQueuePush(&evbuff, EV_IACT);
  }

  lcounter = ENPUSH ? 0 : lcounter + 1;
  if (lcounter >= PUSHTIME)
  {
    CQueuePush(&evbuff, EV_LONG);
    iact_counter = 0;
    lcounter = 0;
  }

  counter += step;
  if (counter >= GOAL)
  {
    counter = 0;
    CQueuePush(&evbuff, EV_TTCK);
  }
  return;
}

void IOInit()
{
  // CONFIG LED PINS
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

  // CONFIG ENCODER PINS
  PCRB(ENCR_LEFT_PORT, CR1, ENCR_LEFT_BIT) = 1;
  PCRB(ENCR_RGHT_PORT, CR1, ENCR_RGHT_BIT) = 1;
  PCRB(ENCR_PUSH_PORT, CR1, ENCR_PUSH_BIT) = 1;
  // INTERRUPTS
  PCRB(ENCR_LEFT_PORT, CR2, ENCR_LEFT_BIT) = 1;
  PCRB(ENCR_RGHT_PORT, CR2, ENCR_RGHT_BIT) = 1;
  PCRB(ENCR_PUSH_PORT, CR2, ENCR_PUSH_BIT) = 1;
  // INTERRUPT SENSITIVETY
  EXTI_CR1 |= (1 << 7);

  // TIMER TIM4
  TIM4_PSCR = 0x7;
  TIM4_ARR = 255;
  TIM4_CNTR = 1;
  TIM4_IER |= 1;
  TIM4_CR1 |= 1;
  asm("rim");
}

void LedsWrite(unsigned char state)
{
  LED0 = (state & 1 << 0) && 1;
  LED1 = (state & 1 << 1) && 1;
  LED2 = (state & 1 << 2) && 1;
  LED3 = (state & 1 << 3) && 1;
  LED4 = (state & 1 << 4) && 1;
}

int main(void)
{
  IOInit();
  LCD_Init();
  LCD_Clear();
  CQueueInit(&evbuff, 10);
  LCD_StrF("Waiting...      ");

  while (1)
  {
    event_t event = (event_t)CQueuePop(&evbuff, EV_NONE);
    switch (mstate)
    {
    case WAIT:
      switch (event)
      {
      case EV_LONG:
        mstate = WORK;
        ledstate = 0;
        CQueueInit(&evbuff, 10);
        LCD_StrF("Working...        ");
        break;
      default:
        break;
      }
      break;
    case WORK:
      switch (event)
      {
      case EV_NONE:
        LedsWrite(ledstate);
        break;
      case EV_LEFT:
        step -= 2;
        step = step < SPDMIN ? SPDMIN : step;
        break;
      case EV_RGHT:
        step += 2;
        step = step > SPDMAX ? SPDMAX : step;
        break;
      case EV_IACT:
        ledstate = 0xFF;
        break;
      case EV_PUSH:
        curr = (curr + 1) % countof(progs);
        idx = 0;
        break;
      case EV_TTCK:
        ledstate = progs[curr].arr[idx];
        idx = (idx + 1) % progs[curr].size;
        break;
      case EV_LONG:
        mstate = WAIT;
        ledstate = 0;
        LedsWrite(ledstate);
        LCD_StrF("Waiting...      ");
        break;
      default:
        break;
      }
      break;
    }
  }
}
