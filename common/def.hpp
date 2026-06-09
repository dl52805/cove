#pragma once

#include <stdlib.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float  f32;
typedef double f64;

constexpr const char *light_blue = "\033[94m";
constexpr const char *green      = "\033[32m";
constexpr const char *red        = "\033[91m";
constexpr const char *purple     = "\033[38;2;186;168;224m";
constexpr const char *gray       = "\033[38;2;84;86;87m";
constexpr const char *reset      = "\033[0m";

#define max(arg1, arg2) (((arg1) > (arg2)) ? (arg1) : (arg2))
#define min(arg1, arg2) (((arg1) < (arg2)) ? (arg1) : (arg2))

