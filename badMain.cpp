#include <windows.h>

#include "Keyboard.h"
#include "Mouse.h"
#include "Window.h"
#include "Renderer.h"

#include <sstream>
#include "Stopwatch.h"

void EnableDebugLayer();


int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
	// Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
	// Using this awareness context allows the client area of the window 
	// to achieve 100% scaling while still allowing non-client window content to 
	// be rendered in a DPI sensitive fashion.
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	EnableDebugLayer();

	Keyboard kb;
	Mouse mouse;

	Window window(hInstance, L"nigga", kb, mouse);
	Renderer renderer(window);

	if (!renderer.is_init())
	{
		return 0;
	}

	window.show_window(nCmdShow);
	

	const float threshhold = 0.3f;



	uint64_t frameCounter = 0;
	double elapsedSeconds = 0.0;
	std::chrono::high_resolution_clock cock;
	auto t0 = cock.now();


	MSG msg = {};
	Stopwatch st;
	while (msg.message != WM_QUIT)
	{
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
			SetWindowTextA(window.window(), buffer);
			frameCounter = 0;
			elapsedSeconds = 0.0;
		}

		float dt = st.dt_float();
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	

		window.end_frame(dt);
		renderer.render();
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

void EnableDebugLayer()
{
#if defined(_DEBUG)
	// Always enable the debug layer before doing anything DX12 related
	// so all possible errors generated while creating DX12 objects
	// are caught by the debug layer.
	Microsoft::WRL::ComPtr<ID3D12Debug> debugInterface;
	HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface));

	if (FAILED(hr))
		throw std::runtime_error("pashol nahui");

	debugInterface->EnableDebugLayer();
#endif
}

// The ParseCommandLineArguments function allows a few of the globally defined variables to be overridden by supplying command-line arguments when the application is executed.

/*
void ParseCommandLineArguments()
{
	int argc;
	wchar_t** argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);

	for (size_t i = 0; i < argc; ++i)
	{
		if (::wcscmp(argv[i], L"-w") == 0 || ::wcscmp(argv[i], L"--width") == 0)
		{
			g_ClientWidth = ::wcstol(argv[++i], nullptr, 10);
		}
		if (::wcscmp(argv[i], L"-h") == 0 || ::wcscmp(argv[i], L"--height") == 0)
		{
			g_ClientHeight = ::wcstol(argv[++i], nullptr, 10);
		}
		if (::wcscmp(argv[i], L"-warp") == 0 || ::wcscmp(argv[i], L"--warp") == 0)
		{
			g_UseWarp = true;
		}
	}

	// Free memory allocated by CommandLineToArgvW
	::LocalFree(argv);
}
*/