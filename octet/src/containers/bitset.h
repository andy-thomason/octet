////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// fixed size boolean bitset
//

namespace octet { namespace containers {
  template < unsigned size_ > class bitset
  {
    unsigned bits_[ ( size_ + 31 ) / 32 ];

  public:

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

    bool intersects( const bitset &b ) const
    {
      unsigned u = 0;
      for( unsigned i = 0; i < ( size_ + 31 ) / 32; ++i )
      {
        u |= bits_[ i ] & b.bits_[ i ];
      }
      return u != 0;
    }

    void make_union( const bitset &b )
    {
      for( unsigned i = 0; i < ( size_ + 31 ) / 32; ++i )
      {
        bits_[ i ] |= b.bits_[ i ];
      }
    }

    bitset make_intersect( const bitset &b ) const
    {
      bitset result;
      for( unsigned i = 0; i < ( size_ + 31 ) / 32; ++i )
      {
        result.bits_[ i ] = bits_[ i ] & b.bits_[ i ];
      }
      return result;
    }

  };
} }
