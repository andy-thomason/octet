////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// C/C++ language subset preprocessor
// This was tested against GCC on a set of header files
//

namespace octet
{
  namespace compiler
  {
    class cpp_preprocessor : public cpp_token_enum {
      enum {
        max_chars_ = 65536,
        max_stack_depth_ = 256,
        max_output_queue_ = 256, // 2^n
        max_define_params_ = 256,
        def_exp_stack_max_ = 128,
      };

      typedef int error_;
      typedef char char_type;
    
      typedef string name_type;

      typedef cpp_tokens::token_type token_type;
    
      enum {
        debug_ifs = 0,
        debug_expressions = 0,

        stringise_escape = 253,
        param_escape = 254,
        stringise_param_escape = 255,
      };
    
      struct include_stack_entry_ {
        const char *current_;
        const char *end_;
        const char *file_name_;
        string currentPath;
        unsigned line_number_;
        bool is_wchar_;
      };
    
      struct if_stack_entry_ {
        bool is_true_ : 1;
        bool had_true_ : 1;
        bool bottom_level_ : 1;
        bool had_if_ : 1;
      };
    
      struct queue_entry_type {
        unsigned flags_;
        unsigned line_number_;
        const char *file_name_;
        const char_type *text_;
      };

      struct map_file_result {
        string filename_;
        const char *source_;
        const char *source_end_;
      };
    
      struct define_type {
        string text_;
        bool has_params_; // check for ()
      };

      struct define_expansion_type {
        string name_;                   // avoid recursive defines
        const char_type *src_;          // saved value of previous lexer_.get_src()
      };

      dictionary< define_type > defines_;

      const char_type *cur_line_;
      if_stack_entry_ *if_stack_ptr_;
      unsigned def_exp_stack_depth_;

      unsigned queue_in_;
      unsigned queue_out_;

      unsigned read_line_number_;
      const char *read_file_name_;

      unsigned last_line_number_;
      const char *last_file_name_;

      unsigned num_skipped_;
  
      if_stack_entry_ if_;
    
      cpp_lexer lexer_;
    
      dynarray<include_stack_entry_> include_stack_;
      bitset< tok_last > unary_op_;

      unsigned char precidence_[ tok_last ];
    
      // special symbol for #undef
      char_type undefined_[ 1 ];

      char_type read_line_[ max_chars_ ];
      char_type write_line_[ max_chars_ ];
      char_type number_line_[ max_chars_ ];
      char_type expand_line_[ max_chars_ ];
      char_type params_[ max_chars_ ];
    
      dynarray<if_stack_entry_> if_stack_;
      queue_entry_type queue_[max_output_queue_];
      const char_type *param_starts_[max_define_params_];
      define_expansion_type def_exp_stack_[def_exp_stack_max_];

      dynarray<char> log_text;

      include_stack_entry_ &include() {
        return include_stack_.back();
      }

      bool map_file(map_file_result &result, const char *filename, const char *first_path) {
        result.filename_.format("%s%s", first_path, filename);
        result.source_ = result.source_end_ = 0;
        return true;
      }
  
      unsigned line_number() {
        return read_line_number_;
      }
  
      const char *file_name() {
        return read_file_name_;
      }
    
      static bool strcmp( char_type *s1, char *s2 ) {
        while( *s1 != 0 && *s2 != 0 && *s1 == *s2 ) {
          s1++;
          s2++;
        }
        return *s1 != 0 || *s2 != 0;
      }
    
      void push_include( const char *current, const char *end, const char *file_name, unsigned line_number ) {
        //size_t length = strlen( file_name );
        string currentPath = file_name;
        int pos = currentPath.filename_pos();
        currentPath.truncate(pos);
      
        {
          include_stack_.resize(include_stack_.size() + 1);
          include_stack_entry_ &entry = include();
          entry.current_ = current;
          entry.end_ = end;
          entry.file_name_ = file_name;
          entry.line_number_ = line_number;
          entry.currentPath = currentPath;
        }
      }
    
      void push_if( bool is_true ) {
        if_stack_.push_back(if_);

        if_.is_true_ = is_true;
        if_.had_true_ = is_true;
        if_.bottom_level_ = false;
        if_.had_if_ = false;
      }
    
      void pop_if() {
        if_ = if_stack_.back();
        if_stack_.pop_back();
      }
  
      void output( unsigned flags, unsigned line_number, const char *file_name, const char_type *text ) {
        queue_entry_type *entry = queue_ + queue_in_;
      
        queue_in_ = ( queue_in_ + 1 ) & ( max_output_queue_ - 1);
        entry->flags_ = flags;
        entry->line_number_ = line_number;
        entry->file_name_ = file_name;
        entry->text_ = text;
        //context_.debug( "o %d %d [%s]\n", queue_in_, queue_out_, text ? (char*)text : "" );
      }
    
      static void format_number( char_type *dest, unsigned line_number, const char *file_name, unsigned flags ) {
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

      bool is_whitespace( char_type chr ) {
        return ( chr > 0 && chr <= ' ' );
      }
    
      bool is_digit( char_type chr ) {
        return ( chr >= '0' && chr <= '9' );
      }
    
      bool is_id_start( char_type chr ) {
        return ( chr >= 'a' && chr <= 'z' ) || ( chr >= 'A' && chr <= 'Z' ) || chr == '_';
      }
    
      bool is_id_middle( char_type chr ) {
        return is_id_start( chr ) || is_digit( chr );
      }
    
      char_type *define_expansion( int index ) {
        define_type &define = defines_.get_value(index);
        if( define.has_params_ ) {
          unsigned num_params = 0;
          char_type *dest = params_;
          char_type *dest_max = params_ + max_chars_ - 1;
          //context_.debug( "define_expansion: [%s] [%s]\n", lexer_.get_src(), define.text_ );
          lexer_.lex_token();
          if( lexer_.type() != tok_lparen ) {
            cpp_log("expected '('\n");
          } else {
            const char_type *start = lexer_.get_src();
            lexer_.lex_token();
            for(;;) // for each parameter
            {
              int level = 0;
              param_starts_[ num_params ] = dest;

              while( is_whitespace( *start ) ) {
                start++;
              }

              bool fail = false;
              while( !fail && ( level != 0 || (lexer_.type() != tok_comma && lexer_.type() != tok_rparen) ) ) {
                while( start != lexer_.get_src() ) {
                  *dest = *start++;
                  dest += dest != dest_max;
                }
                *dest = 0;
                level += lexer_.type() == tok_lparen;
                level -= lexer_.type() == tok_rparen;
                //printf( "%d [%s] %s\n", level, token_name( lexer_.type() ), param_starts_[ num_params ] );
                if( lexer_.type() == tok_newline ) {
                  if( def_exp_stack_depth_ != 0 ) {
                    cpp_log("error: no ) in define expansion\n");
                    fail = true;
                    break;
                  }
                  do
                  {
                    if( include().current_ == include().end_ ) {
                      cpp_log("error: end of file in define expansion\n");
                      fail = true;
                      break;
                    }
                    read_some_text();
                    //printf( "rst: %s\n", read_line_ );
                    start = read_line_;
                    while( is_whitespace( *start ) ) {
                      start++;
                    }
                    lexer_.start( start );
                    lexer_.lex_token();
                    //printf( "..[%s] %s\n", token_name( lexer_.type() ), param_starts_[ num_params ] );
                  } while( lexer_.type() == tok_newline );
                  *dest = ' ';
                  dest += dest != dest_max;
                } else {
                  lexer_.lex_token();
                }
              }

              *dest = 0;
               dest += dest != dest_max;

              //context_.debug( "p: %s\n", param_starts_[ num_params ] );
              num_params++;
              if( num_params == max_define_params_ ) {
                cpp_log("error: too many define parameters\n");
              }

              if( lexer_.type() == tok_comma ) {
                start = lexer_.get_src();
                lexer_.lex_token();
              } else {
                break;
              }
            }
          }
        
          //context_.debug( "expand: %d params\n", num_params );
          char_type *src = (char_type *)define.text_.c_str();
          dest = expand_line_;
          dest_max = expand_line_ + max_chars_ - 1;
          for( ; *src; ) {
            if( (*src & 255) == param_escape ) {
              unsigned param = src[ 1 ] - '0';
              if( param < num_params ) {
                const char_type *psrc = param_starts_[ param ];
                while( *psrc ) {
                  *dest = *psrc++;
                  dest += dest != dest_max;
                }
              }
              src += 2;
            } else if( (*src & 255) == stringise_param_escape ) {
              unsigned param = src[ 1 ] - '0';
              if( param < num_params ) {
                const char_type *psrc = param_starts_[ param ];
                *dest = (char)stringise_escape;
                dest += dest != dest_max;
              
                while( *psrc ) {
                  *dest = *psrc++;
                  dest += dest != dest_max;
                }

                *dest = (char)stringise_escape;
                dest += dest != dest_max;
              }
              src += 2;
            } else if( src[ 0 ] == '\'' || src[ 0 ] == '"' ) {
              char_type quote = src[ 0 ];
              *dest = *src++;
              dest += dest != dest_max;
              while( !( src[ 0 ] == quote && src[ -1 ] != '\\' ) && *src ) {
                *dest = *src++;
                dest += dest != dest_max;
              }
              *dest = *src;
              dest += dest != dest_max;
              src += *src != 0;
            } else {
              *dest = *src++;
              dest += dest != dest_max;
            }
          }
        
          char_type *text = (char_type *)allocator::malloc( ( dest - expand_line_ + 1 ) * sizeof( char_type ) );
          char_type *text_dest = text;
          for( src = expand_line_; src != dest; ++src ) {
            *text_dest++ = *src;
          }
          *text_dest = 0;
          //context_.debug( "result: %s\n\n", text );
          return text;
        } else {
          //context_.debug( "result: %s\n\n", define.text_ );
          return (char_type*)define.text_.c_str();
        }
      }
    
      void expand_defines() {
        //context_.debug( "expand_defines(): %s\n", read_line_ );
        lexer_.start( read_line_ );
        const char_type *src = read_line_;
        char_type *dest = write_line_;
        char_type *dest_max = write_line_ + max_chars_ - 1;
        for(;;) {
          const char_type *prev = lexer_.get_src();

          while( is_whitespace( *prev ) ) {
            prev++;
          }

          lexer_.lex_token();

          if( lexer_.type() == tok_newline ) {
            const char_type *cur = lexer_.get_src();
            if( def_exp_stack_depth_ ) {
              // reached end of line or end of define expansion.
              while( src != cur ) {
                if( (*src & 255) == stringise_escape ) {
                  *dest = '"';
                  src++;
                  dest += dest != dest_max;
                } else {
                  *dest = *src++;
                  dest += dest != dest_max;
                }
              }
              define_expansion_type &exp = def_exp_stack_[ --def_exp_stack_depth_ ];
              lexer_.start( exp.src_ );
              src = lexer_.get_src();
              //context_.debug( "pop: %s\n", src );
            } else {
              while( src != cur ) {
                *dest = *src++;
                dest += dest != dest_max;
              }
              break;
            }
          }
        
          if( lexer_.type() == tok_identifier ) {
            //string name( lexer_.id() );
            //context_.debug( "\nexpand: [%s] %s\n", lexer_.id(), lexer_.get_src() );
            const char *name = (const char*)lexer_.id();
            int index = defines_.get_index( name );

            for( unsigned i = 0; i != def_exp_stack_depth_; ++i ) {
              if( def_exp_stack_[ i ].name_ != name ) {
                index = -1; // already expanding this define
                break;
              }
            }

            if( index != -1 ) {
              if( def_exp_stack_depth_ ) {
                while( src != prev ) {
                  if( (*src & 255) == stringise_escape ) {
                    *dest = '"';
                    src++;
                    dest += dest != dest_max;
                  } else {
                    *dest = *src++;
                    dest += dest != dest_max;
                  }
                }
              } else {
                while( src != prev ) {
                  *dest = *src++;
                  dest += dest != dest_max;
                }
              }

              /*if( dest != write_line_ && dest[ -1 ] != ' ' ) {
                *dest = ' ';
                dest += dest != dest_max;
              }*/
            

              const char_type *expansion = define_expansion( index );

              define_expansion_type &exp = def_exp_stack_[ def_exp_stack_depth_ ];
              exp.src_ = lexer_.get_src();
              exp.name_ = name;
              lexer_.start( expansion );
              if( def_exp_stack_depth_ == def_exp_stack_max_ - 1 ) {
                cpp_log("error: define expansion stack overflow\n");
                break;
              }
              def_exp_stack_depth_++;
              src = lexer_.get_src();
            }
          }
        }

        const char_type *cur = lexer_.get_src();
        while( src != cur ) {
          *dest = *src++;
          dest += dest != dest_max;
        }
        *dest = 0;
      
        //context_.debug( "  result %s\n", write_line_ );

        output( 0x00, line_number(), file_name(), write_line_ );
        def_exp_stack_depth_ = 0;
      }

      void next() {
        for(;;) {
          lexer_.lex_token();
          while( lexer_.type() == tok_newline && def_exp_stack_depth_ ) {
            // reached end of line or end of define expansion.
            define_expansion_type &exp = def_exp_stack_[ --def_exp_stack_depth_ ];
            lexer_.start( exp.src_ );
            lexer_.lex_token();
          }
        
          if( lexer_.type() != tok_identifier ) {
            break;
          }

          const char *name = (const char*)lexer_.id();
          int index = defines_.get_index(name);

          for( unsigned i = 0; i != def_exp_stack_depth_; ++i ) {
            if( def_exp_stack_[ i ].name_ == name ) {
              index = -1; // already expanding this define
              break;
            }
          }

          if( index != -1 ) {
            char_type *expansion = define_expansion( index );
            define_expansion_type &exp = def_exp_stack_[ def_exp_stack_depth_ ];
            exp.src_ = lexer_.get_src();
            exp.name_ = name;
            lexer_.start( expansion );
            if( def_exp_stack_depth_ == def_exp_stack_max_ - 1 ) {
              cpp_log("error: define expansion stack overflow\n");
              break;
            }
            def_exp_stack_depth_++;
          } else {
            break;
          }
        }
        //context_.debug( "[%s]\n", token_name( lexer_.type() ) );
      }
  
      void next_raw() {
        lexer_.lex_token();
        //context_.debug( "[%s]\n", token_name( lexer_.type() ) );
      }
  
      void expect( token_type tok ) {
        if( lexer_.type() != tok ) {
          cpp_log("error: expected %s got %s\n", cpp_tokens::token_name( lexer_.type() ), cpp_tokens::token_name( tok ));
        } else {
          next();
        }
      }
    
      int64_t expression( unsigned precidence=0 ) {
        int64_t result = 0;
        //if( debug_expressions ) context_.debug_push( "expression %s %s\n", token_name( lexer_.type() ), lexer_.get_src() );
        if( unary_op_[ lexer_.type() ] ) {
          token_type op = lexer_.type();
          next();
          result = expression( 100 );
          if( op == tok_minus ) {
            result = -result;
          } else if( op == tok_not ) {
            result = !result;
          } else if( op == tok_tilda ) {
            result = ~result;
          }
        } else if( lexer_.type() == tok_identifier ) {
          result = 0;
          if( !strcmp( lexer_.id(), (char*)"defined" ) ) {
            next_raw();
            const char *name = 0;
            if( lexer_.type() == tok_lparen ) {
              next_raw();
              name = (const char*)lexer_.id();
              next_raw();
              expect( tok_rparen );
            } else {
              name = (const char*)lexer_.id();
              next_raw();
            }
            result = defines_.get_index( name ) != -1;
          } else {
            // unknown define, return 0
            next();
          }
        } else if( lexer_.type() == tok_int_constant || lexer_.type() == tok_int64_constant || lexer_.type() == tok_uint_constant || lexer_.type() == tok_uint64_constant ) {
          result = (int64_t)lexer_.value();
          next();
        } else if( lexer_.type() == tok_lparen ) {
          next();
          result = expression();
          expect( tok_rparen );
        } else if( lexer_.type() == tok_rparen ) {
          cpp_log("error: missing '(' in expression\n");
        } else {
          cpp_log("error: expected primary-expression before '%s' token\n", cpp_tokens::token_name( lexer_.type() ) );
        }
      
        unsigned thisPrecidence = precidence_[ lexer_.type() ];
        //if( debug_expressions ) context_.debug( "thisPrecidence=%d precidence=%d tok=%s %s\n", thisPrecidence, precidence, token_name( lexer_.type() ), lexer_.get_src() );
        while( thisPrecidence && precidence <= thisPrecidence ) {
          token_type op = lexer_.type();
          next();
          int64_t rhs = expression( thisPrecidence );
          switch( op ) {
            case tok_comma: result = rhs; break;
            case tok_or: result = result | rhs; break;
            case tok_and: result = result & rhs; break;
            case tok_xor: result = result ^ rhs; break;
            case tok_lt: result = result < rhs; break;
            case tok_gt: result = result > rhs; break;
            case tok_le: result = result <= rhs; break;
            case tok_ge: result = result >= rhs; break;
            case tok_eq: result = result == rhs; break;
            case tok_ne: result = result != rhs; break;
            case tok_shift_left: result = result << rhs; break;
            case tok_shift_right: result = result >> rhs; break;
            case tok_plus: result = result + rhs; break;
            case tok_minus: result = result - rhs; break;
            case tok_star: result = result * rhs; break;
            case tok_divide: result = result / rhs; break;
            case tok_mod: result = result % rhs; break;
            case tok_or_or: result = result || rhs; break;
            case tok_and_and: result = result && rhs; break;
            default: break;
          }
          thisPrecidence = precidence_[ lexer_.type() ];
          //if( debug_expressions ) context_.debug( "..thisPrecidence=%d precidence=%d tok=%s %s\n", thisPrecidence, precidence, token_name( lexer_.type() ), lexer_.get_src() );
        }
        //if( debug_expressions ) context_.debug_pop( "/expression (%d) %s %s\n", result, token_name( lexer_.type() ), lexer_.get_src() );
        return result;
      }
    
      bool find_param( name_type *params, unsigned num_define_params, unsigned *param ) {
        for( unsigned i = 0; i != num_define_params; ++i ) {
          if( params[ i ] == (const char*)lexer_.id() ) {
            *param = i;
            return true;
          }
        }
        return false;
      }

      void hash_define() {
        next_raw();
        if( lexer_.type() != tok_identifier ) {
          cpp_log("error: expected name after #define\n");
          return;
        }

        define_type &define = defines_[(const char*)lexer_.id()];

        name_type params[ max_define_params_ ];
        unsigned num_define_params = 0;
      
        const char_type *start = lexer_.get_src();
      
        bool has_params = false;
      
        if( *start == '(' ) {
          // #define VALUE( xxx ) 
          next_raw();
          next_raw();
          has_params = true;
          while( lexer_.type() == tok_identifier ) {
            if( num_define_params == max_define_params_ ) {
              cpp_log("error: too many #define parameters\n");
              return;
            }
            params[ num_define_params++ ] = (const char*)lexer_.id();
            next_raw();
            if( lexer_.type() != tok_comma ) {
              break;
            }
            next_raw();
          }

          if( lexer_.type() != tok_rparen ) {
            cpp_log("error: no ) in #define\n");
            return;
          }
          start = lexer_.get_src();
          next_raw();
        } else {
          next_raw();
        }

        char_type *dest = write_line_;

        while( is_whitespace( *start ) ) {
          start++;
        }

        bool hash = false;
        while( lexer_.type() != tok_newline ) {
          unsigned param;
          if( lexer_.type() == tok_identifier && find_param( params, num_define_params, &param ) ) {
            *dest++ = hash ? stringise_param_escape : param_escape;
            *dest++ = param + '0';
            hash = false;
          } else if( lexer_.type() == tok_hash ) {
            hash = true;
          } else if( lexer_.type() == tok_hash_hash ) {
            hash = false;
          } else {
            while( start != lexer_.get_src() ) {
              *dest++ = *start++;
            }
            hash = false;
          }

          start = lexer_.get_src();
          next_raw();
        }

        while( start != lexer_.get_src() ) {
          *dest++ = *start++;
        }

        *dest = 0;

        unsigned num_chars = (unsigned)( dest - write_line_ + 1 );
        define.text_.set((const char*)write_line_, num_chars);
        define.has_params_ = has_params;
        //context_.debug( "#define %s [%s] %d\n", name.text(), define.text_, define.has_params_ );
      
        //context_.debug( "index=%d\n", defines_.index( name ) );
      }
    
      void hash_directive( const char_type *src ) {
        //if( debug_ifs ) context_.debug( "[%d%d%d%d]# %s\n", if_.is_true_, if_.had_true_, if_.bottom_level_, if_.had_if_, src );

        while( is_whitespace( *src ) ) {
          ++src;
        }
      
        if( is_id_start( *src ) ) {
          //const char_type *src0 = src;
          long long name = *src++;
          while( is_id_middle( *src ) ) {
            name = name * 256 + *src++;
          }

          lexer_.start( src );

          //context_.debug( "else if( name == 0x%LxL ) // %s\n", name, src0 );
          if( name == 0x69666e646566ll || name == 0x6966646566ll ) { // ifdef/ifndef
            if( if_.is_true_ ) {
              next_raw();
            
              bool result = false;
              if( lexer_.type() != tok_identifier ) {
                expect( tok_identifier );
              } else {
                result = defines_.get_index((const char *)lexer_.id()) != -1;
                if( name == 0x69666e646566ll ) // #ifdef
                {
                  result = !result;
                }
                //context_.debug( "#ifdef %s %lld\n", def_name.text(), result );
                next();
              }
              push_if( result );
              if_.had_if_ = true;
            } else {
              push_if( false );
            }
          } else if( name == 0x6966ll ) // if
          {
            if( if_.is_true_ ) {
              next();
              int64_t result = expression();
              //context_.debug( "if %d\n", result != 0 );
              push_if( result != 0 );
              if_.had_if_ = true;
            } else {
              push_if( false );
            }
          }
          else if( name == 0x656e646966ll ) // endif
          {
            if( if_.bottom_level_ ) {
              cpp_log("error: too many endifs\n");
            } else {
              pop_if();
            }
          } else if( name == 0x656c6966ll ) // elif
          {
            if( if_.bottom_level_ ) {
              cpp_log("error: elif withot if\n");
            } else if( if_.had_if_ ) {
              if( !if_.had_true_ ) {
                next();
                int64_t result = expression();
                //context_.debug( "elif %d\n", result != 0 );
                if_.is_true_ = result != 0;
              } else {
                if_.is_true_ = false;
              }
              if_.had_true_ |= if_.is_true_;
            }
          }
          else if( name == 0x656c7365ll ) { // else
            if( if_.bottom_level_ ) {
              cpp_log("error: else without if\n");
            } else if( if_.had_if_ ) {
              if_.is_true_ = !if_.had_true_;
              if_.had_true_ = true;
            }
          } else if( name == 0x696e636c756465ll ) { // include
            if( if_.is_true_ ) {
              while( is_whitespace( *src ) ) {
                ++src;
              }
    
              if( *src != '"' && *src != '<' ) {
                cpp_log("error: sytnax error in #include");
                return;
              }
      
              char_type terminator = *src++ == '<' ? '>' : '"';
              char filename[ 256 ];
              char *dest = filename;
              while( *src && *src != terminator ) {
                *dest = *src++;
                dest += dest != filename + sizeof( filename ) - 1;
              }
              *dest = 0;
              //context_.debug( "%s\n", filename );
              if( *src != terminator ) {
                cpp_log("error: sytnax error in #include");
                return;
              }
              src++;
  
              while( is_whitespace( *src ) ) {
                ++src;
              }
    
              if( *src != 0 ) {
                cpp_log("error: sytnax error in #include");
              }
  
              map_file_result file;
              if (!map_file( file, filename, include().currentPath.c_str() )) {
                cpp_log("error: include file %s not found", filename);
                return;
              }

              output( 0x0c, line_number(), file_name(), 0 );

              push_include( file.source_, file.source_end_, file.filename_, 1 );

              output( 0x0d, 1, file.filename_, 0 );

              push_if( true );
              if_.bottom_level_ = true;
  
              while( is_whitespace( *src ) ) {
                ++src;
              }
    
              if( *src != 0 ) {
                cpp_log("error: syntax error in #include\n");
              }
            }
          } else if( name == 0x6572726f72ll ) { // error
            if( if_.is_true_ ) {
            }
          } else if( name == 0x707261676d61ll ) { // pragma
            if( if_.is_true_ ) {
            }
          }
          else if( name == 0x7761726e696e67ll ) // warning
          {
            if( if_.is_true_ ) {
            }
          }
          else if( name == 0x646566696e65ll ) // define
          {
            if( if_.is_true_ ) {
              hash_define();
            }
          }
          else if( name == 0x756e646566ll ) // undef
          {
            if( if_.is_true_ ) {
              next_raw();
            
              if( lexer_.type() != tok_identifier ) {
                expect( tok_identifier );
              } else {
                int index = defines_.get_index( (const char*)lexer_.id() );
                if( index != -1 ) {
                  defines_[(const char*)lexer_.id()].text_ = "";
                }
              }
            }
          } else {
            cpp_log("error: unknown # directive\n");
          }
        }
      }
    
      void read_some_text() {
        char_type *dest = read_line_;
        char_type *dest_max = read_line_ + max_chars_ - 1;
        const char *current = include().current_;
        const char *end = include().end_;
        unsigned line_num = include().line_number_;

        //context_.debug( "%s line %d %d\n", include().file_name_, line_num, end - current );
        while( current != end ) {
          char_type chr = *current++;
          if( chr == '\n' ) {
            // newline
            line_num++;
            break;
          } else if( chr == '/' ) {
            if( current != end && *current == '/' ) {
              // c++ comment
              current++;
              while( (current != end && current[ 0 ] != '\n') || current[ -1 ] == '\\' ) {
                line_num += current[ 0 ] == '\n';
                current++;
              }
              break;
            } else if( current != end && *current == '*' ) {
              // c comment
              current++;
              while( current != end && ( current[ 0 ] != '/' || current[ -1 ] != '*' ) ) {
                line_num += current[ 0 ] == '\n';
                current++;
              }
              if( current == end ) {
                cpp_log("error: unterminated comment\n");
                break;
              }

              current++;
              chr = ' ';
              *dest = chr;
              dest += dest != dest_max;
            } else {
              *dest = chr;
              dest += dest != dest_max;
            }
          } else if( chr == '\\' ) {
            // \ escape
            if( current == end ) {
              *dest = chr;
              dest += dest != dest_max;
            } else {
              if( *current == '\n' ) {
                current++;
                line_num++;
              } else {
                *dest = chr;
                dest += dest != dest_max;
              }
            }
          } else if( chr != '\r' ) {
            *dest = chr;
            dest += dest != dest_max;
          }
        }

        if( dest == dest_max ) {
          cpp_log("error: line too long\n");
          cur_line_ = 0;
          return;
        }
        *dest++ = 0;

        include().current_ = current;
        include().line_number_ = line_num;
      
        if( current == end ) {
          //context_.debug( "%s END! line %d %d\n", include().file_name_, line_num, end - current );
        }
        //context_.debug( "[%s]\n", read_line_ );
      }
    
      void read_line() {
        //context_.debug( "read_line() qi=%d qo=%d\n", queue_in_, queue_out_ );
        while( queue_in_ == queue_out_ ) {
          if( include().current_ == include().end_ ) {
            //context_.debug( "read_line() qi=%d qo=%d END!\n", queue_in_, queue_out_ );
            if( include_stack_.size() == 0 ) {
              cur_line_ = 0;
              return;
            } else {
              if( !if_.bottom_level_ ) {
                cpp_log("error: if without endif\n");
                while( !if_.bottom_level_ ) {
                  pop_if();
                }
              }
  
              pop_if();

              include_stack_.pop_back();
              /*if( include_stack_ptr_ != include_stack_ ) {
                output( 0x0e, line_number(), file_name(), 0 );
              }*/
              continue;
            }
          }

          read_line_number_ = include().line_number_;
          read_file_name_ = include().file_name_;
          read_some_text();
 
          char_type *src = read_line_;
          def_exp_stack_depth_ = 0;
        
          while( is_whitespace( *src ) ) {
            ++src;
          }
        
          if( *src == '#' ) {
            hash_directive( src + 1 );
            num_skipped_++;
          } else if( *src == 0 || !if_.is_true_ ) {
            num_skipped_++;
          } else if( if_.is_true_ ) {
            if( num_skipped_ >= 8 ) {
              // avoid too many blank lines
              output( 0x0e, line_number(), file_name(), 0 );
            } 
            expand_defines();
            num_skipped_ = 0;
          }
        }
      }

    public:
      cpp_preprocessor() {
        for( int i = 0; i != tok_last; ++i ) {
          precidence_[ i ] = 0;
        }

        precidence_[ tok_comma ] = 1;
        precidence_[ tok_equals ] = 2;
        precidence_[ tok_times_equals ] = 2;
        precidence_[ tok_divide_equals ] = 2;
        precidence_[ tok_mod_equals ] = 2;
        precidence_[ tok_plus_equals ] = 2;
        precidence_[ tok_minus_equals ] = 2;
        precidence_[ tok_shift_left_equals ] = 2;
        precidence_[ tok_shift_right_equals ] = 2;
        precidence_[ tok_and_equals ] = 2;
        precidence_[ tok_xor_equals ] = 2;
        precidence_[ tok_or_equals ] = 2;
        precidence_[ tok_question ] = 3;
        precidence_[ tok_or_or ] = 4;
        precidence_[ tok_and_and ] = 5;
        precidence_[ tok_or ] = 6;
        precidence_[ tok_and ] = 7;
        precidence_[ tok_xor ] = 8;
        precidence_[ tok_lt ] = 9;
        precidence_[ tok_gt ] = 9;
        precidence_[ tok_le ] = 9;
        precidence_[ tok_ge ] = 9;
        precidence_[ tok_eq ] = 9;
        precidence_[ tok_ne ] = 9;
        precidence_[ tok_shift_left ] = 10;
        precidence_[ tok_shift_right ] = 10;
        precidence_[ tok_plus ] = 11;
        precidence_[ tok_minus ] = 11;
        precidence_[ tok_star ] = 12;
        precidence_[ tok_divide ] = 12;
        precidence_[ tok_mod ] = 12;
        precidence_[ tok_dot_star ] = 13;
        precidence_[ tok_arrow_star ] = 13;
      
        static short const unary_ops[] = { tok_minus, tok_plus, tok_and, tok_star, tok_not, tok_tilda, tok_plus_plus, tok_minus_minus, -1 };
        unary_op_ = unary_ops;
      }

      void begin( const char *source ) {
        const char *end = source;
        while( *end ) {
          end++;
        }

        include_stack_.reset();
        include_stack_.reserve(32);
        if_stack_.reset();
        if_stack_.reserve(32);

        cur_line_ = 0;
        queue_in_ = queue_out_ = 0;
        undefined_[ 0 ] = 0;
      
        last_file_name_ = 0;
        last_line_number_ = 1;
        num_skipped_ = 0;
      
        // output( 0x00, 1, "test.c", 0 );
        // output( 0x00, 1, "<built-in>", 0 );
        // output( 0x00, 1, "<command line>", 0 );
        // output( 0x00, 1, "test.c", 0 );

        push_include( source, end, "?", 1 );

        if_.bottom_level_ = true;
        if_.is_true_ = true;

        next_line();
      }
    
      const char_type *cur_line() {
        return cur_line_;
      }

      const char_type *next_line() {
        //context_.debug( "%d %d\n", queue_in_, queue_out_ );
        if( queue_in_ == queue_out_ ) {
          read_line();
          //context_.debug( "..%d %d\n", queue_in_, queue_out_ );
          if( queue_in_ == queue_out_ ) {
            cur_line_ = 0;
            return cur_line_;
          }
        }

        // read text queue and output one line at a time
        queue_entry_type *entry = queue_ + queue_out_;

        //context_.debug( "i %s\n", entry->text_ );
        if( !entry->text_ ) {
          // this queue entry is a line number only.
          format_number( number_line_, entry->line_number_, entry->file_name_, entry->flags_ );
          cur_line_ = number_line_;
          last_line_number_ = entry->line_number_;
          last_file_name_ = entry->file_name_;
           queue_out_ = ( queue_out_ + 1 ) & ( max_output_queue_ - 1);
        } else if( last_line_number_ + 10 < entry->line_number_ || last_line_number_ > entry->line_number_ ) {
          // this queue entry contains text, but output the line number record first.
          format_number( number_line_, entry->line_number_, entry->file_name_, entry->flags_ );
          cur_line_ = number_line_;
          last_line_number_ = entry->line_number_;
          last_file_name_ = entry->file_name_;
        } else if( entry->line_number_ != last_line_number_ ) {
          // this queue entry contains text, but output a blank line to make up the line number
          number_line_[ 0 ] = 0;
          cur_line_ = number_line_;
          last_line_number_++;
        } else {
          // this queue entry contains text and the line number is in sync. So output the text.
          cur_line_ = entry->text_;
          last_line_number_++;
           queue_out_ = ( queue_out_ + 1 ) & ( max_output_queue_ - 1);
        }
        return cur_line_;
      }
    };
  
  }
}

