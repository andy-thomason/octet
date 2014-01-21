////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// map key_t to value_t.
//
// strings and values are owned by the dictionary.
//
namespace octet { namespace containers {

  /// A support class for hash_map that is used to implement different kinds of key.
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

  /// A map fom a key type to an object type.
  ///
  /// Do not use for strings, use %dictionary instead.
  ///
  /// A hash map is like a dictionary in JavaScript or Python, but works with only one type of key and value.
  ///
  /// Example:
  ///
  ///     hash_map<int, int> int_to_int;
  ///     int_to_int[5] = 7;
  ///     int_to_int[9] = 11;
  ///     printf("[5]=%d [9]=%d\n", int_to_int[5], int_to_int[9]);
  ///
  ///     for (unsigned i = 0; i != int_to_int.size(); ++i) {
  ///       printf("key=d value=%d\n", int_to_int.get_key(i), int_to_int.get_value(i));
  ///     }
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
    // Create an empty map.
    hash_map() {
      init();
    }

    /// Remove all keys and values from the hash map.
    void clear() {
      release();
      init();
    }
  
    /// Access the map by key
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

    /// Does the map have this key?
    bool contains(const key_t &key) {
      unsigned hash = cmp_t::get_hash(key);
      entry_t *entry = find( key, hash );
      return !cmp_t::is_empty(entry->key);
    }

    /// Get an integer that represents the position in the map of this key.
    ///
    /// Note: only valid if the map does not change size.
    int get_index(const key_t &key) {
      unsigned hash = cmp_t::get_hash(key);
      entry_t *entry = find( key, hash );
      return entry ? (int)(entry - entries) : -1;
    }

    /// For a specfic index, get the key.
    ///
    /// Used for iterating through the map or if using find()
    const key_t &get_key(int index) const {
      assert((unsigned)index < max_entries);
      return entries[index].key;
    }

    /// For a specific index, get the value
    const value_t &get_value(int index) const {
      assert((unsigned)index < max_entries);
      return entries[index].value;
    }

    /// bye bye hash map
    ~hash_map() {
      allocator_t::free(entries, sizeof(entry_t) * max_entries);
      entries = 0;
      num_entries = 0;
      max_entries = 0;
    }

    /// Get the maximum number of keys and values in the map.
    ///
    /// Used for iteration.
    unsigned size() { return max_entries; }
    
    //key_t key(unsigned i) { return entries[i].key; }
    //value_t value(unsigned i) { return entries[i].value; }
  };
} }
