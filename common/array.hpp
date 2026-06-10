#pragma once

#include <string.h>

#include "def.hpp"
#include "allocator.hpp"

template<typename T>
struct Array
{
  constexpr static size_t default_capacity = 256;
  constexpr static f32 resize_factor = 1.7;

  Allocator *alloc;

  T *data;
  u32 capacity;
  u32 length;

  Array(u32 capacity = default_capacity,
        Allocator *allocator = new Libc_Alloc())
  {
    this->alloc = allocator;
    this->capacity = capacity;
    this->length = 0;
    data = (T *) alloc->allocate(capacity * sizeof(T)).unwrap();
  }

  Array(Allocator *allocator)
  {
    this->alloc = allocator;
    this->capacity = default_capacity;
    this->length = 0;
    data = (T *) alloc->allocate(capacity * sizeof(T)).unwrap();
  }

  Array(T init_vals[], u32 init_len, Allocator *allocator = new Libc_Alloc())
  {
    this->alloc = allocator;
    capacity = default_capacity;
    while (capacity < init_len) capacity = ((f64) capacity * resize_factor);
    length = init_len;
    data = (T *) alloc->allocate(capacity * sizeof(T)).unwrap();

    for (size_t i = 0; i < init_len; i++)
    {
      data[i] = init_vals[i];
    }
  }

  Array(T init_val, u32 init_len = 1, Allocator *allocator = new Libc_Alloc())
  {
    this->alloc = allocator;
    capacity = default_capacity;
    while (capacity < init_len) capacity = ((f64) capacity * resize_factor);
    length = init_len;
    data = (T *) alloc->allocate(capacity * sizeof(T)).unwrap();

    for (size_t i = 0; i < init_len; i++)
    {
      data[i] = init_val;
    }
  }

  void append(T elem)
  {
    length += 1;
    size_t old_capacity = capacity;
    while (length > capacity) capacity = ((f64) capacity * resize_factor);

    if (capacity > old_capacity)
    {
      data = (T *) alloc->reallocate(data, sizeof(T) * old_capacity,
                                     sizeof(T) * capacity).unwrap();
    }

    data[length - 1] = elem;
  }

  void insert(T elem, u32 index = 0)
  {
    assert((index >= 0) && (index < length));
    length += 1;

    size_t old_capacity = capacity;
    while (length > capacity) capacity = ((f64) capacity * resize_factor);

    if (capacity > old_capacity)
    {
      data = (T *) alloc->reallocate(data, sizeof(T) * old_capacity,
                                     sizeof(T) * capacity).unwrap();
    }

    for (size_t i = length - 1; i > index; i--)
    {
      data[i] = data[i - 1];
    }
    data[index] = elem;
  }

  T remove(u32 index)
  {
    assert((index >= 0) && (index < length));

    T removed = data[index];
    for (size_t i = index; i < length; i++)
    {
      data[i] = data[i + 1];
    }
    length -= 1;
    return removed;
  }

  T remove_last()
  {
    assert(length > 0);

    length -= 1;
    return data[length];
  }

  void reset()
  {
    length = 0;
  }

  T& operator[](u32 index)
  {
    assert((index >= 0) && (index < length));

    return data[index];
  }

  const T& operator[](u32 index) const
  {
    assert((index >= 0) && (index < length));

    return data[index];
  }
};
