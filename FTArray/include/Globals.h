#pragma once

#include <cassert>

#if _WIN32 || _WIN64
#if _WIN64
#define FT_ENV64BIT
#else
#define FT_ENV32BIT
#endif
#endif

#ifdef NDEBUG
#define FT_ASSERT(x)
#else
#define FT_ASSERT(x) assert(x)
#endif

#define FT_INVALID_INDEX (-1)

#if defined(_WIN64)
constexpr int FT_ALLOC_SIZE_PRIME = 31;
#else
constexpr int FT_ALLOC_SIZE_PRIME = 29;
#endif
