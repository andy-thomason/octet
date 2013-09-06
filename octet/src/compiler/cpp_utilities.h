namespace octet
{
  template < unsigned size_ > struct cpp_set
  {
    void clear()
    {
      for( unsigned i = 0; i < ( size_ + 31 ) / 32; ++i )
      {
        bits_[ i ] = 0;
      }
    }

    void operator =( const char *members )
    {
      clear();
      while( members[ 0 ] )
      {
        if( members[ 1 ] == '-' )
        {
          for( unsigned i = (unsigned char)members[ 0 ]; i <= (unsigned char)members[ 2 ]; ++i ) setbit( i );
          members += 3;
        } else
        {
          setbit( (unsigned char)members[ 0 ] );
          members++;
        }
      }
    }

    void operator =( const short *members )
    {
      clear();
      while( members[ 0 ] != -1 )
      {
        setbit( (unsigned char)members[ 0 ] );
        members++;
      }
    }

    unsigned operator[] ( unsigned index )
    {
      return ( bits_[ index >> 5 ] >> ( index & 31 ) ) & 1;
    }
    
    void setbit( unsigned bit )
    {
      bits_[ bit >> 5 ] |= 1 << ( bit & 31 );
    }

    bool intersects( const set &b ) const
    {
      unsigned u = 0;
      for( unsigned i = 0; i < ( size_ + 31 ) / 32; ++i )
      {
        u |= bits_[ i ] & b.bits_[ i ];
      }
      return u != 0;
    }

    void make_union( const set &b )
    {
      for( unsigned i = 0; i < ( size_ + 31 ) / 32; ++i )
      {
        bits_[ i ] |= b.bits_[ i ];
      }
    }

    set make_intersect( const set &b ) const
    {
      set result;
      for( unsigned i = 0; i < ( size_ + 31 ) / 32; ++i )
      {
        result.bits_[ i ] = bits_[ i ] & b.bits_[ i ];
      }
      return result;
    }

    unsigned bits_[ ( size_ + 31 ) / 32 ];
  };

  template < typename data_type, typename allocator_type > class cpp_simple_array
  {
  protected:
    data_type *data_;
    unsigned length_;
  public:
    simple_array()
    {
      data_ = 0;
      length_ = 0;
    }

    simple_array( unsigned length, allocator_type &allocator )
    {
      set_length( length, allocator );
    }

    ~simple_array()
    {
      delete [] data_;
    }

    void set_length( unsigned length, allocator_type &allocator )
    {
      length_ = length;
      data_ = new (allocator) data_type[ length ];
    }

    data_type &last()
    {
      return data_[ length_ - 1 ];
    }

    operator data_type*() const
    {
      return data_;
    }

    unsigned length() const
    {
      return length_;
    }

    void operator=( simple_array &b )
    {
      data_ = b.data_;
      length_ = b.length_;
      b.data_ = 0;
      b.length_ = 0;
    }

    template < typename key_type > int index( const key_type &key ) const
    {
      for( unsigned i = 0; i != length_; ++i )
      {
        if( data_[ i ] == key )
        {
          return (int)i;
        }
      }
      return -1;
    }
  };

  template < class data_type, class allocator_type > class grow_array : public simple_array< data_type, allocator_type >
  {
    typedef simple_array< data_type, allocator_type > sa;
    unsigned capacity_;
  public:
    grow_array()
    {
      capacity_ = 0;
    }

    grow_array( unsigned length, allocator_type &allocator )
    {
      capacity_ = 0;
      set_length( length, allocator );
    }

    ~grow_array()
    {
    }

    void set_capacity( unsigned new_capacity, allocator_type &allocator )
    {
      data_type *new_data = new (allocator) data_type[ new_capacity ];
      for( unsigned i = 0; i != sa::length_; ++i )
      {
        new_data[ i ] = sa::data_[ i ];
      }
      capacity_ = new_capacity;
      sa::data_ = new_data;
    }

    void set_length( unsigned length, allocator_type &allocator )
    {
      if( length > capacity_ )
      {
        set_capacity( length > capacity_ * 2 ? length : capacity_ * 2, allocator );
      }
      sa::length_ = length;
    }

    void append( const data_type &datum, allocator_type &allocator )
    {
      set_length( sa::length_ + 1, allocator );
      sa::data_[ sa::length_ - 1 ] = datum;
    }

    data_type &append( allocator_type &allocator )
    {
      set_length( sa::length_ + 1, allocator );
      return sa::last();
    }

    void insert( const data_type &datum, unsigned index, allocator_type &allocator )
    {
      set_length( sa::length_ + 1, allocator );
      for( unsigned i = sa::length_; i != index; --i )
      {
        sa::data_[ i ] = sa::data_[ i-1 ];
      }
      sa::data_[ index ] = datum;
    }

    void remove( unsigned index, allocator_type &allocator )
    {
      unsigned imax = sa::length_ - 1;
      for( unsigned i = index; i != imax; ++i )
      {
        sa::data_[ i ] = sa::data_[ i+1 ];
      }
      set_length( sa::length_ - 1, allocator );
    }

    unsigned capacity() const
    {
      return capacity_;
    }

    void operator=( grow_array &b )
    {
      sa::operator=( b );
      capacity_ = b.capacity_;
      b.capacity_ = 0;
    }
  };

  template < typename key_type, typename value_type, typename allocator_type > struct dictionary
  {
    struct keyvalue
    {
      key_type key_;
      value_type value_;
    };
    
    dictionary()
    {
      num_unused_ = 0;
    }
    
    void put( const key_type &key, const value_type &value, allocator_type &allocator )
    {
      grow( allocator );
      keyvalue &addr = get_put_addr( key );
      addr.key_ = key;
      addr.value_ = value;
      
      num_unused_++;
    }
    
    value_type &put_addr( const key_type &key, allocator_type &allocator )
    {
      grow( allocator );
      keyvalue &addr = get_put_addr( key );
      addr.key_ = key;
      
      num_unused_++;

      return addr.value_;
    }
    
    int index( const key_type &key ) const
    {
    	if( data_.length() != 0 )
    	{
	      unsigned hash = key.hash();
	      unsigned mask = ( data_.length() - 1 );
	      unsigned index = hash & mask, index0 = index;
	      do
	      {
	        keyvalue &data = data_[ index ];
	        if( data.key_.is_free() )
	        {
	          return -1;
	        }
	        if( data.key_ == key )
	        {
	          return (int)index;
	        }
	        index = ( index + 1 ) & mask;
	      } while( index != index0 );
    	}
      return -1;
    }
    
    int next( int index, const key_type &key )
    {
      for(;;)
      {
        index = ( index + 1 ) & ( data_.length() - 1 );
        if( data_[ index ].key_.is_free() )
        {
        	return -1;
        }
        if( data_[ index ].key_ == key )
        {
        	return index;
        }
      }
    }

    keyvalue &operator[]( int index )
    {
      return data_[ index & ( data_.length() - 1 ) ];
    }

    void grow( allocator_type &allocator )
    {
      if( num_unused_ >= data_.length() / 2 )
      {
        unsigned length = data_.length();
        simple_array < keyvalue, allocator_type > tmp = data_;
        data_.set_length( length == 0 ? 8 : length * 2, allocator );
        for( unsigned i = 0; i < length; ++i )
        {
          keyvalue &addr = get_put_addr( tmp[ i ].key_ );
          addr.key_ = tmp[ i ].key_;
          addr.value_ = tmp[ i ].value_;
        }
      }
    }

    keyvalue &get_put_addr( const key_type &key )
    {
      for(;;)
      {
        unsigned hash = key.hash();
        unsigned mask = ( data_.length() - 1 );
        unsigned index = hash & mask, index0 = index;
        do
        {
          keyvalue &data = data_[ index ];
          if( data.key_.is_free() )
          {
            return data;
          }
          index = ( index + 1 ) & mask;
        } while( index != index0 );

      }
    }
    
    simple_array < keyvalue, allocator_type > data_;
    unsigned num_unused_;
  };

	// names are used as keys for preprocessor defines, symbols and types.
	// the hash entry makes it easier to build an index.
  template < class context_type > class name
  {
  	typedef typename context_type::char_type char_type;
  	char_type *text_;
  	unsigned hash_;
 	public:
 		name()
 		{
 			text_ = 0;
 			hash_ = 0;
 		}

		// create a new name
 		name( const char_type* text, context_type &context )
 		{
 			unsigned hash = 0;
 			unsigned length = 0;
 			for( const char_type *src = text; *src; ++src )
 			{
 				hash += *src;
 				hash += hash << 5;
 				hash += hash >> 3;
 				length++;
 			}
 			text_ = new (context) char_type[ length+1 ];
 			char_type*dest = text_;
 			for( const char_type *src = text; *src; ++src )
 			{
 				*dest++ = *src;
 				hash += *src;
 				hash += hash << 7;
 				hash += hash >> 1;
 			}
 			*dest = 0;
 			hash_ = hash;
 		}

 		// comparing names
 		bool operator==( const name &rhs ) const
 		{
 			if( rhs.hash_ == hash_ )
 			{
 				char_type *s1, *s2;
 				for( s1 = text_, s2 = rhs.text_; *s1 && *s2 && *s1 == *s2; )
 				{
 					s1++; s2++;
 				}
 				return *s1 == 0 && *s2 == 0;
 			}
 			return false;
 		}
 		
 		// hash part of a key
 		unsigned hash() const
 		{
 			return hash_;
 		}
 		
 		// text part of a key
 		const char_type *text() const
 		{
 			return text_;
 		}
 		
 		// free slot in a hash table
 		bool is_free() const
 		{
 			return text_ == 0;
 		}
 		
 		// used to undefine symbols
 		void set_text( char_type *text )
 		{
 			text_ = text;
 		}
 		
 		// get a string ( for debugging )
 		const char *to_string( char *dest, unsigned length )
 		{
 			const char *dest0 = dest;
 			unsigned i = 0;
 			while( i < length - 1 && text_[ i ] )
 			{
 				*dest++ = (char)text_[ i++ ];
 			}
 			*dest = 0;
 			return dest0;
 		}
  };

  // strings are arrays of characters
  template < class context_type > class string
  {
  	typedef typename context_type::char_type char_type;
  	char_type *text_;
  	unsigned length_;

 	public:
    string()
    {
      text_ = 0;
      length_ = 0;
    }

    string( context_type &context, const char_type *str )
    {
      length_ = context_type::strlen( str );
 			text_ = new (context) char_type[ length_+1 ];
      context_type::strcpy( text_, str );
    }

    string( context_type &context, string &str )
    {
      length_ = str.length_;
 			text_ = new (context) char_type[ length_+1 ];
      context_type::strcpy( text_, str.text_ );
    }

    void append( context_type &context, const char_type *str )
    {
      unsigned length = context_type::strlen( str );
      char_type *old_text = text_;
 			text_ = new (context) char_type[ length_ + length + 1 ];
      if( old_text != NULL )
      {
        context_type::strcpy( text_, old_text );
      }
      context_type::strcpy( text_ + length_, str );
      length_ += length;
    }

 		const char *to_string( char *dest, unsigned length )
 		{
 			const char *dest0 = dest;
 			const char *dest_max = dest + length - 5;
      *dest++ = '"';
      for( unsigned i = 0; i != length_ && dest < dest_max; ++i )
      {
        char_type chr = text_[ i ];
        if( chr == '\\' || chr == '"' )
        {
          *dest++ = '\\';
          *dest++ = chr;
        } else if( chr >= ' ' && chr <= 0x7e )
        {
          *dest++ = chr;
        } else if( chr == '\t' )
        {
          *dest++ = '\\';
          *dest++ = 't';
        } else if( chr == '\n' )
        {
          *dest++ = '\\';
          *dest++ = 'n';
        } else if( chr == '\r' )
        {
          *dest++ = '\\';
          *dest++ = 'r';
        } else
        {
          *dest++ = '\\';
          *dest++ = '0' + chr >> 6;
          *dest++ = '0' + chr >> 3 & 7;
          *dest++ = '0' + chr & 7;
        }
      }
      *dest++ = '"';
 			*dest = 0;
 			return dest0;
 		}

    const char_type *text()
    {
      return text_;
    }
  };

  class option
  {
  public:
    enum Type
    {
      type_bool,
      type_text,
    };
  private:
    unsigned length;
    char *name;
    char *description;
    char *option_text;
    enum Type type;
  public:
    option( char *name, char *default_text, char *description, enum Type type )
    {
      //fprintf( stderr, "name=%s\n", name );
      this->name = name;
      this->length = name == NULL ? 0 : strlen( name );
      this->option_text = default_text;
      this->description = description;
      this->type = type;
    }
  
    bool parse( int argc, char **argv, int &index )
    {
      int i = index;
  
      //fprintf( stderr, "[%s: parse %d/%d: %s]\n", name, i, argc, argv[ i ] );
      if( !strcmp( argv[ i ], "--help" ) )
      {
        fprintf( stderr, "%-30s%s\n", description, name );
        return false;
      }
  
      if( ( name == NULL && argv[i][0] != '-' ) || ( name != NULL && !memcmp( name, argv[ i ], length ) ) )
      {
        if( type == type_bool )
        {
          option_text = "t";
          ++index;
          return true;
        } else if( type == type_text )
        {
          char *arg = argv[i][ length ] ? argv[i] + length : i < argc ? argv[ ++i ] : NULL;
          if( option_text != NULL )
          {
            fprintf( stderr, "error: more than one %s\n", description );
            index = i + 1;
            return true;
          }
          if( arg == NULL )
          {
            fprintf( stderr, "error: no %s specified\n", description );
            index = i + 1;
            return true;
          }
      
          option_text = arg;
    
          //fprintf( stderr, "opt %s=%s\n", name, arg );
          index = i + 1;
          return true;
        }
      }
  
      return false;
    }
  
    char *text()
    {
      return option_text;
    }
  
    void set_text( char *value )
    {
      option_text = value;
    }
  };
}

*/
