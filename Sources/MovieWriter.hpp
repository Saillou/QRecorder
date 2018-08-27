#ifndef MOVIE_WRITER_HPP
#define MOVIE_WRITER_HPP

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>

#include "Chronometre.hpp"

#include <iostream>

class MovieWriter {
	
public:
	MovieWriter() :
		_isRecording(false),
		_fileName(""),
		_ptrWriter(nullptr)
	{
	}
	
	~MovieWriter() {
		stop();
		
		if(_ptrWriter)
			delete _ptrWriter;
	}
	
	// Methods
	void start(const std::string& fileName, const cv::Size& sizeFrame, double fps, int encodeType = cv::VideoWriter::fourcc('M', 'P', '4', '2')) {
		if(!_ptrWriter)
			_ptrWriter = new cv::VideoWriter(fileName, encodeType, fps, sizeFrame);
		else
			_ptrWriter->open(fileName, encodeType, fps, sizeFrame);
		
		_fileName 		= fileName;
		_isRecording 	= true;
	}
	void stop() {
		_fileName 		= "";
		_isRecording 	= false;
		
		if(_ptrWriter)
			_ptrWriter->release();
	}
	
	void saveFrame(const cv::Mat& frame) {
		if(!_isRecording || !_ptrWriter || frame.empty()) 
			return;
		
		_ptrWriter->write(frame);
	}
	
	// Getters
	bool isRecording() const {
		return _isRecording;
	}
	const std::string& getName() const {
		return _fileName;
	}
	
	// Statics
	int fourcc(char c1, char c2, char c3, char c4) {
		return cv::VideoWriter::fourcc(c1, c2, c3, c4);
	}
	
private:
	// Disable copy
	MovieWriter& operator=(const MovieWriter &) = delete;
	MovieWriter(const MovieWriter&) = delete;

	// Members
	bool _isRecording;
	std::string _fileName;
	cv::VideoWriter* _ptrWriter;
};

#endif 