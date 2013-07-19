////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// map key_t to value_t.
//
// strings and values are owned by the dictionary.
//
// A hash map is like a dictionary in JavaScript or Python, but works with only one type of key and value.
//
// example:
//   hash_map<chars, int> chars_to_int;
//   chars_to_int["x"] = 1;
//   chars_to_int["y"] = 2;
//   int x = chars_to_int["x"];
//   int y = chars_to_int["y"];
//
template <typename key_t, typename value_t, class allocator_t=allocator> class hash_map {
  // internal gubbins to implement the hash map
  struct entry_t { key_t key; unsigned hash; value_t value; };
  entry_t *entries;
  unsigned num_entries;
  unsigned max_entries;
  bool is_entry(const key_t &key);
  unsigned calc_hash(const key_t &key);
  
  // internal method to find an existing key in the map
  entry_t *find( const key_t &key, unsigned hash ) {
    unsigned mask = max_entries - 1;
    for (unsigned i = 0; i != max_entries; ++i) {
      entry_t *entry = &entries[ ( i + hash ) & mask ];
      if (!entry->key) {
        return entry;
      }
      if (entry->hash == hash && entry->key == key) {
        return entry;
      }
    }
    assert(0 && "hash_map: this should not happen");
    return 0;
  }
  
  // increase the size of the map if we have run out of space
  void expand() {
    entry_t *old_entries = entries;
    unsigned old_max_entries = max_entries;
    entries = (entry_t *)allocator_t::malloc(sizeof(entry_t) * max_entries*2);
    memset(entries, 0, sizeof(entry_t) * max_entries*2);
    max_entries *= 2;
    for (unsigned i = 0; i != old_max_entries; ++i) {
      entry_t *old_entry = &old_entries[i];
      if (old_entry->key) {
        entry_t *new_entry = find(old_entry->key, old_entry->hash);
        *new_entry = *old_entry;
      }
    }
    allocator_t::free(old_entries, sizeof(entry_t) * old_max_entries);
  }
public:
  // allocate a small map for starters that has a small number of elements.
  hash_map() {
    num_entries = 0;
    max_entries = 4;
    entries = (entry_t*)allocator_t::malloc(sizeof(entry_t) * max_entries);
    memset(entries, 0, sizeof(entry_t) * max_entries);
  }
  
  // access the 
  // eg. my_map["fred"]
  value_t &operator[]( const key_t &key ) {
    unsigned hash = calc_hash(key);
    entry_t *entry = find( key, hash );
    if (!entry->key) {
      // reducing this ratio decreases hot search time at the
      // expense of size (cold search time).
      if (num_entries > max_entries * 3 / 4) {
        expand();
        entry = find(key, hash);
      }
      num_entries++;
      entry->key = key;
      entry->hash = hash;
    }
    return entry->value;
  }

  // bye bye hash map
  ~hash_map() {
    allocator_t::free(entries, sizeof(entry_t) * max_entries);
    entries = 0;
    num_entries = 0;
    max_entries = 0;
  }

  // get the number of used entries in the map
  int size() { return num_entries; }
  entry_t *get_entries() { return entries; }
};
