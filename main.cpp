#include <unistd.h>
#include "gpio.h"
#include <stdlib.h>
#include <stdio.h>
#include "pwm.h"
#include "msp_pwm.h"
#include "cd74ac153.h"
#include "sonar.h"
#include <string.h>
#include "find2.h"

#include <curses.h>
#include <iostream>

// #include "opencv2/objdetect/objdetect.hpp"
// #include "opencv2/highgui/highgui.hpp"
// #include "opencv2/imgproc/imgproc.hpp"

// using namespace cv;

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
	
	CvCapture* capture = 0;	
	capture = cvCreateCameraCapture(3);
	if (!capture)
	{
		return EXIT_FAILURE;
	}
	int pmin=50,pmax=100;

	double fps = 15;
	const char *filename2 = "/home/vc3.avi";
	IplImage* frame = cvQueryFrame( capture );
	CvSize size = cvSize(frame->width, frame->height);
    CvVideoWriter * writer = cvCreateVideoWriter(filename2, CV_FOURCC('X','V','I','D'), fps, size, 1);
	assert(writer!=0);
	double* xy;
	double* xy2;

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
		// Delay(6);
		// ROLL_SET(24);
		ROLL_SET(50);
		// PITCH_SET(6);
		PITCH_SET(50);
		THROTTLE_SET(0);
		// YAW_SET(26);
		YAW_SET(50);
		MODE_SET(0);
		SetSPI2PWM(buf, 5);
	
		Delay(1);
		YAW_SET(100);
		SetSPI2PWM(buf, 5);
	
		Delay(4);
		// YAW_SET(26);
		YAW_SET(50);
		SetSPI2PWM(buf, 5);
		printf("Start\n");
	}


	//==========================================
	// 					CAMTEST
	//==========================================
	// if ((argc > 1) && strcmp(argv[1], "camtest") == 0)
	// {
	// 	printf("Camtest\n");
	// 	THROTTLE_SET(3);
	// 	SetSPI2PWM(buf, 5);
	// 	int kk = 0;
	// 	int ll = 0;
	// 	int mm = 0;
	// 	while(1)
	// 	{	
	// 			// ll++;

	// 			xy=find(capture, pmin, pmax, writer);

	// 			// if (ll == 65535)
	// 			// {
					
	// 				// ll = 0;
	// 				// mm++;
	// 				if ((int)xy[0] != (int)-1)
	// 				{
	// 					if (xy[2] < -0.2)
	// 					{
	// 						ROLL_SET(0);
	// 						SetSPI2PWM(buf, 5);
	// 						printf("Лево\n");
	// 					}
	// 					if (xy[2] >= -0.2 && xy[2] <= 0.2)
	// 					{
	// 						printf("Центр\n");
	// 						ROLL_SET(38);
	// 						SetSPI2PWM(buf, 5);
	// 					}
	// 					if (xy[2] > 0.2 )
	// 					{
	// 						printf("Право\n");
	// 						ROLL_SET(100);
	// 						SetSPI2PWM(buf, 5);
	// 					}
	// 				}
	// 				// if (xy[3] < 3)
	// 				// {
	// 				// 	kk++;
	// 				// }
	// 			// }
				
	// 			// if (mm > 5) break;
	// 			// if (kk > 3)	break;
	// 			char c = cvWaitKey(33);
	// 			if (c == 27) break;

	// 	}
	// 	cvReleaseCapture( &capture );
	// 	cvReleaseVideoWriter(&writer);
	// }

	//==========================================
	// 					ВЗЛЕТ
	//==========================================
	int thr = 65;
	if ((argc > 1) && (strcmp(argv[1],"kt3") == 0 || strcmp(argv[1], "sonar") == 0 || strcmp(argv[1], "cam") == 0 || strcmp(argv[1], "up") == 0))
	{
		printf("Взлет\n");
		THROTTLE_SET(10);
		SetSPI2PWM(buf, 5);
		SetSPI2PWM(buf, 5);
		THROTTLE_SET(72);
		SetSPI2PWM(buf, 5);
		Delay(2);

		printf("Висим!!!\n");
		MODE_SET(100);
		THROTTLE_SET(55);
		printf("Режим.\n");
		SetSPI2PWM(buf, 5);
		Delay(1);

		// Вперед лететь
		PITCH_SET(40);
		SetSPI2PWM(buf, 5);

		Delay(3);
		for (int i = 0; i < 500; ++i)
		{
			usleep(1000);
		}
		PITCH_SET(50);
		SetSPI2PWM(buf, 5);
	}

	//==========================================
	// 				ТЕСТ СОНАРОВ
	//==========================================
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
	// 			ПОЛЕТ ПО КАМЕРЕ
	//==========================================
	if ((argc > 1) && !strcmp(argv[1], "cam"))
	{
		printf("Камера\n");
		int k = 0;
		int l = 0;
		int m = 0;
		int flag1 = 0;

		int flag2 = 0;

		while(1)
		{	
			xy2 = find2 (capture, writer);
			// xy=find(capture, pmin, pmax, writer);
			if (xy2[0] == 1)
			{
				printf("%d - y\n", xy2[2]);
				if (xy2[3] < -0.2)
				{
					PITCH_SET(47);
					SetSPI2PWM(buf, 5);
					for (int i = 0; i < 500; ++i)
					{
						usleep(1000);
					}
					PITCH_SET(50);
					SetSPI2PWM(buf, 5);
					flag1 = 0;
				}
				if (xy2[3] > 0.2)
				{
					PITCH_SET(53);
					SetSPI2PWM(buf, 5);
					for (int i = 0; i < 500; ++i)
					{
						usleep(1000);
					}
					PITCH_SET(50);
					SetSPI2PWM(buf, 5);
					flag1 = 0;
				}
				if (xy2[3] > -0.2 && xy2[3] < 0.2)
				{
					// PITCH_SET(50);
					// SetSPI2PWM(buf, 5);
					printf("%f - x; %f - y\n", xy2[1], xy2[2]);
					flag1 = 1;
				}
						if (xy2[4] > -0.2 && xy2[4] < 0.2)
				{
					// ROLL_SET(50);
					// SetSPI2PWM(buf, 5);
					printf("%f - x; %f - y\n", xy2[1], xy2[2]);
					flag2 = 1;
				}
				if (xy2[4] < -0.2)
				{
					ROLL_SET(53);
					SetSPI2PWM(buf, 5);
					for (int i = 0; i < 500; ++i)
					{
						usleep(1000);
					}
					ROLL_SET(50);
					SetSPI2PWM(buf, 5);
					flag2 = 0;
				}
				if (xy2[4] > 0.2)
				{
					ROLL_SET(47);
					SetSPI2PWM(buf, 5);	
					for (int i = 0; i < 500; ++i)
					{
						usleep(1000);
					}
					ROLL_SET(45);
					SetSPI2PWM(buf, 5);	
					flag2 = 0;
				}
			}

			if (flag2 && flag1) break;
			char c = cvWaitKey(33);
			if (c == 27) break;

		}
		printf("%f - x; %f - y\n", xy2[1], xy2[2]);
		cvReleaseCapture( &capture );
		cvReleaseVideoWriter(&writer);
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
		// ROLL_SET(10);
		// Delay(1);
		// SetSPI2PWM(buf, 5);
		// ROLL_SET(24);
		// SetSPI2PWM(buf, 5);
		// Delay(1);

		// ROLL_SET(38);
		// SetSPI2PWM(buf, 5);
		// Delay(1);
		// ROLL_SET(24);
		// SetSPI2PWM(buf, 5);
		// Delay(1);

		// ROLL_SET(10);
		// SetSPI2PWM(buf, 5);
		// Delay(1);
		// ROLL_SET(24);
		// SetSPI2PWM(buf, 5);
		// Delay(1);

		// ROLL_SET(38);
		// SetSPI2PWM(buf, 5);
		// Delay(1);
		// ROLL_SET(24);
		// SetSPI2PWM(buf, 5);
		// Delay(2);
		// Delay(6);
	}

	//==========================================
	// 					ПОСАДКА
	//==========================================
	if ((argc > 1) && (!strcmp(argv[1], "kt3") || !strcmp(argv[1], "sonar") || !strcmp(argv[1], "cam")))
	{
		PITCH_SET(55);
		SetSPI2PWM(buf, 5);
		Delay(1);

		printf("Посадка\n");
		// PITCH_SET(6);
		PITCH_SET(50);
		SetSPI2PWM(buf, 5);

		MODE_SET(0);
		SetSPI2PWM(buf, 5);
	
		THROTTLE_SET(66);
		SetSPI2PWM(buf, 5);
		Delay(2);
		// THROTTLE_SET(63);
		// SetSPI2PWM(buf, 5);
		// Delay(1);
		// for (int qq = 0; qq < 500; ++qq)
		// {
		// 	usleep(1000);
		// }
		
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
					SetSPI2PWM(buf, 5);
					break;
				}
				case 's':
				{
					g_chan[2] -= 10;
					if (g_chan[2] <= 0) g_chan[2] = 0;
					printw("%d ", g_chan[2]);
					THROTTLE_SET(g_chan[2]);
					SetSPI2PWM(buf, 5);
					break;
				}
				case 'a': 
				{
					g_chan[0] -= 10;
					if (g_chan[0] <= 0) g_chan[0] = 0;
					printw("%d ", g_chan[0]);
					YAW_SET(g_chan[0]);
					SetSPI2PWM(buf, 5);
					break;
					
				}
				case 'd':
				{
					g_chan[0] += 10;
					if (g_chan[0] >= 100) g_chan[0] = 100;
					printw("%d ", g_chan[0]);
					YAW_SET(g_chan[0]);
					SetSPI2PWM(buf, 5);
					break;
				}
				
				case 'k': 
				{
					g_chan[1] += 10;
					if (g_chan[1] >= 100) g_chan[1] = 100;
					printw("%d ", g_chan[1]);
					PITCH_SET(g_chan[1]);
					SetSPI2PWM(buf, 5);
					break;
				}
				case 'i':
				{
					g_chan[1] -= 10;
					if (g_chan[1] <= 0) g_chan[1] = 0;
					printw("%d ", g_chan[1]);
					PITCH_SET(g_chan[1]);
					SetSPI2PWM(buf, 5);
					break;
				}
				case 'j': 
				{
					g_chan[3] -= 10;
					if (g_chan[3] <= 0) g_chan[3] = 0;
					printw("%d ", g_chan[3]);
					ROLL_SET(g_chan[3]);
					SetSPI2PWM(buf, 5);
					break;
					
				}
				case 'l':
				{
					g_chan[3] += 10;
					if (g_chan[3] >= 100) g_chan[3] = 100;
					printw("%d ", g_chan[3]);
					ROLL_SET(g_chan[3]);
					SetSPI2PWM(buf, 5);
					break;
				}
				case 'f': printw("Mode stabilize. "); MODE_SET(0);   break;
				case 'g': printw("Mode Althold. "); MODE_SET(100); break;
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
				//case 1: g_Prc = &g_Prc1; ROLL_SET(*g_Prc); break;
				//case 2: g_Prc = &g_Prc2; PITCH_SET(*g_Prc); break;
				//case 3: g_Prc = &g_Prc3; THROTTLE_SET(*g_Prc); break;
				//case 4: g_Prc = &g_Prc4; YAW_SET(*g_Prc); break;
				//case 5: g_Prc = &g_Prc5; MODE_SET(*g_Prc); break;
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
