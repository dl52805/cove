#pragma once

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

