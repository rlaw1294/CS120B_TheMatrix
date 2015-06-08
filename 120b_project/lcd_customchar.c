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
/*
 * Custom_character_display.c
 *
 * Created: 5/19/2014 5:35:49 PM
 * Author: I-Tunes File (Jesse R. Garcia)
 * I, Jesse R. Garcia, hereby grant permission of use of the following software 
 * free of charge at the users discretion.
 * Sources:
 *	http://www.eng.utah.edu/~cs3710/slides/OtherIOx2.pdf
 *  http://winavr.scienceprog.com/example-avr-projects/simplified-avr-lcd-routines.html
 *  http://ieee.ucr.edu/cs120b/lcd/
 *  https://docs.google.com/document/d/1jOQ_47YrEyHdo-u9gBFaOsZT5hyGBkMQS8yxEIuIOZA/edit
 *
 *  Disclaimer: 
 *				I, Jesse R. Garcia, do not claim ownership to any of the four sources
 *			stated above. Furthermore, I do not claim ownership to the names Mario, Link, 
 *			The Legend of Zelda, NES, Buzz Lightyear or Atari 2600 or their designs.  
 *          They are (or were)copyrighted material of the Nintendo Corporation,
 *			Disney, and Atari.			
 */ 

#include <avr/io.h>
//#include "io.c" // Our io.c file from lab 5
#include <avr/pgmspace.h> // Built in avr library

// This is how we define a binary array of our character pattern.
// Each const represents a 5x8 pixel grid, with ONES representing 
// illuminated pixels, and ZEROS representing non-illuminated pixels.
const uint8_t dungeon_key[] PROGMEM = {
	0b01110,
	0b10001,
	0b01110,
	0b00100,
	0b00100,
	0b00110,
	0b00100,
	0b00110
}; // A basic, dungeon-esque key

const uint8_t pixel_man[] PROGMEM = {
	0b00100,
	0b01010,
	0b11111,
	0b10101,
	0b00100,
	0b01010,
	0b01010,
	0b11011
	
}; // biped that kinda looks like Buzz Lightyear on an Atari 2600

const uint8_t Links_shield[] PROGMEM = {
	0b00000,
	0b01110,
	0b11011,
	0b10001,
	0b11011,
	0b11011,
	0b11111,
	0b01110
}; // vaguely represents Links shield from The Legend of Zelda for the NES

const uint8_t Mario_TL[] PROGMEM = {
	0b10001,
	0b10011,
	0b10011,
	0b10101,
	0b10101,
	0b10110,
	0b01000,
	0b01011
};

const uint8_t Mario_TM[] PROGMEM = {
	0b11110,
	0b11111,
	0b10010,
	0b00010,
	0b10001,
	0b00011,
	0b00000,
	0b11110
};

const uint8_t Mario_TR[] PROGMEM = {
	0b00011,
	0b11011,
	0b01001,
	0b00101,
	0b00010,
	0b11011,
	0b00111,
	0b00111
};

const uint8_t Mario_BL[] PROGMEM = {
	0b10111,
	0b01111,
	0b10011,
	0b10001,
	0b10011,
	0b11011,
	0b10111,
	0b01111
};

const uint8_t Mario_BM[] PROGMEM = {
	0b11111,
	0b11111,
	0b01101,
	0b11111,
	0b11111,
	0b10011,
	0b00001,
	0b00001
};

const uint8_t Mario_BR[] PROGMEM = {
	0b11011,
	0b11101,
	0b10011,
	0b00011,
	0b10011,
	0b10111,
	0b11011,
	0b11101
};// The preceding six declarations can be called to display a 3x2 block that resembles classic Mario

const uint8_t Skull[] PROGMEM = {
	0b01110,
	0b11111,
	0b10101,
	0b11111,
	0b01110,
	0b01110,
	0b00000
};

const uint8_t Star_atop[] PROGMEM = {
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b00011,
	0b00010,
	0b00001,
	0b00000,
};

const uint8_t Star_btop[] PROGMEM = {
	0b00000,
	0b00001,
	0b00001,
	0b00010,
	0b11110,
	0b10000,
	0b10000,
	0b10000
};

const uint8_t Star_ctop[] PROGMEM = {
	0b11000,
	0b00100,
	0b00100,
	0b00010,
	0b00011,
	0b10100,
	0b10100,
	0b00000
};

const uint8_t Star_dtop[] PROGMEM = {
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b11110,
	0b00010,
	0b00100,
	0b01000
};

const uint8_t Star_abot[] PROGMEM = {
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b00001,
	0b00001,
	0b00000
};

const uint8_t Star_bbot[] PROGMEM = {
	0b01101,
	0b00101,
	0b01000,
	0b11001,
	0b10011,
	0b00110,
	0b11000,
	0b00000
};

const uint8_t Star_cbot[] PROGMEM = {
	0b00010,
	0b11110,
	0b00000,
	0b11100,
	0b00010,
	0b00001,
	0b00000,
	0b00000
};

const uint8_t Star_dbot[] PROGMEM = {
	0b01000,
	0b10000,
	0b10000,
	0b01000,
	0b01000,
	0b00100,
	0b11100,
	0b00000
};

// The following function can be found in the lcd_lib.c file found on the accompanying website
// The only change I made was the function names to our pre-built functions (LCD_WriteCommand and LCD_WriteData)
// as they have the same functionality.
void LCDdefinechar(const uint8_t *pc,uint8_t char_code){
	uint8_t a, pcc;
	uint16_t i;
	a=(char_code<<3)|0x40;
	for (i=0; i<8; i++){
		pcc=pgm_read_byte(&pc[i]);
		LCD_WriteCommand(a++);
		LCD_WriteData(pcc);
	}
}