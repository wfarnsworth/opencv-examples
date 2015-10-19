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
#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>
#include "opencv2/objdetect/objdetect.hpp"

using namespace std;
using namespace cv;


int my_Algorithm( Mat& img, Mat& result,CascadeClassifier& cascade, int scaleFactor, int minNeighbors, int minSize )
{
    int i = 0;
    double t = 0;
    vector<Rect> faces;
    Mat gray;

    cvtColor( img, gray, CV_BGR2GRAY );
    equalizeHist( gray, gray );

    t = (double)cvGetTickCount();

//	Parameters:	
//	image – Matrix of type CV_8U containing the image in which to detect objects.
//	objects – Vector of rectangles such that each rectangle contains the detected object.
//	scaleFactor – Specifies how much the image size is reduced at each image scale.
//	minNeighbors – Speficifes how many neighbors should each candiate rectangle have to retain it.
//	flags – This parameter is not used for new cascade and have the same meaning for old cascade as in function cvHaarDetectObjects.
//	minSize – The minimum possible object size. Objects smaller than that are ignored.
    cascade.detectMultiScale( gray, faces,
		(scaleFactor?(1.0+(double)(scaleFactor)/10.0):1.1),
		minNeighbors,
		0
		|CV_HAAR_DO_CANNY_PRUNING
        |CV_HAAR_FIND_BIGGEST_OBJECT
        //|CV_HAAR_DO_ROUGH_SEARCH
        //|CV_HAAR_SCALE_IMAGE
        ,
        Size(minSize, minSize) ); // minSize
    t = (double)cvGetTickCount() - t;
    printf( "\ndetection time = %g ms", t/((double)cvGetTickFrequency()*1000.) );
    for( vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++, i++ )
    {
        Point center;
		int radius;
        center.x = cvRound((r->x + r->width*0.5));
        center.y = cvRound((r->y + r->height*0.5));
        radius = cvRound((r->width + r->height)*0.25);
		circle( img, center, radius, CV_RGB(255,0,0), 3, 8, 0 );
    }
	return( t/((double)cvGetTickFrequency()*1000.) );
}


int main( int argc, char *argv[] )
{
	cv::VideoCapture cap;
	int				 ScaleFactor;
	int				 MinNeighbor;
	int				 MinSize;
	int				 framenum;
	int				 frametime, algtime, framedelay;
	bool			 Camera;
    CascadeClassifier cascade;

	// Open the video file
	Camera = true;
	frametime = 33;
    if( argc == 1 || (argc == 2 && strlen(argv[1]) == 1 && isdigit(argv[1][0])))
        cap.open(argc == 2 ? argv[1][0] - '0' : 0);
    else if( argc == 2 )
	{
		printf( "\nOpening file - %s", argv[1] );
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
			return 0;
		}
		else
		{
			printf( "\n\n!Error! - Faild to connect to web camera" ); 		
			return 0;
		}

		return 0;
	}
	else
		printf( " - success" );

	if( cap.set( CV_CAP_PROP_FRAME_WIDTH, 1080.0 ) == 0 )
		printf( "\nFailed to set WIDTH" );

	double width = cap.get( CV_CAP_PROP_FRAME_WIDTH );
	double height = cap.get( CV_CAP_PROP_FRAME_HEIGHT );
	printf( "\nFrame Size = %f, %f", width, height ); 

    if( !cascade.load( "haarcascade_frontalface_alt2.xml" ) )
    {
        cerr << "ERROR: Could not load classifier cascade" << endl;
		return 0;
	}

	// current video frame
	cv::Mat frame; 
	cv::Mat result;

	cv::namedWindow("Parameters");
	ScaleFactor = 10;
	MinNeighbor = 2;
	MinSize = 30;

	cv::createTrackbar( "ScaleF", "Parameters", &ScaleFactor, 10, 0 );
	cv::createTrackbar( "MinN", "Parameters", &MinNeighbor, 10, 0 );
	cv::createTrackbar( "MinSize", "Parameters", &MinSize, 200, 0 );
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

		if( framenum++ > 5 )
			algtime = my_Algorithm( frame, result, cascade, ScaleFactor, MinNeighbor, MinSize );
		else
			continue;

//-------------------------------------------------------------------------
// Algorithm Done
//-------------------------------------------------------------------------

		// show foreground
		cv::imshow("Face Detection",frame);

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
