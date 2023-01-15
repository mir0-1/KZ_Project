#pragma once
#include <Windows.h>
#include <opencv2/opencv.hpp>

using namespace cv;

class MotionCamera
{
	private:
		VideoCapture defaultCameraVideoSrc;

		HBITMAP hBitmapFrame;
		BITMAPINFOHEADER bmi = { sizeof(BITMAPINFOHEADER) };
		HDC hdcMem;
		HDC hdcWindow;
		HWND hTargetDisplayWindow;

		Mat prevFrame;
		Mat currentFrame;
		bool isFirstFrameCaptured = true;
		bool isFirstMotionDetection = true;

		std::vector<int> saveFileParams;

		double maxValueForBinary;
		double diffThreshold;
		int pixelCountThreshold;

	private:
		void initSaveFileParams();
		void initBitmap();
		void initHDC(HWND hwnd);

	public:
		void captureFrame();
		bool detectMotion();
		void setValueForPixelsAboveThreshold(double maxValueForBinary);
		void setDiffThreshold(double diffThreshold);
		void setPixelCountThreshold(int pixelCountThreshold);
		void saveCurrentFrameToFile(const TCHAR* fileName);
		void addDateAndTimeToCurrentFrame(int x, int y, int thickness, double fontScale);
		void addTextToCurrentFrame(int x, int y, const TCHAR* buffer, int thickness, double fontScale);
		void displayCurrentFrame(int x, int y);
		void doBeep();

	public:
		MotionCamera(HWND hdc);
		~MotionCamera();
};