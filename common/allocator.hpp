#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

struct Allocator
{
  enum struct Error
  {
    none,
    out_of_memory,
    size_overflow,
  };

  struct Result
  {
    void *ptr;
    Error error;

    Result(Error error)
    {
      this->ptr = nullptr;
      this->error = error;
    }

    Result(void *ptr)
    {
      this->ptr = ptr;
      this->error = Error::none;
    }

    void *unwrap()
    {
      if (error != Error::none)
      {
        fprintf(stderr, "allocation error!\n");
        exit(1);
      }
      return ptr;
    }
  };

  virtual Result allocate(size_t size)
  {
    return Result(Error::out_of_memory);
  }

  virtual void deallocate(void *ptr) {}

  virtual Result reallocate(void *ptr, size_t old_size, size_t new_size)
  {
    return Result(Error::out_of_memory);
  }
};

struct Libc_Alloc : Allocator
{
  Result allocate(size_t size)
  {
    if (size == 0) return Result(Error::size_overflow);
    void *ptr = calloc(1, size);
    if (ptr == nullptr) return Result(Error::out_of_memory);
    return Result(ptr);
  }

  void deallocate(void *ptr)
  {
    assert(ptr != nullptr);
    free(ptr);
  }

  Result reallocate(void *ptr, size_t old_size, size_t new_size)
  {
    (void) old_size;
    assert(ptr != nullptr);
    if (new_size == 0) return Result(Error::size_overflow);
    void *new_ptr = realloc(ptr, new_size);
    if (new_ptr == nullptr) return Result(Error::out_of_memory);
    return Result(new_ptr);
  }
};

