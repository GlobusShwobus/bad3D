#include <sstream>

#include "Stopwatch.h"

#include "Application.h"

#include "DemoCube2.h"

#include "ViewPtr.h"
#include "Utils.h"
#include <fstream>


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

void write_my_root_sig_to_file1(ID3D12Device4* device)
{
	// create a root signature
	// check for root sig version, 1.1 is recommended
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	// allow input layout and deny unnecessary acces to certain pipeline stages
	D3D12_ROOT_SIGNATURE_FLAGS rootsigflags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
												D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
												D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
												D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
												D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	// root sig desc
	D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootsigdesc = {};
	rootsigdesc.Version = featureData.HighestVersion;

	if (featureData.HighestVersion == D3D_ROOT_SIGNATURE_VERSION_1_1)
	{
		const UINT matrix_32bit_value_count = sizeof(DirectX::XMMATRIX) / sizeof(UINT); //16
		D3D12_ROOT_PARAMETER1 rootParameters11[3] = {
			ROOT_PARAMETER::constant(D3D12_SHADER_VISIBILITY_VERTEX, 0,  matrix_32bit_value_count),
			ROOT_PARAMETER::constant(D3D12_SHADER_VISIBILITY_VERTEX, 1,  matrix_32bit_value_count),
			ROOT_PARAMETER::constant(D3D12_SHADER_VISIBILITY_VERTEX, 2,  matrix_32bit_value_count)
		};

		rootsigdesc.Desc_1_1 = ROOT_DESCRIPTION::description(_countof(rootParameters11), rootParameters11, rootsigflags);
	}
	else
	{
		throw std::runtime_error{ "sad times" };
	}

	// serialize
	Microsoft::WRL::ComPtr<ID3DBlob> rootSigBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

	execute_and_test_hresult(
		D3D12SerializeVersionedRootSignature(&rootsigdesc, &rootSigBlob, &errorBlob)
	);


	std::ofstream file("DemoCubeRootSig.bin", std::ios::binary);
	if (!file)
	{
		throw std::runtime_error{
			"Failed to open root signature output file"
		};
	}
	file.write(
		static_cast<const char*>(rootSigBlob->GetBufferPointer()),
		static_cast<std::streamsize>(rootSigBlob->GetBufferSize())
	);
	if (!file)
	{
		throw std::runtime_error{
			"Failed to write root signature file"
		};
	}
	//execute_and_test_hresult(
	//	device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature))
	//);
}

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

HRESULT enable_GPU_debug_layer()
{
	HRESULT hr = S_OK;
#if defined(_DEBUG)
	// Always enable the debug layer before doing anything DX12 related
	// so all possible errors generated while creating DX12 objects
	// are caught by the debug layer.
	Microsoft::WRL::ComPtr<ID3D12Debug> debugInterface;
	hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface));

	if (SUCCEEDED(hr))
		debugInterface->EnableDebugLayer();
#endif
	return hr;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{

	// Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
	// Using this awareness context allows the client area of the window 
	// to achieve 100% scaling while still allowing non-client window content to 
	// be rendered in a DPI sensitive fashion.
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	// turn on debug layer before initalizing Direct3D 12 device. Doing it after will cause the device to be released.
	enable_GPU_debug_layer();
	UINT x, y;
	find_centered_pos(1280, 720, x,y);

	AppWinDesc winDesc;
	winDesc.window_name = L"Demo";
	winDesc.window_style = WS_OVERLAPPEDWINDOW;
	winDesc.x = x;
	winDesc.y = y;
	winDesc.cw = 1280;
	winDesc.ch = 720;
	winDesc.hInstance = hInstance;

	std::unique_ptr<IGame> demo1 = std::make_unique<DemoCube2>();
	try {
		auto& app = Application::instance();

		app.initialise(winDesc);

		//write_my_root_sig_to_file1(app.get_device());

		demo1->load_content();

		app.set_game(ViewPtr{ demo1.get() });

		app.run();

		app.shutdown();
	}
	catch (const std::exception& e)
	{
		MessageBoxA(nullptr, e.what(), "Fatal Error", MB_OK | MB_ICONERROR);
	}

	return 0;
}
