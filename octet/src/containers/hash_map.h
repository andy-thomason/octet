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
namespace octet {

  class hash_map_cmp {
  public:
    // mix in some bits from higher positions to lower positions
    static unsigned fuzz_hash(unsigned hash) { return hash ^ (hash >> 3) ^ (hash >> 5); }

    static unsigned get_hash(void *key) { return fuzz_hash((unsigned)(intptr_t)key); }
    static unsigned get_hash(int key) { return fuzz_hash((unsigned)key); }
    static unsigned get_hash(unsigned key) { return fuzz_hash((unsigned)key); }
    static unsigned get_hash(uint64_t key) { return fuzz_hash((unsigned)(key ^ (key >> 32))); }

    static bool is_empty(void *key) { return !key; }
    static bool is_empty(int key) { return !key; }
    static bool is_empty(unsigned key) { return !key; }
    static bool is_empty(uint64_t key) { return !key; }

    //template <typename T> static bool equals(const T &lhs, const T &rhs) { return lhs == rhs; }
  };

  template <typename key_t, typename value_t, class cmp_t=hash_map_cmp, class allocator_t=allocator> class hash_map {
    // internal gubbins to implement the hash map
    struct entry_t { key_t key; unsigned hash; value_t value; };

    entry_t *entries;
    unsigned num_entries;
    unsigned max_entries;

    // internal method to find an existing key in the map
    entry_t *find( const key_t &key, unsigned hash ) {
      unsigned mask = max_entries - 1;
      for (unsigned i = 0; i != max_entries; ++i) {
        entry_t *entry = &entries[ ( i + hash ) & mask ];
        if (cmp_t::is_empty(entry->key)) {
          return entry;
        }
        if (entry->hash == hash && entry->key == key) {
          return entry;
        }
      }
      assert(0 && "hash_map: all entries are used. expand() did not get called");
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
        if (!cmp_t::is_empty(old_entry->key)) {
          entry_t *new_entry = find(old_entry->key, old_entry->hash);
          *new_entry = *old_entry;
        }
      }
      allocator_t::free(old_entries, sizeof(entry_t) * old_max_entries);
    }

    void release() {
      allocator_t::free(entries, sizeof(entry_t) * max_entries);
      entries = 0;
      num_entries = 0;
      max_entries = 0;
    }

    void init() {
      num_entries = 0;
      max_entries = 4;
      entries = (entry_t*)allocator_t::malloc(sizeof(entry_t) * max_entries);
      memset(entries, 0, sizeof(entry_t) * max_entries);
    }
  public:
    // allocate a small map for starters that has a small number of elements.
    hash_map() {
      init();
    }
  
    void clear() {
      release();
      init();
    }
  
    // access the 
    // eg. my_map["fred"]
    value_t &operator[]( const key_t &key ) {
      unsigned hash = cmp_t::get_hash(key);
      entry_t *entry = find( key, hash );
      if (cmp_t::is_empty(entry->key)) {
        // reducing this ratio decreases hot search time at the
        // expense of size (cold search time).
        if (num_entries >= max_entries * 3 / 4) {
          expand();
          entry = find(key, hash);
        }
        num_entries++;
        entry->key = key;
        entry->hash = hash;
      }
      return entry->value;
    }

    bool contains(const key_t &key) {
      unsigned hash = cmp_t::get_hash(key);
      entry_t *entry = find( key, hash );
      return !cmp_t::is_empty(entry->key);
    }

    int get_index(const key_t &key) {
      unsigned hash = cmp_t::get_hash(key);
      entry_t *entry = find( key, hash );
      return entry ? entry - entries : -1;
    }

    // bye bye hash map
    ~hash_map() {
      allocator_t::free(entries, sizeof(entry_t) * max_entries);
      entries = 0;
      num_entries = 0;
      max_entries = 0;
    }

    // stl-style iterators are bloated. This is a simpler iterator scheme
    unsigned size() { return max_entries; }
    key_t key(unsigned i) { return entries[i].key; }
    value_t value(unsigned i) { return entries[i].value; }
  };
}
