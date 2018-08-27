#include <iostream>

#include <opencv2/highgui.hpp>

#include "CodeDecoder.hpp"
#include "MovieWriter.hpp"

// ------------------  M a i n ---------------------
/*
	use [program.exe] -q to make it quiet.
*/

int main(int argc, char* argv[]) {
	// Activate OpenCL if possible
	cv::setUseOptimized(true);
	
	const cv::Rect ZONE = cv::Rect(245, 165, 150, 150);
	const bool VOLUBILE = argc == 2 ? strcmp(argv[1], "-q") != 0 : true;
	
	
	// ---- Camera and frame ----
	MovieWriter writer;
	cv::VideoCapture cap(0);
	
	if(!cap.isOpened()) {
		std::cout << "Could not open the camera" << std::endl;
		return -1;
	}
	
	cv::Mat frame = cv::Mat::zeros(
		static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT)), 
		static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH)), 
		CV_8UC3
	);
		
	if(frame.cols < ZONE.width + 2*ZONE.x || frame.rows < ZONE.height + 2*ZONE.y) {
		std::cout << "Bad dimensions (" << frame.size() << " != 640x480)" << std::endl;
		return -1;
	}
		
		
	// ------ Create GUI ------
	cv::namedWindow("Camera");
	
	// ----- Main part ----
	CodeDecoder decoder;
	
	// ------ Loop it ------	
	while(cv::waitKey(1) != 27) { // Escape key
		cap >> frame;
		if(frame.empty()) 
			continue;
		
		cv::Mat frameRaw = frame.clone();
		
		// - Decode
		std::string qrCodeData = decoder.decode(frame(ZONE));
		
		if(!qrCodeData.empty()) {
			if(qrCodeData == "START" && !writer.isRecording()) {
				writer.start("Recordings/" + Chronometre::date() + ".avi", frame.size(), 30);
				std::cout << "------- Start recording: " << writer.getName() << " --------" << std::endl;
			}
			if(qrCodeData == "STOP" && writer.isRecording()) {
				writer.stop();
				std::cout << "------- Stop recording. -------- " << std::endl;
			}
		}
		
		// - Screen display
		cv::rectangle(frame, ZONE, writer.isRecording() ? cv::Scalar(0,0,255) : cv::Scalar(0,255,0), 2);
		cv::imshow("Camera", frame);
		
		// - Recording
		if(writer.isRecording())
			writer.saveFrame(frameRaw);
		
		// - Console display
		if(VOLUBILE) {
			if(!qrCodeData.empty())
				std::cout << "QR code = " << qrCodeData << " | ";
			std::cout << decoder.decodeTime() << "ms." << std::endl;
		}
	}
	
	cap.release();
	return 0;
}
