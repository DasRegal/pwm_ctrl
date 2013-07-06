#include <unistd.h>
#include "gpio.h"
#include <stdlib.h>
#include <stdio.h>
#include "pwm.h"
#include "msp_pwm.h"

#include <curses.h>
#include <iostream>

// 30 вывод
#define MISO 114
// 34 вывод
#define MOSI 105
// 31 вывод
#define CLK 87

int buf[5];

	#define		ROLL_SET(x) 	(buf[0] = x * 10 + 1000 + 57)
	#define		PITCH_SET(x) 	(buf[1] = x * 10 + 1000 + 57)
	#define 	THROTTLE_SET(x)	(buf[2] = x * 10 + 1000 + 57)
	#define 	YAW_SET(x)		(buf[3] = x * 10 + 1000 + 57)
	#define 	MODE_SET(x) 	(buf[4] = x * 10 + 1000 + 57)


int* g_Prc;
int g_Prc1;
int g_Prc2;
int g_Prc3;
int g_Prc4;
int g_Prc5;

void Init(void)
{
	g_Prc1 = 50;
	g_Prc2 = 50;
	g_Prc3 = 0;
	g_Prc4 = 50;
	g_Prc5 = 50;
}

void KeyUP(int* prc)
{
	*prc += 10;
	if(*prc > 100)
		*prc = 100;
		
}

void KeyDOWN(int* prc)
{
	*prc -= 10;
	if(*prc < 0)
		*prc = 0;
}

int main()
{
	int x;
	InitSPI2PWM(MISO, MOSI, CLK);

	ROLL_SET(50);
	PITCH_SET(50);
	THROTTLE_SET(0);
	YAW_SET(110);
	MODE_SET(0);
	SetSPI2PWM(buf, 5);
		
	Init();

	for (int i = 0; i < 7000; i++) usleep(1000);
	YAW_SET(50);
	SetSPI2PWM(buf, 5);
	// printf("Start\n");

	// THROTTLE_SET(60);
	// SetSPI2PWM(buf, 5);
	// for (int i = 0; i < 2000; i++) usleep(1000);

	// MODE_SET(100);
	// SetSPI2PWM(buf, 5);

	// PITCH_SET(40);
	// SetSPI2PWM(buf, 5);
	// for (int i = 0; i < 2000; i++) usleep(1000);
	// PITCH_SET(50);
	// SetSPI2PWM(buf, 5);

	// for (int i = 50; i >= 0; i = i - 10)
	// {
	// 	for (int i = 0; i < 1000; i++) usleep(1000);
	// 	THROTTLE_SET(i);
	// 	SetSPI2PWM(buf, 5);
	// }

	// YAW_SET(0);
	// SetSPI2PWM(buf, 5);
	// for (int i = 0; i < 5000; i++) usleep(1000);
	// YAW_SET(50);
	// SetSPI2PWM(buf, 5);

	int t = 5;
	int dt;
	int dprc = 0;
	
	dt = (t * 1000 * 1000) / 100;
	
	// for (int i = 0; i < (t * 1000 * 1000); i += dt)
	// {
	// 	YAW_SET(dprc);
	// 	SetSPI2PWM(buf, 5);
	// 	dprc++;
	// 	usleep(dt);
	// }
	

	initscr();	
	raw();
	keypad(stdscr, TRUE);
	noecho();
	
	printw("GO!");

	bool loop;
	loop = true;
	char ch;
	int chan;
	chan = 1;
	g_Prc = &g_Prc1;
	while( loop == true)
	{
		ch = getch();
		switch (ch)
		{
			case 'w': KeyUP(g_Prc); printw("%d ", *g_Prc); break;
			case 's': KeyDOWN(g_Prc); printw("%d ", *g_Prc); break;
			case 'a': MODE_SET(0);   break;
			case 'd': MODE_SET(100); break;
			case '1': printw("Roll: "); chan = 1; break;
			case '2': printw("Pitch: "); chan = 2; break;
			case '3': printw("Throttle: "); chan = 3; break;
			case '4': printw("Yaw: "); chan = 4; break;
			case '5': printw("Mode: "); chan = 5; break;
			case 'q':
			case 'Q':loop = false;
		}
		
		switch (chan)
		{
			case 1: g_Prc = &g_Prc1; ROLL_SET(*g_Prc); break;
			case 2: g_Prc = &g_Prc2; PITCH_SET(*g_Prc); break;
			case 3: g_Prc = &g_Prc3; THROTTLE_SET(*g_Prc); break;
			case 4: g_Prc = &g_Prc4; YAW_SET(*g_Prc); break;
			case 5: g_Prc = &g_Prc5; MODE_SET(*g_Prc); break;
		}
		SetSPI2PWM(buf, 5);
	}
	refresh();
	getch();
	endwin();

	return 0;
}
