#include <unistd.h>
#include "gpio.h"
#include <stdlib.h>
#include <stdio.h>
#include "pwm.h"
// #include "msp_pwm.h"
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

// // 27 вывод
// #define MISO 88
// // 28 вывод
// #define CS1 98
// // 30 вывод
// #define MOSI 114
// // 18 вывод
// #define CLK 115
// // // 34 вывод
// // #define CS1	105
// // 31 вывод
// #define CS2	87

// int chan_buf[5];

	// #define		ROLL_SET(x) 	(chan_buf[0] = x * 10 + 1000 + 230)
	// #define		PITCH_SET(x) 	(chan_buf[1] = x * 10 + 1000 + 230)
	// #define 	THROTTLE_SET(x)	(chan_buf[2] = x * 10 + 1000 + 230)
	// #define 	YAW_SET(x)		(chan_buf[3] = x * 10 + 1000 + 230)
	// #define 	MODE_SET(x) 	(chan_buf[4] = x * 10 + 1000 + 230)

int* g_Prc;
int g_Prc1;
int g_Prc2;
int g_Prc3;
int g_Prc4;
int g_Prc5;

//-------------------Функция проверки существования файла---------------------------------------------
bool exists(const char *fname)
{
	FILE *file;
	if (file = fopen(fname, "r"))
	{
		fclose(file);
		return true;
	}
	return false;
}
//----------------------------------------------------------------------------------------------------

int ReadSonar(ESwLine line)
{
	cd74ac.SwitchLine(line);
	return sonar.ReadValue();
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

	// spi.Init(MISO, MOSI, CLK);
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
		//-------------------------Захват видео с камер (или из файла)----------------------------------------
#if (defined (CAMERA1))
	CvCapture* capture1 = 0;
	capture1 = cvCreateCameraCapture(NCam1);

	if (!capture1)
	{
#ifdef OUTPUT_CONSOLE
		printf("Video from the first camera is not captured.");
#endif
		return EXIT_FAILURE;//Захвата видеопотока не произошло.
	}
#endif

#if (defined (CAMERA2))
	CvCapture* capture2 = 0;
	capture2 = cvCreateCameraCapture(NCam2);
	if (!capture2)
	{
#ifdef OUTPUT_CONSOLE
		printf("Video from the second camera is not captured.");
#endif
		return EXIT_FAILURE;//Захвата видеопотока не произошло.
	}
#endif

#if (defined (FROM_FILE1) || defined (FROM_FILE2))
	//char* file = "d:\\OpenCV\\CROCVIDEO\\17.avi";
	char* file ="d:\\OpenCV\\CrocFind\\Main1\\3107_2.avi";
#ifdef OUTPUT_CONSOLE
	printf("[i] file: %s\n", file);
#endif
#if defined (FROM_FILE1)
	CvCapture* capture1 = 0;
	capture1 = cvCreateFileCapture (file);
	if (!capture1)
	{
#ifdef OUTPUT_CONSOLE
		printf("Video from the first camera is not captured.");
#endif
		return EXIT_FAILURE;//Захвата видеопотока не произошло.
	}
#endif
#if defined (FROM_FILE2)
	CvCapture* capture2 = 0;
	capture2 = cvCreateFileCapture (file);
	if (!capture2)
	{
#ifdef OUTPUT_CONSOLE
		printf("Video from the second camera is not captured.");
#endif
		return EXIT_FAILURE;//Захвата видеопотока не произошло.
	}
#endif
#endif
	//----------------------------------------------------------------------------------------------------

//--------------Генерация названий видеофайлов и калибровочных изображений для записи-----------------
#if (defined (WRITE_ALG_CAMERA1) || defined (WRITE_CAMERA1) || defined (WRITE_ALG_CAMERA2) || defined (WRITE_CAMERA2) || defined (WRITE_MARKER) || defined (WRITE_FLOOR))
	const unsigned short int n=35;
	char filename_alg_c1[n];
	char filename_c1[n];
	char filename_alg_c2[n];
	char filename_c2[n];
	char filename_marker[n];
	char filename_floor[n];

	int nVideoFile = 0;
	bool FileName = false;
	while(!FileName)
	{
		sprintf(filename_alg_c1, "/home/Flight_%d_cam1_alg.avi", nVideoFile);
		sprintf(filename_alg_c2, "/home/Flight_%d_cam2_alg.avi", nVideoFile);
		sprintf(filename_c1, "/home/Flight_%d_cam1.avi", nVideoFile);
		sprintf(filename_c2, "/home/Flight_%d_cam2.avi", nVideoFile);
		sprintf(filename_marker, "/home/Flight_%d_cam2_marker.jpg", nVideoFile);
		sprintf(filename_floor, "/home/Flight_%d_cam2_floor.jpg", nVideoFile);

		if (exists(filename_alg_c1) || exists(filename_alg_c2) || exists(filename_c1) || exists(filename_c2) || exists(filename_marker) || exists(filename_floor))
		{
			nVideoFile++;
		}
		else 
		{
			FileName = true;
		}
	}
#endif
	//----------------------------------------------------------------------------------------------------

	//-------------------------Инициализизация структур редакторов для записи видео-----------------------
#if (defined (WRITE_ALG_CAMERA1) || defined (WRITE_CAMERA1))
	//double fps1 = cvGetCaptureProperty (capture1, CV_CAP_PROP_FPS);
	//Почему-то запись работает только с частотой записи 15 кадров в секунду.
	double fps1 = 15;
	CvSize size1 = cvSize( (int)cvGetCaptureProperty( capture1, CV_CAP_PROP_FRAME_WIDTH), (int)cvGetCaptureProperty( capture1, CV_CAP_PROP_FRAME_HEIGHT));
	cvSetCaptureProperty(capture1,CV_CAP_PROP_BRIGHTNESS,50);
	cvSetCaptureProperty(capture1,CV_CAP_PROP_CONTRAST,50);
	cvSetCaptureProperty(capture1,CV_CAP_PROP_SATURATION,50);

#ifdef WRITE_ALG_CAMERA1
	CvVideoWriter * writer_alg1 = cvCreateVideoWriter(filename_alg_c1, CV_FOURCC('X','V','I','D'), fps1, size1, 1);
	assert(writer_alg1!=0);
#endif
#if defined WRITE_CAMERA1
	CvVideoWriter * writer1 = cvCreateVideoWriter(filename_c1, CV_FOURCC('X','V','I','D'), fps1, size1, 1);
	assert(writer1!=0);
#endif

#endif


#if (defined (WRITE_ALG_CAMERA2) || defined (WRITE_CAMERA2))
	//double fps2 = cvGetCaptureProperty (capture2, CV_CAP_PROP_FPS);
	double fps2 =15;
	CvSize size2 = cvSize( (int)cvGetCaptureProperty( capture2, CV_CAP_PROP_FRAME_WIDTH), (int)cvGetCaptureProperty( capture2, CV_CAP_PROP_FRAME_HEIGHT));
	cvSetCaptureProperty(capture2,CV_CAP_PROP_BRIGHTNESS,50);
	cvSetCaptureProperty(capture2,CV_CAP_PROP_CONTRAST,50);
	cvSetCaptureProperty(capture2,CV_CAP_PROP_SATURATION,50);
#if defined WRITE_ALG_CAMERA2
	CvVideoWriter * writer_alg2 = cvCreateVideoWriter(filename_alg_c2, CV_FOURCC('X','V','I','D'), fps2, size2, 1);
	assert(writer_alg2!=0);
#endif
#if defined WRITE_CAMERA2
	CvVideoWriter * writer2 = cvCreateVideoWriter(filename_c2, CV_FOURCC('X','V','I','D'), fps2, size2, 1);
	assert(writer2!=0);
#endif

#endif
	//----------------------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------------------
	//---То, что должно быть объявлено и выполнено до цикла поиска примерно в такой последовательности----
	//----------------------------------------------------------------------------------------------------

	bool WBm=false, Fl=false;//Были ли сперва получены и проанализированы кадры маркера и пола для калибровки.

#if (defined (CAMERA1) || defined (FROM_FILE1)) 
	IplImage* frame1 = 0;
#endif
#if (defined (CAMERA2) || defined (FROM_FILE2)) 
	IplImage* frame2 = 0;
#endif

	int x;
	// InitSPI2PWM(MISO, MOSI, CLK);

	spi.Init(MISO, MOSI, CLK);

	// CvCapture* capture = 0;	
	// capture = cvCreateCameraCapture(3);
	// if (!capture)
	// {
	// 	return EXIT_FAILURE;
	// }
	// int pmin=50,pmax=100;

	// double fps = 15;
	// const char *filename2 = "/home/vc3.avi";
	// IplImage* frame = cvQueryFrame( capture );
	// CvSize size = cvSize(frame->width, frame->height);
 //    CvVideoWriter * writer = cvCreateVideoWriter(filename2, CV_FOURCC('X','V','I','D'), fps, size, 1);
	// assert(writer!=0);
	// double* xy;
	// // double* xy2;
	// int* xy2;

	// Init();

	// if (argc == 1)
	// {
	// 	printf("No params\n");
	// 	return 1;
	// }

	// int spi1, spi2;
	// spi2 = spi.InitCS(CS1);

	// spi1 = spi.InitCS(CS2);
	
	// char val = 'A';
	// // chan_buf[0] = 234;
	// int asd = 0;
	// while(asd < 3)
	// {	
	// 	asd++;
	// 	// GPIOClr(CS1);
	// 	// GPIOSet(CS1);
	// 	spi.ClrCS(spi2);
	// 	spi.SetCS(spi2);
	// 	spi.ClrCS(spi2);
	// 	// usleep(500);
	// 	// GPIOClr(CS1);
	// 	// usleep(500);
	// 	spi.WriteByte(0x01);
	// 	spi.WriteByte(0x03);
	// 	spi.WriteByte(0xE8);
	// 	spi.SetCS(spi2);

	// 	// usleep(500);
	// 	// GPIOSet(CS1);
	// 	// usleep(500);

	
	// 	Delay(1);


	// 	spi.ClrCS(spi2);

	// 	// usleep(500);
	// 	// GPIOClr(CS1);
	// 	// usleep(500);
	// 	spi.WriteByte(0x01);
	// 	spi.WriteByte(0x05);
	// 	spi.WriteByte(0xDC);
	// 	spi.SetCS(spi2);

	// 	// usleep(500);
	// 	// GPIOSet(CS1);
	// 	// usleep(500);

	// 	Delay(1);

	// 	spi.ClrCS(spi1);
	// 	spi.WriteByte(0x00);
	// 	val = spi.ReadByte();
	// 	printf("%d \n", val);
	// 	spi.SetCS(spi1);

	// 	spi.ClrCS(spi1);
	// 	spi.WriteByte(0x00);
	// 	val = spi.ReadByte();
	// 	printf("%d \n", val);
	// 	spi.SetCS(spi1);

	// 	spi.ClrCS(spi1);
	// 	spi.WriteByte(0x00);
	// 	val = spi.ReadByte();
	// 	printf("%d \n", val);
	// 	spi.SetCS(spi1);

	// 	Delay(1);

	// }

	//==========================================
	//				    ARMED
	//==========================================
	if ((argc > 1) && strcmp(argv[1], "nostart") != 0)
	{	
		// printf("Delay\n");
		// Delay(9);
		printf("Armed\n");
		Armed();
	}

	//==========================================
	// 					ВЗЛЕТ
	//==========================================
	int thr = 65;
	if ((argc > 1) && (strcmp(argv[1],"kt3") == 0 || strcmp(argv[1], "sonar") == 0 || strcmp(argv[1], "cam") == 0 || strcmp(argv[1], "up") == 0))
	{
		printf("Взлет\n");

		// Throttle, Time, Alt
		LiftUp(72, 2, 58);
	////////////////////Получение изображения маркера. Должно быть вызвано однажды над кругом после старта.
#if (defined (CAMERA2) || defined (FROM_FILE2))
	IplImage* marker = 0;
	marker = cvQueryFrame(capture2); //Взять из камеры.
	//marker = cvLoadImage("d:\\OpenCV\\CrocFind\\Main1\\MC2.bmp"); //Взять из файла.
	if(!marker)
	{
#ifdef OUTPUT_CONSOLE
		printf("Frame marker for camera 2 is not received.");
#endif
		return EXIT_FAILURE;
	}
	cvSaveImage(filename_marker,marker); //Сохранение в обязательном порядке, так как потом считывание следующей функцией.
	cvReleaseImage (&marker);
	WBm=true;
#endif
	////////////////////////////////////////////////////////////////////////////////////////////////////

	}

	//==========================================
	// 				ТЕСТ СОНАРОВ
	//==========================================
	if ((argc > 1) && !strcmp(argv[1], "test"))
	{
		int data;
		while(1)
		{
			data = ReadSonar(SONAR1);
			printf("Data1: - %d ", data);
			data = ReadSonar(SONAR2);
			printf("Data2: - %d\n", data);
		}
	}

	//==========================================
	// 			ПОЛЕТ ПО КАМЕРЕ
	//==========================================
	if ((argc > 1) && !strcmp(argv[1], "cam"))
	{
		printf("Полет вперед\n");

		PITCH_SET(42);
		SetSPI2PWM(chan_buf, 5);
		Delay(2);
		printf("Error\n");
	////////////////////Функция получения цветов пола. Должна быть вызвана единожды в месте, где точно нет маркера.	
#if (defined (CAMERA2) || defined (FROM_FILE2))
	frame2 = cvQueryFrame(capture2); //Взять из камеры.
	printf("Ошибка взятия кадра из камеры\n");
	//frame2 = cvLoadImage("d:\\OpenCV\\CrocFind\\Main1\\FC2.bmp");	//Взять из файла.
	if(!frame2)
	{
#ifdef OUTPUT_CONSOLE
		printf("Frame floor for camera 2 is not received.");
#endif
		return EXIT_FAILURE;
	}
	getFloor (frame2, filename_floor, filename_marker); 
	printf("Ошибка 1\n");
	Fl=true;
#endif
	////////////////////////////////////////////////////////////////////////////////////////////////////

		printf("Камера\n");

		bool isDownCam;
		isDownCam = false;
		int dx = 5;
		//------------Цикл обработки кадров и поиска маркера--------------------------------------------------
		while (1)
		{

			if (!isDownCam)
			{
						//--------------------Первая (верхняя) камера - поиск и при необходимости запись------------------
				#if (defined (CAMERA1) || defined (FROM_FILE1))
						//Получение кадра.
						frame1 = cvQueryFrame(capture1);
						printf("Ошибка 2\n");
						if(!frame1)
						{
				#ifdef OUTPUT_CONSOLE
							printf("Frame for camera 1 is not received.");
				#endif
							return EXIT_FAILURE;
						}
						//Запись необработанного кадра.
				#if (defined(WRITE_CAMERA1) && (defined (CAMERA1) || defined (FROM_FILE1))) 
						cvWriteFrame(writer1, frame1);
				#endif
						//Выполнение поиска.
						//1. Нужно не забыть выполнить сперва калибровку цветов маркера.
						//2. Нужно выполнить затем калибровку цветов пола.
						//3. Выполнять поиск, используя полученное.
						// if (WBm && Fl)//Проверка наличия кадров с маркером и полом.
						// {
				#if (defined(WRITE_ALG_CAMERA1) && ((defined (CAMERA1) || defined (FROM_FILE1))))  
							double* xy1 = find1 (frame1,  writer_alg1);
				#elif (defined (CAMERA1) || defined (FROM_FILE1))
							double* xy1 = find1 (frame1);
				#endif
							//Вывод результатов поиска.
				#ifdef OUTPUT_CONSOLE_FRAME_RES
							printf ("PPExist = %.4lf, Ux = %.4lf, S = %.4lf \n\n", xy1[0], xy1[1], xy1[2]);
				#endif
						// }
				#endif
				
						//----------------------------------------------------------------------------------------------------

				if (xy1[0] == 1)
				{
					if (xy1[1] < -0.2)
					{
				ROLL_SET(FR_ROLL - dx);
				SetSPI2PWM(chan_buf, 5);
				Delay(1);
				ROLL_SET(FR_ROLL + dx);
				SetSPI2PWM(chan_buf, 5);
				for (int i = 0; i < 500; ++i)
				{
					usleep(1000);
				}
				ROLL_SET(FR_ROLL);
				SetSPI2PWM(chan_buf, 5);
					}
					if (xy1[1] > 0.2)
					{
				ROLL_SET(FR_ROLL + dx);
				SetSPI2PWM(chan_buf, 5);
				Delay(1);

				ROLL_SET(FR_ROLL - dx);
				SetSPI2PWM(chan_buf, 5);
				for (int i = 0; i < 500; ++i)
				{
					usleep(1000);
				}

				ROLL_SET(FR_ROLL);
				SetSPI2PWM(chan_buf, 5);
					}
				}

			}

					//--------------------Вторая (нижняя) камера - поиск и при необходимости запись-----------------------
			#if (defined (CAMERA2) || defined (FROM_FILE2))
					frame2 = cvQueryFrame(capture2);
					if(!frame2)
					{
			#ifdef OUTPUT_CONSOLE
						printf("Frame for camera 2 is not received.");
			#endif
						return EXIT_FAILURE;
					}
					//Запись необработанного кадра.
			#if (defined(WRITE_CAMERA2)) 
					cvWriteFrame(writer2, frame2);
			#endif
					//Выполнение поиска.
					//1. Нужно не забыть выполнить сперва калибровку цветов маркера.
					//2. Нужно выполнить затем калибровку цветов пола.
					//3. Выполнять поиск, используя полученное.
					// if (WBm && Fl)//Проверка наличия кадров с маркером и полом.
					// {
			#if (defined(WRITE_ALG_CAMERA2) && ((defined (CAMERA2) || defined (FROM_FILE2))))  
						unsigned short int* xy2 = find2 (frame2,  writer_alg2);
			#elif (defined (CAMERA2) || defined (FROM_FILE2))
						unsigned short int* xy2 = find2 (frame2);
			#endif
					//Вывод результатов поиска.
			#ifdef OUTPUT_CONSOLE_FRAME_RES
						printf ("PP = %d, Up = %d, Right = %d, Down = %d, Left = %d \n\n", xy2[0], xy2[1], xy2[2],  xy2[3], xy2[4]);
			#endif
					// }
			#endif
					//----------------------------------------------------------------------------------------------------

			if (xy2[0] > 0)
			{
				isDownCam = true;
				if (xy2[0] == 4 || (xy2[1] == 1 && xy2[3] == 1)  || (xy2[2] == 1 && xy2[4] == 1))
				{
					break;
				}

				// Движение вправо
				if (xy2[1] == 1)
				{
					ROLL_SET(FR_ROLL + dx);
					SetSPI2PWM(chan_buf, 5);
					Delay(1);
					ROLL_SET(FR_ROLL - dx);
					SetSPI2PWM(chan_buf, 5);
					for (int i = 0; i < 500; ++i)
					{
						usleep(1000);
					}
					ROLL_SET(FR_ROLL);
					SetSPI2PWM(chan_buf, 5);
				}
				else
				// Движение влево
				if (xy2[3] == 1)
				{
					ROLL_SET(FR_ROLL - dx);
					SetSPI2PWM(chan_buf, 5);
					Delay(1);
					ROLL_SET(FR_ROLL + dx);
					SetSPI2PWM(chan_buf, 5);
					for (int i = 0; i < 500; ++i)
					{
						usleep(1000);
					}
					ROLL_SET(FR_ROLL);
					SetSPI2PWM(chan_buf, 5);
				}
				else
				// Движение вперед
				if (xy2[4] == 1)
				{
					PITCH_SET(FR_PITCH - dx);
					SetSPI2PWM(chan_buf, 5);
					Delay(1);
					PITCH_SET(FR_PITCH + dx);
					SetSPI2PWM(chan_buf, 5);
					for (int i = 0; i < 500; ++i)
					{
						usleep(1000);
					}
					PITCH_SET(FR_PITCH);
					SetSPI2PWM(chan_buf, 5);
				}
				else
				// Движение назад
				if (xy2[2] == 1)
				{
					PITCH_SET(FR_PITCH + dx);
					SetSPI2PWM(chan_buf, 5);
					Delay(1);
					PITCH_SET(FR_PITCH - dx);
					SetSPI2PWM(chan_buf, 5);
					for (int i = 0; i < 500; ++i)
					{
						usleep(1000);
					}
					PITCH_SET(FR_PITCH);
					SetSPI2PWM(chan_buf, 5);
				}

			}

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
				SetSPI2PWM(chan_buf, 5);
			}
			else
			if (data < 1500)
			{
				YAW_SET(40);
				SetSPI2PWM(chan_buf, 5);
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

		// // Pitch, Time
		// FlyForward(47, 3);
		PITCH_SET(45);
		SetSPI2PWM(chan_buf, 5);
		Delay(4);

		// YAW_SET(55);
		// SetSPI2PWM(chan_buf, 5);
		// Delay(4);
		// YAW_SET(FR_YAW);
		// SetSPI2PWM(chan_buf, 5);
		// Delay(1);
		// PITCH_SET(50);
		// SetSPI2PWM(chan_buf, 5);
	}

	//==========================================
	// 					ПОСАДКА
	//==========================================
	if ((argc > 1) && (!strcmp(argv[1], "kt3") || !strcmp(argv[1], "sonar") || !strcmp(argv[1], "cam")))
	{
		printf("Посадка\n");

		PITCH_SET(53);
		SetSPI2PWM(chan_buf, 5);
		Delay(1);
		PITCH_SET(50);
		SetSPI2PWM(chan_buf, 5);

		// Throttle, Time
		LiftDown(65, 2);
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
					SetSPI2PWM(chan_buf, 5);
					break;
				}
				case 's':
				{
					g_chan[2] -= 10;
					if (g_chan[2] <= 0) g_chan[2] = 0;
					printw("%d ", g_chan[2]);
					THROTTLE_SET(g_chan[2]);
					SetSPI2PWM(chan_buf, 5);
					break;
				}
				case 'a': 
				{
					g_chan[0] -= 10;
					if (g_chan[0] <= 0) g_chan[0] = 0;
					printw("%d ", g_chan[0]);
					YAW_SET(g_chan[0]);
					SetSPI2PWM(chan_buf, 5);
					break;
					
				}
				case 'd':
				{
					g_chan[0] += 10;
					if (g_chan[0] >= 100) g_chan[0] = 100;
					printw("%d ", g_chan[0]);
					YAW_SET(g_chan[0]);
					SetSPI2PWM(chan_buf, 5);
					break;
				}
				
				case 'k': 
				{
					g_chan[1] += 10;
					if (g_chan[1] >= 100) g_chan[1] = 100;
					printw("%d ", g_chan[1]);
					PITCH_SET(g_chan[1]);
					SetSPI2PWM(chan_buf, 5);
					break;
				}
				case 'i':
				{
					g_chan[1] -= 10;
					if (g_chan[1] <= 0) g_chan[1] = 0;
					printw("%d ", g_chan[1]);
					PITCH_SET(g_chan[1]);
					SetSPI2PWM(chan_buf, 5);
					break;
				}
				case 'j': 
				{
					g_chan[3] -= 10;
					if (g_chan[3] <= 0) g_chan[3] = 0;
					printw("%d ", g_chan[3]);
					ROLL_SET(g_chan[3]);
					SetSPI2PWM(chan_buf, 5);
					break;
					
				}
				case 'l':
				{
					g_chan[3] += 10;
					if (g_chan[3] >= 100) g_chan[3] = 100;
					printw("%d ", g_chan[3]);
					ROLL_SET(g_chan[3]);
					SetSPI2PWM(chan_buf, 5);
					break;
				}
				case 'f': printw("Mode stabilize. "); MODE_SET(0);   break;
				case 'g': printw("Mode Althold. "); MODE_SET(100); break;
				case 'z': printw("ThrUP. "); THROTTLE_SET(100); SetSPI2PWM(chan_buf, 5); break;
				case 'c': printw("ThrDOWN. "); THROTTLE_SET(0); SetSPI2PWM(chan_buf, 5); break;
				case '1': printw("Roll: "); chan = 1; break;
				case '2': printw("Pitch: "); chan = 2; break;
				case '3': printw("Throttle: "); chan = 3; break;
				case '4': printw("Yaw: "); chan = 4; break;
				case '5': printw("Mode: "); chan = 5; break;
				case '[': printw("Start... "); THROTTLE_SET(0); YAW_SET(100); SetSPI2PWM(chan_buf, 5); Delay(4); YAW_SET(50); SetSPI2PWM(chan_buf, 5); printw("Ok. "); break;
				case ']': printw("Stop... "); THROTTLE_SET(0); YAW_SET(0); SetSPI2PWM(chan_buf, 5); Delay(4); YAW_SET(50); SetSPI2PWM(chan_buf, 5); printw("Ok. "); break;
				case 'r': {
						printw("Reset... ");
						ROLL_SET(50);
						PITCH_SET(50);
						THROTTLE_SET(0);
						YAW_SET(50);
						MODE_SET(0);
						SetSPI2PWM(chan_buf, 5);
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
			SetSPI2PWM(chan_buf, 5);
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

	//-------------Освобождение памяти, разрушение окон и т.д.--------------------------------------------
#if (defined (CAMERA1) || defined (FROM_FILE1))
	cvReleaseCapture( &capture1 );
#ifdef WRITE_ALG_CAMERA1
	cvReleaseVideoWriter(&writer_alg1);
#endif
#ifdef WRITE_CAMERA1
	cvReleaseVideoWriter(&writer1);
#endif
#endif

#if (defined (CAMERA2) || defined (FROM_FILE2))
#ifdef WRITE_ALG_CAMERA2
	cvReleaseVideoWriter(&writer_alg2);
#endif
#ifdef WRITE_CAMERA2
	cvReleaseVideoWriter(&writer2);
#endif
#endif

#ifdef SHOW_WINDOWS
	cvDestroyAllWindows();
#endif
	//----------------------------------------------------------------------------------------------------

	return 0;
}

void Delay(int time)
{
	for (int i = 0; i < time*1000; i++) usleep(1000);
}