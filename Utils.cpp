#include "Utils.h"
#include <string>
#include <stdexcept>

void throw_error_code_translation(DWORD error_code)
{
	// TODO:: add maybe a message box
	// TODO:: add maybe a stack trace
	LPVOID lpMsgBuf = nullptr;

	DWORD len = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		error_code,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf,
		0, NULL
	);

	if (len == 0 || lpMsgBuf == nullptr)
		throw std::runtime_error("unknown error (code " + std::to_string(error_code) + ")");

	std::string msg((LPSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
	throw std::runtime_error(msg);
}

void execute_and_test_hresult(HRESULT hr)
{
	if (FAILED(hr))
		throw_error_code_translation(static_cast<DWORD>(hr));
}

void execute_and_test_BOOL(BOOL b)
{
	if (b == 0)
		throw_error_code_translation( GetLastError() );
}