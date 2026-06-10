#pragma once

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "allocator.hpp"

#include "def.hpp"
#include "util.hpp"

struct String8
{
  Allocator *alloc;

  // when the string is allocated on the stack, we do not care about
  // the `capacity` field and use the following `stack_size` instead
  constexpr static size_t stack_size = 63;

  union
  {
    u8 stack[stack_size + 1];
    u8 *buffer;
  };

  u32 length;
  u32 capacity;

  u32 default_capacity = 128;

  bool s_alloc;

  char *c_str()
  {
    if (s_alloc) return (char *) stack;
    else return (char *) buffer;
  }

  String8(Allocator *allocator)
  {
    alloc = allocator;
    length = 0;
    s_alloc = true;
    stack[0] = '\0';
    this->capacity = default_capacity;
  }

  String8() {}

  String8(size_t capacity, Allocator *allocator = new Libc_Alloc())
  {
    alloc = allocator;
    if (capacity <= stack_size) s_alloc = true;
    else
    {
      buffer = (u8 *) alloc->allocate(capacity).unwrap();
      s_alloc = false;
    }

    this->capacity = capacity;
  }

  String8(const char *data, Allocator *allocator = new Libc_Alloc())
    : String8(data, strlen(data), allocator) {}

  String8(const char *data, u64 size,
          Allocator *allocator = new Libc_Alloc())
  {
    alloc = allocator;
    if (size <= stack_size)
    {
      memcpy(stack, data, size);
      stack[size] = '\0';
      length = size;
      s_alloc = true;
    } else
    {
      capacity = default_capacity;
      while ((size + 1) > capacity) capacity *= 2;
      buffer = (u8 *) alloc->allocate(capacity * sizeof(u8)).unwrap();
      memcpy(buffer, data, size);
      buffer[size] = '\0';
      length = size;
      s_alloc = false;
    }
  }

  static void read_from_file(String8 *str, const char *file_path,
                             Allocator *allocator = new Libc_Alloc())
  {
    str->alloc = allocator;

    FILE *fp = fopen(file_path, "rb");
    fseek(fp, 0L, SEEK_END);
    size_t file_size = ftell(fp);
    rewind(fp);

    if (file_size <= stack_size)
    {
      size_t read_len = fread(
        str->stack, sizeof(char), file_size, fp
      );
      str->stack[read_len] = '\0';
      str->length = read_len;
      str->s_alloc = true;
    }
    else
    {
      str->capacity = max(file_size + 1, str->default_capacity);
      u8 *file_buf = (u8 *) allocator->allocate(str->capacity).unwrap();
      size_t read_len = fread(
        file_buf, sizeof(char), file_size, fp
      );
      file_buf[read_len] = '\0';
      str->buffer = file_buf;
      str->s_alloc = false;
      str->length = read_len;
    }
  }

  void cat(const char *data)
  {
    assert(data != nullptr);

    cat(data, strlen(data));
  }

  void cat(char c)
  {
    if (s_alloc)
    {
      if ((length + 1) <= stack_size)
      {
        stack[length++] = c;
        stack[length] = '\0';
      }
      else
      {
        capacity = default_capacity;
        while ((2 + length) > capacity) capacity *= 2;

        u8 *str_data = (u8 *) alloc->allocate(capacity * sizeof(u8)).unwrap();
        memcpy(str_data, stack, length);
        buffer = str_data;

        buffer[length++] = c;
        buffer[length] = '\0';

        s_alloc = false;
      }
    }
    else {
      u32 old_capacity = capacity;
      while ((2 + length) > capacity) capacity *= 2;

      if (old_capacity < capacity)
      {
        buffer = (u8 *) alloc->reallocate(
          buffer, old_capacity, capacity).unwrap();
      }

      buffer[length++] = c;
      buffer[length] = '\0';
    }
  }

  void cat(const char *data, u64 size)
  {
    if (s_alloc)
    {
      if ((size + length) <= stack_size)
      {
        u8 *cpy_buf = &(stack[length]);
        memcpy(cpy_buf, data, size);
        length += size;
        stack[length] = '\0';
      }
      else
      {
        capacity = default_capacity;
        while ((size + 1 + length) > capacity) capacity *= 2;

        u8 *str_data = (u8 *) alloc->allocate(capacity * sizeof(u8)).unwrap();
        memcpy(str_data, stack, length);
        buffer = str_data;

        u8 *cpy_buf = &(buffer[length]);
        memcpy(cpy_buf, data, size);

        length += size;
        buffer[length] = '\0';

        s_alloc = false;
      }
    }
    else {
      u32 old_capacity = capacity;
      while ((size + 1 + length) > capacity) capacity *= 2;

      if (old_capacity < capacity)
      {
        buffer = (u8 *) alloc->reallocate(
          buffer, old_capacity, capacity).unwrap();
      }

      u8 *cpy_buf = &(buffer[length]);
      memcpy(cpy_buf, data, size);

      length += size;
      buffer[length] = '\0';
    }
  }

  void cut(size_t size)
  {
    if (size > length) return;

    if (s_alloc)
    {
      stack[length - size] = '\0';
    }
    else {
      buffer[length - size] = '\0';
    }
    length -= size;
  }

  void erase(size_t pos, size_t size)
  {
    if (size > length) return;
    if ((pos < 0) || (pos >= length)) return;
    if (pos + size > length) return;

    if (s_alloc)
    {
      u8 *remainder = &(stack[pos + size]);
      size_t remainder_size = length - (pos + size);
      u8 *cpy_buf = &(stack[pos]);
      memcpy(cpy_buf, remainder, remainder_size);
      stack[length - size] = '\0';
    }
    else {
      u8 *remainder = &(buffer[pos + size]);
      size_t remainder_size = length - (pos + size);
      u8 *cpy_buf = &(buffer[pos]);
      memcpy(cpy_buf, remainder, remainder_size);
      buffer[length - size] = '\0';
    }

    length -= size;
  }

  bool ends_in(const char *tail)
  {
    return ends_in(tail, strlen(tail));
  }

  bool ends_in(const char *tail, size_t tail_size)
  {
    if (tail_size > length) return false;
    // necessary in case the string is stack allocated
    char *data_buffer = c_str();
    char *tail_buffer = (char *) &data_buffer[length - tail_size];
    return memcmp(tail_buffer, tail, tail_size) == 0;
  }

  void to_lower()
  {
    char *data_buffer = c_str();
    for (int i = 0; i < length; i++)
    {
      data_buffer[i] = tolower(data_buffer[i]);
    }
  }

  void deinit()
  {
    if (!s_alloc) alloc->deallocate(buffer);
  }

  u32 hash()
  {
    char *str_ptr = c_str();
    return fnv1a_hash(str_ptr, length);
  }
};

struct String8_View
{
  u8 *buffer;
  u32 length;

  String8_View() {}

  String8_View(String8 str)
  {
    buffer = (u8 *) str.c_str();
    length = str.length;
  }

  String8_View(String8 *str)
  {
    buffer = (u8 *) str->c_str();
    length = str->length;
  }

  String8_View(char *buffer, u32 length)
  {
    this->buffer = (u8 *) buffer;
    this->length = length;
  }

  String8_View(char *buffer) : String8_View(buffer, strlen(buffer)) {}

  String8_View(const char *buffer) : String8_View((char *) buffer, strlen(buffer)) {}

  char *c_str()
  {
    return (char *) buffer;
  }

  u32 hash()
  {
    return fnv1a_hash((char *) buffer, length);
  }

  bool operator==(const String8_View& other) const
  {
    return (strncmp((char *) this->buffer,
                    (char *) other.buffer, length) == 0);
  }

  bool operator!=(const String8_View& other) const
  {
    return (strncmp((char *) this->buffer,
                    (char *) other.buffer, length) != 0);
  }
};
