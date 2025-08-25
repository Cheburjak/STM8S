#pragma once
#include "config.h"

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
    
void LCD_WriteI(char byte);
void LCD_WriteS(const char* str1, const char* str2);
void LCD_Clear();
void LCD_Init();


        
        
