#pragma once

#include <cassert>

#if _WIN32 || _WIN64
#if _WIN64
#define FT_ENV64BIT
#else
#define FT_ENV32BIT
#endif
#endif

#if defined(FT_ENV64BIT)
#define FT_WIN32_WIN64_VALUE_SWITCH(value32, value64) value64
#elif defined(FT_ENV32BIT)
#define FT_WIN32_WIN64_VALUE_SWITCH(value32, value64) value32
#else
#error "Must define either FT_ENV32BIT or FT_ENV64BIT"
#endif

#ifdef NDEBUG
#define FT_ASSERT(x)
#else
#define FT_ASSERT(x) assert(x)
#endif

#define FT_INVALID_INDEX (-1)

constexpr int FT_ALLOC_SIZE_PRIME = FT_WIN32_WIN64_VALUE_SWITCH(29, 31);