#pragma once

#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/ml/ml.hpp"
#include "opencv2/video/video.hpp"
//#include "opencv2/features/features.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/legacy/legacy.hpp"
#include "opencv2/flann/flann.hpp"
#include <list>
#include <math.h>
#include <iostream>

double* find1 (IplImage* frame1);
unsigned short int* find2 (IplImage* frame2);

void getFloor (IplImage* floor, char* fname_floor, char* fname_marker);