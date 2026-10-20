#pragma once

constexpr unsigned long long KB(unsigned long long x) noexcept {
    return x * 1024ULL;
}

constexpr unsigned long long MB(unsigned long long x) noexcept {
    return x * 1024ULL * 1024ULL;
}

enum PageSize : unsigned long long
{
    _64KB = KB(64),
    _1MB = MB(1),
    _2MB = MB(2),
    _4MB = MB(4),
    _8MB = MB(8),
    _16MB = MB(16),
    _32MB = MB(32),
    _64MB = MB(64),
    _128MB = MB(128),
    _256MB = MB(256)
};