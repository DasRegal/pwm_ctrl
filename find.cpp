#include "find.h"
#include "define.h"
/*	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Важное условие работы программы - цвет пола должен быть менее ярким, чем белая 
часть стартового маркера и более ярким, чем чёрная часть маркера.
Также важно получить изображение маркера на старте так, чтобы не было видно ничего кроме него,
а затем изображение пола при полном отсутствии маркера.
*/

int getW=230, getB=20, getF=50;//Полученные значения цветов с кадров маркера и пола для ипользования в функциях поиска маркера.
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
double* find1 (IplImage* frame1)
{
	//--------------------------------Область алгоритма---------------------------------------------------
	double xy[3]={0.0};

	IplImage* frameWB = cvCreateImage(cvSize(frame1->width,frame1->height),8,1);
	cvConvertImage(frame1,frameWB,0);

	int Xmin=frame1->width, Xmax=0, Ymin=frame1->height, Ymax=0;
	double MX=0, MY=0, DX=0, DY=0;
	int XYcount=0;

	std::list<short unsigned int> MXX,MYY;

	/*for (int y=1; y<frameWB->height-3; y+=5) //контраст i-того и i+2 пикселей (вертикальных)
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

	for (int y=1; y<frameWB->height-3; y+=5)//переделать//////
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
	/*CvPoint pt = cvPoint( cvRound( MX ), cvRound( MY ) );
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
	}*/

	MXX.clear();
	MYY.clear();
	cvReleaseImage ( &frameWB);
	//----------------------------------------------------------------------------------------------------
	return xy;
}
#endif



/* ======================================================================
** Функция поиска маркера для второй (нижней) камеры.
** ======================================================================
** Возвращаемые значения функции double* find2:
//xy2[0] - Количество найденных направлений на маркер. От 0 до 4.
//xy2[1] = 1 - Есть маркер вверху ОТНОСИТЕЛЬНО КАДРА. xy2[1] = 0 - нет вверху.
//xy2[2] = 1 - Справа.
//xy2[3] = 1 - Внизу.
//xy2[4] = 1 - Слева.
//xy2[5] = 1 - Есть хотя бы 2 перпендикулярные прямые. xy[5]=0 - нет ни одной.
Перпендикулярные прямые ищутся, если в кадре есть хотя бы 1 из направлений на круг.
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
			/*((uchar *)(src->imageData + y*src->widthStep))[3*x+1]=255;
			((uchar *)(src->imageData + y*src->widthStep))[3*x+3]=255;
			((uchar *)(src->imageData + y*src->widthStep))[3*x+2]=255;*/

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
unsigned short int* find2 (IplImage* frame2)
{
	//--------------------------------Область алгоритма---------------------------------------------------
	unsigned short int xy[6]={0};

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

	// if(xy[0]>0)//Поиск перпендикулярных линий.
	// {
		class Vect///Ограничение на количество найденных линий!!! Может быть ошибка, если вдруг превысит.
{
public: int x[1000], y[1000];
		Vect()
		{
			for (int i=0; i<1000; i++)
			{
				x[i]=0; 
				y[i]=0;
			}
		}
} Vec;

		IplImage* dst=0; 
		// хранилище памяти для хранения найденных линий
		CvMemStorage* storage = cvCreateMemStorage(0);
		CvSeq* lines = 0;
		int i = 0;
		dst = cvCreateImage( cvGetSize(frame2), 8, 1 );
		// детектирование границ
		cvCanny( frame2, dst, 50, 100, 3 );
		// конвертируем в цветное изображение
		//cvCvtColor( dst, color_dst, CV_GRAY2BGR );

		// нахождение линий
		lines = cvHoughLines2( dst, storage, CV_HOUGH_PROBABILISTIC, 1, CV_PI/180, 50, 200, 50 );

		std::list<short unsigned int> MXX,MYY;
		// Нарисуем найденные линии и найдём соответствующие им вектора.
		for( i = 0; i < lines->total; i++ ){
			CvPoint* line = (CvPoint*)cvGetSeqElem(lines,i);
			cvLine( frame2, line[0], line[1], CV_RGB(255,0,0), 3, CV_AA, 0 );
			Vec.x[i]=line[1].x-line[0].x;///////// Для каждого отрезка найдём вектор (xn-ck,yn-yk)
			Vec.y[i]=line[1].y-line[0].y;

			MXX.push_back(line[0].x);
			MXX.push_back(line[1].x);
			MYY.push_back(line[0].y);
			MYY.push_back(line[1].y);
		}

		int countPer=0;

		const short int Os=100;
		for (int i=0; i<lines->total; i++)
		{
			CvPoint* line = (CvPoint*)cvGetSeqElem(lines,i);
			/////for (std::list<short unsigned int>::iterator x2=VX.begin(),std::list<short unsigned int>::iterator y2=VY.begin();x2!=VX.end(),y2!=VY.end();x2++,y2++)
			for (int j=i+1; j<lines->total; j++)
			{
				CvPoint* line2 = (CvPoint*)cvGetSeqElem(lines,j);
				if  ( ((Vec.x[i])*(Vec.x[j])+(Vec.y[i])*(Vec.y[j]))>=-Os && ((Vec.x[i])*(Vec.x[j])+(Vec.y[i])*(Vec.y[j]))<=Os)//Перпендикулярность
				{
					countPer++;
				}
			}
		}

		if (lines->total!=0 && countPer>4)
		{
			int MX=0,MY=0;
			for(std::list<short unsigned int>::iterator i=MXX.begin();i!=MXX.end();i++)
			{
				MX+=(*i);
			}
			for(std::list<short unsigned int>::iterator i=MYY.begin();i!=MYY.end();i++)
			{
				MY+=(*i);
			}
			MX/=(lines->total*2); MY/=(lines->total*2);
			//const short int k2=10;

			//Рисуем крестик
			/*CvPoint* line=new CvPoint();
			line[0].x=MX-k2;
			line[0].y=MY;
			line[1].x=MX+k2;
			line[1].y=MY;
			cvLine( frame2, line[0], line[1], CV_RGB(0,255,0), 3, CV_AA, 0 );
			line[0].x=MX;
			line[0].y=MY-k2;
			line[1].x=MX;
			line[1].y=MY+k2;
			cvLine( frame2, line[0], line[1], CV_RGB(0,255,0), 3, CV_AA, 0 );
			*/
			
			xy[5]=1;
		}

		// освобождаем ресурсы
		cvReleaseMemStorage(&storage);
		cvReleaseImage(&dst);
		MXX.clear();
		MYY.clear();
	// }
	//----------------------------------------------------------------------------------------------------

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
	getW=(getW-getF)/2+getF;
	getB=(getF-getB)/4+getB;
	W_B=getW-getB;
	cvReleaseImage ( &frameflWB);
	cvReleaseImage ( &framemWB);
	//----------------------------------------------------------------------------------------------------
#if defined (WRITE_FLOOR)
	cvSaveImage(fname_floor,floor);
#endif
	return;
}
