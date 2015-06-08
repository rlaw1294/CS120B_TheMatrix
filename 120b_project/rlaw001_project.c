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

#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <timer.h>
#include <avr/io.h>
#include "io.c"
#include "lcd_customchar.c"


//game overall
unsigned char isgameover = 0;
unsigned char points = 0;
unsigned char ischangescreen = 0;
unsigned char isreset = 0;
unsigned char iswaitreset = 0;

//blue USER
char user_row[4] = {0x08, 0x08, 0x10, 0x10};
char user_col[4] = {~0x10, ~0x08, ~0x10, ~0x08};

//PlusBlock
unsigned char plusblock_row = 0x01;
unsigned char plusblock_col = 0xFB;

//Enemies
unsigned char enemy_row[7] = {0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char enemy_col[7] = {~0x01, ~0x01, ~0x00, ~0x00, ~0x00, ~0x00, ~0x00};
unsigned char enemy_wait[7] = {10, 15, 3, 4, 5, 6, 2};
unsigned char enemy_count = 1;



void reset_globals() {
	//game overall
	isgameover = 0;
	points = 0;
	ischangescreen = 0;
	isreset = 0;
	iswaitreset = 0;
	
	//blue USER
	user_row[0] = 0x08;
	user_row[1] = 0x08;
	user_row[2] = 0x10;
	user_row[3] = 0x10;
	user_col[0] = ~0x10;
	user_col[1] = ~0x08;
	user_col[2] = ~0x10;
	user_col[3] = ~0x08;

	//PlusBlock
	plusblock_row = 0x01;
	plusblock_col = 0xFB;

	//Enemies
	enemy_count = 1;	
	for (unsigned char i=0; i<7; i++) {
		enemy_row[i] = 0x80;
		enemy_col[i] = ~0x01;
		enemy_wait[i] = 10;
	}
	return;
}

void transmit_data(unsigned char data) {
	int i;
	for (i = 0; i < 8 ; ++i) {
		// Sets SRCLR to 1 allowing data to be set
		// Also clears SRCLK in preparation of sending data
		PORTC = (PORTC & 0xE0) | 0x08;
		// set SER = next bit of data to be sent.
		PORTC |= ((data >> i) & 0x01) << 4;
		// set SRCLK = 1. Rising edge shifts next bit of data into the shift register
		PORTC |= 0x02;
	}
	// set RCLK = 1. Rising edge copies data from “Shift” register to “Storage” register
	PORTC |= 0x04;
	// clears all lines in preparation of a new transmission
	PORTC &= 0xE0;
}

void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}

void Set_A2D_Pin(unsigned char pinNum) {
	ADMUX = (pinNum <= 0x07) ? pinNum : ADMUX;
}

void Display_Gameover() {
	LCD_DisplayString(5, "GAME OVER        Score: ");
	LCD_Cursor(3);
	LCD_WriteData(0b00001000);
	LCD_Cursor(15);
	LCD_WriteData(0b00001000);
	LCD_Cursor(28);
	LCD_WriteData(points/10 + '0');
	LCD_Cursor(29);
	LCD_WriteData(points%10 + '0');
	LCD_Cursor(0);
	return;
}

void Display_Star(unsigned char i) { //i is position ranging from 1 - 13
	//LCD_Cursor(i);
	//LCD_WriteData(0b00001000);
	LCD_Cursor(i+1);
	LCD_WriteData(0b00001001);
	LCD_Cursor(i+2);
	LCD_WriteData(0b00001010);
	LCD_Cursor(i+3);
	LCD_WriteData(0b00001011);
	LCD_Cursor(i+16);
	LCD_WriteData(0b00001100);
	LCD_Cursor(i+17);
	LCD_WriteData(0b00001101);
	LCD_Cursor(i+18);
	LCD_WriteData(0b00001110);
	LCD_Cursor(i+19);
	LCD_WriteData(0b00001111);
	LCD_Cursor(0);
	return;
}

void Display_Initiate_Ingame ()
{
	LCD_ClearScreen();
	LCD_DisplayString(1, "Score:          Enemies:");
	LCD_Cursor(8); // score tens
	LCD_WriteData(points/10 + '0');
	LCD_Cursor(9); // score one
	LCD_WriteData(points%10 + '0');
	
	LCD_Cursor(26); //enemy ones
	LCD_WriteData(enemy_count + '0');
	Display_Star(12);
	LCD_Cursor(0);
	return;
}

void Display_Update_Ingame ()
{
	LCD_Cursor(8); // score tens
	LCD_WriteData(points/10 + '0');
	LCD_Cursor(9); // score one
	LCD_WriteData(points%10 + '0');
		
	LCD_Cursor(26); //enemy ones
	LCD_WriteData(enemy_count + '0');
	LCD_Cursor(0);
	return;
}


enum User_States {set_x, set_y, read_x, read_y, gameover};
int User_Tick(int state) {
	
    // === Local Variables ===
	unsigned short x_joystick = 512;
	unsigned short y_joystick = 512;
	unsigned char tmpA = PINA;
	unsigned char pina2 = ~tmpA & 0x04;
	
    // === Transitions ===
    switch (state) {
		case -1:
			state = set_x;
			break;
   		case set_x:
		   if (isgameover) { state = gameover; }   		   
		   else { state = read_x; }
		   break;
		case read_x:
			if (isgameover) { state = gameover; }   		
			else { state = set_y; }
			break;
		case set_y:
			if (isgameover) { state = gameover; }   		
			else { state = read_y; }
			break;
		case read_y:
		    if (isgameover) { state = gameover; }   		
			else { state = set_x; }
			break;
		case gameover:
			if (isreset) { state = -1; }
			else { state = gameover; }
		default:
			state = set_x;
			break;
    }
    
    // === Actions ===
    switch (state) {
		case set_x:
			Set_A2D_Pin(0);
			break;
		case read_x:
			x_joystick = ADC;
			break;
		case set_y:
			Set_A2D_Pin(1);
			break;
		case read_y:
			y_joystick = ADC;
			break;
		case gameover:
			if (pina2) { isreset = 1; }
			break;
		default:
			break;  
    }
	
	if (y_joystick < 312) //up
	{
		if (user_row[0] != 0x01)
		{
			for (unsigned i=0; i<4; i++)
			{
				user_row[i] = user_row[i] >> 1;
			}
		}
	}
	else if (y_joystick > 712) //down
	{
		if (user_row[0] != 0x40 && user_row[2] != 0x40)
		{
			for (unsigned i=0; i<4; i++)
			{
				user_row[i] = (user_row[i] << 1);
			}
		}
	}
	
	if (x_joystick < 312) //left
	{
		if(user_col[0] != 0x7F)
		{
			for (unsigned i=0; i<4; i++)
			{
				 user_col[i] = (user_col[i] << 1) | 0x01;
			}
		}
	}
	else if(x_joystick >712) //right
	{
		if (user_col[0] != 0xFE && user_col[1] != 0xFE && user_col[3] != 0xFE)
		{
			for (unsigned i=0; i<4; i++)
			{
				 user_col[i] = (user_col[i] >> 1) | 0x80;
			}
		}
	}
	
    return state;    
};



enum Enemy_States {enemy_hunt, enemy_gameover};
int Enemy_Tick(int state)
{
	// == Local Variables ==
	unsigned char rand_cols[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
	unsigned char enemy_count_temp = enemy_count;
	
	// ==Transitions==
	switch(state) {
		case -1:	
			state = enemy_hunt;
			break;
		case enemy_hunt:
			if (!isgameover) { state = enemy_hunt; }
			else { state = enemy_gameover; }
			break;
		case enemy_gameover:
			if (isreset) { state = -1; iswaitreset = 0; }
			else { state = enemy_gameover; }
			break;
		default:
			state = enemy_hunt;
			break;
	}
	
	// ==Actions==
	switch(state) {
		case enemy_hunt:
			for (unsigned i=0; i<enemy_count; i++) //revive enemy
			{
				if (enemy_row[i]==0x00)
				{
					enemy_row[i] = 0x80;
					enemy_col[i] = ~rand_cols[rand() % 8];
					enemy_wait[i] = (rand() % 10)+3;
				}
			}
			for (unsigned i=0; i<enemy_count; i++) //move enemy
			{
				if (enemy_row[i] == 0x80 && enemy_wait[i]!=0) 
				{
					enemy_wait[i] = enemy_wait[i] - 1;
				}
				else 
				{
					enemy_row[i] = enemy_row[i] >> 1;
				}		
			}
			
			//COLLISION Detection
			for (unsigned i=0; i<enemy_count; i++) //enemy particle
			{
				for (unsigned j=0; j<4; j++) //user particles
				{
					if (enemy_col[i]==user_col[j] && enemy_row[i]==user_row[j])
					{
						isgameover = 1;
						ischangescreen = 1;
						break;
					}					
				}
			}
			break;
		case enemy_gameover:
			break;
		default:
			break;
	}

	return state;
}	
	
enum PlusBlock_States {plusblock_sit, plusblock_wait, plusblock_gameover};
int PlusBlock_Tick(int state)
{
	// == Local Variables ==
	unsigned char rand_rows[6] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20};
	unsigned char rand_cols[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
	static unsigned char iswaitupdate = 0;
	
	// ==Transitions==
	switch(state) {
		case -1:
			if (iswaitreset) { state = -1; }
			else { state = plusblock_sit; }
			break;
		case plusblock_sit:
			if (iswaitupdate > 0) { state = plusblock_wait; }
			else if (!isgameover) { state = plusblock_sit; }
			else { state = plusblock_gameover; }
			break;
		case plusblock_wait:
			if (iswaitupdate > 0) { state = plusblock_wait; }
			else if (!isgameover) { state = plusblock_sit; }
			else { state = plusblock_gameover; }
			break;
		case plusblock_gameover:
			if (isreset) { state = -1; iswaitreset = 1; }
			else { state = plusblock_gameover; }
			break;
		default:
			state = plusblock_sit;
			break;
	}

	// ==Actions==
	switch(state) {
		case plusblock_sit:
			for (unsigned char i=0; i<4; i++) //collision with plusblock
			{
				if (plusblock_col==user_col[i] && plusblock_row==user_row[i])
				{
					points++;
					if (points<=4) {enemy_count=1;}
					else if (points <= 8) { enemy_count = 2; }
					else if (points <= 16) {enemy_count = 3; }
					else if (points <= 32) {enemy_count = 4; }
					else if (points <= 64) {enemy_count = 5; }
					else { enemy_count = 6; }
					//if (enemy_count_temp != enemy_count) { ischangescreen = 1; }					
					iswaitupdate = 4;
					ischangescreen = 1;
				}
			}
			break;
		case plusblock_wait:
			iswaitupdate--;
			plusblock_row = 0x00;
			plusblock_col = 0xFF;
			if (iswaitupdate==0)
			{
				plusblock_row = rand_rows[rand() % 6];
				plusblock_col = ~rand_cols[rand() % 8];
		
				for (unsigned char j=0; j<4; j++) //make sure not already at user_block
				{
					if(plusblock_col == user_col[j])
					{
						if (plusblock_col==~0x01 || plusblock_col==~0x02)
						{
							plusblock_col = ~0x80;
						}
						else
						{
							plusblock_col = ~0x01;
						}
						break;
					}
				}
			}		
			break;
		case plusblock_gameover:
			break;
		default:
			break;
	}

	return state;	
}


enum Output_States {mat_dis_user, mat_dis_enemy, mat_dis_plusblock, mat_dis_gameover};
int Output_Tick(int state)
{
	// == Local Variables ==
	/*unsigned tmpC = 0x00; //row -> Use transmit_data()
	unsigned tmpD = 0xFF; //red column
	unsigned tmpB = 0xFF; //blue column*/
	static unsigned output_user_count = 4;
	static unsigned output_plusblock_count = 1;
	static unsigned char count = 0;
	
	// == Transitions ==
	switch (state) {
		case -1:
			if (iswaitreset) { state = -1; }
			else
			{
				reset_globals();
				Display_Initiate_Ingame();
				state = mat_dis_user;			
			}
			break;
		case mat_dis_user:
			if (isgameover) { state = mat_dis_gameover; }
			else if (count < output_user_count) 
			{
				state = mat_dis_user;
			}
			else 
			{
				state = mat_dis_plusblock;
				count = 0;
			}				
			break;
		case mat_dis_plusblock:
			if (isgameover) { state = mat_dis_gameover; }
			else if (count < output_plusblock_count)
			{
				state = mat_dis_plusblock;
			}
			else
			{
				state = mat_dis_enemy;
				count = 0;
			}
			break;
		case mat_dis_enemy:
			if (isgameover) { state = mat_dis_gameover; }
			else if (count < enemy_count)
			{
				state = mat_dis_enemy;
			}
			else
			{
				state = mat_dis_user;
				count = 0;
			}
			break;
		case mat_dis_gameover:
			if (isreset){ state = -1; }
			else { state = mat_dis_gameover; }				
			break;
		default:
			state = mat_dis_user;
			break;
	}
	
	// == Actions ==
	switch (state) {
		case mat_dis_user:
			PORTD = 0xFF;
			transmit_data(user_row[count]);
			PORTB = user_col[count];
			count++;
			break;
		case mat_dis_plusblock:
			PORTD = 0xFF;
			transmit_data(plusblock_row);
			PORTB = plusblock_col;
			count ++;
			break;
		case mat_dis_enemy:
			PORTB = 0xFF;
			transmit_data(enemy_row[count]);
			PORTD = enemy_col[count];
			count++;
			break;
		case mat_dis_gameover:
			PORTB = 0xFF;
			transmit_data(points);
			PORTD = 0x00;
			break;
		default:
			break;
	}
	if(!isgameover)
	{
		if (ischangescreen)
		{
			Display_Update_Ingame();
			ischangescreen = 0;
		}			
	}
	else 
	{
		if (ischangescreen)
		{
			Display_Gameover();
			ischangescreen = 0;
		}
	}
	
	return state;
}


//--------Find GCD function --------------------------------------------------
unsigned long int findGCD(unsigned long int a, unsigned long int b)
{
        unsigned long int c;
        while(1){
                c = a%b;
                if(c==0){return b;}
                a = b;
b = c;
        }
        return 0;
}
//--------End find GCD function ----------------------------------------------

//--------Task scheduler data structure---------------------------------------
// Struct for Tasks represent a running process in our simple real-time operating system.
typedef struct _task {
        /*Tasks should have members that include: state, period,
                a measurement of elapsed time, and a function pointer.*/
        signed char state; //Task's current state
        unsigned long int period; //Task period
        unsigned long int elapsedTime; //Time elapsed since last task tick
        int (*TickFct)(int); //Task tick function
} task;

//--------End Task scheduler data structure-----------------------------------


int main()
{
        DDRB = 0xFF; PORTB = 0x00; // PORTB set to output, outputs init 0s
		DDRD = 0xFF; PORTD = 0x00;
		DDRC = 0xFF; PORTC = 0x00;
		
		DDRA = 0x00; PORTA = 0xFF; //joystick input
		
		// Initializes the LCD display
		LCD_init();
		LCD_ClearScreen();

		//LCDdefinechar(Star_atop,0);				
		LCDdefinechar(Skull, 0);
		LCDdefinechar(Star_btop,1);
		LCDdefinechar(Star_ctop,2);
		LCDdefinechar(Star_dtop,3);
		LCDdefinechar(Star_abot,4);
		LCDdefinechar(Star_bbot,5);
		LCDdefinechar(Star_cbot,6);
		LCDdefinechar(Star_dbot,7);
			
		ADC_init();

        // Period for the tasks
		unsigned long int Output_Tick_calc = 1;
		unsigned long int User_Tick_calc = 35;
		unsigned long int PlusBlock_Tick_calc = 35;
		unsigned long int Enemy_Tick_calc = 135;
		
        //Calculating GCD
        unsigned long int tmpGCD = 1;
		tmpGCD = findGCD(Output_Tick_calc, User_Tick_calc);
		tmpGCD = findGCD(tmpGCD, PlusBlock_Tick_calc);
        tmpGCD = findGCD(tmpGCD, Enemy_Tick_calc);
		
        //Greatest common divisor for all tasks or smallest time unit for tasks.
        unsigned long int GCD = tmpGCD;

        //Recalculate GCD periods for scheduler
		unsigned long int Output_Tick_period = Output_Tick_calc/GCD;
		unsigned long int User_Tick_period = User_Tick_calc/GCD;
		unsigned long int PlusBlock_Tick_period = PlusBlock_Tick_calc/GCD;
		unsigned long int Enemy_Tick_period = Enemy_Tick_calc/GCD;
		
        //Declare an array of tasks 
        static task task1, task2, task3, task4;
        task *tasks[] = { &task1, &task2, &task3, &task4};
        const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

		// == Tasks ==
		//Output Task
		task1.state = -1;
		task1.period = Output_Tick_period;
		task1.elapsedTime = Output_Tick_period;
		task1.TickFct = &Output_Tick;
		
		//User Update Task
		task2.state = -1;
		task2.period = User_Tick_period;
		task2.elapsedTime = User_Tick_period;
		task2.TickFct = &User_Tick;
		
		//PlusBlock Task
		task3.state = -1;
		task3.period = PlusBlock_Tick_period;
		task3.elapsedTime = PlusBlock_Tick_period;
		task3.TickFct = &PlusBlock_Tick;
		
		//Enemy Task
		task4.state = -1;
		task4.period = Enemy_Tick_period;
		task4.elapsedTime = Enemy_Tick_period;
		task4.TickFct = &Enemy_Tick;
		
        // Set the timer and turn it on
        TimerSet(GCD);
        TimerOn();

        unsigned short i; // Scheduler for-loop iterator
        while(1) {
                // Scheduler code
                for ( i = 0; i < numTasks; i++ ) {
                        // Task is ready to tick
                        if ( tasks[i]->elapsedTime == tasks[i]->period ) {
                                // Setting next state for task
                                tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
                                // Reset the elapsed time for next tick.
                                tasks[i]->elapsedTime = 0;
                        }
                        tasks[i]->elapsedTime += 1;
                }
                while(!TimerFlag);
                TimerFlag = 0;
        }

        // Error: Program should not exit!
        return 0;
}