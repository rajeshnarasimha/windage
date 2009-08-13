#include <iostream>

#include "PGRCamera.h"
#include "Tracker/ChessboardTracker.h"
#include "Tracker/ModifiedSURFTracker.h"
#include "Reconstructor/Reconstructor.h"
#include "Utils/Logger.h"
#include "Utils/Utils.h"

#define STEREO_MODE

const int WIDTH = 640;
const int HEIGHT = 480;

void main()
{
	windage::Logger* log = new windage::Logger(&std::cout);
	windage::Logger* fpslog = new windage::Logger(&std::cout);

	// 640 x 480
//	windage::Tracker* tracker = new windage::ChessboardTracker();
//	tracker->Initialize(1071.406, 1079.432, 317.678, 196.800, -0.277075, 0.938586, -0.010295, -0.006803, 7, 8, 2.80);
	// 320 x 240
//	tracker->Initialize( 535.703,  539.716, 158.839, 098.400, -0.277075, 0.938586, -0.010295, -0.006803, 7, 8, 28.0);

	IplImage* referenceImage = cvLoadImage("reference.png", 0);
	windage::Tracker* tracker1 = new windage::ModifiedSURFTracker();
//	((windage::ModifiedSURFTracker*)tracker1)->Initialize(535.703, 539.716, 158.839, 98.400, -0.277075, 0.938586, -0.010295, -0.006803, referenceImage, 28.10, 21.10, 75);
	((windage::ModifiedSURFTracker*)tracker1)->Initialize(1071.406, 1079.432, 317.678, 196.800, -0.277075, 0.938586, -0.010295, -0.006803, referenceImage, 28.10, 21.10, 75);
	((windage::ModifiedSURFTracker*)tracker1)->InitializeOpticalFlow(WIDTH, HEIGHT, 5, cvSize(15, 15), 5);
	((windage::ModifiedSURFTracker*)tracker1)->SetOpticalFlowRunning(true);

	CPGRCamera* camera1 = new CPGRCamera();
	camera1->open();
	camera1->start();

	cvNamedWindow("test1");
	IplImage* input1 = cvCreateImage(cvSize(WIDTH, HEIGHT), IPL_DEPTH_8U, 4);
	IplImage* gray1 = cvCreateImage(cvGetSize(input1), IPL_DEPTH_8U, 1);

	char message[100];


#ifdef STEREO_MODE
	windage::Tracker* tracker2 = new windage::ModifiedSURFTracker();
//	((windage::ModifiedSURFTracker*)tracker2)->Initialize(535.703, 539.716, 158.839, 98.400, -0.277075, 0.938586, -0.010295, -0.006803, referenceImage, 28.10, 21.10, 75);
	((windage::ModifiedSURFTracker*)tracker2)->Initialize(1071.406, 1079.432, 317.678, 196.800, -0.277075, 0.938586, -0.010295, -0.006803, referenceImage, 28.10, 21.10, 75);
	((windage::ModifiedSURFTracker*)tracker2)->InitializeOpticalFlow(WIDTH, HEIGHT, 5, cvSize(15, 15), 5);
	((windage::ModifiedSURFTracker*)tracker2)->SetOpticalFlowRunning(true);

	CPGRCamera* camera2 = new CPGRCamera();
	camera2->open();
	camera2->start();

	cvNamedWindow("test2");
	IplImage* input2 = cvCreateImage(cvSize(WIDTH, HEIGHT), IPL_DEPTH_8U, 4);
	IplImage* gray2 = cvCreateImage(cvGetSize(input2), IPL_DEPTH_8U, 1);

#endif

	fpslog->updateTickCount();
	bool processing = true;
	while(processing)
	{
		double fps = fpslog->calculateFPS();
		fpslog->updateTickCount();
		
		log->updateTickCount();
		camera1->update();
		cvResize(camera1->GetIPLImage(), input1);
		cvCvtColor(input1, gray1, CV_BGRA2GRAY);
		log->log("capture", log->calculateProcessTime());

		log->updateTickCount();
		int result = tracker1->UpdateCameraPose(gray1);
		tracker1->DrawDebugInfo(input1);
		tracker1->DrawInfomation(input1, 10.0);
		log->log("tracking", log->calculateProcessTime());

		log->log("result", result);
		log->logNewLine();

		sprintf(message, "FPS : %lf", fps);
		windage::Utils::DrawTextToImage(input1, cvPoint(10, 20), message);
		
		cvShowImage("test1", input1);

#ifdef STEREO_MODE
		camera2->update();
		cvResize(camera2->GetIPLImage(), input2);
		cvCvtColor(input2, gray2, CV_BGRA2GRAY);

		result = tracker2->UpdateCameraPose(gray2);
		tracker2->DrawDebugInfo(input2);
		tracker2->DrawInfomation(input2, 10.0);

		cvShowImage("test2", input2);
#endif

		char ch = cvWaitKey(1);
		switch(ch)
		{
		case 'p':
		case 'P':
#ifdef STEREO_MODE
			windage::Reconstructor::Calc3DPointApproximation(tracker1->GetCameraParameter(), tracker2->GetCameraParameter(), cvPoint(0, 0), cvPoint(0, 0));
#endif
			break;
		case 'q':
		case 'Q':
			processing = false;
			break;
		}
	}

	camera1->stop();
	camera1->close();

#ifdef STEREO_MODE
	camera2->stop();
	camera2->close();
#endif

}