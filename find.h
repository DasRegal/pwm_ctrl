#pragma once

//#include <cv.h>
//#include <highgui.h>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <list>
#include <math.h>
#include <iostream>
#include "define.h"

#if (defined(WRITE_ALG_CAMERA1) &&  ((defined (CAMERA1) || defined (FROM_FILE1)))) 
		double* find1 (IplImage* frame1, CvVideoWriter * writer_alg1);
#elif  (defined (CAMERA1) || defined (FROM_FILE1))
		double* find1 (IplImage* frame1);
#endif

#if (defined(WRITE_ALG_CAMERA2) && ((defined (CAMERA2) || defined (FROM_FILE2)))) 
		unsigned short int* find2 (IplImage* frame2, CvVideoWriter * writer_alg2);
#elif (defined (CAMERA2) || defined (FROM_FILE2))
		unsigned short int* find2 (IplImage* frame2);
#endif

void getFloor (IplImage* floor, char* fname_floor, char* fname_marker);