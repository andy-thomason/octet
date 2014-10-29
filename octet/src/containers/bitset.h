////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// fixed size boolean bitset
//

namespace octet { namespace containers {
  /// Fixed size bit set class.
  /// Bit sets are useful for keeping track of multiple boolean states.
  /// For example, if we have a particle system, we can track the "live" state of a particle
  /// by using only one bit per particle.
  ///
  /// Example
  ///
  ///     bitset<256> is_upper_case;
  ///     for (unsigned i = 'A'; i <= 'Z'; ++i) {
  ///       is_upper_case.setbit(i);
  ///     }
  ///
  ///     now we can test if a character is upper case:
  ///     if (bitset[chr]) printf("%c is upper case\n", chr);
  ///
  template < unsigned size_ > class bitset
  {
    unsigned bits_[ ( size_ + 31 ) / 32 ];

  public:
    /// clear the bit set to zeros
    void clear()
    {
      for( unsigned i = 0; i < ( size_ + 31 ) / 32; ++i )
      {
        bits_[ i ] = 0;
      }
    }

    /// In the case of a character bitset, set these character values to "true"
    /// Example:
    ///
    ///   bitset<256> numbers;
    ///   numbers = "0123456789";
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

    /// In the case of numeric sets, set these members to 1.
    void operator =( const short *members )
    {
      clear();
      while( members[ 0 ] != -1 )
      {
        setbit( (unsigned char)members[ 0 ] );
        members++;
      }
    }

    /// Return 1 if a specific bit is set.
    unsigned operator[] ( size_t index ) const
    {
      return ( bits_[ index >> 5 ] >> ( index & 31 ) ) & 1;
    }
    
    /// Set a bit to one.
    void setbit( unsigned bit )
    {
      bits_[ bit >> 5 ] |= 1 << ( bit & 31 );
    }

    /// Reset a bit to zero.
    void clearbit( unsigned bit )
    {
      bits_[ bit >> 5 ] &= ~(1 << ( bit & 31 ));
    }

    /// Return true if thse two sets intersect.
    bool intersects( const bitset &b ) const
    {
      unsigned u = 0;
      for( unsigned i = 0; i < ( size_ + 31 ) / 32; ++i )
      {
        u |= bits_[ i ] & b.bits_[ i ];
      }
      return u != 0;
    }

    /// Make the union of sets a and b into a.
    void operator |( const bitset &b )
    {
      for( unsigned i = 0; i < ( size_ + 31 ) / 32; ++i )
      {
        bits_[ i ] |= b.bits_[ i ];
      }
    }

    /// Make the intersection of sets a and b into a.
    bitset operator &( const bitset &b ) const
    {
      bitset result;
      for( unsigned i = 0; i < ( size_ + 31 ) / 32; ++i )
      {
        result.bits_[ i ] = bits_[ i ] & b.bits_[ i ];
      }
      return result;
    }

    /// Make the union of sets a and b into a.
    bitset operator ~() const
    {
      bitset result;
      for( unsigned i = 0; i < ( size_ + 31 ) / 32; ++i )
      {
        result.bits_[ i ] = ~bits_[ i ];
      }
      return result;
    }

    operator bool() const {
      unsigned tot = 0;
      for( unsigned i = 0; i < ( size_ + 31 ) / 32; ++i )
      {
        tot |= bits_[ i ];
      }
      return tot != 0;
    }

    const char *toString(char *buf, size_t size) {
      const char *result = buf;
      *buf++ = '[';
      for (size_t i = 0; i != size_ && i != size-3; ++i) {
        *buf++ = (*this)[i] ? 'X' : '.';
      }
      *buf++ = ']';
      *buf = 0;
      return result;
    }
  };
} }
