#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include "DXGI1_2.h"
#include <d3d11.h>
#include <iostream>       // std::cout
#include <thread>         // std::thread
#include <mutex>          // std::mutex
#include <algorithm>
#include <string>
#include <assert.h>
#include <string.h>
#include <strsafe.h>
#include <tchar.h>

#include "../ScreenMirrorDll/ComPtr.h"
#include "../ScreenMirrorDll/ScreenMirror.h"

// #pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")


// Driver types supported
D3D_DRIVER_TYPE gDriverTypes[] =
{
	D3D_DRIVER_TYPE_HARDWARE
};
UINT gNumDriverTypes = ARRAYSIZE(gDriverTypes);

// Feature levels supported
D3D_FEATURE_LEVEL gFeatureLevels[] =
{
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_10_1,
	D3D_FEATURE_LEVEL_10_0,
	D3D_FEATURE_LEVEL_9_1
};

UINT gNumFeatureLevels = ARRAYSIZE(gFeatureLevels);

void saveBMP(void* buffer, UINT bufferSize, UINT threadId, UINT index)
{
	BITMAPINFO	lBmpInfo;

	// BMP 32 bpp

	ZeroMemory(&lBmpInfo, sizeof(BITMAPINFO));

	lBmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

	lBmpInfo.bmiHeader.biBitCount = 32;

	lBmpInfo.bmiHeader.biCompression = BI_RGB;

	lBmpInfo.bmiHeader.biWidth = 1920;

	lBmpInfo.bmiHeader.biHeight = 1080;

	lBmpInfo.bmiHeader.biPlanes = 1;

	lBmpInfo.bmiHeader.biSizeImage = 1920 * 1080 * 4;


	std::unique_ptr<BYTE> pBuf(new BYTE[lBmpInfo.bmiHeader.biSizeImage]);
	memcpy(pBuf.get(), buffer, bufferSize);


	WCHAR lMyDocPath[MAX_PATH];

	HRESULT hr = SHGetFolderPath(nullptr, CSIDL_PERSONAL, nullptr, SHGFP_TYPE_CURRENT, lMyDocPath);

	if (FAILED(hr))
		return;

	std::wstring lFilePath = std::wstring(lMyDocPath) + L"\\ScreenShot-" 
		+ std::to_wstring(threadId) + L"-" + std::to_wstring(index) + L".bmp";

	FILE* lfile = nullptr;

	auto lerr = _wfopen_s(&lfile, lFilePath.c_str(), L"wb");

	if (lerr != 0)
		return;

	if (lfile != nullptr)
	{

		BITMAPFILEHEADER	bmpFileHeader;

		bmpFileHeader.bfReserved1 = 0;
		bmpFileHeader.bfReserved2 = 0;
		bmpFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + lBmpInfo.bmiHeader.biSizeImage;
		bmpFileHeader.bfType = 'MB';
		bmpFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		fwrite(&bmpFileHeader, sizeof(BITMAPFILEHEADER), 1, lfile);
		fwrite(&lBmpInfo.bmiHeader, sizeof(BITMAPINFOHEADER), 1, lfile);
		fwrite(pBuf.get(), lBmpInfo.bmiHeader.biSizeImage, 1, lfile);

		fclose(lfile);

		// ShellExecute(0, 0, lFilePath.c_str(), 0, 0, SW_SHOW);
	}

	pBuf.release();
}

void foo(ScreenMirrorWrapper* wrapper, int threadId)
{
	void* buffer = malloc(1920 * 1080 * 4);
	UINT bufferSize = 1920 * 1080 * 4;

	wrapper->StartCapture();

	for (int i = 0; i < 103; i++) {
		::Sleep(40);
		wrapper->GetScreenFrame(buffer, bufferSize);
		if (i % 100 == 0) {
			printf("Grabbed %d frame \n", i);
			saveBMP(buffer, bufferSize, threadId, i);
		}
	}

	wrapper->CloseCapture();
}

int main()
{
	HRESULT hr;

	ScreenMirrorWrapper* wrapper = new ScreenMirrorWrapper();
	wrapper->Initialize();
	wrapper->SelectWindow();

	//std::thread thread1(foo, wrapper, 1);
	//std::thread thread2(foo, wrapper, 2);
	//std::thread thread3(foo, wrapper, 3);
	//std::thread thread4(foo, wrapper, 4);

	////::Sleep(500);

	//thread1.join();
	//thread2.join();
	//thread3.join();
	//thread4.join();

	printf("Done\n");

	delete wrapper;
	return 0;
}