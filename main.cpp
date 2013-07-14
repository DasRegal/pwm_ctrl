#include <unistd.h>
#include "gpio.h"
#include <stdlib.h>
#include <stdio.h>
#include "pwm.h"
#include "msp_pwm.h"
#include "cd74ac153.h"
#include "sonar.h"
#include <string.h>

#include <curses.h>
#include <iostream>

	#define		SONAR1	LINE1		
	#define		SONAR2	LINE2		
	#define		SONAR3	LINE3

	CSonar sonar;

// 30 вывод
#define MISO 114
// 34 вывод
#define MOSI 105
// 31 вывод
#define CLK 87

int buf[5];

	#define		ROLL_SET(x) 	(buf[0] = x * 10 + 1000 + 230)
	#define		PITCH_SET(x) 	(buf[1] = x * 10 + 1000 + 230)
	#define 	THROTTLE_SET(x)	(buf[2] = x * 10 + 1000 + 230)
	#define 	YAW_SET(x)		(buf[3] = x * 10 + 1000 + 230)
	#define 	MODE_SET(x) 	(buf[4] = x * 10 + 1000 + 230)

int* g_Prc;
int g_Prc1;
int g_Prc2;
int g_Prc3;
int g_Prc4;
int g_Prc5;

int ReadSonar(ESwLine line)
{
	cd74ac.SwitchLine(line);
	return sonar.ReadValue();
}

void Delay(int time)
{
	for (int i = 0; i < time*1000; i++) usleep(1000);
}

void Init(void)
{
	g_Prc1 = 50;
	g_Prc2 = 50;
	g_Prc3 = 0;
	g_Prc4 = 50;
	g_Prc5 = 50;
	sonar.Create("ttySAC3");
	cd74ac.Init();	
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

int main(int argc, char const *argv[])
{
	int x;
	InitSPI2PWM(MISO, MOSI, CLK);


	Init();

	if (argc == 1)
	{
		printf("No params\n");
		return 1;
	}

	//==========================================
	//				    ARMED
	//==========================================
	if ((argc > 1) && strcmp(argv[1], "nostart") != 0)
	{	
		printf("Armed\n");
		Delay(5);
		ROLL_SET(35);
		PITCH_SET(34);
		THROTTLE_SET(0);
		YAW_SET(40);
		MODE_SET(0);
		SetSPI2PWM(buf, 5);
	
		Delay(1);
		YAW_SET(100);
		SetSPI2PWM(buf, 5);
	
		Delay(4);
		YAW_SET(40);
		SetSPI2PWM(buf, 5);
		printf("Start\n");
	}

	//==========================================
	// 					ВЗЛЕТ
	//==========================================
	if ((argc > 1) && (strcmp(argv[1],"kt3") == 0 || strcmp(argv[1], "sonar") == 0))
	{
		printf("Взлет\n");
		THROTTLE_SET(75);
		SetSPI2PWM(buf, 5);
		Delay(2);

		MODE_SET(100);
		SetSPI2PWM(buf, 5);

		// Вперед лететь
		PITCH_SET(23);
		SetSPI2PWM(buf, 5);
	}

	if ((argc > 1) && !strcmp(argv[1], "test"))
	{
		int data;
		while(1)
		{
			data = ReadSonar(SONAR3);
			printf("Data: - %d\n", data);
		}
	}
	//==========================================
	// 			ПОЛЕТ ПО СОНАРАМ
	//==========================================
	if ((argc > 1) && !strcmp(argv[1], "sonar"))
	{	
		printf("Полет по сонарам\n");
		int data;
		while(1)
		{
			data = ReadSonar(SONAR3);
			printf("Data: - %d\n", data);
			if (data > 2500 && data < 4000)
			{	
				YAW_SET(60);
				SetSPI2PWM(buf, 5);
			}
			else
			if (data < 1500)
			{
				YAW_SET(40);
				SetSPI2PWM(buf, 5);
			}
			else break;
		}
	}

	//==========================================
	// 				ПОЛЕТ ВПЕРЕД
	//==========================================
	if ((argc > 1) && !strcmp(argv[1], "kt3"))
	{
		printf("Полет вперед\n");
		Delay(8);
	}

	//==========================================
	// 					ПОСАДКА
	//==========================================
	if ((argc > 1) && (!strcmp(argv[1], "kt3") || !strcmp(argv[1], "sonar")))
	{
		PITCH_SET(37);
		SetSPI2PWM(buf, 5);
		Delay(1);

		printf("Посадка\n");
		PITCH_SET(34);
		MODE_SET(0);
		SetSPI2PWM(buf, 5);
	
		THROTTLE_SET(62);
		SetSPI2PWM(buf, 5);
		Delay(2);
		
		THROTTLE_SET(0);
		SetSPI2PWM(buf, 5);
	}
	
	//==========================================
	//		   		РУЧНОЙ РЕЖИМ
	//==========================================
	if (argc > 1 && strcmp(argv[1], "arm") == 0)
	{
		printf("Ручной режим\n");
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
				case 'a': printw("Mode stabilize. "); MODE_SET(0);   break;
				case 'd': printw("Mode Althold. "); MODE_SET(100); break;
				case 'z': printw("ThrUP. "); THROTTLE_SET(100); SetSPI2PWM(buf, 5); break;
				case 'c': printw("ThrDOWN. "); THROTTLE_SET(0); SetSPI2PWM(buf, 5); break;
				case '1': printw("Roll: "); chan = 1; break;
				case '2': printw("Pitch: "); chan = 2; break;
				case '3': printw("Throttle: "); chan = 3; break;
				case '4': printw("Yaw: "); chan = 4; break;
				case '5': printw("Mode: "); chan = 5; break;
				case '[': printw("Start... "); THROTTLE_SET(0); YAW_SET(100); SetSPI2PWM(buf, 5); Delay(4); YAW_SET(50); SetSPI2PWM(buf, 5); printw("Ok. "); break;
				case ']': printw("Stop... "); THROTTLE_SET(0); YAW_SET(0); SetSPI2PWM(buf, 5); Delay(4); YAW_SET(50); SetSPI2PWM(buf, 5); printw("Ok. "); break;
				case 'r': {
						printw("Reset... ");
						ROLL_SET(50);
						PITCH_SET(50);
						THROTTLE_SET(0);
						YAW_SET(50);
						MODE_SET(0);
						SetSPI2PWM(buf, 5);
						printw("Ok. ");
						break;
				}
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
	}

	//==========================================
	// 				  DISARMED
	//==========================================
	printf("Disarmed\n");
	THROTTLE_SET(0);
	YAW_SET(0);
	SetSPI2PWM(buf, 5);
	Delay(4);
	YAW_SET(40);
	SetSPI2PWM(buf, 5);

	return 0;
}
