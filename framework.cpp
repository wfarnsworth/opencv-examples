#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <stdio.h>

int main( int argc, char** argv )
{
	cvNamedWindow( "Press Any Key to Quit", CV_WINDOW_AUTOSIZE );
	CvCapture *capture = cvCreateCameraCapture(0);
	if( capture == NULL )
	{
		printf( "\n!ERROR! - Could not open camera at /dev/video0\n\n" );
		printf( "\nIf you are using VMWare, verify that the camera is connected" );
		printf( "\nto the VMWare image by selecting 'Virtual Machine' in the" );
		printf( "\nVMWare player window header above. " );
		printf( "\nThen select 'removable devices' and verify your camera is connected.\n\n\n" );
		assert(0);
	}

	IplImage *frame;

	while(1)
	{
		frame = cvQueryFrame( capture );
		if( !frame ) break;
		cvShowImage( "Press Any Key to Quit", frame );
		char c = cvWaitKey(33);
		if( c != -1 ) break;
	}

	cvReleaseCapture( &capture );
	cvDestroyWindow( "Press Any Key to Quit" );
}

