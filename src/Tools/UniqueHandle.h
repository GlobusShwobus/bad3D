#pragma once

#include <utility>

#include <App/badWin32.h>

class UniqueHandle
{
public:
	UniqueHandle() noexcept = default;
	explicit UniqueHandle(HANDLE h) noexcept : mHandle(h) {}
	~UniqueHandle() { reset(); }

	UniqueHandle(const UniqueHandle&) = delete;
	UniqueHandle& operator=(const UniqueHandle&) = delete;

	UniqueHandle(UniqueHandle&& rhs) noexcept
		: mHandle(std::exchange(rhs.mHandle, nullptr))
	{
	}

	UniqueHandle& operator=(UniqueHandle&& rhs) noexcept
	{
		if (this != &rhs)
			reset(std::exchange(rhs.mHandle, nullptr));
		return *this;
	}

	void reset(HANDLE h = nullptr) noexcept
	{
		if (mHandle)
			::CloseHandle(mHandle);
		mHandle = h;
	}

	HANDLE get() const noexcept { return mHandle; }
	explicit operator bool() const noexcept { return mHandle != nullptr; }

private:
	HANDLE mHandle = nullptr;
};