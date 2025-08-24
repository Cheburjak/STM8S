#pragma once
#include <stdbool.h>

#if !defined(LCD_RS_PORT) || !defined(LCD_RS_BIT) || \
    !defined(LCD_RW_PORT) || !defined(LCD_RW_BIT) || \
    !defined(LCD_E_PORT)  || !defined(LCD_E_BIT)  || \
    !defined(LCD_D4_PORT) || !defined(LCD_D4_BIT) || \
    !defined(LCD_D5_PORT) || !defined(LCD_D5_BIT) || \
    !defined(LCD_D6_PORT) || !defined(LCD_D6_BIT) || \
    !defined(LCD_D7_PORT) || !defined(LCD_D7_BIT)
    
#error "You have to define LCD_RS, LCD_RW, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7 - with PORT and BIT sufix" \
      "EXAMPLE: #define LCD_RS_PORT A"\
      "EXAMPLE: #define LCD_RS_BIT 0"

#endif
        
#define BIN8(d7,d6,d5,d4,d3,d2,d1,d0) ((d7<<7)|(d6<<6)|(d5<<5)|(d4<<4)|(d3<<3)|(d2<<2)|(d1<<1)|(d0))

// NAME RESOVLING
// EXAMPLE: PRB(B, DDR, 7) => PB_DDR_DDR7
// EXAMPLE: PRB(B, IDR, 7) => PB_IDR_IDR7       
#define _MAKE_NAME_(PORT, REG, BIT) P##PORT##_##REG##_##REG##BIT
#define PRB(PORT, REG, BIT) _MAKE_NAME_(PORT, REG, BIT)
// NAME RESOVLING       
#define CR1 1
#define CR2 2
// EXAMPLE: PCRB(B, 1, 7) => PB_CR1_C17
// EXAMPLE: PCRB(B, 2, 7) => PB_CR2_C27
#define _MAKE_CR_NAME_(PORT, NUM, BIT) P##PORT##_CR##NUM##_##C##NUM##BIT
#define PCRB(PORT, NUM, BIT) _MAKE_CR_NAME_(PORT, NUM, BIT)
    
//MPU
#define DDR_WRITE 1
#define DDR_READ 0

//IR - instruction register
//DR - data register
#define RS_INSTRUCTION 0
#define RS_DATA 1
        
//RW
#define RW_READ 1
#define RW_WRITE 0

#define LCD_DB4 PRB(LCD_D4_PORT, ODR, LCD_D4_BIT)
#define LCD_DB5 PRB(LCD_D5_PORT, ODR, LCD_D5_BIT)
#define LCD_DB6 PRB(LCD_D6_PORT, ODR, LCD_D6_BIT)
#define LCD_DB7 PRB(LCD_D7_PORT, ODR, LCD_D7_BIT)
#define LCD_RS  PRB(LCD_RS_PORT, ODR, LCD_RS_BIT)
#define LCD_RW  PRB(LCD_RW_PORT, ODR, LCD_RW_BIT)
#define LCD_E   PRB(LCD_E_PORT,  ODR, LCD_E_BIT)     

void LCD_Wait()
{
//  bool rw, rs, d7; 
//  d7 = PRB(LCD_D7_PORT, DDR, LCD_D7_BIT);
//  rw = PRB(LCD_RS_PORT, ODR, LCD_RS_BIT);
//  rs = PRB(LCD_RW_PORT, ODR, LCD_RW_BIT);
//  
//  LCD_RS = RS_INSTRUCTION;
//  LCD_RW = RW_READ;
//  PRB(LCD_D7_PORT, DDR, LCD_D7_BIT) = DDR_READ;
//  
//  for(bool bf = true; bf; )
//  {
//    LCD_E = true;
//    DELAY_MICRO5();
//    bf = PRB(LCD_D7_PORT, IDR, LCD_D7_BIT);
//    LCD_E = false;
//    DELAY_MICRO5();    
//    LCD_E = true;
//    DELAY_MICRO5();
//    LCD_E = false;
//    DELAY_MICRO5();
//  }
//  
//  PRB(LCD_D7_PORT, DDR, LCD_D7_BIT) = d7;
//  PRB(LCD_RS_PORT, ODR, LCD_RS_BIT) = rs;
//  PRB(LCD_RW_PORT, ODR, LCD_RW_BIT) = rw; 
  delay_ms(1);
}

void LCD_Cmit()
{
  LCD_E = true;
  DELAY_5MKS();
  LCD_E = false;
  DELAY_5MKS();
}

void LCD_WriteI(char byte)
{
  LCD_Wait();
  LCD_RS = RS_INSTRUCTION;
  LCD_DB7 = (byte & 0x80) && 1;
  LCD_DB6 = (byte & 0x40) && 1;
  LCD_DB5 = (byte & 0x20) && 1;
  LCD_DB4 = (byte & 0x10) && 1;
  LCD_Cmit();
  LCD_DB7 = (byte & 0x8) && 1;
  LCD_DB6 = (byte & 0x4) && 1;
  LCD_DB5 = (byte & 0x2) && 1;
  LCD_DB4 = (byte & 0x1) && 1;
  LCD_Cmit();
}

void LCD_WriteD(char byte)
{
  LCD_Wait();
  LCD_RS = RS_DATA;
  LCD_DB7 = (byte & 0x80) && 1;
  LCD_DB6 = (byte & 0x40) && 1;
  LCD_DB5 = (byte & 0x20) && 1;
  LCD_DB4 = (byte & 0x10) && 1;
  LCD_Cmit();
  LCD_DB7 = (byte & 0x8) && 1;
  LCD_DB6 = (byte & 0x4) && 1;
  LCD_DB5 = (byte & 0x2) && 1;
  LCD_DB4 = (byte & 0x1) && 1;
  LCD_Cmit();
}

void LCD_WriteS(const char* str1, const char* str2)
{
  if (str1)
  {
    LCD_WriteI(BIN8(1,0,0,0,0,0,0,0));
    do LCD_WriteD(*str1++); while(*str1);
  }
  if (str2)
  {
    LCD_WriteI(BIN8(1,1,0,0,0,0,0,0));
    do LCD_WriteD(*str2++); while(*str2);
  }
}

void LCD_Clear()
{
  LCD_WriteI(1);
  delay_ms(1);
}

void LCD_Init()
{
  // Configuration for RS, RW, E
  PCRB(LCD_RS_PORT, CR1, LCD_RS_BIT) = 1;
  PCRB(LCD_RW_PORT, CR1, LCD_RW_BIT) = 1;
  PCRB(LCD_E_PORT, CR1, LCD_E_BIT) = 1;
  PRB(LCD_RS_PORT, DDR, LCD_RS_BIT) = DDR_WRITE;
  PRB(LCD_RW_PORT, DDR, LCD_RW_BIT) = DDR_WRITE;
  PRB(LCD_E_PORT, DDR, LCD_E_BIT) = DDR_WRITE;
  
  // Configuration for D4, D5, D6, D7
  PCRB(LCD_D4_PORT, CR1, LCD_D4_BIT) = 1;
  PCRB(LCD_D5_PORT, CR1, LCD_D5_BIT) = 1;
  PCRB(LCD_D6_PORT, CR1, LCD_D6_BIT) = 1;
  PCRB(LCD_D7_PORT, CR1, LCD_D7_BIT) = 1;
  PRB(LCD_D4_PORT, DDR, LCD_D4_BIT) = DDR_WRITE;
  PRB(LCD_D5_PORT, DDR, LCD_D5_BIT) = DDR_WRITE;
  PRB(LCD_D6_PORT, DDR, LCD_D6_BIT) = DDR_WRITE;
  PRB(LCD_D7_PORT, DDR, LCD_D7_BIT) = DDR_WRITE; 
  
  LCD_RS = RS_INSTRUCTION;
  LCD_RW = RW_WRITE;
  
  delay_ms(15);
  //-----RESET---STAGE1-----------
  LCD_DB5 = 1;
  LCD_DB4 = 1;
  LCD_Cmit();
  delay_ms(5);
  LCD_DB5 = 1;
  LCD_DB4 = 1;
  LCD_Cmit();
  //-----RESET---STAGE2-----------
  delay_ms(1);
  LCD_DB5 = 1;
  LCD_DB4 = 1;
  LCD_Cmit();
  delay_ms(5);
  LCD_DB5 = 1;
  LCD_DB4 = 0;
  LCD_Cmit();
  //-----CONFIG-------------------
  //FUNCTION SET | DL = 0(4bit) | N = 1(2lines) | F = 0(5*8) |
  LCD_WriteI(BIN8(0,0,1,0,1,0,0,0));
  //DISPLAY IS ON | CURSOR IS OFF | BLINK IS OFF
  LCD_WriteI(BIN8(0,0,0,0,1,1,0,0));
  //ENTRY MODE | INCREMENT | SHIFT IS OFF
  LCD_WriteI(BIN8(0,0,0,0,0,1,1,0));
}


        
        
