#pragma once

#include "def.hpp"
#include "allocator.hpp"

#include <initializer_list>

template<typename K, typename V>
struct Hash_Table
{
  // NOTE(dl): this hash table requires that the Allocator zero
  // initializes the table memory, otherwise every table entry's
  // `occupied` field may not be set to false by default
  struct Entry
  {
    K key;
    V value;
    bool deleted;
    bool occupied;
    Entry(K key, V val, bool deleted = false, bool occupied = false)
      : key(key), value(val), deleted(deleted), occupied(occupied) {}
  };

  struct Init_Entry
  {
    K key;
    V value;
  };

  static constexpr u32 default_capacity = 1 << 5; // 32
  static constexpr f64 load_factor = 0.70;

  Allocator *alloc;

  Entry *table;
  u32 capacity;
  u32 size;

  u32 (*hash_fn)(K *key) = nullptr;

  Hash_Table(std::initializer_list<Init_Entry> list,
             Allocator *allocator = new Libc_Alloc())
  {
    this->alloc = allocator;
    u32 capacity = default_capacity;
    while (capacity * load_factor < list.size()) capacity *= 2;
    this->capacity = capacity;
    this->size = 0;
    table = (Entry *) alloc->allocate(capacity * sizeof(Entry)).unwrap();

    for (const Init_Entry& item : list)
    {
      insert(item.key, item.value);
    }
  }

  Hash_Table(u32 capacity = default_capacity,
             Allocator *allocator = new Libc_Alloc())
  {
    assert(capacity > 0);
    this->alloc = allocator;
    this->capacity = capacity;
    this->size = 0;
    table = (Entry *) alloc->allocate(capacity * sizeof(Entry)).unwrap();
  }

  Hash_Table(Allocator *allocator)
  {
    this->alloc = allocator;
    this->capacity = default_capacity;
    this->size = 0;
    table = (Entry *) alloc->allocate(capacity * sizeof(Entry)).unwrap();
  }

  void set_hash_fn(u32 (*hash_fn)(K *key))
  {
    this->hash_fn = hash_fn;
  }

  u32 compute_hash(K *key)
  {
    u32 i = 0;
    while (true)
    {
      u32 hash_idx;
      if constexpr (!__is_class(K)) hash_idx = hash_fn(key) + i * i;
      else hash_idx = key->hash() + i * i;
      hash_idx &= (capacity - 1);

      Entry curr_entry = table[hash_idx];
      if ((!curr_entry.occupied) || (curr_entry.key == *key)
          || (curr_entry.deleted))
      {
        return hash_idx;
      }

      i += 1;
    }
  }

  u32 compute_hash(K *key, Entry *new_table, u32 new_capacity)
  {
    u32 i = 0;
    while (true)
    {
      u32 hash_idx;
      if constexpr (!__is_class(K)) hash_idx = hash_fn(key) + i * i;
      else hash_idx = key->hash() + i * i;
      hash_idx &= (new_capacity - 1);

      Entry curr_entry = new_table[hash_idx];
      if ((!curr_entry.occupied) || (curr_entry.key == *key)
          || (curr_entry.deleted))
      {
        return hash_idx;
      }

      i += 1;
    }
  }

  i64 find_index(K *key)
  {
    u32 i = 0;
    while (true)
    {
      u32 hash_idx;
      if constexpr (!__is_class(K)) hash_idx = hash_fn(key) + i * i;
      else hash_idx = key->hash() + i * i;
      hash_idx &= (capacity - 1);

      Entry curr_entry = table[hash_idx];
      if (!curr_entry.occupied) return -1;
      if (curr_entry.key == *key)
      {
        if (curr_entry.deleted) return -1;
        return hash_idx;
      }

      i += 1;
    }
  }

  V *find(K key)
  {
    i64 hash_idx = find_index(&key);
    if (hash_idx == -1) return nullptr;
    return &(table[hash_idx].value);
  }

  void insert(K key, V value)
  {
    u32 hash_idx = compute_hash(&key);

    if (table[hash_idx].occupied)
    {
      table[hash_idx].key   = key;
      table[hash_idx].value = value;

      if (table[hash_idx].deleted)
      {
        table[hash_idx].deleted = false;
        size += 1;
      }
    }
    else
    {
      table[hash_idx].occupied = true;
      table[hash_idx].deleted  = false;
      table[hash_idx].key   = key;
      table[hash_idx].value = value;
      size += 1;
    }

    if (((f64) (size) / (f64) (capacity)) > load_factor)
    {
      u32 old_capacity = capacity;
      capacity <<= 1;
      Entry *new_table = (Entry *) alloc->allocate(capacity).unwrap();
      for (u32 i = 0; i < old_capacity; i++)
      {
        Entry curr_entry = table[i];
        if (curr_entry.occupied && !curr_entry.deleted)
        {
          u32 new_idx = compute_hash(&key, new_table, capacity);
          new_table[new_idx] = curr_entry;
        }
      }
      alloc->deallocate(table);
      table = new_table;
    }
  }

  void remove(K key)
  {
    i64 hash_idx = find_index(&key);
    if (hash_idx == -1) return;
    table[hash_idx].deleted = true;
    size -= 1;
  }

  void clear()
  {
    for (u32 i = 0; i < capacity; i++)
    {
      if (table[i].occupied) table[i].deleted = true;
    }
    size = 0;
  }
};
