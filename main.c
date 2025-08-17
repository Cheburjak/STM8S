#include "config.h"
#include "naming.h"
#include "iostm8s105k4.h"
#include "CQueue.h"
#include <stdbool.h>

unsigned char prog1[] = {1,2,4,8,16,32,64,128};
unsigned char prog2[] = {128,64,32,16,8,4,2,1};
unsigned char prog3[] = {1,3,7,15,31,63,127,255};
unsigned char prog4[] = {255,127,63,31,15,7,3,1};
unsigned char prog5[] = {
      1,2,4,8,16,32,64,128,
      128,64,32,16,8,4,2,1,
      1,3,7,15,31,63,127,255,
      255,127,63,31,15,7,3,1
};


#define ENLEFT      PRB(ENCR_LEFT_PORT, IDR, ENCR_LEFT_BIT) 
#define ENRGHT      PRB(ENCR_RGHT_PORT, IDR, ENCR_RGHT_BIT)  
#define ENPUSH      PRB(ENCR_PUSH_PORT, IDR, ENCR_PUSH_BIT)  

#define LED0        PRB(LED1_PORT, ODR, LED1_BIT) 
#define LED1        PRB(LED2_PORT, ODR, LED2_BIT)
#define LED2        PRB(LED3_PORT, ODR, LED3_BIT)
#define LED3        PRB(LED4_PORT, ODR, LED4_BIT)
#define LED4        PRB(LED5_PORT, ODR, LED5_BIT)

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
  EV_LONG,
} event_t;

typedef enum
{
  WORK = 0,
  WAIT
} state_t;

//GLOBALS
#define PROGRAMS    5
#define SPDMAX      96
#define SPDMIN      3
#define GOAL        0xFF
#define LSTART      0x8
#define LEND        0x80
#define PUSHWAITING 0x64

CQueue evbuff;
state_t mstate = WAIT;
unsigned char ledstate = 0;
unsigned char step = SPDMIN;
unsigned char curr = 0;

typedef void(*action_t)(void);
void program1();
void program2();
void program3();
void program4();
void program5();
action_t actions[PROGRAMS] = {program1, program2, program3, program4, program5};

#pragma vector = EXTI3_vector
__interrupt void PinHundler(void)
{
  if (!ENPUSH)
  {  
    CQueuePush(&evbuff, EV_PUSH);
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
      enstate = EN_WAIT;
    }
    break;
  case EN_SCND:
    if (!ENLEFT && !ENRGHT)
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
  static int lcounter = 0;
  TIM4_SR &= ~(1 << 0);
  
  lcounter = ENPUSH ? 0 : lcounter + 1;
  if (lcounter >= PUSHWAITING)
  {
    lcounter = 0;
    CQueuePush(&evbuff, EV_LONG);
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
  //CONFIG LED PINS
  PRB(LED1_PORT, DDR, LED1_BIT)=1;
  PRB(LED2_PORT, DDR, LED2_BIT)=1;
  PRB(LED3_PORT, DDR, LED3_BIT)=1;
  PRB(LED4_PORT, DDR, LED4_BIT)=1;
  PRB(LED5_PORT, DDR, LED5_BIT)=1;
  
  PCRB(LED1_PORT, CR1, LED1_BIT)=1;
  PCRB(LED2_PORT, CR1, LED2_BIT)=1;
  PCRB(LED3_PORT, CR1, LED3_BIT)=1;
  PCRB(LED4_PORT, CR1, LED4_BIT)=1;
  PCRB(LED5_PORT, CR1, LED5_BIT)=1;
  
  //CONFIG ENCODER PINS
  PCRB(ENCR_LEFT_PORT, CR1, ENCR_LEFT_BIT)=1;
  PCRB(ENCR_RGHT_PORT, CR1, ENCR_RGHT_BIT)=1;
  PCRB(ENCR_PUSH_PORT, CR1, ENCR_PUSH_BIT)=1;
  // INTERRUPTS
  PCRB(ENCR_LEFT_PORT, CR2, ENCR_LEFT_BIT)=1;
  PCRB(ENCR_RGHT_PORT, CR2, ENCR_RGHT_BIT)=1;
  PCRB(ENCR_PUSH_PORT, CR2, ENCR_PUSH_BIT)=1;  
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
  LED0 = (state & 1<<0) && 1;
  LED1 = (state & 1<<1) && 1;
  LED2 = (state & 1<<2) && 1;
  LED3 = (state & 1<<3) && 1;
  LED4 = (state & 1<<4) && 1;
}


int main(void)
{ 
  IOInit();
  CQueueInit(&evbuff, 10);  
  
  while (1)
  {
    event_t event = (event_t)CQueuePop(&evbuff, EV_NONE);
    switch (mstate)
    {
    case WAIT:
      switch(event)
      {
      case EV_LONG:
        mstate = WORK;
        ledstate = 0;
        CQueueInit(&evbuff, 10); 
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
      case EV_LONG:
        mstate = WAIT;
        ledstate = 0;
        LedsWrite(ledstate);
        break;
      default:
        break;
      }
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
