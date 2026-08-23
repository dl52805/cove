#pragma once

#include <sys/mman.h>
#include <unistd.h>

#include "def.hpp"

inline u32 osx_page_size()
{
  static u32 pagesize = (u32) sysconf(_SC_PAGESIZE);
  return pagesize;
}

inline void *osx_reserve(u64 size)
{
  u64 gb_align_size = (size + (gigabytes(1) - 1)) & ~((u64) (gigabytes(1) - 1));
  void *ptr = mmap(
    nullptr,
    gb_align_size,
    PROT_NONE,
    MAP_PRIVATE | MAP_ANONYMOUS,
    -1,
    0
  );
  if (ptr == MAP_FAILED) return nullptr;
  return ptr;
}

inline void osx_commit(void *ptr, u64 size)
{
  u64 page_align_size = (size + (osx_page_size() - 1)) & ~((u64) (osx_page_size() - 1));
  madvise(ptr, page_align_size, MADV_FREE_REUSE);
  mprotect(ptr, page_align_size, PROT_READ | PROT_WRITE);
}

inline void osx_release(void *ptr, u64 size)
{
  munmap(ptr, size);
}

inline void osx_decommit(void *ptr, u64 size)
{
  u64 page_align_size = (size + (osx_page_size() - 1)) & ~((u64) (osx_page_size() - 1));
  madvise(ptr, page_align_size, MADV_FREE_REUSABLE);
  mprotect(ptr, page_align_size, PROT_NONE);
}
