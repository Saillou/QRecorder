#ifndef CODE_DECODER_HPP
#define CODE_DECODER_HPP

#include <fstream>
#include <thread>
#include <atomic>
#include <mutex>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "Chronometre.hpp"

class CodeDecoder {
	
public:
	CodeDecoder() :
		_img(cv::Mat::zeros(200, 200, CV_8UC3)),
		_lastDecode(""),
		_isAlive(true),
		_decodeTime(0),
		_pThread(nullptr)
	{
		_pThread = new std::thread(&CodeDecoder::_decode, this);
	}
	
	~CodeDecoder() {
		// Stop and delete thread
		_isAlive = false;
		
		if(!_pThread) {
			if(_pThread->joinable())
				_pThread->join();
			delete _pThread;
		}			
	}
	
	// Methods
	std::string decode(const cv::Mat& img) {
		_mutFrame.lock();
			_img = img.clone();
		_mutFrame.unlock();
		
		_mutRes.lock();
			std::string res = _lastDecode;
		_mutRes.unlock();
		
		return res;
	}
	
	
	int64_t decodeTime() const {
		return _decodeTime;
	}
	
private:
	// Disable copy
	CodeDecoder& operator=(const CodeDecoder &) = delete;
	CodeDecoder(const CodeDecoder&) = delete;
	
	// Members
	cv::Mat					_img;
	std::string 			_lastDecode;
	std::atomic<bool> 		_isAlive;
	std::atomic<int64_t>	_decodeTime;
	std::thread*			_pThread;
	std::mutex				_mutFrame;
	std::mutex				_mutRes;
	
	// Methods
	void _decode() {
		Chronometre chrono;
		do {	
			chrono.beg();
			std::string qrCodeData = "";
			
			// Pic
			cv::Mat imgGray;
			_mutFrame.lock();
				cv::cvtColor(_img, imgGray, cv::COLOR_BGR2GRAY);
			_mutFrame.unlock();
			
#ifdef _WIN32			
			_decodeWindows(imgGray, qrCodeData);
#else
			_decodeUbuntu(imgGray, qrCodeData);
#endif
			
			// Push result
			_mutRes.lock();
				_lastDecode = qrCodeData;
			_mutRes.unlock();
			
			chrono.end();
			_decodeTime = chrono.ms();
		} while(_isAlive);
	}
	
	void _decodeWindows(const cv::Mat& imgGray, std::string& qrCodeData) {
		// Call zbar
		cv::imwrite("Tmp/temp.bmp", imgGray);
		system("..\\..\\Windows_ZBar\\zbarimg.exe -q Tmp/temp.bmp > Tmp/resTmp.txt");
		
		// Read console destination txt
		std::string res;
		std::ifstream resFile("Tmp/resTmp.txt");
		resFile >> res;
		resFile.close();
		
		// Formate result
		size_t pos = res.find(':');
		if(pos != std::string::npos)
			qrCodeData = res.substr(pos+1);
	}
	
	void _decodeUbuntu(const cv::Mat& /*imgGray*/, std::string& qrCodeData) {
		qrCodeData = "";
	}

};

#endif 