#pragma once

#include <stdio.h>
#include <stdarg.h>

#include "def.hpp"

constexpr uintptr_t fnv1a_hash(char *data, size_t size)
{
  size_t hash = 14695981039346656037ull;
  for (size_t i = 0; i < size; i++)
  {
    hash ^= (u64) data[i];
    hash *= 1099511628211ull;
  }
  return (uintptr_t) hash;
}

template<typename T>
constexpr u32 primitive_hash(T *data)
{
  return fnv1a_hash((char *) data, sizeof(T));
}

__attribute__((format(printf, 3, 4)))
inline void debug_log_impl(const char *file, int line, const char *fmt, ...)
{
  fprintf(stdout, "[LOG]: (%s:%d) ", file, line);
  va_list args;
  va_start(args, fmt);
  vfprintf(stdout, fmt, args);
  va_end(args);
  fprintf(stdout, "\n");
}

// debug_log macro
#define debug_log(fmt, ...) \
  debug_log_impl(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

