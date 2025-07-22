#include "iostm8s105k4.h"
#include <stdbool.h>

#define P_ENLEFT    PD_IDR_IDR5
#define P_ENRGHT    PD_IDR_IDR6
#define P_ENPUSH    PD_IDR_IDR4

#define M_ENLEFT    (0x1 << 5)
#define M_ENRGHT    (0x1 << 6)
#define M_ENPUSH    (0x1 << 4)

#define P_LED_0     PC_ODR_ODR3
#define P_LED_1     PC_ODR_ODR4
#define P_LED_2     PC_ODR_ODR5
#define P_LED_3     PC_ODR_ODR6
#define P_LED_4     PC_ODR_ODR7
#define P_LED       PC_ODR
#define P_LED_DDR   PC_DDR
#define P_LED_CR1   PC_CR1
#define P_LED_CR2   PC_CR2

#define M_LED_0     (0x1 << 3)
#define M_LED_1     (0x1 << 4)
#define M_LED_2     (0x1 << 5)
#define M_LED_3     (0x1 << 6)
#define M_LED_4     (0x1 << 7)

#define SPDMAX  48
#define SPDMIN  3
#define GOAL    0xFF

enum
{
  WAIT = 0,
  FRST,
  SCND
} enstate = WAIT;
enum
{
  STBY = 0,
  LEFT,
  RGHT,
  PUSH,
  TIMR
} event = STBY;

unsigned char ledstate = 0x8;
int step = SPDMIN;

#pragma vector = EXTI3_vector
__interrupt void PinHundler(void)
{
  if (!P_ENPUSH)
  {  
    event = PUSH;
    enstate = WAIT;
    return;
  }
  switch (enstate)
  {
  case WAIT:
    if (!P_ENRGHT && P_ENLEFT)
      enstate = FRST;
    else if (!P_ENLEFT && P_ENRGHT)
      enstate = SCND;
    break;
  case FRST:
    if (!P_ENLEFT && !P_ENRGHT)
    {
      event = RGHT;
      enstate = WAIT;
    }
    break;
  case SCND:
    if (!P_ENLEFT && !P_ENRGHT)
    {
      event = LEFT;
      enstate = WAIT;
    }
    break;
  }
  return;
}

#pragma vector = TIM4_OVR_UIF_vector
__interrupt void Timer4Hundler(void)
{
  static int counter = 0;
  TIM4_SR &= ~(1 << 0);

  counter += step;

  if (counter >= GOAL)
  {
    counter = 0;
    event = TIMR;
  }
  return;
}

void forward()
{
  ledstate = ledstate < 0x80 ? ledstate << 1 : 0x8;
}

void backward()
{
  ledstate = ledstate > 0x8 ? ledstate >> 1 : 0x80;
}

void IOInit()
{
  //! LED
  P_LED_DDR |= M_LED_0 | M_LED_1 | M_LED_2 | M_LED_3 | M_LED_4;
  P_LED_CR1 |= M_LED_0 | M_LED_1 | M_LED_2 | M_LED_3 | M_LED_4;
  
  //! ENCODER
  // PIN MODE
  PD_CR1 |= M_ENLEFT | M_ENRGHT | M_ENPUSH;
  // INTERRUPTS
  PD_CR2 |= M_ENLEFT | M_ENRGHT | M_ENPUSH;
  // INTERRUPT SENSITIVETY
  EXTI_CR1 |= (1 << 7);

  //!TIMER TIM4
  TIM4_PSCR = 0x7;
  TIM4_ARR = 255;
  TIM4_CNTR = 1;
  TIM4_IER |= 1;
  TIM4_CR1 |= 1;

  asm("rim");
}
typedef void(*action_t)(void);

int main(void)
{
  IOInit();
  action_t action = forward;  
  while (1)
  {
    switch (event)
    {
    case STBY:
      P_LED = ledstate;
      break;
    case LEFT:
      event = STBY;
      step--;
      step = step < SPDMIN ? SPDMIN : step;      
      break;
    case RGHT:
      event = STBY;
      step++;
      step = step > SPDMAX ? SPDMAX : step;
      break;
    case PUSH:
      event = STBY;
      action = action == forward ? backward : forward;
      break;
    case TIMR:
      event = STBY;
      action();
      break;
    }
    
  }
}
