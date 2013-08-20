#include "main.h"
#include "gpio.h"
#include "pwm.h"
#include "msp_pwm.h"
#include "sonar.h"
#include "flight_routines.h"
#include "spi.h"
#include "mavlink.h"
#include "find.h"
#include "define.h"

#include <unistd.h>
#include <stdio.h>
#include <curses.h>
#include <math.h>
#include <iostream>

#define		SONAR1	0
#define		R_SONAR	1
#define		F_SONAR	2
#define 	L_SONAR	3
#define 	B_SONAR	4
#define _USE_MATH_DEFINES

int FlyProfile(float);

 // 27 вывод
#define MISO 88
// 30 вывод
#define MOSI 114
// 18 вывод
#define CLK 115
// 34 вывод
#define CS1	105
// 31 вывод
#define CS2	87

int zgyro;

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
	if (line == SONAR1) val *= 5.8;
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
	InitMavlink();
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

time_t now;

int main(int argc, char const *argv[])
{

	if (argc == 1)
	{
		printf("No params\n");
		return 1;
	}

	Init();

	// // debug = true;
	// // verbose = true;

	//========================================
	//			ИНИЦИАЛИЗАЦИЯ КАМЕР
	//========================================
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
		capture2 = cvCaptureFromCAM(NCam2);
		if (!capture2)
		{
	#ifdef OUTPUT_CONSOLE
			printf("Video from the second camera is not captured.");
	#endif
			return EXIT_FAILURE;//Захвата видеопотока не произошло.
		}
	#endif

	#if (defined (FROM_FILE1) || defined (FROM_FILE2))
		//char* file1 = "d:\\OpenCV\\CROCVIDEO\\17.avi";
		char* file1 ="d:\\OpenCV\\CrocFind\\Main1\\3107_2.avi";
	#ifdef OUTPUT_CONSOLE
		printf("File1: %s\n", file1);
	#endif
	#if defined (FROM_FILE1)
		CvCapture* capture1 = 0;
		capture1 = cvCreateFileCapture (file1);
		if (!capture1)
		{
	#ifdef OUTPUT_CONSOLE
			printf("Video from the first camera is not captured.");
	#endif
			return EXIT_FAILURE;//Захвата видеопотока не произошло.
		}
	#endif
	#if defined (FROM_FILE2)
		//char* file2 = "d:\\OpenCV\\CROCVIDEO\\17.avi";
		char* file2 ="d:\\OpenCV\\CrocFind\\Main1\\vid2.avi";
	#ifdef OUTPUT_CONSOLE
		printf("File2: %s\n", file2);
	#endif
		CvCapture* capture2 = 0;
		capture2 = cvCreateFileCapture (file2);
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
		const unsigned short int n=30;
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

		//-------------------------Инициализация структур редакторов для записи видео-------------------------
	#if (defined (WRITE_ALG_CAMERA1) || defined (WRITE_CAMERA1))
		//double fps1 = cvGetCaptureProperty (capture1, CV_CAP_PROP_FPS);
		//Почему-то запись работает только с частотой записи 15 кадров в секунду.
		double fps1 = 15;
		CvSize size1 = cvSize( (int)cvGetCaptureProperty( capture1, CV_CAP_PROP_FRAME_WIDTH), (int)cvGetCaptureProperty( capture1, CV_CAP_PROP_FRAME_HEIGHT));
		//cvSetCaptureProperty(capture1,CV_CAP_PROP_BRIGHTNESS,50);
		//cvSetCaptureProperty(capture1,CV_CAP_PROP_CONTRAST,50);
		//cvSetCaptureProperty(capture1,CV_CAP_PROP_SATURATION,50);

	#ifdef WRITE_ALG_CAMERA1
		CvVideoWriter * writer_alg1;
		if ((argc > 1) && !strcmp(argv[1], "cam"))
		{
			writer_alg1 = cvCreateVideoWriter(filename_alg_c1, CV_FOURCC('X','V','I','D'), fps1, size1, 1);
		}
		assert(writer_alg1!=0);
	#endif
	#if defined WRITE_CAMERA1
		CvVideoWriter * writer1;
		if ((argc > 1) && !strcmp(argv[1], "cam"))
		{
			writer1 = cvCreateVideoWriter(filename_c1, CV_FOURCC('X','V','I','D'), fps1, size1, 1);
		}
		assert(writer1!=0);
	#endif

	#endif


	#if (defined (WRITE_ALG_CAMERA2) || defined (WRITE_CAMERA2))
		//double fps2 = cvGetCaptureProperty (capture2, CV_CAP_PROP_FPS);
		double fps2 =15;
		CvSize size2 = cvSize( (int)cvGetCaptureProperty( capture2, CV_CAP_PROP_FRAME_WIDTH), (int)cvGetCaptureProperty( capture2, CV_CAP_PROP_FRAME_HEIGHT));
		// cvSetCaptureProperty(capture2,CV_CAP_PROP_BRIGHTNESS,10);
		// cvSetCaptureProperty(capture2,CV_CAP_PROP_CONTRAST,10);
		// cvSetCaptureProperty(capture2,CV_CAP_PROP_SATURATION,10);
	#if defined WRITE_ALG_CAMERA2
		CvVideoWriter * writer_alg2;
		if ((argc > 1) && !strcmp(argv[1], "cam"))
		{
			writer_alg2 = cvCreateVideoWriter(filename_alg_c2, CV_FOURCC('X','V','I','D'), fps2, size2, 1);
		}
		assert(writer_alg2!=0);
	#endif
	#if defined WRITE_CAMERA2
		CvVideoWriter * writer2;
		if ((argc > 1) && !strcmp(argv[1], "cam"))
		{
			writer2 = cvCreateVideoWriter(filename_c2, CV_FOURCC('X','V','I','D'), fps2, size2, 1);
		}
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
	//========================================

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

	// Для новой батареии - 68, старая - 74
	int thr = 80;
	if (thr > 74)
	{
		thr = 80;
		printf("Превышение тяги!\n");
	}
	float data;
	// if ((argc > 1) && (strcmp(argv[1],"kt3") == 0 || strcmp(argv[1], "sonar") == 0 || strcmp(argv[1], "cam") == 0 || strcmp(argv[1], "up") == 0))
	if ((argc > 1) && strcmp(argv[1],"test") && strcmp(argv[1],"arm"))
	{
		
		printf("Взлет\n");

		// THROTTLE_SET(46);
		// Delay(2);
		// THROTTLE_SET(thr);
		// Delay(2);


		////////////////////Получение изображения маркера. Должно быть вызвано однажды над кругом после старта.
		if ((argc > 1) && (strcmp(argv[1], "cam") == 0))
		{
			#if (defined (CAMERA2) || defined (FROM_FILE2))
				frame2 = cvQueryFrame(capture2);
				//frame2 = cvLoadImage("d:\\OpenCV\\CrocFind\\Main1\\Marker2.bmp"); //Взять из файла.
				if(!frame2)
				{
			#ifdef OUTPUT_CONSOLE
					printf("Frame marker for camera 2 is not received.");
			#endif
					return EXIT_FAILURE;
				}
				cvSaveImage(filename_marker, frame2); //Сохранение в обязательном порядке, так как потом считывание следующей функцией.
				printf("Первый кадр\n");		
				WBm=true;
			#endif
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////
		// Throttle, Time, Alt
		// Для новой батареии Alt - 54, старая - 58

		// LiftUp(thr, 2, 55);
//-------------------------------
		// int flyThr = 46;
		// int max;
		// max = flyThr;
		// float curh;
		// curh = ReadSonar(SONAR1);
		// printf("Высота: %f\n", curh);
		// float dat;
		// usleep(1000);
		// while (curh < 100)
		// {
		// 	dat = ReadSonar(SONAR1);
		// 	// printf("%d\t", i);
		// 	// printf("%d\n", FlyProfile(i));
		// 	if ((dat - curh) < 1)
		// 	{
		// 		flyThr++;
		// 		max = flyThr;
		// 	}
		// 	else 
		// 	{	
		// 		if (FlyProfile(dat) < max) flyThr = max;
		// 		else 
		// 		{
		// 			flyThr = FlyProfile(dat);
		// 			max = flyThr;
		// 		}
		// 	}
		// 	if (flyThr > thr) flyThr = thr;
		// 	printf("Fly throttle: %d\t", flyThr);
		// 	printf("Sonar: %f\n", dat);
		// 	THROTTLE_SET(flyThr);
		// 	curh = dat;
		// 	usleep(10000);
		// }

		time_t fly = time(0);
		printf("Time 1: %d\t", fly);
		for (int h = 20; h < 150; h++)
		{
			THROTTLE_SET(FlyProfile(h));
			// printf("Throttle %d\n", FlyProfile(h));
			usleep(500);
		}
		fly = time(0);
		printf("Time 2: %d\n", fly);

		MODE_SET(100);
		THROTTLE_SET(54);
		printf("Режим.\n");
		//-------------------------------


		
		//ВЗЛЕТ ПО ПОКАЗАНИЯМ НИЖНЕГО СОНАРА (НЕ РАБОТАЕТ, Т.К. ВЫДАЕТСЯ НЕПОНЯТНАЯ ОШИБКА)
		
		// data = ReadSonar(SONAR1);
		// while (data < 110)
		// {
		
		// 	data = ReadSonar(SONAR1);
		
		// 	// for (int i = 0; i < 10; ++i)
		// 	// 	{
		// 	// 		usleep(1000);
		// 	// 	}			
		// }

		// MODE_SET(100);
		// printf("Режим.\n");
		// THROTTLE_SET(54);
	}

	//==========================================
	// 				ТЕСТ СОНАРОВ
	//==========================================
	if ((argc > 1) && !strcmp(argv[1], "test"))
	{
		float data;
		int asd = 0;
		while(asd < 1000)
		{
			asd++;
			data = ReadSonar(SONAR1);
			printf("Нижний сонар: - %f : ", data);
			data = ReadSonar(L_SONAR);
			printf("Левый сонар: - %f\n", data);
			data = ReadSonar(R_SONAR);
			printf("Правый сонар: - %f : ", data);
			data = ReadSonar(F_SONAR);
			printf("Передний сонар: - %f : ", data);
			data = ReadSonar(B_SONAR);
			printf("Задний сонар: - %f : ", data);
			for (int i = 0; i < 10; ++i)
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
		printf("Взлет\n");

		// Throttle, Time, Alt
		LiftUp(70, 2, 63);
		YAW_SET(FR_YAW+30);
		printf("Поворот\n");
		YAW_SET(FR_YAW+35);
		Delay(2);		
		YAW_SET(FR_YAW);
		Delay(1);
	}	

	//==========================================
	// 			ПОЛЕТ ПО КАМЕРЕ
	//==========================================
	if ((argc > 1) && !strcmp(argv[1], "cam"))
	{
		printf("Полет вперед\n");
		PITCH_SET(FR_PITCH - 15);

		int del = 0;
		while (del < 200)
		{
			del++;
			frame2 = cvQueryFrame(capture2);
			// serial_wait(fd);
			// printf("Compass delay: %d\n", g_Compass);
		}
		PITCH_SET(FR_PITCH - 10);
		////////////////////Функция получения цветов пола. Должна быть вызвана единожды в месте, где точно нет маркера.	
		#if (defined (CAMERA2) || defined (FROM_FILE2))
			frame2 = cvQueryFrame(capture2); //Взять из камеры.
			//frame2 = cvLoadImage("d:\\OpenCV\\CrocFind\\Main1\\Floor111.bmp");	//Взять из файла.
			if(!frame2)
			{
		#ifdef OUTPUT_CONSOLE
				printf("Frame floor for camera 2 is not received.");
		#endif
				return EXIT_FAILURE;
			}
			getFloor (frame2, filename_floor, filename_marker); 
			printf("Второй кадр\n");
			Fl=true;
		#endif
		////////////////////////////////////////////////////////////////////////////////////////////////////

		printf("Полет по камере и сонарам\n");
		float data;
		int dy = 20;
		bool isDownCam;
		bool isOnSonar = true;
		bool isFirstFrwrd = true;
		
		isDownCam = true;
		bool FirstDot = true;
		int dx = 20;
		
		int th = 4;
		int gaz = FR_THROTTLE;

		//------------Цикл обработки кадров и поиска маркера--------------------------------------------------
		while (1)
		{

			// data = ReadSonar(SONAR1);
			// printf("Высота: - %f \t", data);
			// if (data > 100 && data < 150)
			// {	
			// }
			// else
			// if (data < 100)
			// {
			// 	gaz += th;
			// 	if (gaz > 100) gaz = 100;
			// 	printf("Вверх: %d\n", gaz);
			// 	THROTTLE_SET(gaz);
			// }
			// else
			// if (data > 150)
			// {
			// 	gaz -= th;
			// 	if (gaz < 0) gaz = 0;
			// 	printf("Вниз: %d\n", gaz);
			// 	THROTTLE_SET(gaz);
			// }
			// else
			// {
				int koef = 2;
				for (int i = 0; i < 10; ++i)
				{
					serial_wait();
				}
				printf("Compass: %d \t Head: %d", g_Compass, g_Head);
				if ((g_Compass - g_Head) < 0)
				{
					printf("\tВправо YAW\n");
					YAW_SET(FR_YAW - (g_Compass - g_Head) * koef);
					for (int i = 0; i < 50; ++i)
					{
						usleep(1000);
					}
					YAW_SET(FR_YAW);
				}
				else
				if ((g_Compass - g_Head) > 0)	
				{
					printf("\tВлево YAW\n");
					YAW_SET(FR_YAW - (g_Compass - g_Head) * koef);
					for (int i = 0; i < 50; ++i)
					{
						usleep(1000);
					}
					YAW_SET(FR_YAW);
				}

				if (!isDownCam)
				{
					
							//--------------------Первая (верхняя) камера - поиск и при необходимости запись------------------
					#if (defined (CAMERA1) || defined (FROM_FILE1))
							//Получение кадра.
							frame1 = cvQueryFrame(capture1);
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
							//if (WBm && Fl)//Проверка наличия кадров с маркером и полом.
							//{
							double* xy1 = find1 (frame1);
							double XY10=xy1[0];//Наличие центра маркера в кадре.
							double XY11=xy1[1];//Отклонение от центра.
							double XY12=xy1[2];
							//Вывод результатов поиска.
					#ifdef OUTPUT_CONSOLE_FRAME_RES
							printf ("PPExist = %.4lf, Ux = %.4lf, S = %.4lf \n\n", xy1[0], xy1[1], xy1[2]);
					#endif
							//}
					#ifdef SHOW_WINDOWS
							const char *windowName1 = "WindowCamera1";
							cvNamedWindow(windowName1, CV_WINDOW_AUTOSIZE);
							cvShowImage(windowName1, frame1);
					#endif
					
					
					#endif
							//----------------------------------------------------------------------------------------------------
					
							//----------------------------------------------------------------------------------------------------
							//Реакция на результаты поиска.
					#if (defined (CAMERA1) || defined (FROM_FILE1))
						if (XY10==1)// 
						{
							if (XY11<-0.2)//YAW Влево.
							{
								// char textbuf[10];
								// sprintf(textbuf, "On LEFT");
								// CvFont myFont;
								// cvInitFont(&myFont,	CV_FONT_HERSHEY_PLAIN,5,5,0,10,8);
								// cvPutText(frame2,textbuf,cvPoint(frame2->width/4,frame2->height/2+20),&myFont,cvScalar(0,255,255));

								ROLL_SET(FR_ROLL - dx);
								//SetSPI2PWM(chan_buf, 5);
								//Delay(1);

								ROLL_SET(FR_ROLL + dx);
								//SetSPI2PWM(chan_buf, 5);
								// for (int i = 0; i < 500; ++i)
								// {
								// 	usleep(1000);
								// }
								// ROLL_SET(FR_ROLL);
								//SetSPI2PWM(chan_buf, 5);

							}
							else if (XY11>0.2)//YAW Вправо.
							{
								// char textbuf[10];
								// sprintf(textbuf, "On RIGHT");
								// CvFont myFont;
								// cvInitFont(&myFont,	CV_FONT_HERSHEY_PLAIN,5,5,0,10,8);
								// cvPutText(frame2,textbuf,cvPoint(frame2->width/4,frame2->height/2+20),&myFont,cvScalar(0,255,255));

								ROLL_SET(FR_ROLL + dx);
								//SetSPI2PWM(chan_buf, 5);
								//Delay(1);

								ROLL_SET(FR_ROLL - dx);
								//SetSPI2PWM(chan_buf, 5);
								// for (int i = 0; i < 500; ++i)
								// {
								// 	usleep(1000);
								// }

								ROLL_SET(FR_ROLL);
								//SetSPI2PWM(chan_buf, 5);
							}
						}
						#ifdef WRITE_ALG_CAMERA1
								printf("Первая камера\n");
								cvWriteFrame(writer_alg1, frame1);
						#endif
					#endif
							//----------------------------------------------------------------------------------------------------
				}

				if (isOnSonar)
				{
					data = ReadSonar(R_SONAR);
					printf("Data R: - %f \t", data);
					if (data > 150 && data < 250)
					{	
						// ROLL_SET(FR_ROLL);
						// for (int i = 0; i < 10; ++i)
						// {
						// 	usleep(1000);
						// }
					}
					else
					if (data < 150)
					{
						printf("Влево\n");
						ROLL_SET(FR_ROLL - dy);
						for (int i = 0; i < 100; ++i)
						{
							usleep(1000);
						}
						//Выравнивание
						ROLL_SET(FR_ROLL);
					}
					else
					if (data > 250)
					{
						printf("Вправо\n");
						ROLL_SET(FR_ROLL + dy);
						for (int i = 0; i < 100; ++i)
						{
							usleep(1000);
						}
						//Выравнивание
						ROLL_SET(FR_ROLL);
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
						//if (WBm && Fl)//Проверка наличия кадров с маркером и полом.
						//{
						unsigned short int* xy2 = find2 (frame2);
						unsigned short int XY20=xy2[0];
						unsigned short int XY21=xy2[1];
						unsigned short int XY22=xy2[2];
						unsigned short int XY23=xy2[3];
						unsigned short int XY24=xy2[4];
						unsigned short int XY25=xy2[5];
						//Вывод результатов поиска.
				#ifdef OUTPUT_CONSOLE_FRAME_RES
						printf ("PP = %d, Up = %d, Right = %d, Down = %d, Left = %d \n\n", xy2[0], xy2[1], xy2[2],  xy2[3], xy2[4]);
				#endif
						//}
				#ifdef SHOW_WINDOWS
						const char *windowName2 = "WindowCamera2";
						cvNamedWindow(windowName2, CV_WINDOW_AUTOSIZE);
						cvShowImage(windowName2, frame2);
				#endif


				#endif
						//----------------------------------------------------------------------------------------------------

				//----------------------------------------------------------------------------------------------------
				//Реакция на результаты поиска.
				#if (defined (CAMERA2) || defined (FROM_FILE2))

					if (XY20 > 0)
					{
						isDownCam = true;
						isOnSonar = false;
						if (FirstDot)
						{
							printf("Сонар отключен\n");
							FirstDot = false;
						}
						if (XY20 > 2)
						{
							if (isFirstFrwrd)
							{
								isFirstFrwrd = false;
								PITCH_SET(FR_PITCH + 8);
								Delay(1);
	
								PITCH_SET(FR_PITCH);
							}
						}
						if (XY20 == 4 || XY25 == 1/*|| (XY21 == 1 && XY23 == 1)  || (XY22 == 1 && XY24 == 1)*/)
						{
							char textbuf[10];
							sprintf(textbuf, "LANDING");
							CvFont myFont;
							cvInitFont(&myFont,	CV_FONT_HERSHEY_PLAIN,5,5,0,10,8);
							if (XY20 == 4) //Посадка по 4 точкам и перпендикулярным отрезкам. Желтый текст.
							{
								cvPutText(frame2,textbuf,cvPoint(frame2->width/4,frame2->height/2+20),&myFont,cvScalar(0,255,255));
							}
							else if (XY25==1)//Посадка только по перпендикулярным отрезкам. Красный текст.
							{
								cvPutText(frame2,textbuf,cvPoint(frame2->width/4,frame2->height/2+20),&myFont,cvScalar(0,255,0));
							}
							cvWriteFrame(writer_alg2, frame2);
							break;
						}
						else
						{
							isOnSonar = true;
							FirstDot = true;
						}

						// Движение вправо
						if (XY21 == 1 && XY23 == 0)
						{
							ROLL_SET(FR_ROLL + dx);
							for (int i = 0; i < 50; ++i)
							{
								usleep(1000);
							}
							ROLL_SET(FR_ROLL);
						}
						
						// Движение влево
						if (XY23 == 1 && XY21 == 0)
						{
							ROLL_SET(FR_ROLL - dx);
							for (int i = 0; i < 50; ++i)
							{
								usleep(1000);
							}
							ROLL_SET(FR_ROLL);
						}
						
						// Движение вперед
						if (XY24 == 1 && XY22 == 0)
						{
							// char textbuf3[12];
							// sprintf(textbuf3, "Frwrd ROLL");
							// CvFont myFont3;
							// cvInitFont(&myFont3,	CV_FONT_HERSHEY_PLAIN,5,5,0,10,8);
							// cvPutText(frame2,textbuf3,cvPoint(frame2->width/4,frame2->height/2+20),&myFont3,cvScalar(0,255,255));

							PITCH_SET(FR_PITCH - dx);
							// for (int i = 0; i < 200; ++i)
							// {
							// 	usleep(1000);
							// }
							PITCH_SET(FR_PITCH);

						}
						
						// Движение назад
						if (XY22 == 1 && XY24 == 0)
						{
							// char textbuf4[12];
							// sprintf(textbuf4, "Back ROLL");
							// CvFont myFont4;
							// cvInitFont(&myFont4,	CV_FONT_HERSHEY_PLAIN,5,5,0,10,8);
							// cvPutText(frame2,textbuf4,cvPoint(frame2->width/4,frame2->height/2+20),&myFont4,cvScalar(0,255,255));

							PITCH_SET(FR_PITCH + dx);
							for (int i = 0; i < 50; ++i)
							{
								usleep(1000);
							}
							PITCH_SET(FR_PITCH);
						}

					}
				// if (XY20==4) //Посадка.
				// {
				// 	char textbuf[10];
				// 	sprintf(textbuf, "LANDING");
				// 	CvFont myFont;
				// 	cvInitFont(&myFont,	CV_FONT_HERSHEY_PLAIN,5,5,0,10,8);
				// 	cvPutText(frame2,textbuf,cvPoint(frame2->width/4,frame2->height/2+20),&myFont,cvScalar(0,255,255));
				// }
				// /*if () //Roll влево. 
				// {

				// }*/

				#ifdef WRITE_ALG_CAMERA2
						printf("Вторая камера\n");
						system("date");
						cvWriteFrame(writer_alg2, frame2);
				#endif
				#endif
						char c = cvWaitKey(33);
						//----------------------------------------------------------------------------------------------------
			// } //else
		}

	}

	//==========================================
	// 			ПОЛЕТ ПО СОНАРАМ
	//==========================================
	if ((argc > 1) && !strcmp(argv[1], "sonar"))
	{	
		printf("Полет по сонарам\n");
		float data;
		int dy = 5;
		while(1)
		{
			data = ReadSonar(R_SONAR);
			printf("Data R: - %f \t", data);
			if (data > 150 && data < 250)
			{	
				// ROLL_SET(FR_ROLL);
				for (int i = 0; i < 10; ++i)
				{
					usleep(1000);
				}
			}
			else
			if (data < 150)
			{
				printf("Влево\n");
				ROLL_SET(FR_ROLL - dy);
				for (int i = 0; i < 500; ++i)
				{
					usleep(1000);
				}
				//Торможение
				/*ROLL_SET(FR_ROLL + dy);
				for (int i = 0; i < 250; ++i)
				{
					usleep(1000);
				}*/
				//Выравнивание
				ROLL_SET(FR_ROLL);
			}
			else
			if (data > 250)
			{
				printf("Вправо\n");
				ROLL_SET(FR_ROLL + dy);
				for (int i = 0; i < 500; ++i)
				{
					usleep(1000);
				}
				//Торможение
				/*ROLL_SET(FR_ROLL - dy);
				for (int i = 0; i < 250; ++i)
				{
					usleep(1000);
				}*/
				//Выравнивание
				ROLL_SET(FR_ROLL);
			}
			data = ReadSonar(F_SONAR);
			printf("Data F: - %f \t", data);
			if (data <= 250)
			{
				PITCH_SET(FR_PITCH + 4);
				Delay(1);
				PITCH_SET(FR_PITCH);
				break;
			}
		}

		//Поворот по компасу на 90 градусов
		// while(g_Compass == 0)	serial_wait(fd);

		/*printf("Compass1: %d\n", g_Compass);
		int deg;
		if (g_Compass < 90) deg = 360 - (90 - g_Compass);
		else deg = g_Compass - 90;
		printf("Азимут первого поворота: %d\n", deg);
		int dc = 5;
		YAW_SET(FR_YAW-4);
		while (g_Compass > (deg + dc))
		{
			serial_wait(fd);
		}		
		YAW_SET(FR_YAW);
		PITCH_SET(FR_PITCH - 6);
		Delay(2);
		PITCH_SET(FR_PITCH + 2);
		Delay(1);
		PITCH_SET(FR_PITCH);*/
		//Конец поворта по компасу на 90 градусов
		// serial_wait(fd);
	}

	//==========================================
	// 				ПОЛЕТ ВПЕРЕД
	//==========================================
		int asd = 0;
		time_t dif;
	if ((argc > 1) && !strcmp(argv[1], "kt3"))
	{
		printf("Полет Г\n");

		time_t diff = time(0);
		now = time(0);

		int diffUR = 0;
		int diffUF = 0;

		PITCH_SET(FR_PITCH - 10);
		while ((now - diff) < 30)
		{	
			now = time(0);
			if ((now - diff) > 7)
			{
				FR_PITCH - 5;
			}
			printf("Sonar: %f\n", ReadSonar(SONAR1));
			int koef = 4;
			// for (int i = 0; i < 10; ++i)
			// {
				serial_wait();
			// }
			printf("Compass: %d Head: %d\t", g_Compass, g_Head);
			if ((g_Compass - g_Head) < 0)
			{
				int kr = 1;
				printf("Вправо YAW; ");
				if ((int)(FR_YAW - (g_Compass - g_Head) * koef * kr) < 0)
				{
					YAW_SET(0);	
				}
				else
				if ((int)(FR_YAW - (g_Compass - g_Head) * koef * kr) > 100)
				{
					YAW_SET(100);	
				}
				else YAW_SET(FR_YAW - (g_Compass - g_Head) * koef * kr);
				// for (int i = 0; i < 50; ++i)
				// {
				int kroll = 1;
				// ROLL_SET(FR_ROLL + 1 * kroll);
				printf("Roll: %d\t", FR_ROLL + 1 * kroll);
				usleep(1000);
				// }
				YAW_SET(FR_YAW);
				// ROLL_SET(FR_ROLL);
			}
			else
			if ((g_Compass - g_Head) > 0)	
			{
				int kl = 1;
				printf("Влево YAW\t");
				if ((int)(FR_YAW - (g_Compass - g_Head) * koef * kl) < 0)
				{
					YAW_SET(0);	
				}
				else
				if ((int)(FR_YAW - (g_Compass - g_Head) * koef * kl) > 100)
				{
					YAW_SET(100);	
				}
				else YAW_SET(FR_YAW - (g_Compass - g_Head) * koef * kl);
				// for (int i = 0; i < 50; ++i)
				// {
				int kroll = 1;
				// ROLL_SET(FR_ROLL - 1 * kroll);
				printf("Roll: %d\t", FR_ROLL - 1 * kroll);
				usleep(1000);
				// }
				YAW_SET(FR_YAW);
				// ROLL_SET(FR_ROLL);
			}
			else
			{
				printf("Середина YAW\t\n");
			}

			float data3;
			data3 = ReadSonar(R_SONAR);
			printf("Data R: - %f ", data3);
			printf("Разница R: %f\t", data3 - diffUR);
			diffUR = data3;
			if (data3 > 150 && data3 < 250)
			{	
				usleep(1000);
			}
			else
			if (data3 < 150)
			{
				printf("Влево ");
				// ROLL_SET(FR_ROLL - 15);
				// for (int i = 0; i < 10; ++i)
				// {
					// usleep(1000);
				// }
				//Выравнивание
				// ROLL_SET(FR_ROLL);
			}
			else
			if (data3 > 250)
			{
				printf("Вправо\t");
				// ROLL_SET(FR_ROLL + 15);
				// for (int i = 0; i < 10; ++i)
				// {
					// usleep(1000);
				// }
				// //Выравнивание
				// ROLL_SET(FR_ROLL);
			}

			data3 = ReadSonar(F_SONAR);
			printf("Data F: - %f ", data3);
			printf("Разница F: %f", data3 - diffUF);
			diffUF = data3;
			if (data3 < 200)
			{
				printf("Посадка\n");
				PITCH_SET(FR_PITCH + 10);
				break;
			}
			else
			{
				usleep(1000);
			}

			printf("\n");

		}

		Delay(1);
		PITCH_SET(FR_PITCH);	
	
	}

	//==========================================
	// 					ПОСАДКА
	//==========================================
	if ((argc > 1) && (strcmp(argv[1], "test")) && (strcmp(argv[1], "arm")))
	{
		printf("Посадка\n");
		system("date");

		// PITCH_SET(FR_PITCH + 4);
		// Delay(1);
		// PITCH_SET(FR_PITCH);

		// Throttle, Time
//		LiftDown(thr, 1);
		MODE_SET(0);

		while(thr > 40 && ReadSonar(SONAR1) > 40)
		{
			printf("Высота: %f, ", ReadSonar(SONAR1));
			thr -= 7;
			printf("%d, ", thr);
			THROTTLE_SET(thr);
			usleep(30000);
			thr += 7;
			printf("%d\n", thr);
			THROTTLE_SET(thr);
			usleep(30000);
			thr -= 1;
		}
		THROTTLE_SET(54);
		usleep(50000);
		THROTTLE_SET(0);



		printf("Двигатели отключены\n");
		system("date");		
		serial_wait();
		printf("Compass: %d\n", g_Compass);	

	}
	// close_port(fd);
	CloseMavlink();
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
					printw("T:%d ", g_chan[2]);
					THROTTLE_SET(g_chan[2]);
					//SetSPI2PWM(chan_buf, 5);
					break;
				}
				case 's':
				{
					g_chan[2] -= 10;
					if (g_chan[2] <= 0) g_chan[2] = 0;
					printw("T:%d ", g_chan[2]);
					THROTTLE_SET(g_chan[2]);
					//SetSPI2PWM(chan_buf, 5);
					break;
				}
				case 'a': 
				{
					g_chan[0] -= 1;
					if (g_chan[0] <= 0) g_chan[0] = 0;
					printw("Y:%d ", g_chan[0]);
					YAW_SET(g_chan[0]);
					//SetSPI2PWM(chan_buf, 5);
					break;
					
				}
				case 'd':
				{
					g_chan[0] += 1;
					if (g_chan[0] >= 100) g_chan[0] = 100;
					printw("Y:%d ", g_chan[0]);
					YAW_SET(g_chan[0]);
					//SetSPI2PWM(chan_buf, 5);
					break;
				}
				
				case 'k': 
				{
					g_chan[1] += 1;
					if (g_chan[1] >= 100) g_chan[1] = 100;
					printw("P:%d ", g_chan[1]);
					PITCH_SET(g_chan[1]);
					//SetSPI2PWM(chan_buf, 5);
					break;
				}
				case 'i':
				{
					g_chan[1] -= 1;
					if (g_chan[1] <= 0) g_chan[1] = 0;
					printw("P:%d ", g_chan[1]);
					PITCH_SET(g_chan[1]);
					//SetSPI2PWM(chan_buf, 5);
					break;
				}
				case 'j': 
				{
					g_chan[3] -= 1;
					if (g_chan[3] <= 0) g_chan[3] = 0;
					printw("R:%d ", g_chan[3]);
					ROLL_SET(g_chan[3]);
					//SetSPI2PWM(chan_buf, 5);
					break;
					
				}
				case 'l':
				{
					g_chan[3] += 1;
					if (g_chan[3] >= 100) g_chan[3] = 100;
					printw("R:%d ", g_chan[3]);
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

	//-------------Освобождение памяти, разрушение окон и т.д.--------------------------------------------
	if ((argc > 1) && (strcmp(argv[1], "cam") == 0))
	{
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
	}
	return 0;
}

void Delay(int time)
{
	sleep(time);
}

int FlyProfile(float h)
{
	float norm_deg;
	float deg;
	float rad;
	float sinus;
	float thr;

	norm_deg = 0.4;
	deg = (h - 20) * norm_deg;
	rad = deg / 57.2974693618;
	sinus = sin(rad);
	thr = (int)(46 + 28 * sinus);

	return thr;
}