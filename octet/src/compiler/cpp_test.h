#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef WIN32
  #undef UNICODE
  #include <windows.h>
  #define FC_NOINLINE
  #pragma warning( disable : 4996 )
  static const char *search_paths[] = { "./", 0 };
  #include <malloc.h>
#else
  #include <sys/fcntl.h>
  #include <sys/mman.h>
  #define FC_NOINLINE __attribute__( ( noinline ) )
  static const char *search_paths[] = { "./", "/usr/lib/gcc/x86_64-linux-gnu/4.0.3/include", "/usr/include", "/usr/include/linux", 0 };
#endif

#include "fc_utilities.h"
#include "fc_lexer.h"
#include "fc_preprocessor.h"




namespace fc
{
  class debug_support
  {
    int depth;
  
    const char *indentText( int d )
    {
      int d2 = 100 - d * 2;
      return "                                                                                                    " + ( d2 < 0 ? 0 : d2 );
    }
    
  public:
  	debug_support()
  	{
    	depth = 0;
  	}
  
    /*void debug( const char *format, ... )
    {
      va_list args;
      va_start( args, format );
      fprintf( stderr, "%*s", depth*2, "" );
      vfprintf( stderr, format, args );
    }
    
    void debug_push( const char *format, ... )
    {
      va_list args;
      va_start( args, format );
      fprintf( stderr, "%*s", depth*2, "" );
      vfprintf( stderr, format, args );
      depth++;
    }
    
    void debug_pop( const char *format, ... )
    {
      depth--;
      va_list args;
      va_start( args, format );
      fprintf( stderr, "%*s", depth*2, "" );
      vfprintf( stderr, format, args );
    }*/
    
    const char *push()
    {
      return indentText( depth++ );
    }
    
    const char *pop()
    {
      return indentText( --depth );
    }
    
    const char *indent()
    {
      return indentText( depth );
    }
    
    bool debug_expressions()
    {
    	return true;
    }
  };
  
  class file_mapper
  {
  public:
    struct map_file_result
    {
      const char *source_;
      const char *source_end_;
      const char *filename_;
      bool ok_;
    };
  
    //const char **searchPaths;
    std::vector< std::string > searchPaths;
    typedef std::vector< std::string >::iterator pathIterator;
  public:
  	file_mapper( const char **search_paths )
  	{
  	  while( *search_paths )
  	  {
  	    searchPaths.push_back( *search_paths );
  	    search_paths++;
  	  }
  	}
  	
  	void addIncludePath( const char *path )
  	{
	    searchPaths.push_back( path );
  	}
  	
    map_file_result try_file( const char *filename )
    {
      map_file_result result;
      result.ok_ = false;
      //cerr << "trying " << filename << "\n";
      
      #ifndef WIN32
        int file = open( filename, O_RDONLY, 0444 );
        if( file >= 0 )
        {
          struct stat file_stat;
          fstat( file, &file_stat );
          //printf( "%s: file=%d len=%d\n", filename, file, file_stat.st_size );
          result.source_ = (const char *)mmap( 0, file_stat.st_size, PROT_READ, MAP_PRIVATE, file, 0 );
          //printf( "source=%p\n", source );
          result.source_end_ = result.source_ + file_stat.st_size;
          result.filename_ = strdup( filename );
          result.ok_ = true;
        }
        return result;
      #else
        HANDLE file = CreateFile( filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );
        if( file == INVALID_HANDLE_VALUE )
        {
          result.ok_ = false;
          result.source_ = result.source_end_ = 0;
        } else
        {
          LARGE_INTEGER size;
          BOOL gfs = GetFileSizeEx( file, &size );
          HANDLE mapping = CreateFileMapping( file, 0, PAGE_READONLY, size.HighPart, size.LowPart, 0 );
          result.source_ = (const char*)MapViewOfFile( mapping, FILE_MAP_READ, 0, size.HighPart, size.LowPart );

          result.source_end_ = result.source_ + size.LowPart;
          result.filename_ = strdup( filename );
          result.ok_ = true;
          fwrite( result.source_, size.LowPart, 1, stderr );
        }
        return result;
      #endif
    }
    
    // create a mapping of a file in memory
    // return the start and end of the memory region
    // use the search path to find files
    map_file_result map_file( const char *filename, const char *currentPath )
    {
      char tmp[ 1024 ];
      map_file_result result;
      result.ok_ = false;
  
      if( filename[0] == '/' )
      {
        // absolute path name
        return try_file( filename );
      }
      
      size_t length = strlen( filename );

      if( currentPath )
      {
        size_t path_len = strlen( currentPath );

        if( path_len + length + 3 < sizeof( tmp ) )
        {
          memcpy( tmp, currentPath, path_len );

          if( path_len && tmp[ path_len-1] != '/' && tmp[ path_len-1] != '\\' )
          {
            tmp[ path_len++ ] = '/';
          }

          strcpy( tmp + path_len, filename );
          result = try_file( tmp );
        }
      }
      
      if( !result.ok_ )
      {
        for( pathIterator path = searchPaths.begin(); path != searchPaths.end(); ++path )
        {
          size_t path_len = strlen( (*path).data() );
  
          if( path_len + length + 3 > sizeof( tmp ) )
          {
            break;
          }
  
          memcpy( tmp, (*path).data(), path_len );
  
          if( path_len && tmp[ path_len-1] != '/' && tmp[ path_len-1] != '\\' )
          {
            tmp[ path_len++ ] = '/';
          }
  
          strcpy( tmp + path_len, filename );
          result = try_file( tmp );
  
          if( result.ok_ )
          {
            // file found
            break;
          }
        }
      }
  
      if( !result.ok_ )
      {
        printf( "file: %s not found\n", filename );
      }
  
      return result;
    }
  };
  
  const char *default_search_paths[] = { "./", 0 };
  
  class default_context : public debug_support, public file_mapper
  {
  public:
  	typedef char char_type;
  	typedef long long int64_type;
  	typedef unsigned long long uint64_type;
  
    enum
    {
      max_chars_ = 65536,
      max_stack_depth_ = 256,
      max_output_queue_ = 256, // 2^n
      max_define_params_ = 256,
      define_stack_max_ = 128,
    };
    
  private:
    #if 0
  
    void error_arg( const char *filename, unsigned line_number, fc::preprocessor_errors::error_ error, va_list args )
    {
      const char *format = uk_english( error );
      fprintf( stderr, "%s:%d:error: ", filename, line_number );
      vfprintf( stderr, format, args );
      fprintf( stderr, "\n" );
      ::exit( 1 );
    }
    
    void error_arg( const char *filename, unsigned line_number, const char *text, va_list args )
    {
      fprintf( stderr, "%s:%d:error: ", filename, line_number );
      vfprintf( stderr, text, args );
      fprintf( stderr, "\n" );
      ::exit( 1 );
    }
    #endif

    const char *fileName;
    unsigned lineNumber;    
    char errorBuffer[ 256 ];
    
  public:
    default_context() : file_mapper( default_search_paths )
    {
      lineNumber = 1;
      fileName = "";
    }
  
    default_context( const char **search_paths ) : file_mapper( search_paths )
    {
      lineNumber = 1;
      fileName = "";
    }
  
    default_context &getContext()
    {
      return *this;
    }
    
    void setFileName( const char *value )
    {
      fileName = value;
    }
    
    unsigned setLineNumber( unsigned value )
    {
      lineNumber = value;
    }
    
    const char *pperror( fc::preprocessor_errors::error_ error )
    {
      switch( error )
      {
        case fc::preprocessor_errors::none: return "ok";
        case fc::preprocessor_errors::stack_overflow: return "too many includes or ifs";
        case fc::preprocessor_errors::line_too_long: return "line too long";
        case fc::preprocessor_errors::unterminated_comment: return "/* with no */";
        case fc::preprocessor_errors::unknown: return "unknown preprocessor directive";
        case fc::preprocessor_errors::syntax: return "syntax error";
        case fc::preprocessor_errors::expect: return "expected %s got %s";
        case fc::preprocessor_errors::internal: return "internal error";
        case fc::preprocessor_errors::file_not_found: return "file %s not found";
      }
      return "unknown";
    }

    const char *errorText()
    {
      errorBuffer[ 0 ] = 0;
      if( strlen( fileName ) < sizeof( errorBuffer ) - 20 )
      {
        sprintf( errorBuffer, "%s(%d): ", fileName, lineNumber );
      }
      return errorBuffer;
    }
  
    /*void is_true( bool expression, const char *text, ... )
    {
    	if( expression )
    	{
  	    va_list args;
  	    va_start( args, text );
  	    error_arg( "", 0, text, args );
    	}
    }*/
    
    /*void error( const char *filename, unsigned line_number, fc::preprocessor_errors::error_ error, ... )
    {
      va_list args;
      va_start( args, error );
      error_arg( filename, line_number, error, args );
    }
  
    void error( const char *filename, unsigned line_number, const char *text, ... )
    {
      va_list args;
      va_start( args, text );
      error_arg( filename, line_number, text, args );
    }*/
  
    double ascii_to_double( const char_type *src )
    {
      return 0;
    }
    
    void exit( int value )
    {
      ::exit( value );
    }
    
    void zero( void *dest, size_t size )
    {
      memset( dest, 0, size );
    }
    
    void *alloc( size_t size )
    {
      return malloc( size );
    }

    static void strcpy( char_type *dest, const char_type *src )
    {
      while( *src ) *dest++ = *src++;
      *dest = 0;
    }
    
    static unsigned strlen( const char_type *src )
    {
      unsigned len = 0;
      while( *src++ ) len++;
      return len;
    }
    
    void format_number( char_type *dest, unsigned line_number, const char *file_name, unsigned flags )
    {
      sprintf(
        (char*)dest,
        "# %d \"%s\"%s%s%s%s",
        line_number,
        file_name,
        flags & 1 ? " 1" : "",
        flags & 2 ? " 2" : "",
        flags & 4 ? " 3" : "",
        flags & 8 ? " 4" : ""
      );
    }
    
  };
  
  /*
  static void test_preprocessor()
  {
    char *source =
    	"#define __WORDSIZE 64\n"
    	"#define __USER_LABEL_PREFIX__\n"
    	"#define __THROW __attribute__((__nothrow__))\n"
    	"#define __GNUC_MINOR__ 2\n"
    	"#define __GNUC__ 4\n"
    	//"#define __cplusplus\n"
    	"#include \"data/test.c\"\n";
  
    default_context context( search_paths );
  
    fc::preprocessor< default_context > preproc( context, source );
    
    while( preproc.cur_line() )
    {
      printf( "%s\n", (const char*)preproc.cur_line() );
      preproc.next_line();
    }
  }
  */
}

void *operator new( size_t bytes, fc::default_context &context )
{
  return malloc( bytes );
}

void *operator new[]( size_t bytes, fc::default_context &context )
{
  return malloc( bytes );
}

void operator delete( void *mem, fc::default_context &context )
{
  free( mem );
}

void operator delete[]( void *mem, fc::default_context &context )
{
  free( mem );
}


