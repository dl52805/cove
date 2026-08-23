#pragma once

// osx
#if defined(__APPLE__)
  #include "os_macos.hpp"
#endif

// linux
#if defined(__linux__)
  #error os layer not implemented
#endif

// windows
#if defined(_WIN32)
  #error os layer not implemented
#endif

inline u32 os_page_size()
{
  #if defined(__APPLE__)
    return osx_page_size();
  #else
    #error os layer not implemented
  #endif
}

inline void *os_reserve(u64 size)
{
  #if defined(__APPLE__)
    return osx_reserve(size);
  #else
    #error os layer not implemented
  #endif
}

inline void os_commit(void *ptr, u64 size)
{
  #if defined(__APPLE__)
    osx_commit(ptr, size);
  #else
    #error os layer not implemented
  #endif
}

inline void os_release(void *ptr, u64 size)
{
  #if defined(__APPLE__)
    osx_release(ptr, size);
  #else
    #error os layer not implemented
  #endif
}

inline void os_decommit(void *ptr, u64 size)
{
  #if defined(__APPLE__)
    osx_decommit(ptr, size);
  #else
    #error os layer not implemented
  #endif
}

