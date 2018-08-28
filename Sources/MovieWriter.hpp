#ifndef MOVIE_WRITER_HPP
#define MOVIE_WRITER_HPP

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>

#include "Chronometre.hpp"

#include <iostream>
#include <atomic>
#include <mutex>
#include <thread>

class MovieWriter {
	
public:
	MovieWriter() :
		_isRecording(false),
		_isAlive(true),
		_fpsRate(0.0),
		_ptrWriter(nullptr),
		_frameBuffed(cv::Mat::zeros(1,1,CV_8UC3)),
		_fileName(""),
		_pThread(nullptr)
	{
		_pThread = new std::thread(&MovieWriter::_saveFrame, this);
	}
	
	~MovieWriter() {
		_isAlive = false;
		stop();
		
		if(_pThread) {
			if(_pThread->joinable())
				_pThread->join();
			delete _pThread;
		}	
		
		if(_ptrWriter)
			delete _ptrWriter;
	}
	
	// Methods
	void start(const std::string& fileName, const cv::Size& sizeFrame, double fps, int encodeType = cv::VideoWriter::fourcc('M', 'P', '4', '2')) {
		_mutCapture.lock();
			if(!_ptrWriter)
				_ptrWriter = new cv::VideoWriter(fileName, encodeType, fps, sizeFrame);
			else
				_ptrWriter->open(fileName, encodeType, fps, sizeFrame);
		_mutCapture.unlock();

		_mutFrame.lock();
			_frameBuffed = cv::Mat::zeros(sizeFrame, CV_8UC3);
		_mutFrame.unlock();
		
		_fileName 		= fileName;
		_fpsRate		= fps;
		_isRecording 	= true;
	}
	void stop() {
		_isRecording 	= false;
		_fileName 		= "";
		_fpsRate		= 0.0;
		
		_mutCapture.lock();
			if(_ptrWriter)
				_ptrWriter->release();
		_mutCapture.unlock();
	}
	
	void saveFrame(const cv::Mat& frame) {
		if(!_isRecording || !_ptrWriter || frame.empty()) 
			return;
			
		// _ptrWriter->write(frame);
		_mutFrame.lock();
			_frameBuffed = frame.clone();
		_mutFrame.unlock();
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
	std::atomic<bool> 	_isRecording;
	std::atomic<bool> 	_isAlive;
	std::atomic<double> _fpsRate;
	
	cv::VideoWriter* 	_ptrWriter;
	cv::Mat 			_frameBuffed;
	
	std::string 	_fileName;
	std::thread*	_pThread;
	std::mutex 		_mutFrame;
	std::mutex 		_mutCapture;
	
	// Methods
	void _saveFrame() {
		Chronometre chronoWrite;
		Chronometre chronoFps;
		int64_t lastWriteTime = 0;
		
		while(_isAlive) {		
			if(!_isRecording || _fpsRate <= 0) {
				Chronometre::wait(30);
				continue;
			}
			
			int64_t timeToWait = (int64_t)(1000/_fpsRate) - chronoFps.elapsed_ms() - lastWriteTime;
			if(timeToWait > 1) 
				Chronometre::wait(timeToWait);
			
			_mutFrame.lock();
				cv::Mat frame = _frameBuffed.clone();
			_mutFrame.unlock();
			
			
			chronoWrite.beg();
			_mutCapture.lock();
				_ptrWriter->write(frame);
			_mutCapture.unlock();
			chronoWrite.end();
			
			chronoFps.beg();
			lastWriteTime = chronoWrite.ms();
		}
	}
};

#endif 
