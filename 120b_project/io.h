/*  Name & E-mail: Rachel Law (rlaw001@ucr.edu)
*	Lab Section: 23
*	Date: 6/6/14
*	Assignment: Custom Lab
*	Exercise Description: "The Matrix": Control a blue square and collect
*	the other blue dots on the LED Matrix for points. Avoid the red bullets
*	being fired at you. When you get hit, you lose. Control the square with 
*	the joystick. When you die, you can push the joystick button to reset.
*	Score and Enemy count are displayed on the LCD screen.
*	Use of LED Matrix, LCD Screen with custom characters, and a joystick as
*	build-upons.
*	
*	I acknowledge all content contained herein, excluding template or example
*	code, is my own original work.
*	Credit to UCR CS120B for providing content inside 'io.h', 'io.c', and 'timer.h' files.
*   Also credit to 'I-Tunes File (Jesse R. Garcia)' for providing 
*	LCDdefinechar(const uint8_t *pc,uint8_t char_code) in 'lcd_customchar.c'
*	as well as sample arrays to use in the function.
*/
#ifndef __io_h__
#define __io_h__

void LCD_init();
void LCD_ClearScreen(void);
void LCD_WriteCommand (unsigned char Command);
void LCD_Cursor (unsigned char column);
void LCD_DisplayString(unsigned char column ,const unsigned char *string);
void delay_ms(int miliSec);
#endif

