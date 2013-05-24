#include <unistd.h>
#include "gpio.h"
#include <stdlib.h>
#include <stdio.h>
#include "pwm.h"


#include <curses.h>

#include <iostream>



// 30 вывод
#define PWM_CHAN1 114
// 18 вывод 
#define PWM_CHAN2 115
// 34 вывод
#define PWM_CHAN3 105
// 31 вывод
#define PWM_CHAN4 87

int* g_Prc;
int g_Prc1;
int g_Prc2;
int g_Prc3;
int g_Prc4;

void Init(void)
{
	g_Prc1 = 50;
	g_Prc2 = 50;
	g_Prc3 = 50;
	g_Prc4 = 50;
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
	CPWMCtrl roll;
	CPWMCtrl pitch;
	CPWMCtrl throttle;
	CPWMCtrl yaw;

	roll.Create(PWM_CHAN1);
	roll.Setdt(50);
	
	pitch.Create(PWM_CHAN2);
	pitch.Setdt(50); 

	throttle.Create(PWM_CHAN3);
	throttle.Setdt(0);
	
	yaw.Create(PWM_CHAN4);
	yaw.Setdt(50);
	
	Init();

	//yaw.Setdt(100);
	//for (int i = 0; i < 5000; i++) usleep(1000);
	yaw.Setdt(0);

	int t = 5;
	int dt;
	int dprc = 0;
	
	dt = (t * 1000 * 1000) / 100;
	
	for (int i = 0; i < (t * 1000 * 1000); i += dt)
	{
		yaw.Setdt(dprc);
		dprc++;
		usleep(dt);
	}
	

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
			case /*KEY_UP*/ 'w': KeyUP(g_Prc); printw("%d ", *g_Prc); break;
			case /*KEY_DOWN*/ 's': KeyDOWN(g_Prc); printw("%d ", *g_Prc); break;
			//case KEY_LEFT: printw("LEFT\n"); break;
			//case KEY_RIGHT: printw("RIGHT\n"); break;
			case '1': printw("Chan 1: "); chan = 1; break;
			case '2': printw("Chan 2: "); chan = 2; break;
			case '3': printw("Chan 3: "); chan = 3; break;
			case '4': printw("Chan 4: "); chan = 4; break;
			case 'q':
			case 'Q':loop = false;
		}
		
		switch (chan)
		{
			case 1: g_Prc = &g_Prc1; roll.Setdt(*g_Prc); break;
			case 2: g_Prc = &g_Prc2; pitch.Setdt(*g_Prc); break;
			case 3: g_Prc = &g_Prc3; throttle.Setdt(*g_Prc); break;
			case 4: g_Prc = &g_Prc4; yaw.Setdt(*g_Prc); break;
		}
	}
	refresh();
	getch();
	endwin();

	return 0;
}