#include "find2.h"
#define G 255
#define R 0
#define B 0

#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp" 


double* find2 (CvCapture* capture, CvVideoWriter * wr)
{
	double xy[5]={0.0};
	IplImage* src = cvQueryFrame( capture );
	if(!src)
	{
		xy[0]=-1;
		return xy;
	}

	IplImage* dst=0; 
	IplImage* color_dst=0;

	// хранилище памяти для хранения найденных линий
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* lines = 0;
	int i = 0;
	//IplImage* dst2=cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );

	color_dst = cvCreateImage( cvGetSize(src), 8, 3 );
	dst = cvCreateImage( cvGetSize(src), 8, 1 );
	//cvConvertImage(src,dst,0);
	//cvSmooth(src, dst2, CV_BLUR_NO_SCALE, 2, 2);
	//cvAdaptiveThreshold(dst, dst2, 250, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 7, 1);

	// детектирование границ
	cvCanny( src, dst, 50, 100, 3 );

	// конвертируем в цветное изображение
	cvCvtColor( dst, color_dst, CV_GRAY2BGR );

	// нахождение линий
	lines = cvHoughLines2( dst, storage, CV_HOUGH_PROBABILISTIC, 1, CV_PI/180, 50, 200, 50 );

	class Vect
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

	std::list<short unsigned int> MXX,MYY;
	// нарисуем найденные линии
	for( i = 0; i < lines->total; i++ ){
		CvPoint* line = (CvPoint*)cvGetSeqElem(lines,i);
		cvLine( color_dst, line[0], line[1], CV_RGB(255,0,0), 3, CV_AA, 0 );
		Vec.x[i]=line[1].x-line[0].x;///////// Для каждого отрезка найдём вектор (xn-ck,yn-yk)
		Vec.y[i]=line[1].y-line[0].y;
		
		MXX.push_back(line[0].x);
		MXX.push_back(line[1].x);
		MYY.push_back(line[0].y);
		MYY.push_back(line[1].y);
	}
	int countPer=0, countPar=0, countN=0;
		
	int Os=100;
	//double a1=0.0, b1=0.0, a2=0.0, b2=0.0;
	for (int i=0; i<lines->total; i++)
	{
		CvPoint* line = (CvPoint*)cvGetSeqElem(lines,i);
		/////for (std::list<short unsigned int>::iterator x2=VX.begin(),std::list<short unsigned int>::iterator y2=VY.begin();x2!=VX.end(),y2!=VY.end();x2++,y2++)
		for (int j=i+1; j<lines->total; j++)
		{
			CvPoint* line2 = (CvPoint*)cvGetSeqElem(lines,j);
			if(((Vec.x[i])*(Vec.y[j])-(Vec.x[j])*(Vec.y[i]))>=-2 && ((Vec.x[i])*(Vec.y[j])-(Vec.x[i])*(Vec.y[j]))<=2)//Параллельность
			{
				countPar++;
			}
			else if  ( ((Vec.x[i])*(Vec.x[j])+(Vec.y[i])*(Vec.y[j]))>=-Os && ((Vec.x[i])*(Vec.x[j])+(Vec.y[i])*(Vec.y[j]))<=Os)//Перпендикулярность
			{
				countPer++;
				/*MXX.push_back(line[0].x);
				MXX.push_back(line[1].x);
				MYY.push_back(line[0].y);
				MYY.push_back(line[1].y);*/
			}
			else //всё остальное
			{
				countN++;
			}
		}
		//printf ("Per= %d, Par = %d, N = %d \n\n Vsego = %d, Lt = %d \n\n ",countPer, countPar, countN, countPer+countPar+countN, lines->total);

	}

	////////////Если каждая из линий (параллельна или перпендикулярна всем (почти-2-4- 5) остальным (но не иначе))
	///////////В идеале 8 прямых, по 3 параллельных и 4 перпендикулярных для каждой
	//for (std::list<short unsigned int>::iterator x1=VX.begin(), std::list<short unsigned int>::iterator y1=VY.begin(); x1!=VX.end(),y1!=VY.end();x1++,y1++)
	/////////В цикле выясним перпендикулярность x1*x2+y1*y2=0 - перпендикулярны
	//int vs= countPer+countPar+countN;
	/*double sumx=0.0,sumy=0.0;
	printf ("Per= %d, Par = %d, N = %d \n\n Vsego = %d, Lt = %d, X =%f, Y=%f \n\n ",countPer, countPar, countN, countPer+countPar+countN, lines->total,sumx,sumy);
	int maxp=0, jj=0;
	for (int i=0; i<lines->total;i++)
	{
		if (Vec.p[i] > maxp)
		{
			maxp=Vec.p[i];
			Vec.maxp[i]=true;
			Vec.maxp[jj]=false;
			jj=i;
		}
	}
	CvPoint* line = (CvPoint*)cvGetSeqElem(lines,jj);
	/////for (std::list<short unsigned int>::iterator x2=VX.begin(),std::list<short unsigned int>::iterator y2=VY.begin();x2!=VX.end(),y2!=VY.end();x2++,y2++)
	for (int j=jj+1; j<lines->total; j++)
	{
		CvPoint* line2 = (CvPoint*)cvGetSeqElem(lines,j);
		if  ( ((Vec.x[i])*(Vec.x[j])+(Vec.y[i])*(Vec.y[j]))>=-Os && ((Vec.x[i])*(Vec.x[j])+(Vec.y[i])*(Vec.y[j]))<=Os)//Перпендикулярность
		{
			//////////ищем точку пересечения
			/*if ((line[1].x-line[0].x)!=0 && (line[1].x-line[0].x )!=0 && (line2[1].x-line2[0].x)!=0)
			{

			a1=(double)((line[1].y-line[0].y)/(line[1].x-line[0].x));
			b1=(double)( line[0].y - (double)( (line[0].x*(line[1].y-line[0].y))/(line[1].x-line[0].x ) ) );

			a2=(double)((line2[1].y-line2[0].y)/(line2[1].x-line2[0].x));
			b2=(double)( line2[0].y - (double)( (line2[0].x*(line2[1].y-line2[0].y))/(line2[1].x-line2[0].x ) ) );
			///x
			OXY[countPer][0]= ((a1*(b1-b2))/(a2-a1) - b1)*a2;
			//y
			OXY[countPer][1]= (a1*(b1-b2))/(a2-a1);
			sumx += OXY[i][0];
			sumy += OXY[i][1];
			}
			}
			}

			///А также параллельность x1y2-x2y1=0 - параллельность
			//Находить ближайшие к центру координаты и из них - среднее
			*/


			if (lines->total!=0 && countPer>=5)
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
			int k2=10;
				for (int x=MX-k2; x<MX+k2; x++)// горизонтальная линия
			{
				((uchar *)(color_dst->imageData + MY*color_dst->widthStep))[3*x+1]=G;
				((uchar *)(color_dst->imageData + MY*color_dst->widthStep))[3*x+2]=R;
				((uchar *)(color_dst->imageData + MY*color_dst->widthStep))[3*x+3]=B;
			}
			for (int y=MY-k2; y<MY+k2; y++)// вертикальная линия
			{
				((uchar *)(color_dst->imageData + y*color_dst->widthStep))[3*MX+1]=G;
				((uchar *)(color_dst->imageData + y*color_dst->widthStep))[3*MX+2]=R;
				((uchar *)(color_dst->imageData + y*color_dst->widthStep))[3*MX+3]=B;
			}
				/*CvPoint* line=new CvPoint();
				line[0].x=MX-10;
				line[0].y=MY;
				line[1].x=MX+10;
				line[1].y=MY;
				cvLine( color_dst, line[0], line[1], CV_RGB(0,255,0), 3, CV_AA, 0 );
				line[0].x=MX;
				line[0].y=MY-10;
				line[1].x=MX;
				line[1].y=MY+10;
				cvLine( color_dst, line[0], line[1], CV_RGB(0,255,0), 3, CV_AA, 0 );
				*/
				xy[0]=1;
				xy[1]=MX;
				xy[2]=MY;
				
				double xx = (double)(src->width/2);
				double yy = (double)(src->height/2);
				xy[3]=(xy[1]-xx)/xx;
				xy[4]=(xy[2]-yy)/yy;
			}

			//cvNamedWindow( "g", 1 );
			//cvShowImage( "g", dst2 );

			// cvNamedWindow( "Source", 1 );
			// cvShowImage( "Source", src );

			// cvNamedWindow( "Hough", 1 );
			// cvShowImage( "Hough", color_dst );

			cvWriteFrame(wr, color_dst);

			// освобождаем ресурсы
			cvReleaseMemStorage(&storage);
			// cvReleaseImage(&src);
			cvReleaseImage(&dst);
			//	cvReleaseImage(&dst2);
			cvReleaseImage(&color_dst);
			// cvDestroyAllWindows();
			return xy;
		}