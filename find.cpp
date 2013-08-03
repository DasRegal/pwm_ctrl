//#include "stdafx.h"
#include "find.h"
#include "define.h"
/*	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Важное условие работы программы - цвет пола должен быть менее ярким, чем белая 
часть стартового маркера и более ярким, чем чёрная часть маркера.
Также важно получить изображение маркера на старте так, чтобы не было видно ничего кроме него,
а затем изображение пола при полном отсутствии маркера.
*/

int getW=0, getB=0, getF=0;//Полученные значения цветов с кадров маркера и пола для ипользования в функциях поиска маркера.
int W_B=0; //Величина перепада между чёрным и белым.
/* ======================================================================
** Функция поиска маркера для первой (верхней) камеры.
** ======================================================================
** Возвращаемые значения функции double* find1:
//xy1[0] - Есть или нет в кадре посадочная площадка (ПП).
//xy1[0] = 1 - Есть,
//xy1[0] = 0 - нет.
//xy1[1] - Отклонение от -1 до 1 по оси х (слева или справа от центральной вертикальной оси кадра находится центр ПП).
//xy1[2] - Расстояние.
** ======================================================================
*/
#if (defined (CAMERA1) || defined (FROM_FILE1))
#if (defined(WRITE_ALG_CAMERA1) && (defined (CAMERA1) || defined (FROM_FILE1))) 
double* find1 (IplImage* frame1, CvVideoWriter * writer_alg1)
#elif  (defined (CAMERA1) || defined (FROM_FILE1))
double* find1 (IplImage* frame1)
#endif
{
	//--------------------------------Область алгоритма---------------------------------------------------
	double xy[3]={0.0};

	IplImage* frameWB = cvCreateImage(cvSize(frame1->width,frame1->height),8,1);
	cvConvertImage(frame1,frameWB,0);

	int Xmin=frame1->width, Xmax=0, Ymin=frame1->height, Ymax=0;
	double MX=0, MY=0, DX=0, DY=0;
	int XYcount=0;
	
	std::list<short unsigned int> MXX,MYY;

	for (int y=1; y<frameWB->height-3; y+=5) //контраст i-того и i+2 пикселей (вертикальных)
	{
		uchar *ptr = (uchar*) (
			frameWB->imageData + y * frameWB->widthStep) ;
		for (int x=1; x<frameWB->width; x++)
		{
			uchar *ptr2 = (uchar*) (
				frameWB->imageData + (y+2) * frameWB->widthStep);
			if( W_B<=abs (ptr[x]- ptr2[x]))
			{
				MX+=x; MY+=y+1;
				MXX.push_back(x);
				MYY.push_back(y+1);
				XYcount++;
				/*if (ptr[x]- ptr2[x]<0)
				{
					((uchar *)(frameWB2->imageData + (y)*frameWB2->widthStep))[x]=0; //черный
					((uchar *)(frameWB2->imageData + (y+2)*frameWB2->widthStep))[x]=255; //белый
				}
				else
				{
					((uchar *)(frameWB2->imageData + (y+2)*frameWB2->widthStep))[x]=0; //черный
					((uchar *)(frameWB2->imageData + (y)*frameWB2->widthStep))[x]=255; //белый
				}*/
			}
		}
	}
	if (XYcount!=0 && XYcount>40)
	{
		MX/=XYcount; MY/=XYcount;
		for(std::list<short unsigned int>::iterator i=MXX.begin();i!=MXX.end();i++)
		{
			DX+=abs(*i -MX);
		}
		for(std::list<short unsigned int>::iterator i=MYY.begin();i!=MYY.end();i++)
		{
			DY+=abs(*i -MY);
		}
		DX=abs(DX/XYcount); DY=abs(DY/XYcount);

		int k=1; 
		double fw2 = (double)(frame1->width/2);
		xy[0]=1;
		xy[1]=(xy[0]-fw2)/fw2;

		for (int y=1; y<frameWB->height-3; y+=5)
		{
			uchar *ptr = (uchar*) (
				frameWB->imageData + y * frameWB->widthStep) ;
			for (int x=1; x<frameWB->width; x++)
			{

				uchar *ptr2 = (uchar*) (
					frameWB->imageData + (y+2) * frameWB->widthStep); 

				if	((W_B>abs (ptr[x]- ptr2[x])) && (x<(MX+k*DX) )&& (x>(MX-k*DX))&& (y<(MY+k*DY)) && (y>(MY-k*DY)))
				{
					if (x<Xmin)
					{
						Xmin=x;
					}
					if (x>Xmax)
					{
						Xmax=x;
					}
					if (y<Ymin)
					{
						Ymin=y;
					}
					if (y>Ymax)
					{
						Ymax=y;
					}
				}
			}
		}
		CvPoint pt = cvPoint( cvRound( MX ), cvRound( MY ) );
			CvSize   axes = cvSize(Xmax-Xmin, Ymax-Ymin);
			cvEllipse(
			frame1,
			pt, // Центр
			axes, // Длина большой и малой оси
			0, // Угол от оси X, против часовой стрелки
			0, // Начальный угол
			360, // Конечный угол
			CV_RGB(255,0,0), // Цвет
			3
			);
		
		xy[2]=(double)3000/(Xmax-Xmin);
	}
	else 
	{
		xy[0]=0;
		xy[1]=0;
		xy[2]=0;
	}

	MXX.clear();
	MYY.clear();
	cvReleaseImage ( &frameWB);
	//----------------------------------------------------------------------------------------------------

#ifdef SHOW_WINDOWS
	const char *windowName1 = "WindowCamera1";
	cvNamedWindow(windowName1, CV_WINDOW_AUTOSIZE);
	cvShowImage(windowName1, frame1);
#endif

#ifdef WRITE_ALG_CAMERA1
	cvWriteFrame(writer_alg1, frame1);
#endif
	return xy;
}
#endif



/* ======================================================================
** Функция поиска маркера для второй (нижней) камеры.
** ======================================================================
** Возвращаемые значения функции double* find2:
//xy1[0] - Количество найденных направлений на маркер. От 0 до 4.
//xy1[1] = 1 - Есть маркер вверху ОТНОСИТЕЛЬНО КАДРА. xy1[1] = 0 - нет вверху.
//xy1[2] = 1 - Справа.
//xy1[3] = 1 - Внизу.
//xy1[4] = 1 - Слева.
** ======================================================================
*/
#define block_size 20	
int delta_edge = 5; //Отступ от краёв кадра.

#define delta_brightness_max 2

//const int block_size2 = block_size*block_size;
const int B = 256; //Для "гистограммы".

#define Radius 5

int brightness_spot (IplImage* src, int lvu_block_x, int lvu_block_y)
{
	int brightness=0;
	int l_12 = 0;
	int countlB[B]={0};
	int max=0;
	for (int y = lvu_block_y; y < (lvu_block_y+block_size);y++)
	{
		uchar *ptr = (uchar*) (src->imageData + y * src->widthStep);
		for (int x = lvu_block_x; x < (lvu_block_x+block_size); x++)
		{
			brightness = (ptr [3*x+1]+ptr [3*x+2] +ptr [3*x+3] )/3;
			countlB[brightness]++;
			((uchar *)(src->imageData + y*src->widthStep))[3*x+1]=255;
			((uchar *)(src->imageData + y*src->widthStep))[3*x+3]=255;
			((uchar *)(src->imageData + y*src->widthStep))[3*x+2]=255;

		}
	}
	for (int bw=0; bw<B;bw++)//Поиск максимума.
	{
		if (countlB[bw]>max)
		{
			max=countlB[bw];
			l_12 = bw;
		}
	}
	return l_12;
}

#if (defined (CAMERA2) || defined (FROM_FILE2))
#if (defined(WRITE_ALG_CAMERA2) && (defined (CAMERA2) || defined (FROM_FILE2))) 
unsigned short int* find2 (IplImage* frame2, CvVideoWriter * writer_alg2)
#elif  (defined (CAMERA2) || defined (FROM_FILE2))
unsigned short int* find2 (IplImage* frame2)
#endif
{
	//--------------------------------Область алгоритма---------------------------------------------------
	unsigned short int xy[5]={0};

	int up = brightness_spot (frame2, (frame2->width/2)-block_size/2, delta_edge);
	int right = brightness_spot (frame2, frame2->width-block_size-delta_edge, (frame2->height/2)-block_size/2);
	int down = brightness_spot (frame2, (frame2->width/2)-block_size/2, frame2->height-block_size-delta_edge);
	int left = brightness_spot (frame2, delta_edge, (frame2->height/2)-block_size/2);

	if ( up>=getW-delta_brightness_max || up<=getB+delta_brightness_max)
	{
		xy[0]++;
		xy[1]=1;
		CvPoint pt = cvPoint( cvRound( frame2->width/2 ), cvRound( delta_edge*3 ) );
		cvCircle(frame2, pt, cvRound( Radius ), CV_RGB(255,0,0), 10 );

		if ( up>=getW-delta_brightness_max )
		{
			cvCircle(frame2, pt, cvRound( Radius/2 ), CV_RGB(255,255,255), 5 );
		}
		else
		{
			cvCircle(frame2, pt, cvRound( Radius/2 ), CV_RGB(0,0,0), 5 );
		}
	}
	//if ( abs(right-w)<=delta_brightness_max || abs(right-b)<=delta_brightness_max)
	if ( right>=getW-delta_brightness_max || right<=getB+delta_brightness_max)
	{
		xy[0]++;
		xy[2]=1;
		CvPoint pt = cvPoint( cvRound(frame2->width - delta_edge*3  ), cvRound( frame2->height/2 ) );
		cvCircle(frame2, pt, cvRound( Radius ), CV_RGB(255,0,0), 10 );

		if ( right>=getW-delta_brightness_max)
		{
			cvCircle(frame2, pt, cvRound( Radius/2 ), CV_RGB(255,255,255), 5 );
		}
		else
		{
			cvCircle(frame2, pt, cvRound( Radius/2 ), CV_RGB(0,0,0), 5 );
		}
	}
	//if ( abs(down-w)<=delta_brightness_max || abs(down-b)<=delta_brightness_max)
	if ( down>=getW-delta_brightness_max || down<=getB+delta_brightness_max)
	{
		xy[0]++;
		xy[3]=1;
		CvPoint pt = cvPoint( cvRound( frame2->width/2 ), cvRound(frame2->height - delta_edge*3 ) );
		cvCircle(frame2, pt, cvRound( Radius ), CV_RGB(255,0,0), 10 );

		if ( down>=getW-delta_brightness_max)
		{
			cvCircle(frame2, pt, cvRound( Radius/2 ), CV_RGB(255,255,255), 5 );
		}
		else
		{
			cvCircle(frame2, pt, cvRound( Radius/2 ), CV_RGB(0,0,0), 5 );
		}

	}
	//if ( abs(left-w)<=delta_brightness_max || abs(left-b)<=delta_brightness_max)
	if ( left>=getW-delta_brightness_max || left<=getB+delta_brightness_max)
	{
		xy[0]++;
		xy[4]=1;
		CvPoint pt = cvPoint( cvRound( delta_edge*3 ), cvRound(frame2->height/2 ) );
		cvCircle(frame2, pt, cvRound( Radius ), CV_RGB(255,0,0), 10 );
		if (left>=getW-delta_brightness_max)
		{
			cvCircle(frame2, pt, cvRound( Radius/2 ), CV_RGB(255,255,255), 5 );
		}
		else
		{
			cvCircle(frame2, pt, cvRound( Radius/2 ), CV_RGB(0,0,0), 5 );
		}
	}
	//----------------------------------------------------------------------------------------------------

#ifdef SHOW_WINDOWS
	const char *windowName2 = "WindowCamera2";
	cvNamedWindow(windowName2, CV_WINDOW_AUTOSIZE);
	cvShowImage(windowName2, frame2);
#endif

#ifdef WRITE_ALG_CAMERA2
	cvWriteFrame(writer_alg2, frame2);
#endif
	return xy;
}
#endif



/* ======================================================================
** Функция для второй (нижней) камеры. Предназначена для взятия и анализа
** кадра с изображением пола (асфальта) во время полёта точно не над 
** маркером. Обработка ранее полученного кадра маркера с помощью текущего.
** ======================================================================
** Устанавливает:
** //getW - Полученное значение "белого" цвета маркера.
** //getB - Полученное значение "чёрного" цвета маркера.
** //getF - Полученное значение "серого" цвета пола.
** ======================================================================
*/
void getFloor (IplImage* floor, char* fname_floor, char* fname_marker)
{
	//--------------------------------Область алгоритма---------------------------------------------------

	IplImage* frameflWB = cvCreateImage(cvSize(floor->width,floor->height),8,1);
	cvConvertImage(floor,frameflWB,0);
	int countflB[B]={0};
	int max=0;
	for (int y=0; y<frameflWB->height;y++)
	{
		uchar *ptr = (uchar*) (
			frameflWB->imageData + y * frameflWB->widthStep) ;
		for (int x=0; x<frameflWB->width;x++)
		{
			countflB[ptr[x]]++;
		}
	}
	for (int i=0; i<B;i++) //Поиск серого как максимума.
	{
		if (countflB[i]>max)
		{
			max=countflB[i];
			getF = i;
		}
	}
	max=0;
	IplImage* marker = cvLoadImage (fname_marker);
	IplImage* framemWB = cvCreateImage(cvSize(marker->width,marker->height),8,1);
	cvConvertImage(marker,framemWB,0);
	int countmB[B]={0};
	for (int y=0; y<framemWB->height;y++)
	{
		uchar *ptr = (uchar*) (
			framemWB->imageData + y * framemWB->widthStep) ;
		for (int x=0; x<framemWB->width;x++)
		{
			countmB[ptr[x]]++;
		}
	}
	for (int b=0; b<getF;b++)//Поиск чёрного как максимума до серого.
	{
		if (countmB[b]>max)
		{
			max=countmB[b];
			getB = b;
		}
	}
	max=0;
	for (int w=getF+1; w<B;w++)//Поиск белого как максимума после серого.
	{
		if (countmB[w]>max)
		{
			max=max=countmB[w];
			getW = w;
		}
	}
	W_B=getW-getB;
	cvReleaseImage ( &frameflWB);
	cvReleaseImage ( &framemWB);
	//----------------------------------------------------------------------------------------------------
#if defined (WRITE_FLOOR)
	cvSaveImage(fname_floor,floor);
#endif
	return;
}
