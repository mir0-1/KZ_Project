#include "MotionCamera.h"

void MotionCamera::captureFrame()
{
	if (isFirstFrameCaptured)
		isFirstFrameCaptured = false;
	else
		currentFrame.copyTo(prevFrame);

	defaultCameraVideoSrc >> currentFrame;
}

void MotionCamera::initSaveFileParams()
{
	saveFileParams.push_back(IMWRITE_JPEG_QUALITY);
	saveFileParams.push_back(95);
}

void MotionCamera::initBitmap()
{
	bmi.biWidth = defaultCameraVideoSrc.get(CAP_PROP_FRAME_WIDTH);
	bmi.biHeight = -defaultCameraVideoSrc.get(CAP_PROP_FRAME_HEIGHT);
	bmi.biPlanes = 1;
	bmi.biBitCount = 24;
	bmi.biCompression = BI_RGB;
	bmi.biSizeImage = 0;

	hBitmapFrame = CreateDIBitmap(hdcWindow, &bmi, CBM_INIT, nullptr, (BITMAPINFO*)& bmi, DIB_RGB_COLORS);
}

void MotionCamera::initHDC(HWND hwnd)
{
	hdcWindow = GetDC(hwnd);
	hdcMem = CreateCompatibleDC(hdcWindow);
}

bool MotionCamera::detectMotion()
{
	if (isFirstMotionDetection)
	{
		isFirstMotionDetection = false;
		return false;
	}

	Mat currentGrayFrame, prevGrayFrame, diff;

	cvtColor(currentFrame, currentGrayFrame, COLOR_BGR2GRAY);
	cvtColor(prevFrame, prevGrayFrame, COLOR_BGR2GRAY);
	absdiff(currentGrayFrame, prevGrayFrame, diff);
	threshold(diff, diff, diffThreshold, maxValueForBinary, THRESH_BINARY);

	int whitePixelCount = countNonZero(diff);

	if (whitePixelCount > pixelCountThreshold)
		return true;

	return false;
}

void MotionCamera::setValueForPixelsAboveThreshold(double maxValueForBinary)
{
	this->maxValueForBinary = maxValueForBinary;
}

void MotionCamera::setDiffThreshold(double lowerThreshold)
{
	this->diffThreshold = lowerThreshold;
}

void MotionCamera::setPixelCountThreshold(int pixelCountThreshold)
{
	this->pixelCountThreshold = pixelCountThreshold;
}

void MotionCamera::saveCurrentFrameToFile(const TCHAR* fileName)
{
	imwrite(fileName, currentFrame, saveFileParams);
}

void MotionCamera::addDateAndTimeToCurrentFrame(int x, int y, int thickness, double fontScale)
{
	time_t rawtime;
	struct tm timeinfo;
	char timebuf[60];

	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);
	strftime(timebuf, 60, "%d.%m.%Y %H:%M:%S", &timeinfo);
	addTextToCurrentFrame(x, y, timebuf, thickness, fontScale);
}

void MotionCamera::addTextToCurrentFrame(int x, int y, const TCHAR* buffer, int thickness, double fontScale)
{
	if (y < 0)
		y = currentFrame.rows + y;

	if (x < 0)
		x = currentFrame.cols + x;

	Point org(x, y);
	const int fontFace = FONT_HERSHEY_COMPLEX;
	putText(currentFrame, buffer, org, fontFace, fontScale, Scalar::all(0), thickness+2, LINE_AA);
	putText(currentFrame, buffer, org, fontFace, fontScale, Scalar::all(255), thickness, LINE_AA);
}

void MotionCamera::displayCurrentFrame(int x, int y)
{
	SelectObject(hdcMem, hBitmapFrame);
	SetDIBits(hdcMem, hBitmapFrame, 0, currentFrame.rows, currentFrame.data, (BITMAPINFO*)& bmi, DIB_RGB_COLORS);
	BitBlt(hdcWindow, 0, 0, currentFrame.cols, currentFrame.rows, hdcMem, 0, 0, SRCCOPY);
}

void MotionCamera::doBeep()
{
	PlaySound(TEXT("beep.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

MotionCamera::MotionCamera(HWND hWnd)
{
	defaultCameraVideoSrc.open(0);
	initSaveFileParams();
	initHDC(hWnd);
	initBitmap();
}

MotionCamera::~MotionCamera()
{
	ReleaseDC(hTargetDisplayWindow, hdcWindow);
	DeleteObject(hBitmapFrame);
	DeleteDC(hdcMem);
}
