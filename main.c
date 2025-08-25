#include "iostm8s105k4.h"
#include "config.h"
#include "naming.h"
#include "delay.h"
#include "lcd.h"
#include "CQueue.h"
#include "menu.h"
#include "leds.h"
#include <stdbool.h>

#define _countof(ARR) (sizeof(ARR) / sizeof(ARR[0]))
#define NULL 0
#define GOAL 0xFF
#define PUSHTIME 0x64
#define IACTTIME 0x13E

#define ENLEFT PRB(ENCR_LEFT_PORT, IDR, ENCR_LEFT_BIT)
#define ENRGHT PRB(ENCR_RGHT_PORT, IDR, ENCR_RGHT_BIT)
#define ENPUSH PRB(ENCR_PUSH_PORT, IDR, ENCR_PUSH_BIT)

const char *empty = "                ";
const char *swaiting = "  ..waiting...  ";
const char *sworking = "  ..working...  ";

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

enum
{
  WORK = 0,
  WAIT,
  MENU
} mstate = WAIT;


extern unsigned char led_prog;
extern unsigned char led_state;
extern unsigned char led_speed;


CQueue evbuff;
int iact_counter = 0;
value_t working = 1;

/*---------------------------------------------------------------------
                              MAIN MENU
-----------------------------------------------------------------------*/

MObj root_menu, config_menu;
MObj speed_slider, prog_slider, sleep_combo;

MItem root_menu_items[] =
    {{"|    Config    |", .child = &config_menu},
     {"|   Go sleep   |", .child = &sleep_combo}};

MItem config_menu_items[] =
    {{"|    Speed     |", .child = &speed_slider},
     {"|   Program    |", .child = &prog_slider}};

MItem sleep_combo_items[] =
    {{"<     YES      >", .value = 0},
     {"<     NO       >", .value = 1}};

MObj root_menu = {
    .type = TMENU,
    .items = root_menu_items,
    .curr = 0,
    .size = _countof(root_menu_items),
    .parent = NULL};

MObj config_menu = {
    .type = TMENU,
    .items = config_menu_items,
    .curr = 0,
    .size = _countof(config_menu_items),
    .parent = NULL};

MObj sleep_combo = {
    .type = TCOMBO,
    .items = sleep_combo_items,
    .curr = 0,
    .size = _countof(sleep_combo_items),
    .true_value = &working,
    .parent = NULL};

MObj speed_slider = {
    .type = TSLIDER,
    .true_value = &led_speed,
    .min_bound = SPDMIN,
    .max_bound = SPDMAX,
    .parent = NULL};

MObj prog_slider = {
    .type = TSLIDER,
    .true_value = &led_prog,
    .min_bound = 0,
    .max_bound = LEDS_PC - 1,
    .parent = NULL};

/*---------------------------------------------------------------------*/

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

  counter += led_speed;
  if (counter >= GOAL)
  {
    counter = 0;
    CQueuePush(&evbuff, EV_TTCK);
  }
  return;
}

void IOInit()
{
  // CONFIG ENCODER PINS
  PCRB(ENCR_LEFT_PORT, CR1, ENCR_LEFT_BIT) = 1;
  PCRB(ENCR_RGHT_PORT, CR1, ENCR_RGHT_BIT) = 1;
  PCRB(ENCR_PUSH_PORT, CR1, ENCR_PUSH_BIT) = 1;
  // CONFIG ENCODER INTERRUPTS
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


/*---------------------------------------------------------------------
                              MAIN LOOP
-----------------------------------------------------------------------*/

int main(void)
{
  CQueueInit(&evbuff, 10);
  IOInit();
  
  LEDS_Init();
  LEDS_Clear(); 

  LCD_Init();
  LCD_Clear();
  LCD_WriteS(swaiting, NULL);

  MObj *root = &root_menu;
  while (1)
  {
    event_t event = (event_t)CQueuePop(&evbuff, EV_NONE);
    switch (mstate)
    {
    //----------------------------WAIT STATE------------------------
    case WAIT:
      switch (event)
      {
      case EV_LONG:
        mstate = WORK;
        CQueueInit(&evbuff, 10);
        LCD_WriteS(sworking, empty);
        break;
      default:
        break;
      }
      break;
    //------------------------------MENU STATE------------------------
    case MENU:
      switch (event)
      {
      case EV_NONE:
        LEDS_Write();
        break;
      case EV_LEFT:
        MO_Left(root);
        LCD_WriteS(MO_Title(root), MO_Repr(root));
        break;
      case EV_RGHT:
        MO_Right(root);
        LCD_WriteS(MO_Title(root), MO_Repr(root));
        break;
      case EV_IACT:
        root = MO_Back(root);
        LCD_WriteS(MO_Title(root), MO_Repr(root));
        break;
      case EV_PUSH:
        root = MO_Push(root);
        if (working == 0)
        {
          LEDS_Clear();
          LEDS_Write();
          LCD_WriteS(swaiting, empty);
          mstate = WAIT;
          working = 1;
        }
        else
        {
          LCD_WriteS(MO_Title(root), MO_Repr(root));
        }
        break;
      case EV_TTCK:
        LEDS_Next();
        break;
      case EV_LONG:
        break;
      default:
        break;
      }
      if (root == NULL)
      {
        mstate = WORK;
        LCD_WriteS(sworking, empty);
        break;
      }
      break;
    //------------------------------WORKING---------------------------
    case WORK:
      switch (event)
      {
      case EV_NONE:
        LEDS_Write();
        break;
      case EV_LEFT:
        LEDS_SpeedDown();
        break;
      case EV_RGHT:
        LEDS_SpeedUp();
        break;
      case EV_IACT:
        break;
      case EV_PUSH:
        LEDS_NextP();
        break;
      case EV_TTCK:
        LEDS_Next();
        break;
      case EV_LONG:
        mstate = MENU;
        root = &root_menu;
        LCD_WriteS(MO_Title(root), MO_Repr(root));
        break;
      default:
        break;
      }
      break;
      //--------------------------------------------------------------
    }
  }
}
