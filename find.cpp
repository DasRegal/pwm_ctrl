// #include "StdAfx.h"
#include "find.h"
#define G 255
#define R 0
#define B 0


double* find (CvCapture* capture, int pMin, int pMax, CvVideoWriter * wr)
{
	double xy[4]={0.0};
	IplImage* frame = cvQueryFrame( capture );
	if(!frame)
	{
		return xy;
	}
	IplImage* frameWB = cvCreateImage(cvSize(frame->width,frame->height),8,1);
	IplImage* frameWB2 = cvCreateImage(cvSize(frame->width,frame->height),8,1);
	cvConvertImage(frame,frameWB,0);

	int cx=0;
	int Xmin=frame->width, Xmax=0, Ymin=frame->height, Ymax=0;
	double MX=0, MY=0, DX=0, DY=0;
	int XYcount=0;
	
	std::list<short unsigned int> MXX,MYY;

	for (int y=0.5*frameWB->height; y<frameWB->height-3; y++) //контраст i-того и i+2 пикселей (вертикальных)
	{
		uchar *ptr = (uchar*) (
			frameWB->imageData + y * frameWB->widthStep) ;
		for (int x=1; x<frameWB->width; x++)
		{

			uchar *ptr2 = (uchar*) (
				frameWB->imageData + (y+2) * frameWB->widthStep);
			if((pMax>abs (ptr[x]- ptr2[x])) && (abs (ptr[x]- ptr2[x])>pMin))
			{
				MX+=x; MY+=y+1;
				MXX.push_back(x);
				MYY.push_back(y+1);
				XYcount++;
				if (ptr[x]- ptr2[x]<0)
				{
					((uchar *)(frameWB2->imageData + (y)*frameWB2->widthStep))[x]=0; //черный
					((uchar *)(frameWB2->imageData + (y+2)*frameWB2->widthStep))[x]=255; //белый
				}
				else
				{
					((uchar *)(frameWB2->imageData + (y+2)*frameWB2->widthStep))[x]=0; //черный
					((uchar *)(frameWB2->imageData + (y)*frameWB2->widthStep))[x]=255; //белый
				}
			}
		}
	}
	
	if (XYcount!=0 && XYcount>50)
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

		int k=2; 
		double fw2 = (double)(frame->width/2);
		xy[0]=MX;
		xy[1]=MY;
		xy[2]=(xy[0]-fw2)/fw2;

		for (int y=1; y<frameWB->height-3; y++)
		{
			uchar *ptr = (uchar*) (
				frameWB->imageData + y * frameWB->widthStep) ;
			for (int x=1; x<frameWB->width; x++)
			{

				uchar *ptr2 = (uchar*) (
					frameWB->imageData + (y+2) * frameWB->widthStep); 

				if	((pMax>abs (ptr[x]- ptr2[x])) && (abs (ptr[x]- ptr2[x]) >pMin) && (x<(MX+k*DX) )&& (x>(MX-k*DX))&& (y<(MY+k*DY)) && (y>(MY-k*DY)))
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
		if (!(Xmin==frame->width && Xmax==0 && Ymin==frame->height && Ymax==0))///≈сли записывать видео с выполненным алгоритмом - нужно раскомментировать.
		{
			for (int x=Xmin; x<Xmax; x++)// ¬ерхн€€  и нижн€€ границы.
			{
				((uchar *)(frame->imageData + Ymin*frame->widthStep))[3*x+1]=G;
				((uchar *)(frame->imageData + Ymin*frame->widthStep))[3*x+2]=R;
				((uchar *)(frame->imageData + Ymin*frame->widthStep))[3*x+3]=B;
				((uchar *)(frame->imageData + Ymax*frame->widthStep))[3*x+1]=G;
				((uchar *)(frame->imageData + Ymax*frame->widthStep))[3*x+2]=R;
				((uchar *)(frame->imageData + Ymax*frame->widthStep))[3*x+3]=B;
			}
			for (int y=Ymin; y<Ymax; y++)// Ћева€ и права€ границы.
			{
				((uchar *)(frame->imageData + y*frame->widthStep))[3*Xmin+1]=G;
				((uchar *)(frame->imageData + y*frame->widthStep))[3*Xmin+2]=R;
				((uchar *)(frame->imageData + y*frame->widthStep))[3*Xmin+3]=B;
				((uchar *)(frame->imageData + y*frame->widthStep))[3*Xmax+1]=G;
				((uchar *)(frame->imageData + y*frame->widthStep))[3*Xmax+2]=R;
				((uchar *)(frame->imageData + y*frame->widthStep))[3*Xmax+3]=B;
			}
			for (int x=Xmin+ (Xmax-Xmin)/4; x<Xmax- (Xmax-Xmin)/4; x++)// горизонтальна€ лини€
			{
				((uchar *)(frame->imageData + ((Ymax-Ymin)/2+Ymin)*frame->widthStep))[3*x+1]=G;
				((uchar *)(frame->imageData + ((Ymax-Ymin)/2+Ymin)*frame->widthStep))[3*x+2]=R;
				((uchar *)(frame->imageData + ((Ymax-Ymin)/2+Ymin)*frame->widthStep))[3*x+3]=B;
			}
			for (int y=Ymin+ (Ymax-Ymin)/4; y<Ymax- (Ymax-Ymin)/4; y++)// вертикальна€ лини€
			{
				((uchar *)(frame->imageData + y*frame->widthStep))[3*((Xmax - Xmin)/2+Xmin)+1]=G;
				((uchar *)(frame->imageData + y*frame->widthStep))[3*((Xmax - Xmin)/2+Xmin)+2]=R;
				((uchar *)(frame->imageData + y*frame->widthStep))[3*((Xmax - Xmin)/2+Xmin)+3]=B;
			}
		}
		xy[3]=(double)3000/(Xmax-Xmin);
	}
	else 
	{
		xy[0]=-1;
		xy[1]=0;
		xy[2]=0;
		xy[3]=0;
	}
	const char *windowName = "Window";
	cvNamedWindow(windowName, CV_WINDOW_AUTOSIZE);
	cvShowImage(windowName, frame);

	// const char *windowNameWB2 = "WindowWB2";
	// cvNamedWindow(windowNameWB2, CV_WINDOW_AUTOSIZE);
	// cvShowImage(windowNameWB2, frameWB2);

	cvWriteFrame(wr, frame); //«апись видео с выполненным алгоритмом или без него.
	//cvWriteFrame(wr, frameWB2);//«апись точек контраста.---
	MXX.clear();
	MYY.clear();
	cvReleaseImage ( &frameWB);
	cvReleaseImage ( &frameWB2);
	return xy;
}
