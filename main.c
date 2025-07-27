#include "iostm8s105k4.h"
#include "CQueue.h"
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

enum
{
  PM_ST0 = 0, 
  PM_ST1, 
  PM_ST2
} pstate = PM_ST0;

enum
{
  EN_WAIT = 0,
  EN_FRST,
  EN_SCND
} enstate = EN_WAIT;

typedef enum
{
  EV_NONE = 0,
  EV_LEFT,
  EV_RGHT,
  EV_PUSH,
  EV_TTCK,
  EV_STRT,
  EV_STOP
} event_t;

typedef void(*action_t)(void);

void program1();
void program2();
void program3();
void program4();
void program5();

//GLOBALS
#define PROGRAMS    5
#define SPDMAX      64
#define SPDMIN      3
#define GOAL        0xFF
#define LSTART      0x8
#define LEND        0x80

CQueue evbuff;
unsigned char ledstate = LSTART;
unsigned char step = SPDMIN;
unsigned char curr = 0;
action_t actions[PROGRAMS] = {program1, program2, program3, program4, program5};


#pragma vector = EXTI3_vector
__interrupt void PinHundler(void)
{
  if (!P_ENPUSH && P_ENRGHT && P_ENLEFT)
  {  
    CQueuePush(&evbuff, EV_PUSH);
    enstate = EN_WAIT;
    return;
  }
  switch (enstate)
  {
  case EN_WAIT:
    if (!P_ENRGHT && P_ENLEFT)
      enstate = EN_FRST;
    else if (!P_ENLEFT && P_ENRGHT)
      enstate = EN_SCND;
    break;
  case EN_FRST:
    if (!P_ENLEFT && !P_ENRGHT)
    {
      CQueuePush(&evbuff, EV_RGHT);
      enstate = EN_WAIT;
    }
    break;
  case EN_SCND:
    if (!P_ENLEFT && !P_ENRGHT)
    {
      CQueuePush(&evbuff, EV_LEFT);
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
  TIM4_SR &= ~(1 << 0);

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

int main(void)
{
  IOInit();
  CQueueInit(&evbuff, 10); 
 
  while (1)
  {
    event_t event = (event_t)CQueuePop(&evbuff, EV_NONE);
    switch (event)
    {
    case EV_NONE:
      P_LED = ledstate & (M_LED_0 | M_LED_1 | M_LED_2 | M_LED_3 | M_LED_4);
      break;
    case EV_LEFT:
      step-=2;
      step = step < SPDMIN ? SPDMIN : step;      
      break;
    case EV_RGHT:
      step+=2;
      step = step > SPDMAX ? SPDMAX : step;
      break;
    case EV_PUSH:
      curr = (curr+1) % PROGRAMS;
      pstate = PM_ST0;   
      break;
    case EV_TTCK:
      actions[curr]();
      break;
    }
  }
}

void program1()
{
  switch(pstate)
  {
  case PM_ST0:
    ledstate = LSTART;
    pstate = PM_ST1;
    break;
  case PM_ST1:
    if (ledstate < LEND)
        ledstate = ledstate << 1;
    else
        pstate = PM_ST0;
    break;
  }
}

void program2()
{
  switch(pstate)
  {
  case PM_ST0:
    ledstate = LEND;
    pstate = PM_ST1;
    break;
  case PM_ST1:
    if (ledstate > LSTART)
      ledstate =  ledstate >> 1;
    else
      pstate = PM_ST0;
    break;
  }
}

void program3()
{
  switch(pstate)
  {
  case PM_ST0:
    ledstate = LSTART;
    pstate = PM_ST1;
    break;
  case PM_ST1:
    if (ledstate < LEND)
      ledstate |= ledstate << 1;
    else
      pstate = PM_ST2;
    break;
  case PM_ST2:
    if (ledstate > 0)
      ledstate &= ledstate - 1;
    else
      pstate = PM_ST0;
    break;
  }
}

void program4()
{
    switch(pstate)
    {
    case PM_ST0:
      ledstate = LEND;
      pstate = PM_ST1;
      break;  
    case PM_ST1:
      if (ledstate < 0xF8)
        ledstate |= ledstate >> 1;
      else
        pstate = PM_ST2;
      break;
    case PM_ST2:
      if (ledstate >= LSTART)
        ledstate &= ledstate >> 1;
      else
        pstate = PM_ST0;
      break;
    }
}

void program5()
{
    static unsigned char st = 0;
    switch(st)
    {
    case 0:
      program1();
      if (pstate == PM_ST0)
        st = 1;
        return;
    break;
    case 1:
      program2();
      if (pstate == PM_ST0)
        st = 2;
    break;
    case 2:
      program3();
      if (pstate == PM_ST0)
        st = 3;
      break;
    case 3:
      program4();
      if (pstate == PM_ST0)
        st = 0;
      break;
    } 
}
