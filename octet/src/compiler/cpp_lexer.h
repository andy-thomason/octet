////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// C/C++ language subset lexer. A work in progress, needs some tests.
//

namespace octet
{
  namespace compiler
  {
    inline const char *cpp_log(const char *fmt, ...) {
      static dynarray<char> log_text;
      va_list list;
      va_start(list, fmt);
      unsigned size = log_text.size();
      log_text.resize(size + 256);
      int bytes = vsnprintf(&log_text[size], 256-1, fmt, list);
      log_text.resize(size + bytes + 1);
      va_end(list);
      return &log_text[0];
    }

    class cpp_lexer : public cpp_tokens
    {
      typedef char char_type;

      enum {
        max_chars = 256,
      };
    
      // small stuff    
      uint64_t value_;
      uint64_t length_;
      double double_value_;
      token_type type_;
      const char_type *src_;

      dictionary<int> identifiers_;

      char_type id_[max_chars];

    public:
      cpp_lexer() {
      }

      void add_identifier(const char *id, int tok) {
        identifiers_[id] = tok;
      }

      static const char *token_name( int tok ) {
        if (tok < tok_last) {
          return cpp_tokens::token_name( (token_type)tok );
        } else {
          return "??";
        }
      }

      void lex_identifier() {
        id_[ 0 ] = *src_++;
        unsigned num_chars = 1;
        while( is_id_middle( *src_ ) ) {
          id_[ num_chars++ ] = *src_++;
        }
        id_[ num_chars ] = 0;
        type_ = (token_type)tok_identifier;
        int index = identifiers_.get_index(id_);
        if (index != -1) {
          type_ = (token_type)identifiers_.get_value(index);
        }
      }

      // 0x123 0123 12345678l 0.123e21L
      void lex_number() {
        const char_type *src = src_;
        uint64_t value = 0;
        const char_type *src0 = src;
        if( *src == '0' ) {
          src++;
          if( src[ 0 ] == 'x' || src[ 0 ] == 'X' ) {
            src++;
            while( is_hex_digit( *src ) ) {
              value = value * 16 + ( ( *src - ( *src < 'A' ? '0' : 'A'-10 ) ) & 15 );
              src++;
            }
            value_ = value;
            goto after_int;
          } else {
            while( is_octal_digit( *src ) ) {
              value = value * 8 + *src - '0';
              src++;
            }
            if( *src != '.' ) {
              goto after_int;
            }
          }
        }
      
        while( is_digit( *src ) ) {
          src++;
        }
        if( *src == '.' ) {
          src++;
          while( is_digit( *src ) ) {
            src++;
          }
          if( *src == 'e' || *src == 'E' ) goto exponent; else goto after_float;
        } else if( *src == 'e' || *src == 'E' ) {
        exponent:
          src++;
          if( *src == '+' || *src == '-' ) {
            src++;
          }
          while( is_digit( *src ) ) {
            src++;
          }
        after_float:
          if( *src == 'f' || *src == 'F' ) {
            src++;
            type_ = tok_float_constant;
          } else if( *src == 'l' || *src == 'L' ) {
            src++;
            type_ = tok_long_double_constant;
          } else {
            type_ = tok_double_constant;
          }
          double_value_ = atof( (char*)src0 );
        } else {
          while( src0 != src ) {
            value = value * 10 + *src0++ - '0';
          }
          value_ = value;
        after_int:
          type_ = tok_int_constant;
          if( *src == 'l' || *src == 'L' ) {
            src++;
            if( *src == 'l' || *src == 'L' ) {
              src++;
              type_ = tok_int64_constant;
            }
          }
          if( *src == 'u' || *src == 'U' ) {
            type_ = type_ == tok_int_constant ? tok_uint_constant : tok_uint64_constant;
          }
        }
        //printf( "number: %s %Lx\n", token_name( type_), value_ );
        src_ = src;
      }
    
      // \t
      void lex_string_escape( unsigned *chr ) {
        src_++;
        if( *src_ ) {
          switch( *src_ ) {
            case 'a': *chr = 7; break;
            case 'b': *chr = 8; break;
            case 't': *chr = 9; break;
            case 'n': *chr = 10; break;
            default: *chr = *src_; break;
          }
          src_++;
        }
      }

      // "abc"
      void lex_string() {
        char_type *dest = id_;
        ++src_;
        while( *src_ != '"' ) {
          if( *src_ == 0 ) {
            break;
          }
          if( *src_ == '\\' ) {
            unsigned chr = 0;
            lex_string_escape( &chr );
            *dest++ = chr;
          } else {
            *dest++ = *src_++;
          }
        }
        *dest = 0;
        ++src_;
        value_ = (uint64_t)id_;
        type_ = tok_string_constant;
        length_ = dest - id_;
      }

      // 'A'
      void lex_char_constant() {
        uint64_t value = 0;
        ++src_;
        while( *src_ != '"' ) {
          if( *src_ == 0 ) {
            break;
          }
          if( *src_ == '\\' ) {
            unsigned chr = 0;
            lex_string_escape( &chr );
            value = value * 256 + chr;
          } else {
            value = value * 256 + *src_++;;
          }
        }
        ++src_;
        value_ = value;
        type_ = tok_int_constant;
      }

      bool is_punct(char c) {
        if (src_[0] == c) {
          src_++;
          return true;
        } else {
          return false;
        }
      }

      // !=  etc.
      void lex_punctuation() {
        switch (*src_++) {
          case '!': type_ = is_punct('=') ? tok_ne : tok_not; break;
          case '#': type_ = is_punct('#') ? tok_hash_hash : tok_hash; break;
          case '%': type_ = is_punct('=') ? tok_mod_equals : tok_mod; break;
          case '&': type_ = is_punct('=') ? tok_and_equals : is_punct('&') ? tok_and_and : tok_and; break;
          case '(': type_ = tok_lparen; break;
          case ')': type_ = tok_rparen; break;
          case '*': type_ = is_punct('=') ? tok_times_equals : tok_star; break;
          case '+': type_ = is_punct('=') ? tok_plus_equals : is_punct('+') ? tok_plus_plus : tok_plus; break;
          case ',': type_ = tok_comma; break;
          case '-': type_ = is_punct('=') ? tok_minus_equals : is_punct('-') ? tok_minus_minus : is_punct('>') ? ( is_punct('*') ? tok_arrow_star : tok_arrow ) : tok_minus; break;
          case '.':
            if ( is_digit(src_[0]) ) {
              --src_;
              lex_number();
              return;
            }
            type_ =
            
              is_punct('*') ? tok_dot_star :
              ( src_[0] == '.' && src_[1] == '.' ) ? ( src_ += 2, tok_ellipsis ) :
              tok_star
            ;
            break;
          case '/': type_ = is_punct('=') ? tok_divide_equals : tok_divide; break;
          case ':': type_ = is_punct(':') ? tok_colon_colon : tok_colon; break;
          case ';': type_ = tok_semicolon; break;
          case '<': type_ = is_punct('=') ? tok_le : is_punct('<') ? ( is_punct('=') ? tok_shift_left_equals : tok_shift_left ) : tok_lt; break;
          case '=': type_ = is_punct('=') ? tok_eq : tok_equals; break;
          case '>': type_ = is_punct('=') ? tok_ge : is_punct('>') ? ( is_punct('=') ? tok_shift_right_equals : tok_shift_right ) : tok_gt; break;
          case '?': type_ = tok_question; break;
          case '[': type_ = tok_lbracket; break;
          case ']': type_ = tok_rbracket; break;
          case '^': type_ = is_punct('=') ? tok_xor_equals : tok_xor; break;
          case '{': type_ = tok_lbrace; break;
          case '|': type_ = is_punct('=') ? tok_or_equals : is_punct('|') ? tok_or_or : tok_or; break;
          case '}': type_ = tok_rbrace; break;
          case '~': type_ = tok_tilda; break;
          default: assert(0);
        }
      }

      // [.] [.0]  [.*] [...]
      void lex_dot() {
        char_type chr = src_[ 1 ];
        type_ = tok_bad_character;
        if( is_digit( chr ) ) {
          lex_number();
        } else {
          lex_punctuation();
        }
      }

      // identifiers, numbers, strings etc.
      void lex_complex_token() {
        char_type chr = *src_;
        if( punctuation_start_[ chr ] ) {
          // larger, called punctuation lexer
          lex_punctuation();
        } else if( is_digit( chr ) ) {
          lex_number();
        } else if( chr == '"' ) {
          lex_string();
        } else if( chr == '\'' ) {
          lex_char_constant();
        } else if( chr == 0 ) {
          type_ = tok_newline;
        } else {
          type_ = tok_bad_character;
          src_++;
        }
      }

      // small, inlinable lexer
      // ! ( ) [ ] { } etc.
      void lex_token() {
        while( is_whitespace( *src_ ) ) {
          src_++;
        }

        if( is_id_start( *src_ ) ) {
          // identifiers
          lex_identifier();
        } else {
          // all other tokens
          lex_complex_token();
        }
      }
    
      void start( const char_type *src ) {
        src_ = src;
      }
    
      const char_type *get_src() {
        return src_;
      }
    
      char_type *id() {
        return id_;
      }
    
      uint64_t value() {
        return value_;
      }

      double double_value() {
        return double_value_;
      }

      uint64_t length() {
        return length_;
      }

      token_type type() {
        return type_;
      }

    };
  }
}
