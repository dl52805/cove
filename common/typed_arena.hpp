#pragma once

#include <assert.h>
#include <stdio.h>

#include "def.hpp"

#include "os.hpp"

template<typename T>
struct Typed_Arena
{
  Typed_Arena *prev;
  Typed_Arena *current;
  u64 reserved_size;
  u64 commit_size;
  u64 committed;
  u64 base_offset;
  u64 offset;

  u64 idx;
  u64 base_idx;

  constexpr static u64 default_commit_size = 64 * 1024;
  constexpr static u64 default_reserve_size = gigabytes(1);

  static Typed_Arena *create(u64 reserve = default_reserve_size,
                             u64 commit = default_commit_size)
  {
    u64 page = os_page_size();
    u64 reserve_align = align_to(reserve, page);
    u64 commit_align = align_to(commit, page);

    void *mem = os_reserve(reserve_align);
    os_commit(mem, commit_align);

    Typed_Arena *arena = (Typed_Arena *) mem;
    arena->reserved_size = reserve_align;
    arena->commit_size = commit_align;
    arena->committed = commit_align;
    arena->offset = sizeof(Typed_Arena);
    arena->base_offset = 0;
    arena->prev = nullptr;
    arena->current = arena;
    arena->idx = 0;
    arena->base_idx = 0;

    return arena;
  }

  i64 push()
  {
    u64 size = sizeof(T);
    u64 align = alignof(T);

    Typed_Arena *arena = current;
    u64 aligned_offset = align_to(arena->offset, align);
    u64 new_offset = aligned_offset + size;
    new_offset = align_to(new_offset, align);

    if (arena->reserved_size < new_offset)
    {
      u64 reserve = arena->reserved_size;
      u64 commit = arena->commit_size;

      u64 new_size = align_to(size, align)
                     + align_to(sizeof(Typed_Arena), align);
      if (new_size > arena->reserved_size)
      {
        reserve = align_to(new_size, gigabytes(1));
        commit = align_to(new_size, os_page_size());
      }
      Typed_Arena *new_arena = create(reserve, commit);
      new_arena->base_offset = arena->base_offset + arena->reserved_size;
      new_arena->base_idx = arena->idx;
      new_arena->idx = arena->idx;
      new_arena->prev = arena;
      this->current = new_arena;
      arena = new_arena;

      aligned_offset = align_to(arena->offset, align);
      new_offset = aligned_offset + size;
      new_offset = align_to(new_offset, align);
    }

    if (arena->committed < new_offset)
    {
      u64 commit_offset = align_to(new_offset, arena->commit_size);
      commit_offset = min(commit_offset, arena->reserved_size);
      u64 commit = commit_offset - arena->committed;
      u8 *commit_ptr = (u8 *) arena + arena->committed;
      os_commit(commit_ptr, commit);
      arena->committed = commit_offset;
    }

    i64 ret_idx = -1;
    if (arena->committed >= new_offset)
    {
      ret_idx = arena->idx;
      arena->offset = new_offset;
      arena->idx += 1;
    }

    return ret_idx;
  }

  i64 push(T obj)
  {
    static_assert(__is_trivially_copyable(T),
      "can only use `push_obj` on a trivially copyable object");

    i64 idx = push<T>();
    if (idx >= 0) *get<T>(idx) = obj;
    return idx;
  }

  i64 push_arr(u64 n)
  {
    u64 size = sizeof(T);
    u64 align = alignof(T);
    u64 adjusted_size = align_to(size, align);
    u64 total_size = adjusted_size * n;

    Typed_Arena *arena = current;
    u64 aligned_offset = align_to(arena->offset, align);
    u64 new_offset = aligned_offset + total_size;

    if (arena->reserved_size < new_offset)
    {
      u64 reserve = arena->reserved_size;
      u64 commit = arena->commit_size;

      u64 new_size = total_size + align_to(sizeof(Typed_Arena), align);
      if (new_size > arena->reserved_size)
      {
        reserve = align_to(new_size, gigabytes(1));
        commit = align_to(new_size, os_page_size());
      }
      Typed_Arena *new_arena = create(reserve, commit);
      new_arena->base_offset = arena->base_offset + arena->reserved_size;
      new_arena->base_idx = arena->idx;
      new_arena->idx = arena->idx;
      new_arena->prev = arena;
      this->current = new_arena;
      arena = new_arena;

      aligned_offset = align_to(arena->offset, align);
      new_offset = aligned_offset + total_size;
      new_offset = align_to(new_offset, align);
    }

    if (arena->committed < new_offset)
    {
      u64 commit_offset = align_to(new_offset, arena->commit_size);
      commit_offset = min(commit_offset, arena->reserved_size);
      u64 commit = commit_offset - arena->committed;
      u8 *commit_ptr = (u8 *) arena + arena->committed;
      os_commit(commit_ptr, commit);
      arena->committed = commit_offset;
    }

    i64 ret_idx = -1;
    if (arena->committed >= new_offset)
    {
      ret_idx = arena->idx;
      arena->offset = new_offset;
      arena->idx += n;
    }

    return ret_idx;
  }

  T *get(u64 idx)
  {
    Typed_Arena *curr = this->current;
    while (true)
    {
      if (curr->base_idx <= idx) break;
      curr = curr->prev;
    }
    u64 offset = (idx - curr->base_idx) * align_to(sizeof(T), alignof(T))
                 + align_to(sizeof(Typed_Arena), alignof(T));
    u8 *arr = (u8 *) curr;
    return (T *) (arr + offset);
  }

  T *operator[](u64 idx)
  {
    return get(idx);
  }

  u64 length()
  {
    return idx;
  }

  void clear()
  {
    u64 offset_adjust = sizeof(Typed_Arena);

    Typed_Arena *curr = this->current;
    Typed_Arena *prev = nullptr;
    while (true)
    {
      if (curr->base_offset <= offset_adjust) break;
      prev = curr->prev;
      os_release(curr, curr->reserved_size);
      curr = prev;
    }
    this->current = curr;
    u64 new_offset = offset_adjust - curr->base_offset;
    curr->offset = new_offset;

    this->base_idx = 0;
    this->idx = 0;
  }

  void deinit()
  {
    Typed_Arena *curr = this->current;
    Typed_Arena *prev = nullptr;
    while (true)
    {
      prev = curr->prev;
      os_release(curr, curr->reserved_size);
      if (!prev) break;
      curr = prev;
    }
  }
};

