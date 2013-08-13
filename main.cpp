#include <unistd.h>
#include "gpio.h"
#include <stdlib.h>
#include <stdio.h>
#include "pwm.h"
#include "msp_pwm.h"
#include "cd74ac153.h"
#include "sonar.h"
#include <string.h>
#include "flight_routines.h"
#include "main.h"
#include "spi.h"

#include "find.h"
#include "define.h"

#include <curses.h>
#include <iostream>
#include <ctime>

// #include "mavlink/include/mavlink/v1.0/common/mavlink.h"
#include "mavlink/include/mavlink/v1.0/common/mavlink.h"
#include "mavlink/include/mavlink/v1.0/mavlink_types.h"

// Standard includes
#include <cstdlib>
#include <cmath>
#include <inttypes.h>
#include <fstream>
// Serial includes
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#ifdef __linux
#include <sys/ioctl.h>
#endif
// Latency Benchmarking
#include <sys/time.h>
#include <time.h>


	#define		SONAR1	0
	#define		R_SONAR	1
	#define		F_SONAR	2
	#define 	SONAR4	3
	#define 	SONAR5	4

// 27 вывод
#define MISO 88
// // 28 вывод
// #define CS1 98
// 30 вывод
#define MOSI 114
// 18 вывод
#define CLK 115
// 34 вывод
#define CS1	105
// 31 вывод
#define CS2	87

int* g_Prc;
int g_Prc1;
int g_Prc2;
int g_Prc3;
int g_Prc4;
int g_Prc5;

	int cs2 = spi.InitCS(CS1);
	int cs1 = spi.InitCS(CS2);

float ReadSonar(int line)
{
	int val;
	spi.ClrCS(cs1);
	spi.WriteByte(line);
	val = spi.ReadByte();
	spi.SetCS(cs1);

	float k;
	if (line == SONAR1) return val;//val *= 1.98;
	else val = val * 1.55 + 2.4;
	
	return val;
}

void Init(void)
{
	g_Prc1 = 50;
	g_Prc2 = 50;
	g_Prc3 = 0;
	g_Prc4 = 50;
	g_Prc5 = 50;
	spi.Init(MISO, MOSI, CLK);
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
	if (argc == 1)
	{
		printf("No params\n");
		return 1;
	}

	Init();

	//==========================================
	//				    ARMED
	//==========================================
	if ((argc > 1) && strcmp(argv[1], "test") != 0)
	{	
		printf("Armed\n");
		Armed();
	}


	//==========================================
	// 					ВЗЛЕТ
	//==========================================
	if ((argc > 1) && (strcmp(argv[1],"kt3") == 0))
	{
		printf("Взлет\n");

		// Throttle, Time, Alt
		LiftUp(74, 3, 65);	
	}

	//==========================================
	//				ПОЛЕТ ВПЕРЕД
	//==========================================
	if ((argc > 1) && (strcmp(argv[1],"kt3") == 0))
	{
		printf("Вперед\n");
		PITCH_SET(FR_PITCH - 15);
		Delay(3);
		PITCH_SET(FR_PITCH);
		Delay(1);
	}

	//==========================================
	// 				ТЕСТ СОНАРОВ
	//==========================================
	if ((argc > 1) && !strcmp(argv[1], "test"))
	{
		printf("Тест сонаров\n");
		float data;
		while(1)
		{
			data = ReadSonar(SONAR1);
			printf("Нижний сонар: - %f : ", data);
			printf("\n");
			// data = ReadSonar(R_SONAR);
			// printf("Правый сонар: - %f : ", data);
			// data = ReadSonar(F_SONAR);
			// printf("Передний сонар: - %f\n", data);
			for (int i = 0; i < 300; ++i)
			{
				usleep(1000);
			}
		}
	}

	//==========================================
	// 			Взлет и поворот на месте
	//==========================================
	if ((argc > 1) && !strcmp(argv[1], "turn"))
	{
		printf("Turn\n");

		// Throttle, Time, Alt
		LiftUp(70, 2, 63);
		YAW_SET(FR_YAW + 5);
		Delay(1);
	}	

	//==========================================
	// 					ПОСАДКА
	//==========================================
	if ((argc > 1) && (!strcmp(argv[1], "kt3") || !strcmp(argv[1], "sonar") || !strcmp(argv[1], "cam") || strcmp(argv[1], "turn") == 0))
	{
		printf("Посадка\n");

		// PITCH_SET(56);
		// Delay(1);
		// PITCH_SET(FR_PITCH);
		// Throttle, Time
		LiftDown(65, 1);
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
				// case 'w': KeyUP(g_Prc); printw("%d ", *g_Prc); break;
				// case 's': KeyDOWN(g_Prc); printw("%d ", *g_Prc); break;
				case 'w': 
				{
					g_chan[2] += 10;
					if (g_chan[2] >= 100) g_chan[2] = 100;
					printw("%d ", g_chan[2]);
					THROTTLE_SET(g_chan[2]);
					//SetSPI2PWM(chan_buf, 5);
					break;
				}
				case 's':
				{
					g_chan[2] -= 10;
					if (g_chan[2] <= 0) g_chan[2] = 0;
					printw("%d ", g_chan[2]);
					THROTTLE_SET(g_chan[2]);
					//SetSPI2PWM(chan_buf, 5);
					break;
				}
				case 'a': 
				{
					g_chan[0] -= 10;
					if (g_chan[0] <= 0) g_chan[0] = 0;
					printw("%d ", g_chan[0]);
					YAW_SET(g_chan[0]);
					//SetSPI2PWM(chan_buf, 5);
					break;
					
				}
				case 'd':
				{
					g_chan[0] += 10;
					if (g_chan[0] >= 100) g_chan[0] = 100;
					printw("%d ", g_chan[0]);
					YAW_SET(g_chan[0]);
					//SetSPI2PWM(chan_buf, 5);
					break;
				}
				
				case 'k': 
				{
					g_chan[1] += 10;
					if (g_chan[1] >= 100) g_chan[1] = 100;
					printw("%d ", g_chan[1]);
					PITCH_SET(g_chan[1]);
					//SetSPI2PWM(chan_buf, 5);
					break;
				}
				case 'i':
				{
					g_chan[1] -= 10;
					if (g_chan[1] <= 0) g_chan[1] = 0;
					printw("%d ", g_chan[1]);
					PITCH_SET(g_chan[1]);
					//SetSPI2PWM(chan_buf, 5);
					break;
				}
				case 'j': 
				{
					g_chan[3] -= 10;
					if (g_chan[3] <= 0) g_chan[3] = 0;
					printw("%d ", g_chan[3]);
					ROLL_SET(g_chan[3]);
					//SetSPI2PWM(chan_buf, 5);
					break;
					
				}
				case 'l':
				{
					g_chan[3] += 10;
					if (g_chan[3] >= 100) g_chan[3] = 100;
					printw("%d ", g_chan[3]);
					ROLL_SET(g_chan[3]);
					//SetSPI2PWM(chan_buf, 5);
					break;
				}
				case 'f': printw("Mode stabilize. "); MODE_SET(0);   break;
				case 'g': printw("Mode Althold. "); MODE_SET(100); break;
				case 'z': printw("ThrUP. "); THROTTLE_SET(100); /*SetSPI2PWM(chan_buf, 5);*/ break;
				case 'c': printw("ThrDOWN. "); THROTTLE_SET(0); /*SetSPI2PWM(chan_buf, 5);*/ break;
				case '1': printw("Roll: "); chan = 1; break;
				case '2': printw("Pitch: "); chan = 2; break;
				case '3': printw("Throttle: "); chan = 3; break;
				case '4': printw("Yaw: "); chan = 4; break;
				case '5': printw("Mode: "); chan = 5; break;
				case '[': printw("Start... "); THROTTLE_SET(0); YAW_SET(100); /*SetSPI2PWM(chan_buf, 5);*/ Delay(4); YAW_SET(50); /*SetSPI2PWM(chan_buf, 5);*/ printw("Ok. "); break;
				case ']': printw("Stop... "); THROTTLE_SET(0); YAW_SET(0); /*SetSPI2PWM(chan_buf, 5);*/ Delay(4); YAW_SET(50); /*SetSPI2PWM(chan_buf, 5);*/ printw("Ok. "); break;
				case 'r': {
						printw("Reset... ");
						ROLL_SET(50);
						PITCH_SET(50);
						THROTTLE_SET(0);
						YAW_SET(50);
						MODE_SET(0);
						//SetSPI2PWM(chan_buf, 5);
						printw("Ok. ");
						break;
				}
				case 'q':
				case 'Q':loop = false;
			}
			
			switch (chan)
			{
				//case 1: g_Prc = &g_Prc1; ROLL_SET(*g_Prc); break;
				//case 2: g_Prc = &g_Prc2; PITCH_SET(*g_Prc); break;
				//case 3: g_Prc = &g_Prc3; THROTTLE_SET(*g_Prc); break;
				//case 4: g_Prc = &g_Prc4; YAW_SET(*g_Prc); break;
				//case 5: g_Prc = &g_Prc5; MODE_SET(*g_Prc); break;
			}
			//SetSPI2PWM(chan_buf, 5);
		}
		refresh();
		getch();
		endwin();
	}

	//==========================================
	// 				  DISARMED
	//==========================================
	printf("Disarmed\n");
	Disarmed();

	return 0;
}

void Delay(int time)
{
	for (int i = 0; i < time*1000; i++) usleep(1000);
}