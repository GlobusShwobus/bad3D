#include <sstream>

#include "Stopwatch.h"

#include "Application.h"
#include "Demo.h"
#include "Demo_Cube.h"

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

void find_centered_pos(UINT client_width, UINT client_height, UINT& xOut, UINT& yOut)
{
	const DWORD window_style = WS_OVERLAPPEDWINDOW;

	const RECT client_rect{ 0,0,client_width,client_height };
	// adjust client rect to window rect
	RECT window_rect{ 0,0,client_rect.right,client_rect.bottom };
	::AdjustWindowRect(&window_rect, window_style, FALSE);

	// obtain the cursor position then find the monitor where it is in
	POINT cursor_pos;
	::GetCursorPos(&cursor_pos);
	HMONITOR hMonitor = ::MonitorFromPoint(cursor_pos, MONITOR_DEFAULTTONEAREST);
	MONITORINFOEX monitorinfo = {};
	monitorinfo.cbSize = sizeof(MONITORINFOEX);
	::GetMonitorInfoW(hMonitor, &monitorinfo);

	// center window rect within monitors work area
	const RECT& mrect = monitorinfo.rcMonitor;
	const LONG monitor_w = mrect.right - mrect.left;
	const LONG monitor_h = mrect.bottom - mrect.top;

	const LONG window_w = window_rect.right - window_rect.left;
	const LONG window_h = window_rect.bottom - window_rect.top;

	const LONG x = mrect.left + std::max<LONG>(0, (monitor_w - window_w) / 2);
	const LONG y = mrect.top + std::max<LONG>(0, (monitor_h - window_h) / 2);


	xOut = x;
	yOut = y;
}



int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	// set the one global value for module.
	g_hModule = hInstance;

	// Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
	// Using this awareness context allows the client area of the window 
	// to achieve 100% scaling while still allowing non-client window content to 
	// be rendered in a DPI sensitive fashion.
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	// turn on debug layer before initalizing Direct3D 12 device. Doing it after will cause the device to be released.
	enable_GPU_debug_layer();
	UINT x, y;
	find_centered_pos(1280, 720, x,y);

	std::unique_ptr<DemoCube> demo1 = std::make_unique<DemoCube>();
	try {
		auto& app = Application::instance();

		app.initialise_dx12();
		app.initialise_render_window(L"demo", x,y,1280,720);

		demo1->load_content();

		app.set_game(demo1.get());

		app.run();

		demo1->unload_content();
		app.shutdown();
	}
	catch (const std::exception& e)
	{
		MessageBoxA(nullptr, e.what(), "Fatal Error", MB_OK | MB_ICONERROR);
	}

	return 0;
}
