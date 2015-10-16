//Copyright (c) 2012, Berkeley Design Technology, Inc.   All rights reserved.
// Redistribution and use in source and binary forms, with or without modification, are permitted 
// provided that the following conditions are met:
// •	Redistributions of source code must retain the above copyright notice, 
//		this list of conditions and the following disclaimer.
// •	Redistributions in binary form must reproduce the above copyright notice,
//		this list of conditions and the following disclaimer in the documentation 
//		and/or other materials provided with the distribution.
// •	Neither the name of Berkeley Design Technology, Inc. (also known as “BDTI”) 
//		nor the names of its contributors may be used to endorse or promote products 
//		derived from this software without specific prior written permission.
// 	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
//	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
//	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
//	PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS 
//	BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
//	OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
//	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
//	OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
//	WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
//	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
//	OF SUCH DAMAGE.
// 	For further information, contact BDTI at info@BDTI.com.

#include <stdio.h>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>

using namespace cv;

int				 LowThres=50, HighThres=150, FilterSize=1, KernelSize=1;


int my_Algorithm(Mat &frame) 
{
    double t = (double)cvGetTickCount();

	Mat gray, blurred;

	// convert to gray-level image
	cvtColor(frame, gray, CV_BGR2GRAY); 

    // smooth it, otherwise a lot of false edges may be detected
	if( FilterSize == 0 )
	{
		printf( "Gaussian - Off" );
		blurred = gray.clone();
	}
	else
	{
		int Fsize = FilterSize*2+1;
		printf( "Gaussian - %dx%d ", Fsize, Fsize );
		GaussianBlur( gray, blurred, Size(Fsize, Fsize), 0 );
	}

	/// Generate grad_x and grad_y
    Mat grad_x, grad_y, magnitude, angle;

	int Ksize = KernelSize*2+1;
	if( Ksize == 1 ) Ksize = 3; 
	printf( "Kernel - %dx%d ", Ksize, Ksize );
 
    // Gradient X
	//void Sobel(InputArray src, OutputArray dst, int ddepth, int xorder, int yorder, int ksize=3, double scale=1, double delta=0, int borderType=BORDER_DEFAULT )
    Sobel( blurred, grad_x, CV_32F, 1, 0, Ksize, 1, 0, BORDER_DEFAULT );   

    /// Gradient Y  
	//void Sobel(InputArray src, OutputArray dst, int ddepth, int xorder, int yorder, int ksize=3, double scale=1, double delta=0, int borderType=BORDER_DEFAULT )
    Sobel( blurred, grad_y, CV_32F, 0, 1, Ksize, 1, 0, BORDER_DEFAULT );   

	// get magnitude from X/Y Gradients
	// void cartToPolar(InputArray x, InputArray y, OutputArray magnitude, OutputArray angle, bool angleInDegrees=false)
	cartToPolar( grad_x, grad_y, magnitude, angle, true ); 
	threshold( magnitude, magnitude, LowThres, 0, THRESH_TOZERO );
	threshold( grad_x, grad_x, LowThres, 0, THRESH_TOZERO );
	threshold( grad_y, grad_y, LowThres, 0, THRESH_TOZERO );

	// Build a mask for the angle data
	Mat mask, MaskedAngle;
	Mat bmask = Mat::zeros(magnitude.rows, magnitude.cols, CV_8U );
	threshold( magnitude, mask, LowThres, 1, THRESH_BINARY );
	mask.convertTo( bmask, CV_8U );
	angle.copyTo( MaskedAngle, bmask );

	Mat CannyEdges;
	Canny( blurred, CannyEdges, LowThres, HighThres, Ksize );

	t = (double)cvGetTickCount() - t;

	Mat DisplayAngle(MaskedAngle.rows, MaskedAngle.cols, CV_8UC3 );
	MatConstIterator_<float> in = MaskedAngle.begin<float>(), in_end = MaskedAngle.end<float>();
	for(unsigned char *out = DisplayAngle.ptr(); in != in_end; ++in, out+=3)
	{
		if( (*in > 45) && (*in <= 135) )
		{
			*(out+0) = 255;
			*(out+1) = 255;
			*(out+2) = 255;
		} else
		if( (*in > 135) && (*in <= 225) )
		{
			*(out+0) = 255;
			*(out+1) = 0;
			*(out+2) = 0;
		} else
		if( (*in > 225) && (*in <= 315) )
		{
			*(out+0) = 0;
			*(out+1) = 255;
			*(out+2) = 0;
		} else
		if( ((*in > 315) && (*in <= 360)) || ((*in > 0) && (*in <= 45)) )
		{
			*(out+0) = 0;
			*(out+1) = 0;
			*(out+2) = 255;
		} else
		{
			*(out+0) = 0;
			*(out+1) = 0;
			*(out+2) = 0;
		}
	}


	// show foreground
	imshow( "Gaussian Filter",      blurred );
	imshow( "GradientX",			grad_x);
	imshow( "GradientY",			grad_y );
	imshow( "Magnitude",			magnitude );
	imshow( "Angle",				DisplayAngle );
	imshow( "Canny",				CannyEdges );

	return( t/((double)cvGetTickFrequency()*1000.) );
}




int main( int argc, char *argv[] )
{
	VideoCapture	 cap;
	bool			 Camera;
	int				 framenum;
	int				 frametime, algtime, framedelay;
	Mat frame;

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
			waitKey();
		}
		else
		{
			printf( "\n\n!Error! - Faild to connect to web camera" ); 		
			waitKey();
		}

		return 0;
	}

	// current video frame

	namedWindow("Parameters");	
	createTrackbar( "Low Thres",	"Parameters", &LowThres, 255, 0 );
	createTrackbar( "High Thres",	"Parameters", &HighThres, 255, 0 );
	createTrackbar( "Gaus Size",	"Parameters", &FilterSize, 12, 0 );
	createTrackbar( "Sobel Size",	"Parameters", &KernelSize, 3, 0 );

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
			algtime = my_Algorithm( frame );
		else
			continue;


//-------------------------------------------------------------------------
// Algorithm Done
//-------------------------------------------------------------------------

		// introduce a delay
		// or press key to stop
		printf( "Algorithm Time  = %dms\n", algtime );
		if( frametime > algtime )
			framedelay = frametime - algtime;
		else
			framedelay = 10;
		if (cv::waitKey(framedelay)>=0)
				stop= true;
	}
}

