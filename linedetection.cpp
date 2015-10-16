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
// Hough Transforms
//------------------------------------------------------------------------------------------*/
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

cv::Mat gray, edges;

//	threshold – The accumulator threshold parameter. Only those lines are returned that get enough votes (  )
//  minLineLength – The minimum line length. Line segments shorter than that will be rejected
//  maxLineGap – The maximum allowed gap between points on the same line to link them.
int my_Algorithm(cv:: Mat &frame, int Threshold, int MinLineLength, int MaxLineGap ) 
{
	Mat output;

    	double t = (double)cvGetTickCount();

	// convert to gray-level image
	cv::cvtColor(frame, gray, CV_BGR2GRAY); 
	output = frame.clone();

	// Edge detection using Canny
	cv::Canny( gray, edges, 50, 200, 3 );

	std::vector<cv::Vec4i> lines;
	cv::HoughLinesP( edges, lines, 1, CV_PI/180, Threshold, MinLineLength, MaxLineGap );

    	t = (double)cvGetTickCount() - t;
	t = t/((double)cvGetTickFrequency()*1000.);
    	printf( "detection time = %g ms\n", t );
	
	for( size_t i = 0; i < lines.size(); i++ )
    	{
		cv::Vec4i l = lines[i];
		cv::line( output, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0,0,255), 3, CV_AA);
    	}

	imshow("Edges",edges);
	imshow("Hough Lines", output );

	return( t );
}




int main( int argc, char *argv[] )
{
	cv::VideoCapture cap;
	int				 MinLineLength, MaxLineGap;
	int				 Threshold;
	bool			 	Camera;
	int				 framenum;
	int				 frametime, algtime, framedelay;

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

	cv::namedWindow("Parameters");
	Threshold = 50;
	MinLineLength = 50;
	MaxLineGap = 10;
	// Threshold, MinLineLength, MaxLineGap
	cv::createTrackbar( "Threshold", "Parameters", &Threshold, 255, 0 );
	cv::createTrackbar( "MinLengh", "Parameters", &MinLineLength, 255, 0 );
	cv::createTrackbar( "MaxGap", "Parameters", &MaxLineGap, 255, 0 );
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


//-------------------------------------------------------------------------
// Insert Algorithm here
//-------------------------------------------------------------------------

		cv::Mat &frame_ref = frame;
		if( framenum++ > 5 )
			algtime = my_Algorithm( frame_ref, Threshold?Threshold:1,MinLineLength, MaxLineGap );
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
