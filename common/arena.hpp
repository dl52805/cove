#pragma once

#include <stdlib.h>
#include <string.h>

#include "def.hpp"

#include "allocator.hpp"

struct Region
{
  u32 capacity;
  u32 offset;
  Region *next;
  // NOTE(dl): this is not standard C++, technically undefined behavior,
  // however this is supported on clang, gcc, and msvc
  u8 buffer[0];

  static Region *create(size_t capacity)
  {
    u8 *region_mem = new u8[sizeof(Region) + capacity];
    memset(region_mem, 0, sizeof(Region) + capacity);
    Region *region = (Region *) region_mem;
    region->offset = 0;
    region->capacity = capacity;
    region->next = nullptr;
    return region;
  }

  static void destroy(Region *region)
  {
    delete[] (u8 *) region;
  }
};

struct Arena : Allocator
{
  enum struct Impl
  {
    fixed,
    linked,
  };

  struct State
  {
    Region *region;
    u32 offset;
  };

  static constexpr Impl fixed  = Impl::fixed;
  static constexpr Impl linked = Impl::linked;

  constexpr static size_t default_size = 1024 * 8;

  u32 region_size;

  Impl impl;

  Region *begin;
  Region *end;

  Arena(Impl impl = linked, u32 size = default_size)
  {
    this->region_size = size;
    this->impl = impl;

    begin = Region::create(this->region_size);
    end = begin;
  }

  Result allocate(size_t size)
  {
    if (size == 0) return Result(Error::size_overflow);

    // align to the allocation size of a pointer (4/8 bytes)
    constexpr size_t ptr_size = sizeof(uintptr_t);
    size_t alloc_size = (size + (ptr_size - 1)) & ~(ptr_size - 1);

    // check if there are any regions after our arena end that contain
    // enough memory for the allocation, or verify that are at the end
    // of the linked list of regions
    for (; (end->next != nullptr)
            && (end->offset + alloc_size > end->capacity);
         end = end->next) {}

    if (end->offset + alloc_size > end->capacity)
    {
      if (impl == fixed) return Result(Error::out_of_memory);
      size_t new_cap = max(alloc_size, region_size);
      Region *region = Region::create(new_cap);
      end->next = region;
      end = region;
    }

    u8 *begin = &(end->buffer[end->offset]);
    end->offset += alloc_size;

    return Result(begin);
  }

  // deallocation in arena allocators is a no-op
  void deallocate(void *ptr)
  {
    return;
  }

  Result reallocate(void *ptr, size_t old_size, size_t size)
  {
    Result new_alloc = this->allocate(size);
    if (new_alloc.error != Error::none)
    {
      fprintf(stderr, "allocation error!\n");
      return Result(new_alloc.error);
    }
    u8 *new_ptr = (u8 *) new_alloc.ptr;
    u8 *old_ptr = (u8 *) ptr;
    for (int i = 0; i < min(old_size, size); i++)
    {
      new_ptr[i] = old_ptr[i];
    }
    return Result(new_ptr);
  }

  State snapshot()
  {
    return State(end, end->offset);
  }

  void rewind(State state)
  {
    for (Region *region = state.region->next; region != nullptr;
         region = region->next)
    {
      region->offset = 0;
    }
    end = state.region;
    end->offset = state.offset;
  }

  void deinit()
  {
    Region *region = begin;
    while (true)
    {
      if (region == nullptr) break;
      Region *next = region->next;
      Region::destroy(region);
      region = next;
    }
    begin = nullptr;
    end = nullptr;
  }
};
