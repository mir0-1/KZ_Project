#include <windows.h>
#include <opencv2/opencv.hpp>
#include <cmath>

#include "MotionCamera.h"

#define WND_CLASS_NAME TEXT("MotionDetection")

using namespace cv;
using namespace std;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	{
		WNDCLASSEX wc;
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = WndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInstance;
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = WND_CLASS_NAME;
		wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

		if (!RegisterClassEx(&wc))
		{
			MessageBox(NULL, "Error registering window class", "Error", MB_OK);
			return 1;
		}
	}

	HWND hwnd = CreateWindow(WND_CLASS_NAME, "Motion Detection", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, hInstance, NULL);
	if (hwnd == NULL)
	{
		MessageBox(NULL, TEXT("Грешка при създаването на прозореца"), TEXT("Грешка"), MB_OK);
		return 1;
	}

	ShowWindow(hwnd, nShowCmd);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static MotionCamera motionCamera(hwnd);
	static const int cooldownMilliseconds = 3000;
	static int t[2];
	static bool motionDetected;
	static unsigned int startTime;


	switch (msg)
	{
		case WM_CREATE:
			motionCamera.setDiffThreshold(25);
			motionCamera.setValueForPixelsAboveThreshold(255);
			motionCamera.setPixelCountThreshold(10000);
			break;
		case WM_CLOSE:
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_PAINT:
			motionCamera.captureFrame();

			if (!motionDetected && motionCamera.detectMotion())
			{
				startTime = GetTickCount();
				motionDetected = true;
				motionCamera.doBeep();
				motionCamera.addDateAndTimeToCurrentFrame(10, -20, 1, 1.0);
				motionCamera.saveCurrentFrameToFile("result.jpg");
				ShellExecute(hwnd, "open", "result.jpg", NULL, NULL, SW_SHOW);
			}

			else if (motionDetected && GetTickCount() - startTime < cooldownMilliseconds)
				motionCamera.addTextToCurrentFrame(10, 30, "Alert", 1, 1.0);

			else if (GetTickCount() - startTime >= cooldownMilliseconds)
				motionDetected = false;

			motionCamera.displayCurrentFrame(0, 0);
			InvalidateRect(hwnd, NULL, false);
			break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;
}