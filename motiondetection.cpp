/*------------------------------------------------------------------------------------------*\
   This program is free software; permission is hereby granted to use, copy, modify, 
   and distribute this source code, or portions thereof, for any purpose, without fee, 
   subject to the restriction that the copyright notice may not be removed 
   or altered from any source or altered source distribution. 
   The software is released on an as-is basis and without any warranties of any kind. 
   In particular, the software is not guaranteed to be fault-tolerant or free from failure. 
   The author disclaims all warranties with regard to this software, any use, 
   and any consequent failure, is purely the responsibility of the user.
 
\*------------------------------------------------------------------------------------------*/
//------------------------------------------------------------------------------------------*/
//
// Framework by Eric Gregori - BDTi
//
//------------------------------------------------------------------------------------------*/

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>
#include <stdio.h>

using namespace std;
using namespace cv;

// Algortihm variables
cv::Mat gray;			// current gray-level image
cv::Mat background;		// accumulated background
cv::Mat backImage;		// background image
cv::Mat foreground;		// foreground image



int my_Algorithm(cv:: Mat &frame, double learningRate, int threshold ) 
{
	Mat output;
	double t = (double)cvGetTickCount();

	// convert to gray-level image
	cv::cvtColor(frame, gray, CV_BGR2GRAY); 

	// initialize background to 1st frame
	if (background.empty())
		gray.convertTo(background, CV_32F);
		
	// convert background to 8U
	background.convertTo(backImage,CV_8U);

	// compute difference between current image and background
	cv::absdiff(backImage,gray,foreground);

	// apply threshold to foreground image
	cv::threshold(foreground,output,threshold,255,cv::THRESH_BINARY);

	// accumulate background
	cv::accumulateWeighted(gray, background, learningRate, output);
    	t = (double)cvGetTickCount() - t;
	printf( "LearningRate = %g   ", learningRate ); 
    	printf( "detection time = %g ms\n", t/((double)cvGetTickFrequency()*1000.) );

	// show foreground
	cv::imshow("Extracted Foreground", output );

	return( t/((double)cvGetTickFrequency()*1000.) );
}




int main( int argc, char *argv[] )
{
	cv::VideoCapture cap;
	int				 LearningRate;
	int				 Threshold;
	int				 framenum;
	int				 frametime, algtime, framedelay;
	bool			 Camera;

	// Open the video file
	Camera = true;
	frametime = 33;
    if( argc == 1 || (argc == 2 && strlen(argv[1]) == 1 && isdigit(argv[1][0])))
        cap.open(argc == 2 ? argv[1][0] - '0' : 0);
    else if( argc == 2 )
	{
        cap.open(argv[1]);
		Camera = false;
		frametime = 1000/cap.get(CV_CAP_PROP_FPS);
	}

	// check if video successfully opened
	if (!cap.isOpened())
	{
		if( Camera == false )
		{
			printf( "\n\n!Error! - Failed to open file - %s", argv[1] ); 		
			cv::waitKey();
		}
		else
		{
			printf( "\n\n!Error! - Faild to connect to web camera" ); 		
			cv::waitKey();
		}

		return 0;
	}

	// current video frame
	cv::Mat frame; 
	cv::Mat result;

	cv::namedWindow("Parameters");
	LearningRate = 0;
	Threshold = 10;
	cv::createTrackbar( "LearnRate", "Parameters", &LearningRate, 100, 0 );
	cv::createTrackbar( "Threshold", "Parameters", &Threshold, 255, 0 );
	cv::createTrackbar( "delay",     "Parameters", &frametime, 100, 0 );

	bool stop(false);
	framenum = 0;
	algtime = 0;
	// for all frames in video
	while (!stop) {

		// read next frame if any
		if (!cap.read(frame))
		{
			if( !Camera )
			{
				cap.release();
				cap.open(argv[1]);
				cap.read(frame);
			}
			else
				break;
		}

		cv::imshow("Raw Video",frame);

//-------------------------------------------------------------------------
// Insert Algorithm here
//-------------------------------------------------------------------------

		if( framenum++ > 5 )
			algtime = my_Algorithm( frame, (10001-(LearningRate*100))/10000.0, Threshold );
		else
			continue;

//-------------------------------------------------------------------------
// Algorithm Done
//-------------------------------------------------------------------------

		// introduce a delay
		// or press key to stop
		printf( " frametime = %d", frametime, algtime );
		if( frametime > algtime )
			framedelay = frametime - algtime;
		else
			framedelay = 10;
		printf( " framedelay = %d", framedelay );
		if (cv::waitKey(framedelay)>=0)
				stop= true;
	}
}
