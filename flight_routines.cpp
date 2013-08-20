#include "flight_routines.h"
#include "msp_pwm.h"
#include "main.h"
#include "sonar.h"

#include <curses.h>
#include <unistd.h>
#include <stdio.h>

int  chan_buf[5];

void Armed(void)
{
	ROLL_SET(FR_ROLL);
	PITCH_SET(FR_PITCH);
	THROTTLE_SET(0);
	YAW_SET(FR_YAW);
	MODE_SET(0);
	
	Delay(1);
	YAW_SET(100);
	
	Delay(4);
	YAW_SET(FR_YAW);
}

void LiftUp(int throttle, char time, int alt)
{
	// THROTTLE_SET(40);
	// Delay(1);
	// for (int i = 0; i < 500; ++i)
	// {
	// 	usleep(1000);
	// }
	// THROTTLE_SET(throttle);
	// Delay(time);
		
	MODE_SET(100);
	printf("Режим.\n");
	THROTTLE_SET(alt);
	// Delay(1);
}

void FlyForward(int pitch, char time)
{
	PITCH_SET(pitch);

	Delay(time);

	// PITCH_SET(pitch - FR_PITCH);
	// Delay(1);
	PITCH_SET(FR_PITCH);
}

void LiftDown(int throttle, char time)
{
	MODE_SET(0);

	while(throttle > 0)
	{

		throttle -= 7;
		if (throttle < 40 )
		{
			throttle = 0;
			THROTTLE_SET(throttle);
			break;
		}
		THROTTLE_SET(throttle);
		for (int i = 0; i < 100; ++i)
		{
			usleep(1000);
		}
		throttle += 7;
		THROTTLE_SET(throttle);
		for (int i = 0; i < 100; ++i)
		{
			usleep(1000);
		}
		throttle -= 1;
		if (throttle < 40 )
		{
			throttle = 0;
			THROTTLE_SET(throttle);
			break;
		}
	}

	// THROTTLE_SET(throttle);
	// Delay(time);

	// THROTTLE_SET(throttle+4);
	// Delay(1);

	THROTTLE_SET(0);
}

void ArmMode(void)
{
	initscr();	
	raw();
	keypad(stdscr, TRUE);
	noecho();
	
	printw("GO!");

	bool loop;
	loop = true;
	char ch;
	int g_chan[4];
	// ROLL
	g_chan[0] = 50;
	// PITCH
	g_chan[1] = 50;
	// THROTTLE
	g_chan[2] = 0;
	// YAW
	g_chan[3] = 50;
	while( loop == true)
	{
		ch = getch();
		switch (ch)
		{
			case 'w': 
			{
				g_chan[2] += 10;
				if (g_chan[2] >= 100) g_chan[2] = 100;
				printw("T:%d ", g_chan[2]);
				THROTTLE_SET(g_chan[2]);
				break;
			}
			case 's':
			{
				g_chan[2] -= 10;
				if (g_chan[2] <= 0) g_chan[2] = 0;
				printw("T:%d ", g_chan[2]);
				THROTTLE_SET(g_chan[2]);
				break;
			}
			case 'a': 
			{
				g_chan[0] -= 1;
				if (g_chan[0] <= 0) g_chan[0] = 0;
				printw("Y:%d ", g_chan[0]);
				YAW_SET(g_chan[0]);
				break;
				
			}
			case 'd':
			{
				g_chan[0] += 1;
				if (g_chan[0] >= 100) g_chan[0] = 100;
				printw("Y:%d ", g_chan[0]);
				YAW_SET(g_chan[0]);
				break;
			}
			
			case 'k': 
			{
				g_chan[1] += 1;
				if (g_chan[1] >= 100) g_chan[1] = 100;
				printw("P:%d ", g_chan[1]);
				PITCH_SET(g_chan[1]);
				break;
			}
			case 'i':
			{
				g_chan[1] -= 1;
				if (g_chan[1] <= 0) g_chan[1] = 0;
				printw("P:%d ", g_chan[1]);
				PITCH_SET(g_chan[1]);
				break;
			}
			case 'j': 
			{
				g_chan[3] -= 1;
				if (g_chan[3] <= 0) g_chan[3] = 0;
				printw("R:%d ", g_chan[3]);
				ROLL_SET(g_chan[3]);
				break;
				
			}
			case 'l':
			{
				g_chan[3] += 1;
				if (g_chan[3] >= 100) g_chan[3] = 100;
				printw("R:%d ", g_chan[3]);
				ROLL_SET(g_chan[3]);
				break;
			}
			case 'f': printw("Mode stabilize. "); MODE_SET(0);   break;
			case 'g': printw("Mode Althold. "); MODE_SET(100); break;
			case 'z': printw("ThrUP. "); g_chan[2] = 100; THROTTLE_SET(g_chan[2]); break;
			case 'c': printw("ThrDOWN. "); g_chan[2] = 0; THROTTLE_SET(g_chan[2]); break;
			case '[': printw("Start... "); THROTTLE_SET(0); YAW_SET(100); Delay(4); YAW_SET(50); printw("Ok. "); break;
			case ']': printw("Stop... "); THROTTLE_SET(0); YAW_SET(0); Delay(4); YAW_SET(50); printw("Ok. "); break;
			case 'r': {
					printw("Reset... ");
					g_chan[0] = 50;
					g_chan[1] = 50;
					g_chan[2] = 0;
					g_chan[3] = 50;
					ROLL_SET(g_chan[0]);
					PITCH_SET(g_chan[1]);
					THROTTLE_SET(g_chan[2]);
					YAW_SET(g_chan[3]);
					MODE_SET(0);
					printw("Ok. ");
					break;
			}
			case 'q':
			case 'Q':loop = false;
		}		
	}
	refresh();
	endwin();
}

void Disarmed(void)
{
	THROTTLE_SET(0);
	YAW_SET(0);
	Delay(4);
	YAW_SET(FR_YAW);
}