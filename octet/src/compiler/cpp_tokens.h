////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// C/C++ language tokens
//

namespace octet
{
  namespace compiler
  {
    struct cpp_token_enum {
      enum token_type
      {
        tok_bad_character,
  
        tok_identifier, tok_end_of_source, tok_newline,

        tok_float_constant, tok_double_constant, tok_long_double_constant, tok_int_constant, tok_int64_constant, tok_uint_constant, tok_uint64_constant, tok_string_constant, tok_wstring_constant, 
  

        tok_hash, tok_hash_hash, tok_lbracket, tok_rbracket, tok_lparen, tok_rparen, tok_dot, tok_arrow, tok_plus_plus, tok_minus_minus, 
  
        tok_and, tok_star, tok_plus, tok_minus, tok_tilda, tok_not, tok_divide, tok_mod, tok_shift_left, tok_shift_right,

        tok_lt, tok_gt, tok_le, tok_ge, tok_eq, tok_ne,

        tok_xor, tok_or, tok_and_and, tok_or_or, tok_question, tok_colon, tok_equals, tok_times_equals, tok_divide_equals, tok_mod_equals, tok_plus_equals, tok_minus_equals, tok_shift_left_equals, tok_shift_right_equals, tok_and_equals, tok_xor_equals, tok_or_equals,
      
        tok_comma, tok_lbrace, tok_rbrace, tok_semicolon, tok_ellipsis, tok_colon_colon, tok_dot_star, tok_arrow_star,

        //tok_auto, tok_break, tok_case, tok_char, tok_const, tok_continue, tok_default, tok_do, tok_double, tok_else, tok_enum, tok_extern, tok_float, tok_for, tok_goto, tok_if, tok_int, tok_long, tok_register, tok_return, tok_short, tok_signed, tok_sizeof, tok_static, tok_struct, tok_switch, tok_typedef, tok_union, tok_unsigned, tok_void, tok_volatile, tok_while,  tok_try, tok_finally, tok_except, tok_catch, tok_class, tok_delete, tok_friend, tok_inline, tok_new, tok_operator, tok_private, tok_protected, tok_public, tok_template, tok_this, tok_throw, tok_virtual, tok_const_cast, tok_dynamic_cast, tok_explicit, tok_export, tok_mutable, tok_namespace, tok_reinterpret_cast, tok_static_cast, tok_typeid, tok_using, tok_bool, tok_false, tok_true, tok_typename, tok_typeof,
  
        tok_last,
      };
    };

    class cpp_tokens : public cpp_token_enum
    {
    public:

      static const char *token_name(token_type t) {
        static const char token_names[tok_last][4] = {
          "bad",
  
          "id", "end", "nl",

          "f", "d", "ld", "i32", "i64", "u32", "u64", "str", "wst", 

          "#", "##", "[", "]", "(", ")", ".", "->", "++", "--", 
  
          "&", "*", "+", "-", "~", "!", "/", "%", "<<", ">>",

          "<", ">", "<=", ">=", "==", "!=",

          "^", "|", "&&", "||", "?", ":", "=", "*=", "/=", "%=", "+=", "-=", "<<=", ">>=", "&=", "^=", "|=",
          ",", "{", "}", ";", "...", "::", ".*", "->*",
        };
        assert((unsigned)t < tok_last);
        return token_names[(int)t];
      }

      bitset< 256 > whitespace_;
      bitset< 256 > identifier_start_;
      bitset< 256 > punctuation_start_;
      bitset< 256 > identifier_tail_;
      bitset< 256 > hex_digit_;

      token_type first_punctuation() {
        return tok_hash;
      }

      token_type last_punctuation() {
        return tok_arrow_star;
      }

      bool is_whitespace( unsigned chr ) {
        return chr < 256 && whitespace_[ chr ];
      }
    
      bool is_digit( unsigned chr ) {
        return ( (unsigned)chr - '0' ) <= '9'-'0';
      }
    
      bool is_id_start( unsigned chr ) {
        return chr < 256 && identifier_start_[ chr ];
      }
    
      bool is_id_middle( unsigned chr ) {
        return chr < 256 && identifier_tail_[ chr ];
      }
    
      bool is_hex_digit( unsigned chr ) {
        return chr < 256 && hex_digit_[ chr ];
      }
    
      bool is_octal_digit( unsigned chr ) {
        return ( (unsigned)chr - '0' ) <= '7'-'0';
      }

      cpp_tokens() {
        punctuation_start_.clear();
        identifier_start_ = "a-zA-Z_";
        identifier_tail_ = "a-zA-Z0-9_";
        hex_digit_ = "0-9A-Fa-f";
        whitespace_ = " \t";

        for( int i = first_punctuation(); i <= last_punctuation(); ++i ) {
          const char *punctuation = token_name( (token_type)i );
          punctuation_start_.setbit(punctuation[0]);
        }
      }
    };
  }
}

