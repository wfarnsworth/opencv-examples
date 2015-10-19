/*------------------------------------------------------------------------------------------*\
   This file contains material supporting chapter 10 of the cookbook:  
   Computer Vision Programming using the OpenCV Library. 
   by Robert Laganiere, Packt Publishing, 2011.

   This program is free software; permission is hereby granted to use, copy, modify, 
   and distribute this source code, or portions thereof, for any purpose, without fee, 
   subject to the restriction that the copyright notice may not be removed 
   or altered from any source or altered source distribution. 
   The software is released on an as-is basis and without any warranties of any kind. 
   In particular, the software is not guaranteed to be fault-tolerant or free from failure. 
   The author disclaims all warranties with regard to this software, any use, 
   and any consequent failure, is purely the responsibility of the user.
 
   Copyright (C) 2010-2011 Robert Laganiere, www.laganiere.name
\*------------------------------------------------------------------------------------------*/
//------------------------------------------------------------------------------------------*/
//
// Framework by Eric Gregori - BDTi
//
//------------------------------------------------------------------------------------------*/

#include <string>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/video/tracking.hpp>
#include <stdio.h>

using namespace std;
using namespace cv;

cv::Mat gray;						// current gray-level image
cv::Mat gray_prev;					// previous gray-level image
std::vector<cv::Point2f> points[2]; // tracked features from 0->1
std::vector<cv::Point2f> initial;   // initial position of tracked points
std::vector<cv::Point2f> features;  // detected features
std::vector<uchar> status;			// status of tracked features
std::vector<float> err;				// error in tracking



// determine which tracked point should be accepted
bool acceptTrackedPoint(int i) 
{
		return status[i] &&
			// if point has moved
			(abs(points[0][i].x-points[1][i].x)+
			(abs(points[0][i].y-points[1][i].y))>2);
}

// handle the currently tracked points
void handleTrackedPoints(cv:: Mat &frame, cv:: Mat &output) 
{
	// for all tracked points
	for(int i= 0; i < points[1].size(); i++ ) 
	{
		// draw line and circle
		cv::line(output, initial[i], points[1][i], cv::Scalar(255,255,255));
		cv::circle(output, points[1][i], 3, cv::Scalar(255,255,255),-1);
	}
}

// FeatureTracker() : max_count(500), qlevel(0.01), minDist(10.) {}
int my_Algorithm(cv:: Mat &frame, int max_count, double qlevel, double minDist ) 
{
	Mat output;
	double t = (double)cvGetTickCount();

	// convert to gray-level image
	cv::cvtColor(frame, gray, CV_BGR2GRAY); 
	frame.copyTo(output);

	// 1. if new feature points must be added
	if(points[0].size()<=10)				// Check if new points need to be added
	{
		// detect feature points
		cv::goodFeaturesToTrack(gray,		// the image 
								features,   // the output detected features
								max_count,  // the maximum number of features 
								qlevel,     // quality level
								minDist);   // min distance between two features

		// add the detected features to the currently tracked features
		points[0].insert(points[0].end(),features.begin(),features.end());
		initial.insert(initial.end(),features.begin(),features.end());
	}
		
	// for first image of the sequence
	if(gray_prev.empty())
          gray.copyTo(gray_prev);
            
	if( points[0].size() > 0 )   // EMG - 09/22/11 - fix optical flow crashing bug 
	{
		// 2. track features
		cv::calcOpticalFlowPyrLK(gray_prev, gray, // 2 consecutive images
				points[0], // input point position in first image
				points[1], // output point postion in the second image
				status,    // tracking success
				err);      // tracking error
	} 

	// 2. loop over the tracked points to reject the undesirables
	int k=0;
	for( int i= 0; i < points[1].size(); i++ ) 
	{
		// do we keep this point?
		if (acceptTrackedPoint(i)) 
		{
			// keep this point in vector
			initial[k]= initial[i];
			points[1][k++] = points[1][i];
		}
	}

	// eliminate unsuccesful points
    points[1].resize(k);
	initial.resize(k);

//	printf( "\npoints[0] = %d points[1] = %d   ", points[0].size(), points[1].size() );

    t = (double)cvGetTickCount() - t;
	// 3. handle the accepted tracked points
	handleTrackedPoints(frame, output);

	double t2 = (double)cvGetTickCount();
	// 4. current points and image become previous ones
	std::swap(points[1], points[0]);
	cv::swap(gray_prev, gray);
    t2 = (double)cvGetTickCount() - t2;

	printf( "\ndetection time = %g ms   ",(t+t2)/((double)cvGetTickFrequency()*1000.) );

	// show foreground
	cv::imshow("Optical Flow", output );

	return( (t+t2)/((double)cvGetTickFrequency()*1000.) );
}




int main( int argc, char *argv[] )
{
	cv::VideoCapture	cap;
	int					max_count;			// maximum number of features to detect
	int					qlevel;				// quality level for feature detection
	int					minDist;			// minimum distance between two feature points
	int					framenum;
	int					frametime, algtime, framedelay;
	bool				Camera;

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
//	cv::namedWindow("Raw Video");
	max_count = 500;			// maximum number of features to detect
	qlevel = 20;				// .01 - quality level for feature detection
	minDist = 10;				// 10.0 minimum distance between two feature points

	cv::createTrackbar( "Max Count", "Parameters", &max_count, 1000, 0 );
	cv::createTrackbar( "qlevel",    "Parameters", &qlevel, 40, 0 );
	cv::createTrackbar( "minDist",   "Parameters", &minDist, 200, 0 );
	cv::createTrackbar( "delay",     "Parameters", &frametime, 100, 0 );

	bool stop(false);
	framenum = 0;
	algtime = 0;
	// for all frames in video
	while (!stop) 
	{
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

		if( qlevel == 0 ) 
			qlevel = 1;
		if( framenum++ > 5 )
			algtime = my_Algorithm( frame, max_count?max_count:1, (double)qlevel/2000.0, (double)minDist );
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



