#pragma once

#include "def.hpp"
#include "allocator.hpp"

template<typename T>
struct Stack
{
  Allocator *alloc;

  T *data;
  u32 capacity;
  u32 length;

  constexpr static u32 default_capacity = 1 << 6; // 64
  constexpr static f64 resize_factor = 1.7;

  Stack(u32 capacity = default_capacity,
        Allocator *allocator = new Libc_Alloc())
  {
    this->alloc = allocator;
    this->capacity = capacity;
    this->length = 0;
    data = (T *) alloc->allocate(capacity * sizeof(T)).unwrap();
  }

  Stack(T init_vals[], u32 init_len, Allocator *allocator = new Libc_Alloc())
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

  Stack(T init_val, u32 init_len = 1, Allocator *allocator = new Libc_Alloc())
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

  void push(T elem)
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

  T pop()
  {
    assert(length > 0);

    length -= 1;
    return data[length];
  }

  T peek()
  {
    return data[length - 1];
  }

  bool empty()
  {
    assert(length >= 0);
    return length == 0;
  }

  void reset()
  {
    length = 0;
  }
};

