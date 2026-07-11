#include <sstream>

#include "Stopwatch.h"

#include "DX12Window.h"

//	static GRAPHICS_INIT_DESC ParseCommandLineArguments()
//	{
//		GRAPHICS_INIT_DESC desc = {};
//	
//		int argc;
//		wchar_t** argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);
//	
//		for (size_t i = 0; i < argc; ++i)
//		{
//			if (::wcscmp(argv[i], L"-w") == 0 || ::wcscmp(argv[i], L"--width") == 0)
//			{
//				desc.window_width = ::wcstol(argv[++i], nullptr, 10);
//			}
//			if (::wcscmp(argv[i], L"-h") == 0 || ::wcscmp(argv[i], L"--height") == 0)
//			{
//				desc.window_height = ::wcstol(argv[++i], nullptr, 10);
//			}
//			if (::wcscmp(argv[i], L"-warp") == 0 || ::wcscmp(argv[i], L"--warp") == 0)
//			{
//				desc.WARP = true;
//			}
//	
//			// more shit to do later, like fullscreen
//		}
//	
//		::LocalFree(argv);
//	
//		return desc;
//	}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	// Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
	// Using this awareness context allows the client area of the window 
	// to achieve 100% scaling while still allowing non-client window content to 
	// be rendered in a DPI sensitive fashion.
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	// turn on debug layer before initalizing Direct3D 12 device. Doing it after will cause the device to be released.
	enable_GPU_debug_layer();

	try {
		DX12Window gfx(hInstance, L"pepe", 1280, 720, WS_OVERLAPPEDWINDOW);

		while (gfx.is_open())
		{
			gfx.dispatch_event_reading();
			gfx.setup_frame();


			gfx.present_frame();
		}
	}
	catch (const std::exception& e)
	{
		MessageBoxA(nullptr, e.what(), "Fatal Error", MB_OK | MB_ICONERROR);
	}

	return 0;
}
// bvllshit
void update()
{
	static uint64_t frameCounter = 0;
	static double elapsedSeconds = 0.0;
	static std::chrono::high_resolution_clock cock;
	static auto t0 = cock.now();

	frameCounter++;
	auto t1 = cock.now();
	auto deltaTime = t1 - t0;
	t0 = t1;

	elapsedSeconds += deltaTime.count() * 1e-9;
	if (elapsedSeconds > 1.0)
	{
		char buffer[500];
		auto fps = frameCounter / elapsedSeconds;
		sprintf_s(buffer, 500, "FPS: %f\n", fps);
		OutputDebugStringA(buffer);
		frameCounter = 0;
		elapsedSeconds = 0.0;
	}
}
