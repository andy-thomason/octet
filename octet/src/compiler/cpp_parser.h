////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// C/C++ language subset parser
//
// This is a work in progress to be shaped later. We may put a LLVM back-end on it
// as in the original, but also create a more optimal crash lowering back end later.

namespace octet
{
  namespace compiler
  {
    #if 0
    // a very large can of worms that converts one type into another.
    Value *translateCast( Value *llvmSrc, cpp_type *srcType, cpp_type *destType, BasicBlock *bb, fc::default_context &context ) {
      Value *result = NULL;
      const Type *llvmSrcType = srcType->translate( context );
      const Type *llvmDestType = destType->translate( context );
      assert( llvmSrcType == llvmSrc->getType() );

      if( *srcType == *destType ) {
        // no work to do.
        result = llvmSrc;
      } else if( srcType->getIsScalar() && destType->getIsVector() ) {
        const Type *scalarType = destType->getSubType()->translate( context );
        dynarray< Constant * > indices;
      
        // cast to destination's scalar type first. eg. int -> float -> float4
        if( scalarType != llvmSrcType ) {
          llvmSrc = translateCast( llvmSrc, srcType, destType->getSubType(), bb, context );
        }

        if( isa<Constant>( llvmSrc ) ) {
          for( unsigned i = 0; i != 4; ++i ) {
            indices.push_back( cast<Constant>( llvmSrc ) );
          }
          result = ConstantVector::get( indices );
        } else {
          Value *insert = InsertElementInst::Create( Constant::getNullValue( llvmDestType ), llvmSrc, Constant::getNullValue( Type::Int32Ty ), "", bb );
          result = new ShuffleVectorInst( insert, insert, Constant::getNullValue( VectorType::get( Type::Int32Ty, 4 ) ), "", bb );
        }
      } else if( srcType->getIsVector() && destType->getIsScalar() ) {
        if( srcType->getDimension() != 1 ) {
          cpp_log(context.errorText() << "cast from vector to scalar\n";
          result = NULL;
        } else {
          result = new ExtractElementInst( llvmSrc, Constant::getNullValue( Type::Int32Ty ), "", bb );
        }
      } else if( srcType->getIsScalar() && destType->getIsScalar() ) {
        // scalar to scalar conversion
        Instruction::CastOps op = CastInst::getCastOpcode( llvmSrc, true, llvmDestType, true );
        result = CastInst::create( op, llvmSrc, llvmDestType, "", bb );
      } else if( srcType->getIsVector() && destType->getIsVector() ) {
        // eg. vec3 -> vec4  0123
        //     vec4 -> vec3  0122
        //     vec1 -> vec4  0000
        //     vec2 -> vec4  0111
        //     vec4 -> vec2  0111
        dynarray< Constant * > indices;
        int srcDim = srcType->getDimension();
        int destDim = destType->getDimension();
        if( srcDim >= destDim ) {
          result = llvmSrc;
        } else {
          // smear last element
          for( int i = 0; i != 4; ++i ) {
            int index = i < srcDim ? i : srcDim;
            indices.push_back( ConstantInt::get( Type::Int32Ty, index ) );
          }
          result = new ShuffleVectorInst( llvmSrc, llvmSrc, ConstantVector::get( indices ), "", bb );
        }
      } else {
        cpp_log("unable to cast from " << *srcType << " to " << *destType << "\n";
        result = NULL;
      }
      return result;
    }

    inline void getScopeTypes( dynarray< const Type * > &params, cpp_scope *scope, bool isParameterList, fc::default_context &context ) {
      for( cpp_scope::iterator i = scope->begin(); i != scope->end(); ++i ) {
        cpp_type *type = i[0]->getType();
        const Type *llvmParam = type->translate( context );
        if( isParameterList && type->getIsPassByPtr() ) {
          params.push_back( PointerType::get( llvmParam, 0 ) );
        } else {
          params.push_back( llvmParam );
        }
      }
    }
    #endif

    // type equivalence
    bool operator == ( cpp_type &lhs, cpp_type &rhs ) {
      if( &lhs == &rhs ) {
        return true;
      }
      if( lhs.getKind() != rhs.getKind() ) {
        return false;
      }

      cpp_scope *lscope = lhs.getScope();
      cpp_scope *rscope = rhs.getScope();
      cpp_type *lsub = lhs.getSubType();
      cpp_type *rsub = rhs.getSubType();
      if( lhs.getKind() == cpp_type::kind_array ) {
        if( lhs.getDimension() != rhs.getDimension() ) {
          return false;
        }
        if( lsub->getIsPacked() != rsub->getIsPacked() ) {
          return false;
        }
      } else if( lscope != NULL || rscope != NULL ) {
        if( lscope == NULL || rscope == NULL || lscope->size() != rscope->size() ) {
          return false;
        }
        if( lhs.getKind() == cpp_type::kind_struct ) {
          if( lscope != rscope ) {
            return false;
          }
          if( lhs.getIsPacked() != rhs.getIsPacked() ) {
            return false;
          }
        } else {
          // check all the types in the scope. ie. function params
          cpp_scope::iterator l = lscope->begin();
          cpp_scope::iterator r = rscope->begin();
          while( l != lscope->end() ) {
            if( !( (*l)->getType() == (*r)->getType() ) ) {
              return false;
            }
            ++l;
            ++r;
          }
        }
      }

      if( lsub != NULL || rsub != NULL ) {
        if( lsub == NULL || rsub == NULL ) {
          return false;
        }
        if( !( *lsub == *rsub ) ) {
          return false;
        }
      }
    
      return true;
    }
  
    // can cast rhs to lhs?
    bool canCastTo( cpp_type *lhs, cpp_type *rhs ) {
      if( *lhs == *rhs ) {
        return true;
      }
    
      return lhs->getIsScalar() && rhs->getIsScalar();
    }

    #if 0
    // debug  
    OStream &operator <<( OStream &in, const cpp_type &type ) {
      if( type.getIsConst() ) {
        in << "const ";
      }
      if( type.getIsPacked() ) {
        in << "packed ";
      }
      if( type.getIsUniform() ) {
        in << "uniform ";
      }
      if( type.getIsIn() ) {
        if( type.getIsOut() ) {
          in << "inout ";
        } else {
          in << "in ";
        }
      }
      if( type.getIsOut() ) {
        in << "out ";
      }

      switch( (int)type.getKind() ) {
        case cpp_type::kind_struct:
          in << "struct { } ";
          break;
        case cpp_type::kind_array:
          in << *type.getSubType() << "[ " << type.getDimension() << " ] ";
          break;
        case cpp_type::kind_function:
        {
          in << *type.getSubType();
          cpp_scope *scope = type.getScope();
          char *comma = "( ";
          for( cpp_scope::iterator i = scope->begin(); i != scope->end(); ++i ) {
            in << comma << **i;
            comma = ", ";
          }
          in << " )";
        } break;
        case cpp_type::kind_float: in << "float "; break;
        case cpp_type::kind_half: in << "half "; break;
        case cpp_type::kind_bool: in << "bool "; break;
        case cpp_type::kind_void: in << "void "; break;
        case cpp_type::kind_int: in << "int "; break;
        case cpp_type::kind_cfloat: in << "cfloat "; break;
        case cpp_type::kind_cint: in << "cint "; break;
        case cpp_type::kind_sampler: in << "sampler "; break;
        case cpp_type::kind_sampler1D: in << "sampler1D "; break;
        case cpp_type::kind_sampler2D: in << "sampler2D "; break;
        case cpp_type::kind_sampler3D: in << "sampler3D "; break;
        case cpp_type::kind_samplerRECT: in << "samplerRECT "; break;
        case cpp_type::kind_samplerCUBE: in << "samplerCUBE "; break;
        default: in << "type(" << type.getKind() << ")"; break;
      }
      return in;
    }
  
    OStream &operator <<( OStream &in, const cpp_value &value ) {
      return in << *value.getType() << value.getName();
    }

    inline OStream &operator <<( OStream &in, cpp_scope &scope ) {
      in << "{";
      const char *semi = " ";
      for( cpp_scope::orderType::iterator i = scope.begin(); i != scope.end(); ++i ) {
        in << semi << **i;
        semi = "; ";
      }
      in << "}";
      return in;
    }
    #endif

    unsigned doParametersMatch( dynarray< cpp_type * > &actualTypes, cpp_value *search ) {
      cpp_type *searchType = search->getType();
      assert( searchType->getKind() == cpp_type::kind_function );
      //cpp_log(*searchType << "\n";
      if( searchType->getScope()->size() != actualTypes.size() ) {
        return 0;
      }

      cpp_scope *scope = searchType->getScope();
      //cpp_log("num params ok\n";
      dynarray< cpp_type * >::iterator actualType = actualTypes.begin();
    
      size_t numExact = 0;
      size_t numPartial = 0;
      for( cpp_scope::iterator formal = scope->begin(); formal != scope->end(); ++formal, ++actualType ) {
        cpp_type *formalType = (*formal)->getType();
        //cpp_log(*formalType << " vs " << **actualType << "\n";
        if( *formalType == **actualType ) { 
          numExact++;
        } else if( canCastTo( formalType, *actualType ) ) {
          numPartial++;
        }
      }
      //cpp_log(numExact << " exact    " << numPartial << "partial\n";
      return numExact == actualTypes.size() ? 2 : numPartial != 0 ? 1 : 0;
    }

    class cpp_parser : cpp_token_enum {
      cpp_preprocessor preprocessor;
      cpp_lexer lexer;
      bool dontReadLine;
      cpp_tokens::token_type curToken;
      int line_number;
      cpp_scope *invariantScope;
      cpp_scope *globalScope;
      cpp_scope *curScope;
      unsigned structNumber;
      unsigned numTmpVars;
      unsigned numAbstract;

      cpp_type *voidType;
      cpp_type *boolType;
      cpp_type *intType;
      cpp_type *floatType;
      cpp_type *doubleType;
      cpp_type *halfType;
      cpp_type *cintType;
      cpp_type *cfloatType;
      cpp_type *boolTypes1D[ 4 ];
      cpp_type *boolTypes2D[ 4 ][ 4 ];
      cpp_type *intTypes1D[ 4 ];
      cpp_type *intTypes2D[ 4 ][ 4 ];
      cpp_type *floatTypes1D[ 4 ];
      cpp_type *floatTypes2D[ 4 ][ 4 ];
      cpp_type *halfTypes1D[ 4 ];
      cpp_type *halfTypes2D[ 4 ][ 4 ];

      enum { debug = 1, trace_parse = 1 };

      enum toks  { tok_asm = cpp_tokens::tok_last, tok_asm_fragment, tok_auto,tok_bool,tok_break,tok_case,tok_catch,tok_char,tok_class,tok_column,tok_major,tok_compile,tok_const,tok_const_cast,tok_continue,tok_decl,tok_default,tok_delete,tok_discard,tok_do,tok_double,tok_dword,tok_dynamic_cast,tok_else,tok_emit,tok_enum,tok_explicit,tok_extern,tok_false,tok_fixed,tok_float,tok_for,tok_friend,tok_get,tok_goto,tok_half,tok_if,tok_in,tok_inline,tok_inout,tok_int,tok_interface,tok_long,tok_matrix,tok_mutable,tok_namespace,tok_new,tok_operator,tok_out,tok_packed,tok_pass,tok_pixelfragment,tok_pixelshader,tok_private,tok_protected,tok_public,tok_register,tok_reinterpret_cast,tok_return,tok_row,tok_sampler,tok_sampler_state,tok_sampler1D,tok_sampler2D,tok_sampler3D,tok_samplerCUBE,tok_samplerRECT,tok_shared,tok_short,tok_signed,tok_sizeof,tok_static,tok_static_cast,tok_string,tok_struct,tok_switch,tok_technique,tok_template,tok_texture,tok_texture1D,tok_texture2D,tok_texture3D,tok_textureCUBE,tok_textureRECT,tok_this,tok_throw,tok_true,tok_try,tok_typedef,tok_typeid,tok_typename,tok_uniform,tok_union,tok_unsigned,tok_using,tok_vector,tok_vertexfragment,tok_vertexshader,tok_virtual,tok_void,tok_volatile,tok_while, tok_lastlast };

      unsigned char tokenToPrecidence[ tok_lastlast ];
      unsigned char tokenIsRightGrouping[ tok_lastlast ];
    
      const char *getTokenName( unsigned tok ) {
        const char *tok_names[] = { "asm",    "asm_fragment",  "auto",  "bool",  "break",  "case",  "catch",  "char",  "class",  "column",  "major",  "compile",  "const",  "const_cast",  "continue",  "decl",  "default",  "delete",  "discard",  "do",  "double",  "dword",  "dynamic_cast",  "else",  "emit",  "enum",  "explicit",  "extern",  "false",  "fixed",  "float",  "for",  "friend",  "get",  "goto",  "half",  "if",  "in",  "inline",  "inout",  "int",  "interface",  "long",  "matrix",  "mutable",  "namespace",  "new",  "operator",  "out",  "packed",  "pass",  "pixelfragment",  "pixelshader",  "private",  "protected",  "public",  "register",  "reinterpret_cast",  "return",  "row",  "sampler",  "sampler_state",  "sampler1D",  "sampler2D",  "sampler3D",  "samplerCUBE",  "samplerRECT",  "shared",  "short",  "signed",  "sizeof",  "static",  "static_cast",  "string",  "struct",  "switch",  "technique",  "template",  "texture",  "texture1D",  "texture2D",  "texture3D",  "textureCUBE",  "textureRECT",  "this",  "throw",  "true",  "try",  "typedef",  "typeid",  "typename",  "uniform",  "union",  "unsigned",  "using",  "vector",  "vertexfragment",  "vertexshader",  "virtual",  "void",  "volatile",  "while", 0 };
        return tok < cpp_tokens::tok_last ? lexer.token_name( tok ) : tok_names[ tok - cpp_tokens::tok_last ];
      }

      void getNext() {
        lexer.lex_token();
        for(;;) {
          if( lexer.type() == cpp_tokens::tok_newline ) {
            if( dontReadLine || preprocessor.next_line() == NULL ) {
              curToken = cpp_tokens::tok_end_of_source;
              return;
            }
            lexer.start( preprocessor.cur_line() );
            lexer.lex_token();
            if (debug) {
              printf("[%s]\n", (char*)preprocessor.cur_line());
            }
          } else if( lexer.type() == cpp_tokens::tok_hash ) {
            lexer.lex_token();
            if( lexer.type() == cpp_tokens::tok_int_constant ) {
              line_number = (int)lexer.value() - 1;
            }
            while( lexer.type() != cpp_tokens::tok_newline ) {
              lexer.lex_token();
            }
          } else if( lexer.type() == cpp_tokens::tok_identifier ) {
            curToken = cpp_tokens::tok_identifier;
            if( debug ) {
              printf("# tok %s\n", lexer.id());
            }
            return;
          } else {
            curToken = lexer.type();
            if( debug ) {
              printf("# tok %s\n", getTokenName( curToken ));
            }
            return;
          }
        }
      }
      struct intrinsic_t
      {
        char name[ 16 ];
        const char *params;
      };
    
      dictionary< cpp_value * > values;
      dictionary< cpp_type * > typedefs;
      dictionary< cpp_type * > tags;

      cpp_type *findTypedef( const char *name ) {
        int index = typedefs.get_index(name);
        return index == -1 ? NULL : typedefs.get_value(index);
      }
    
      cpp_type *makeTypedef( cpp_type *type, const char *name ) {
        typedefs[name] = type;
        return type;
      }
    
      cpp_type *findTag( const char *name ) {
        int index = tags.get_index(name);
        return index == -1 ? NULL : tags.get_value(index);
      }
    
      cpp_type *makeTag( cpp_type *type, const char *name ) {
        tags[name] = type;
        return type;
      }
      unsigned lineNumber() {
        return 0;
      }
  
      const char *fileName() {
        return "file";
      }
    
      bool expect( unsigned type ) {
        if( curToken != type ) {
          cpp_log("expecting %s, got %s\n", getTokenName( type ), getTokenName( curToken ));
          return false;
        }
        return true;
      }
    
      cpp_expr *makeIntType( cpp_expr *lhs ) {
        if( lhs->getType()->getKind() != cpp_type::kind_int ) {
          return new cpp_expr( cpp_expr::kind_cast, intType, lhs );
        } else {
          return lhs;
        }
      }
    
      cpp_expr *makeBoolType( cpp_expr *lhs ) {
        if( lhs->getType()->getKind() != cpp_type::kind_bool ) {
          cpp_expr *zero = new cpp_expr( cpp_expr::kind_cast, intType, (long long)0 );
          zero = new cpp_expr( cpp_expr::kind_cast, lhs->getType(), zero );
          return new cpp_expr( cpp_expr::kind_ne, boolType, lhs, zero );
        } else {
          return lhs;
        }
      }

      cpp_expr *makeVectorBoolType( cpp_expr *lhs ) {
        if( lhs->getType()->getIsPacked() ) {
          int dim = lhs->getType()->getDimension();
          cpp_expr *zero = new cpp_expr( cpp_expr::kind_cast, intTypes1D[ dim-1 ], (long long)0 );
          zero = new cpp_expr( cpp_expr::kind_cast, lhs->getType(), zero );
          return new cpp_expr( cpp_expr::kind_ne, boolTypes1D[ dim-1 ], lhs, zero );
        } else {
          return makeBoolType( lhs );
        }
      }

      // generate code to cast src to destType    
      cpp_expr *makeCast( cpp_expr *src, cpp_type *destType ) {
        //cpp_type *srcType = src->getType();
        return new cpp_expr( cpp_expr::kind_cast, destType, src );
      }

      // called twice in the case of binary operators to convert the two types into a common type.
      cpp_expr *makeSameType( cpp_expr *lhs, cpp_expr *rhs ) {
        cpp_type *lhsType = lhs->getType();
        cpp_type *rhsType = rhs->getType();
        if( *lhsType == *rhsType ) {
          return lhs;
        } else {
          bool lhsVector = lhsType->getIsVector();
          bool rhsVector = rhsType->getIsVector();
          bool lhsScalar = lhsType->getIsScalar();
          bool rhsScalar = rhsType->getIsScalar();
          if( lhsScalar && rhsScalar ) {
            // scalars go to a bigger, floatier kind
            unsigned lhsKind = lhsType->getKind();
            unsigned rhsKind = rhsType->getKind();
            bool hasCfloat = lhsKind == cpp_type::kind_cfloat || rhsKind == cpp_type::kind_cfloat;
            bool hasFloat = lhsKind == cpp_type::kind_float || rhsKind == cpp_type::kind_float;
            bool hasHalf = lhsKind == cpp_type::kind_half || rhsKind == cpp_type::kind_half;
            if( hasCfloat || hasFloat || hasHalf ) {
              return makeCast( lhs, hasCfloat || hasFloat ? floatType : halfType );
            } else {
              return makeCast( lhs, intType );
            }
          } else if( lhsScalar && rhsVector ) {
            // scalars get cast to vectors first
            return makeCast( lhs, rhsType );
          } else if( lhsVector && rhsScalar ) {
            // but vectors don't get cast to scalars (it will be done the other way round)
            return lhs;
          } else {
            return makeCast( lhs, rhsType );
          }
        }
      }
    
      cpp_expr *parseDeclarators( cpp_type *type, bool allowFunctionBodies, unsigned finalToken, bool allowAbstract ) {
        cpp_expr *expr = NULL;
        for(;;) {
          cpp_value *value = parseDeclarator( type, allowAbstract );
          if( !value ) {
            return NULL;
          }
          bool isFunction = value->getType()->getKind() == cpp_type::kind_function;
          if( value->getType()->getIsIn() || value->getType()->getIsOut() ) {
            cpp_log("error: in/out not allowed here\n");
            return NULL;
          }
          if( cpp_value *found = curScope->getExistingValue( value ) ) {
            if( isFunction && allowFunctionBodies ) {
              cpp_value *search = found;
              for(;;) {
                string l, r;
                cpp_log("checking %s == %s\n", search->getType()->toString(l), search->getType()->toString(r));
                if( *search->getType() == *value->getType() ) {
                  cpp_log("*** same function!\n");
                  // get existing function, but update the function type (parameter names may differ)
                  search->setType( value->getType() );
                  break;
                }
                cpp_value *prev = search;
                search = search->getNextPolymorphic();
                if( search == NULL ) {
                  cpp_log("*** new function!\n");
                  // insert new function at end
                  prev->setNextPolymorphic( value );
                  break;
                }
              }
            } else {
              cpp_log("error: '%s' has been redefined\n", value->getName());
              return NULL;
            }
          } else {
            curScope->addValue( value );
          }
          if( cpp_expr *init = value->getInit() ) {
            if( isFunction ) {
              cpp_log("error: '%s' has been redefined\n", value->getName());
              return NULL;
            }
            cpp_expr *val = new cpp_expr( cpp_expr::kind_value, value );
            cpp_expr *rhs = makeCast( init, value->getType() );
            cpp_expr *assign = new cpp_expr( cpp_expr::kind_equals, value->getType(), val, rhs );
            expr = expr ? new cpp_expr( cpp_expr::kind_comma, assign->getType(), expr, assign ) : assign;
          } else if( (int)curToken == tok_lbrace && isFunction ) {
            cpp_scope *saveScope = curScope;
            curScope = value->getType()->getScope();
            string s;
            cpp_log("parsing body %s\n", curScope->toString(s));
            cpp_statement *functionBody = parseStatement();
            curScope = saveScope;
            if( !functionBody ) {
              return NULL;
            }

            cpp_log("[] done function %s\n", value->getName());
          
            //functionBody->end()
          
            value->setInit( new cpp_expr( cpp_expr::kind_statement, functionBody ) );
            finalToken = 0;
            break;
          }
          if( (int)curToken != tok_comma ) {
            break;
          }
          getNext();
        }

        if( finalToken != 0 ) {
          if( !expect( finalToken ) ) {
            return NULL;
          }
          getNext();
        }

        if( expr == NULL ) {
          expr = new cpp_expr( cpp_expr::kind_nop, voidType );
        }
        return expr;
      }

    
      cpp_value *parseDeclarator( cpp_type *type, bool allowAbstract ) {
        string name;

        if( (int)curToken == tok_lparen ) {
          // eg. int (x[5])();
          getNext();
          cpp_value *value = parseDeclarator( type, allowAbstract );
          if( !expect( tok_rparen ) ) {
            return NULL;
          }
          getNext();
          name = value->getName();
          type = value->getType();
        } else if( allowAbstract && ( (int)curToken == tok_rparen || (int)curToken == tok_comma ) ) {
          // eg. float f( int, int, float );
          char tmp[ 32 ];
          sprintf( tmp, "__%d", numAbstract++ );
          name = tmp;
        } else if( (int)curToken == tok_identifier ) {
          name = lexer.id();
          getNext();
        
          cpp_type *typeDef = findTypedef( name );
          if( typeDef != NULL ) {
            cpp_log("error: not expecting a typedef here\n");
            return NULL;
          }
        } else {
          cpp_log("error: expecting a declarator here\n");
          return NULL;
        }
      
        // many arrays allowed in type
        cpp_type *returned_type = type;
        cpp_type *working_type = (cpp_type *)type;
        bool first = true;
        while( (int)curToken == tok_lbracket ) {
          getNext();
          //cpp_expr *dim = parseExpression( 0 );
          if( !expect( tok_rbracket ) ) {
            return NULL;
          }
          getNext();
        
          // translate immediately.
          /*BasicBlock *bb = NULL;
          Value *cexpr = dim->translate_rvalue( bb, *this, true );
          if( cexpr == NULL || !isa< ConstantInt >( cexpr ) ) {
            cpp_log("error: expecting constant int in []\n");
            return NULL;
          }
        
          uint64_t dimension = cast< ConstantInt >( cexpr )->getValue().getLimitedValue();
          */
          uint64_t dimension = 0;

          if( first ) {
            cpp_type *new_type = new cpp_type( cpp_type::kind_array, type, (unsigned)dimension );
            returned_type = working_type = new_type;
            first = false;
          } else {
            cpp_type *new_type = new cpp_type( cpp_type::kind_array, type, (unsigned)dimension );
            working_type->setSubType( new_type );
            working_type = new_type;
          }
        }
      
        // only one function allowed in type      
        if( (int)curToken == tok_lparen ) {
          cpp_scope *saveScope = curScope;
          curScope = new cpp_scope( curScope );
        
          cpp_type *new_type = new cpp_type( cpp_type::kind_function );
          new_type->setScope( curScope );
          getNext();
        
          unsigned paramOffset = 0;
        
          if( returned_type->getIsPassByPtr() ) {
            cpp_type *rtype = new cpp_type( *returned_type );
            rtype->setIsOut( true );
            cpp_value *value = new cpp_value( rtype, "$return" );
            value->setOffset( paramOffset++ );
            curScope->addValue( value );
            new_type->setSubType( voidType );
            new_type->setIsReturnByValue( true );
          } else {
            new_type->setSubType( returned_type );
          }
          returned_type = new_type;
        
          while( (int)curToken != tok_rparen ) {
            cpp_type *type = parseDeclspec();
            if( !type ) {
              if( (int)curToken == tok_identifier ) {
                cpp_log("error: type %s has not been defined\n", lexer.id());
              } else {
                cpp_log("error: expecting type here");
              }
              return NULL;
            }

            if( type->getKind() == cpp_type::kind_function ) {
              cpp_log("error: functions not allowed here\n");
              return NULL;
            }

            cpp_value *value = parseDeclarator( type, true );
            if( !value ) {
              return NULL;
            }
            value->setOffset( paramOffset++ );
            curScope->addValue( value );

            if( (int)curToken != tok_comma ) {
              break;
            }
            getNext();
          }
        
          curScope = saveScope;

          if( !expect( tok_rparen ) ) {
            return NULL;
          }
          getNext();
        }

        if( !verifyType( returned_type ) ) {
          return NULL;
        }

        cpp_value *value = new cpp_value( returned_type, name );

        if( (int)curToken == tok_colon ) {
          getNext();
          if( (int)curToken == tok_register ) {
            getNext();
            if( !expect( tok_lparen ) ) {
              return NULL;
            }
            getNext();
            if( !expect( tok_identifier ) ) {
              return NULL;
            }
            value->setSemantic( lexer.id() );
            getNext();
            if( !expect( tok_rparen ) ) {
              return NULL;
            }
            getNext();
          } else if( expect( tok_identifier ) ) {
            value->setSemantic( lexer.id() );
            getNext();
          } else {
            return NULL;
          }
        } else if( (int)curToken == tok_equals ) {
          getNext();
          if( (int)curToken == tok_lbrace ) {
            int level = 0;
            do
            {
              level += (int)curToken == tok_lbrace;
              level -= (int)curToken == tok_rbrace;
              getNext();
            } while( level != 0 && (int)curToken != tok_end_of_source );
          } else {
            cpp_expr *expr = parseExpression( 1 );
            value->setInit( makeCast( expr, value->getType() ) );
            //value->setInit( expr );
          }
        }
        return value;
      }
    
      bool verifyType( cpp_type *type ) {
        if( type->getKind() == cpp_type::kind_array ) {
          if( type->getIsVector() && type->getDimension() > 4 ) {
            cpp_log("error: packed types have a maximum dimension of 4\n");
          }
        }
        return true;
      }
    
      cpp_type *parseDeclspec() {
        cpp_type *thisType = NULL;
        cpp_type *newType = NULL;
        bool isIn = false;
        bool isOut = false;
        bool isUniform = false;
        bool isConst = false;
        bool isPacked = false;
   
        for(;;) {
          switch( (int)curToken ) {
            case tok_identifier:
            {
              cpp_type *typeDef = findTypedef( lexer.id() );
              if( typeDef == NULL ) {
                goto finish;
              }
              if( thisType != NULL ) {
                cpp_log("error: not expecting a typedef here\n");
                return NULL;
              } else {
                thisType = typeDef;
              }
              getNext();
            } break;

            case tok_struct:
            {
              getNext();
            
              if( (int)curToken != tok_lbrace ) {
                expect( tok_identifier );
                string structId = lexer.id();
                getNext();
              
                if( cpp_type *tagType = findTag( structId ) ) {
                  printf( "*** old tag %s\n", structId.c_str() );
                  thisType = tagType;
                } else {
                  printf( "*** new tag %s\n", structId.c_str() );
                  thisType = new cpp_type( cpp_type::kind_struct );
                  thisType->setScope( NULL );
                  makeTypedef( thisType, structId );
                  makeTag( thisType, structId );
                }
              } else {
                char tmp[ 20 ];
                sprintf( tmp, "__anon%d", structNumber++ );
                string structId = tmp;
                thisType = new cpp_type( cpp_type::kind_struct );
                thisType->setScope( NULL );
                makeTypedef( thisType, structId );
                makeTag( thisType, structId );
              }
            
              if( (int)curToken == tok_lbrace ) {
                if( thisType->getScope() != NULL ) {
                  cpp_log("error: struct already defined\n");
                  return NULL;
                }
                getNext();

                cpp_scope *scope = new cpp_scope( NULL );
                thisType->setScope( scope );
                unsigned structOffset = 0;

                while( (int)curToken != tok_rbrace ) {
                  cpp_type *type = parseDeclspec();
                  if( !type ) {
                    cpp_log("error: expected type here\n");
                    return NULL;
                  }
                  for(;;) {
                    cpp_value *value = parseDeclarator( type, false );
                    if( !value ) {
                      return NULL;
                    }
                    value->setOffset( structOffset++ );
                    scope->addValue( value );
                    if( (int)curToken != tok_comma ) {
                      break;
                    }
                    getNext();
                  }
                
                  if( !expect( tok_semicolon ) ) {
                    break;
                  }
                  getNext();
                }
              
                if( (int)curToken == tok_rbrace ) {
                  getNext();
                }
              }
            } break;
            case tok_void: newType = voidType; goto typeCommon;
            case tok_bool: newType = boolType; goto typeCommon;
            case tok_int: newType = intType; goto typeCommon;
            case tok_float: newType = floatType; goto typeCommon;
            case tok_half: newType = floatType; goto typeCommon;
            case tok_sampler: newType = new cpp_type( cpp_type::kind_sampler ); goto typeCommon;
            case tok_sampler1D: newType = new cpp_type( cpp_type::kind_sampler1D ); goto typeCommon;
            case tok_sampler2D: newType = new cpp_type( cpp_type::kind_sampler2D ); goto typeCommon;
            case tok_sampler3D: newType = new cpp_type( cpp_type::kind_sampler3D ); goto typeCommon;
            case tok_samplerRECT: newType = new cpp_type( cpp_type::kind_samplerRECT ); goto typeCommon;
            case tok_samplerCUBE: newType = new cpp_type( cpp_type::kind_samplerCUBE ); goto typeCommon;
            typeCommon:
            {
              if( thisType ) {
                cpp_log("error: more than one type specified\n");
                return NULL;
              }
              thisType = newType;
              getNext();
            } break;

            case tok_const:
            {
              isConst = true;
              getNext();
            } break;
            case tok_uniform:
            {
              isUniform = true;
              getNext();
            } break;
            case tok_packed:
            {
              isPacked = true;
              getNext();
            } break;
            case tok_in:
            {
              isIn = true;
              getNext();
            } break;
            case tok_out:
            {
              isOut = true;
              getNext();
            } break;
            case tok_inout:
            {
              isIn = true;
              isOut = true;
              getNext();
            } break;
            default:
            {
              goto finish;
            }
          }
        }
      finish:
        if( thisType == NULL ) {
          if( isIn | isOut | isUniform| isConst| isPacked ) {
            cpp_log("error: qualifier without type\n");
          }
        } else {
          thisType->setIsConst( isConst );
          thisType->setIsUniform( isUniform );
          thisType->setIsIn( isIn );
          thisType->setIsOut( isOut );
          thisType->setIsPacked( isPacked );
        }
        return thisType;
      }
    
      cpp_statement *parseStatement() {
        cpp_statement *result = NULL;
        if( (int)curToken == tok_lbrace ) {
          cpp_scope *saveScope = curScope;
          curScope = new cpp_scope( curScope );
        
          result = new cpp_statement( cpp_statement::kind_compound );
          result->setScope( curScope );
          getNext();
          cpp_statement **prev = result->getStatementsAddr();
          while( (int)curToken != tok_rbrace ) {
            cpp_statement *statement = parseStatement();
            if( statement == NULL ) {
              result = NULL;
              break;
            }
            *prev = statement;
            prev = statement->getNextAddr();
          }
          *prev = 0;
          curScope = saveScope;
        
          if( !expect( tok_rbrace ) ) {
            return NULL;
          }
          getNext();
          return result;
        } else if( (int)curToken == tok_return ) {
          getNext();
          if( (int)curToken != tok_semicolon ) {
            cpp_expr *expr = parseExpression();
            if( expr == NULL ) {
              return NULL;
            }
            result = new cpp_statement( cpp_statement::kind_return );
            result->setExpression( expr );
          } else {
            result = new cpp_statement( cpp_statement::kind_return );
          }
          if( !expect( tok_semicolon ) ) {
            return NULL;
          }
          getNext();
        } else if( (int)curToken == tok_discard ) {
          getNext();
          result = new cpp_statement( cpp_statement::kind_return );
          if( !expect( tok_semicolon ) ) {
            return NULL;
          }
          getNext();
        } else if( (int)curToken == tok_for || (int)curToken == tok_if || (int)curToken == tok_while ) {
          bool isFor = (int)curToken == tok_for;
          bool isIf = (int)curToken == tok_if;
          //bool isWhile = (int)curToken == tok_while;

          getNext();
          if( !expect( tok_lparen ) ) {
            return NULL;
          }
          getNext();

          cpp_scope *saveScope = curScope;
          curScope = new cpp_scope( curScope );

          result = new cpp_statement( isFor ? cpp_statement::kind_for : isIf ? cpp_statement::kind_if : cpp_statement::kind_while );
          result->setScope( curScope );
        
          cpp_expr *expr = NULL;
        
          if( !isFor || (int)curToken != tok_semicolon ) {
            if( cpp_type *type = parseDeclspec() ) {
              expr = parseDeclarators( type, false, 0, false );
            } else {
              expr = parseExpression();
            }
            if( !expr ) {
              return NULL;
            }
          }

          if( isFor ) {
            if( !expect( tok_semicolon ) ) {
              return NULL;
            }
            getNext();
            cpp_expr *expr2 = parseExpression();
            if( !expect( tok_semicolon ) ) {
              return NULL;
            }
            getNext();
            cpp_expr *expr3 = parseExpression();
            expr = new cpp_expr( cpp_expr::kind_for, NULL, expr, expr2, expr3 );
          }
        
          result->setExpression( expr );

          if( !expect( tok_rparen ) ) {
            return NULL;
          }
          getNext();

          cpp_statement *stmt = parseStatement();
          if( stmt == NULL ) {
            return NULL;
          }
          *result->getStatementsAddr() = stmt;
        
          if( isIf && (int)curToken == tok_else ) {
            getNext();
            cpp_statement *else_stmt = parseStatement();
            if( else_stmt == NULL ) {
              return NULL;
            }
            *result->getNextAddr() = else_stmt;
          }
        
          curScope = saveScope;
          return result;
        } else if( (int)curToken == tok_do ) {
          getNext();

          result = new cpp_statement( cpp_statement::kind_dowhile );

          cpp_statement *stmt = parseStatement();
          if( stmt == NULL || !expect( tok_while ) ) {
            return NULL;
          }
          *result->getStatementsAddr() = stmt;
          getNext();

          if( !expect( tok_lparen ) ) {
            return NULL;
          }

          cpp_expr *expr = parseExpression();
          if( expr == NULL ) {
            return NULL;
          }
          result->setExpression( expr );
          /*if( !expect( tok_semicolon ) ) {
            return NULL;
          }
          getNext();*/
          return result;
        } else if( cpp_type *type = parseDeclspec() ) {
          result = new cpp_statement( cpp_statement::kind_declaration );
          cpp_expr *init = parseDeclarators( type, false, tok_semicolon, false );
          if( init == NULL ) {
            return NULL;
          }
          result->setExpression( init );
        } else if( cpp_expr *expr = parseExpression() ) {
          result = new cpp_statement( cpp_statement::kind_expression );
          result->setExpression( expr );
          if( !expect( tok_semicolon ) ) {
            return NULL;
          }
          getNext();
        } else {
          cpp_log("error: expected statement");
        }

        return result;
      }
    
      cpp_expr *parseExpression( unsigned minPrecidence=0 ) {
        cpp_expr *result = 0;
        if( trace_parse ) {
          cpp_log("[+] expr\n");
        }
        if( (int)curToken == tok_minus || (int)curToken == tok_plus ) {
          //unsigned op = curToken;
          getNext();
          cpp_expr * rhs = parseExpression( 100 );
          if( rhs == NULL ) {
            return result;
          }
          cpp_expr *zero = new cpp_expr( cpp_expr::kind_int_value, intType, (long long)0 );
          zero = new cpp_expr( cpp_expr::kind_cast, rhs->getType(), zero );
          result = new cpp_expr( (int)curToken == tok_minus ? cpp_expr::kind_minus : cpp_expr::kind_plus, rhs->getType(), zero, rhs );
        } else if( (int)curToken == tok_not ) {
          //unsigned op = curToken;
          getNext();
          cpp_expr * rhs = parseExpression( 100 );
          if( rhs == NULL ) {
            return result;
          }
          rhs = makeBoolType( rhs );
          cpp_expr *one = new cpp_expr( cpp_expr::kind_int_value, intType, (long long)1 );
          one = new cpp_expr( cpp_expr::kind_cast, rhs->getType(), one );
          result = new cpp_expr( cpp_expr::kind_xor, rhs->getType(), one, rhs );
        } else if( (int)curToken == tok_tilda ) {
          //unsigned op = curToken;
          getNext();
          cpp_expr * rhs = parseExpression( 100 );
          if( rhs == NULL ) {
            return result;
          }
          rhs = makeIntType( rhs );
          cpp_expr *one = new cpp_expr( cpp_expr::kind_int_value, intType, (long long)-1 );
          one = new cpp_expr( cpp_expr::kind_cast, rhs->getType(), one );
          result = new cpp_expr( cpp_expr::kind_xor, rhs->getType(), one, rhs );
        } else if( (int)curToken == tok_plus_plus || (int)curToken == tok_minus_minus ) {
          //unsigned op = curToken;
          getNext();
          cpp_expr *rhs = parseExpression( 100 );
          if( rhs == NULL ) {
            return result;
          }
          cpp_expr *inc = new cpp_expr( cpp_expr::kind_int_value, intType, (long long)1 );
          inc = new cpp_expr( cpp_expr::kind_cast, rhs->getType(), inc );
          result = new cpp_expr( (int)curToken == tok_plus_plus ? cpp_expr::kind_plus_equals : cpp_expr::kind_minus_equals, rhs->getType(), rhs, inc );
        } else if( (int)curToken == tok_identifier ) {
          string name( lexer.id() );
          cpp_type *type = findTypedef( name );
          if( type ) {
            getNext();
            if( !expect( tok_lparen ) ) {
              return NULL;
            }
            cpp_expr *init = parseExpression( 100 );
            result = new cpp_expr( cpp_expr::kind_init, type, init );
          } else {
            cpp_value *value = curScope->lookup( name );
            /*if( value == NULL && buildIntrinsic( name.c_str() ) ) {
              value = curScope->lookup( name );
            }*/
            if( value == NULL ) {
              cpp_log("error: undefined symbol '%s'\n", name.c_str());
              return NULL;
            }
            getNext();
            result = new cpp_expr( cpp_expr::kind_value, value );
          }
        } else if( (int)curToken == tok_int_constant || (int)curToken == tok_int64_constant || (int)curToken == tok_uint_constant || (int)curToken == tok_uint64_constant ) {
          //result = (int64_type)lexer_.value();
          result = new cpp_expr( cpp_expr::kind_int_value, cintType, (long long)lexer.value() );
          getNext();
        } else if( (int)curToken == tok_float_constant || (int)curToken == tok_double_constant ) {
          result = new cpp_expr( cpp_expr::kind_double_value, cfloatType, lexer.double_value() );
          //result = (int64_type)lexer_.value();
          getNext();  
        } else if( (int)curToken == tok_lparen ) {
          getNext();
          cpp_type *type = parseDeclspec();
          if( type ) {
            if( !expect( tok_rparen ) ) {
              return NULL;
            }
            getNext();
            cpp_expr * rhs = parseExpression( 100 );
            result = new cpp_expr( cpp_expr::kind_cast, type, rhs );
          } else {
            result = parseExpression();
            expect( tok_rparen );
            getNext();
          }
        } else if( (int)curToken == tok_rparen ) {
          cpp_log("error: missing '(' in expression\n");
          return NULL;
        } else {
          cpp_log("error: expected expression, got %s\n", getTokenName( curToken ));
          return NULL;
        }
      
        // handle  xxx[ n ] xxx( a, b ) xxx++ and xxx--
        for(;;) {
          if( (int)curToken == tok_dot ) {
            getNext();
            if( (int)curToken != tok_identifier ) {
              cpp_log("error: expected member name after '.', got %s instead\n", getTokenName( curToken ));
              return NULL;
            }
            if( result->getType()->getIsVector() ) {
              cpp_type *newType = new cpp_type( *result->getType() );
              cpp_expr *newExpr = new cpp_expr( cpp_expr::kind_swiz, newType, result );
              unsigned char *swiz = newExpr->getSwiz();
              unsigned numSwiz = 0;
              bool bad = false;
              const char choices[] = "xyzwrgba";
              for( char *p = lexer.id(); *p; ++p ) {
                bad |= numSwiz >= 4;
                const char *which = strchr( choices, *p );
                bad |= which == NULL;
                if( !bad ) {
                  swiz[ numSwiz++ ] = (char)( which - choices ) & 3;
                }
              }
              if( bad ) {
                cpp_log("error:  '%s' is not a valid swizzle\n", lexer.id());
                return NULL;
              }
            
              // single swizzles make scalars
              string s;
              cpp_log("%s\n", newType->toString(s));
              if( numSwiz == 1 ) {
                *newType = *newType->getSubType();
              } else {
                newType->setDimension( numSwiz );
              }
              cpp_log("..%s\n", newType->toString(s));
              getNext();
              result = newExpr;
            } else if( result->getType()->getKind() != cpp_type::kind_struct ) {
              cpp_log("error: left hand size of '.' should be a packed array or structure");
              return NULL;
            } else {
              string name( lexer.id() );
              getNext();
              cpp_value *value = result->getType()->getScope()->lookup( name );
              if( value == NULL ) {
                cpp_log("error: structure does not have a member '%s'.\n", name.c_str());
                return NULL;
              }
              result = new cpp_expr( cpp_expr::kind_dot, result, value );
            }
          } else if( (int)curToken == tok_lparen ) {
            getNext();
          
            if( result->getKind() == cpp_expr::kind_int_value ) {
              //cpp_expr *rhs = parseExpression( 0 );
              if( !expect( tok_rparen ) ) {
                return NULL;
              }
              getNext();
              result = new cpp_expr( cpp_expr::kind_int_value, cintType, (long long )0 );
            } else {
              if( result->getKind() != cpp_expr::kind_value || result->getType()->getKind() != cpp_type::kind_function ) {
                cpp_log("error: calling something that is not a function.");
                return NULL;
              }
              cpp_expr *rhs = parseExpression( 0 );

              if( !expect( tok_rparen ) ) {
                return NULL;
              }
              getNext();

              dynarray< cpp_type * > actualTypes;
              if( rhs->getKind() != cpp_expr::kind_comma ) {
                actualTypes.push_back( rhs->getType() );
              } else {
                for( cpp_expr *expr = rhs; ; expr = expr->getKid( 0 ) ) {
                  if( expr->getKind() == cpp_expr::kind_comma ) {
                    actualTypes.push_back( expr->getKid( 1 )->getType() );
                  } else {
                    actualTypes.push_back( expr->getType() );
                    break;
                  }
                }
              }

              cpp_value *value = NULL;
              cpp_value *firstValue = result->getValue();
              for( cpp_value *search = firstValue; search != NULL; search = search->getNextPolymorphic() ) {
                unsigned score = doParametersMatch( actualTypes, search );
                if( score == 2 ) {
                  // exact match. ignore any others.
                  value = search;
                  break;
                }
                if( score == 1 ) {
                  // partial match, will need casts.
                  if( value != NULL ) {
                    cpp_log("error: more than one function matches parameters\n");
                    result = NULL;
                    break;
                  }
                  value = search;
                }
              }

              if( result != NULL ) {
                if( value == NULL ) {
                  //error( "", 0, "no matching function found\n" );
                  cpp_log("no matching function found: %s", firstValue->getName());
                  const char *comma = "( ";
                  string s;
                  for( dynarray< cpp_type * >::iterator i = actualTypes.begin(); i != actualTypes.end(); ++i ) {
                    cpp_log("%s%s", comma, (*i)->toString(s));
                    comma = ", ";
                  }
                  cpp_log(")\n");
                  for( cpp_value *search = firstValue; search != NULL; search = search->getNextPolymorphic() ) {
                    cpp_log("%s\n", search->toString(s));
                  }
                  result = NULL;
                } else {
                  result->setType( value->getType() );
                  result->setValue( value );
                  result = new cpp_expr( cpp_expr::kind_call, result->getType()->getSubType(), result, rhs );
                }
              }
            }
          } else if( (int)curToken == tok_lbracket ) {
            getNext();
            cpp_expr * rhs = parseExpression();
            expect( tok_rbracket );
            getNext();
            if( result->getType()->getKind() != cpp_type::kind_array ) {
              cpp_log("error: trying to use [] on non-array");
              return NULL;
            }
            result = new cpp_expr( cpp_expr::kind_index, result->getType()->getSubType(), result, rhs );
          } else if( (int)curToken == tok_plus_plus || (int)curToken == tok_minus_minus ) {
            getNext();
            char name[ 32 ];
            sprintf( name, "$tmp%d", numTmpVars++ );
            cpp_value *tmp = new cpp_value( result->getType(), name );
            cpp_expr *inc = new cpp_expr( cpp_expr::kind_int_value, intType, (long long)1 );
            inc = new cpp_expr( cpp_expr::kind_cast, result->getType(), inc );
            inc = new cpp_expr( (int)curToken == tok_plus_plus ? cpp_expr::kind_plus_equals : cpp_expr::kind_minus_equals, result->getType(), result, inc );
            cpp_expr *tmpExpr = new cpp_expr( cpp_expr::kind_value, tmp );
            //cpp_expr *tmpAssign = new cpp_expr( cpp_expr::kind_equals, result->getType(), tmpExpr, result );
            result = new cpp_expr( cpp_expr::kind_comma, result->getType(), inc, tmpExpr );
          } else {
            break;
          }
        }      
        //if( debug_expressions ) cpp_log("[] thisPrecidence=%d minPrecidence=%d tok=%s %s\n", thisPrecidence, minPrecidence, getTokenName( curToken ), lexer_.src() );
      
        // filter out operators with lower precidence than minPrecidence
        //unsigned thisPrecidence = tokenToPrecidence[ curToken ];
        //unsigned thisGrouping = tokenIsRightGrouping[ curToken ];
        //unsigned op = curToken;
        while( tokenToPrecidence[ curToken ] != 0 && tokenToPrecidence[ curToken ] > minPrecidence ) {
          unsigned op = curToken;
          getNext();

          // left grouping operators will parse like ( ( a + b ) + c ) + d    so rhs will accept fewer tokens
          // right grouping operators will parse like a = ( b = ( c = d ) )   so rhs will accept more tokens
          cpp_expr *rhs = parseExpression( tokenToPrecidence[ op ] - tokenIsRightGrouping[ curToken ] );
          if( rhs == NULL ) {
            return NULL;
          }
          switch( op ) {
            cpp_expr::kind_enum kind;

            case tok_comma:
            {
              result = new cpp_expr( cpp_expr::kind_comma, rhs->getType(), result, rhs );
            } break;
            case tok_question:
            {
              if( !expect( tok_colon ) ) {
                return NULL;
              }
               getNext();
              cpp_expr *rhs2 = parseExpression( tokenToPrecidence[ op ] - tokenIsRightGrouping[ curToken ] );
            
              rhs = makeSameType( rhs, rhs2 );
              if( result == NULL ) {
                cpp_log("error: unable to convert types\n");
                return NULL;
              }
              rhs2 = makeSameType( rhs2, rhs );
              if( rhs == NULL ) {
                cpp_log("error: unable to convert types\n");
                return NULL;
              }

              result = new cpp_expr( cpp_expr::kind_question, rhs->getType(), makeVectorBoolType( result ), rhs, rhs2 );
            } break;
            case tok_or: kind = cpp_expr::kind_or; goto binop;
            case tok_and: kind = cpp_expr::kind_and; goto binop;
            case tok_xor: kind = cpp_expr::kind_xor; goto binop;
            case tok_shift_left: kind = cpp_expr::kind_shift_left; goto binop;
            case tok_shift_right: kind = cpp_expr::kind_shift_right; goto binop;
            case tok_plus: kind = cpp_expr::kind_plus; goto binop;
            case tok_minus: kind = cpp_expr::kind_minus; goto binop;
            case tok_star: kind = cpp_expr::kind_star; goto binop;
            case tok_divide: kind = cpp_expr::kind_divide; goto binop;
            case tok_mod: kind = cpp_expr::kind_mod; goto binop;
            case tok_or_or: kind = cpp_expr::kind_or_or; goto binop;
            case tok_and_and: kind = cpp_expr::kind_and_and; goto binop;
            binop:
            {
              result = makeSameType( result, rhs );
              if( result == NULL ) {
                cpp_log("error: unable to convert types\n");
                return NULL;
              }
              rhs = makeSameType( rhs, result );
              if( rhs == NULL ) {
                cpp_log("error: unable to convert types\n");
                return NULL;
              }
              result = new cpp_expr( kind, rhs->getType(), result, rhs );
            } break;

            case tok_lt: kind = cpp_expr::kind_lt; goto relop;
            case tok_gt: kind = cpp_expr::kind_gt; goto relop;
            case tok_le: kind = cpp_expr::kind_le; goto relop;
            case tok_ge: kind = cpp_expr::kind_ge; goto relop;
            case tok_eq: kind = cpp_expr::kind_eq; goto relop;
            case tok_ne: kind = cpp_expr::kind_ne; goto relop;
            relop:
            {
              result = makeSameType( result, rhs );
              if( result == NULL ) {
                cpp_log("error: unable to convert types\n");
                return NULL;
              }
              rhs = makeSameType( rhs, result );
              if( rhs == NULL ) {
                cpp_log("error: unable to convert types\n");
                return NULL;
              }
              result = new cpp_expr( kind, boolType, result, rhs );
            } break;

            case tok_equals:
            {
              rhs = makeCast( rhs, result->getType() );
              result = new cpp_expr( cpp_expr::kind_equals, rhs->getType(), result, rhs );
            } break;
            case tok_divide_equals: kind = cpp_expr::kind_divide_equals; goto assignop;
            case tok_mod_equals: kind = cpp_expr::kind_mod_equals; goto assignop;
            case tok_plus_equals: kind = cpp_expr::kind_plus_equals; goto assignop;
            case tok_minus_equals: kind = cpp_expr::kind_minus_equals; goto assignop;
            case tok_shift_left_equals: kind = cpp_expr::kind_shift_left_equals; goto assignop;
            case tok_shift_right_equals: kind = cpp_expr::kind_shift_right_equals; goto assignop;
            case tok_and_equals: kind = cpp_expr::kind_and_equals; goto assignop;
            case tok_xor_equals: kind = cpp_expr::kind_xor_equals; goto assignop;
            case tok_or_equals: kind = cpp_expr::kind_or_equals; goto assignop;
            case tok_times_equals: kind = cpp_expr::kind_times_equals; goto assignop;
            assignop:
            {
              rhs = makeCast( rhs, result->getType() );
              result = new cpp_expr( cpp_expr::kind_equals, rhs->getType(), result, new cpp_expr( kind, rhs->getType(), result, rhs ) );
            } break;
            default:
            {
              assert( 0 );
            } break;
          }
        }
        if( trace_parse ) {
          if( result != NULL ) {
            string s;
            cpp_log("[-] %s /expr\n", result->getType()->toString(s));
          }
        }
        return result;
      }

      bool parseOuter() {
        if( (int)curToken == tok_semicolon ) {
          getNext();
          return true;
        }
        if( (int)curToken == tok_typedef ) {
          getNext();
          cpp_type *type = parseDeclspec();
          if( !type ) {
            cpp_log("error: expecting type\n");
            return false;
          }
          cpp_value *value = parseDeclarator( type, false );
          if( !value ) {
            cpp_log("error: expecting declarator\n");
            return false;
          }
          makeTypedef( value->getType(), value->getName() );
          if( !expect( tok_semicolon ) ) {
            return false;
          }
          getNext();
          return true;
        }
        cpp_type *type = parseDeclspec();
        if( !type ) {
          cpp_log("error: expecting typedef or declaration\n");
          return false;
        }
        if( (int)curToken == tok_semicolon ) {
          getNext();
        } else {
          cpp_expr *expr = parseDeclarators( type, true, tok_semicolon, false );
          if( expr == NULL ) {
            return false;
          }
        }
      
        return true;
      }

    public:
      cpp_parser() {
        for (int i = tok_last; i != tok_lastlast; ++i) {
          lexer.add_identifier(getTokenName(i), i);
        }
        memset( tokenToPrecidence, 0, sizeof( tokenToPrecidence ) );

        tokenToPrecidence[ tok_comma ] = 1;
        tokenToPrecidence[ tok_equals ] = tokenToPrecidence[ tok_times_equals ] = tokenToPrecidence[ tok_divide_equals ] = tokenToPrecidence[ tok_mod_equals ] = tokenToPrecidence[ tok_plus_equals ] = 2;
        tokenToPrecidence[ tok_minus_equals ] = tokenToPrecidence[ tok_shift_left_equals ] = 2;
        tokenToPrecidence[ tok_shift_right_equals ] = tokenToPrecidence[ tok_and_equals ] = tokenToPrecidence[ tok_xor_equals ] = tokenToPrecidence[ tok_or_equals ] = 2;
        tokenToPrecidence[ tok_question ] = 3;
        tokenToPrecidence[ tok_or_or ] = 4;
        tokenToPrecidence[ tok_and_and ] = 5;
        tokenToPrecidence[ tok_or ] = 6;
        tokenToPrecidence[ tok_and ] = 7;
        tokenToPrecidence[ tok_xor ] = 8;
        tokenToPrecidence[ tok_lt ] = 9; tokenToPrecidence[ tok_gt ] = 9; tokenToPrecidence[ tok_le ] = 9; tokenToPrecidence[ tok_ge ] = 9; tokenToPrecidence[ tok_eq ] = 9; tokenToPrecidence[ tok_ne ] = 9;
        tokenToPrecidence[ tok_shift_left ] = 10; tokenToPrecidence[ tok_shift_right ] = 10;
        tokenToPrecidence[ tok_plus ] = 11; tokenToPrecidence[ tok_minus ] = 11;
        tokenToPrecidence[ tok_star ] = 12; tokenToPrecidence[ tok_divide ] = 12; tokenToPrecidence[ tok_mod ] = 12;
        tokenToPrecidence[ tok_dot ] = 13;
        tokenToPrecidence[ tok_lparen ] = 13;
        tokenToPrecidence[ tok_lbracket ] = 13;
      
        memset( tokenIsRightGrouping, 0, sizeof( tokenIsRightGrouping ) );
        tokenIsRightGrouping[ tok_comma ] = 1;
        tokenIsRightGrouping[ tok_equals ] = tokenIsRightGrouping[ tok_times_equals ] = tokenIsRightGrouping[ tok_divide_equals ] = tokenIsRightGrouping[ tok_mod_equals ] = tokenIsRightGrouping[ tok_plus_equals ] = 1;
        tokenIsRightGrouping[ tok_minus_equals ] = tokenIsRightGrouping[ tok_shift_left_equals ] = 1;
        tokenIsRightGrouping[ tok_shift_right_equals ] = tokenIsRightGrouping[ tok_and_equals ] = tokenIsRightGrouping[ tok_xor_equals ] = tokenIsRightGrouping[ tok_or_equals ] = 1;
        tokenIsRightGrouping[ tok_question ] = 1;

        invariantScope = new cpp_scope( NULL );
        curScope = globalScope = new cpp_scope( invariantScope );
      
        dontReadLine = false;
      
        voidType = makeTypedef( new cpp_type( cpp_type::kind_void ), "void" );
        intType = makeTypedef( new cpp_type( cpp_type::kind_int ), "int" );
        floatType = makeTypedef( new cpp_type( cpp_type::kind_float ), "float" );
        doubleType = makeTypedef( new cpp_type( cpp_type::kind_float ), "double" );
        halfType = makeTypedef( new cpp_type( cpp_type::kind_half ), "half" );
        boolType = makeTypedef( new cpp_type( cpp_type::kind_bool ), "bool" );
        cintType = new cpp_type( cpp_type::kind_cint );
        cfloatType = new cpp_type( cpp_type::kind_cfloat );
      
        for( unsigned i = 1; i <= 4; ++i ) {
          char tmp[ 10 ];

          sprintf( tmp, "bool%d", i );
          cpp_type *bool_type = new cpp_type( cpp_type::kind_bool, i, true );
          makeTypedef( bool_type, tmp );

          sprintf( tmp, "int%d", i );
          cpp_type *int_type = new cpp_type( cpp_type::kind_int, i, true );
          makeTypedef( int_type, tmp );

          sprintf( tmp, "float%d", i );
          cpp_type *float_type = new cpp_type( cpp_type::kind_float, i, true );
          makeTypedef( float_type, tmp );

          sprintf( tmp, "half%d", i );
          cpp_type *half_type = new cpp_type( cpp_type::kind_half, i, true );
          makeTypedef( half_type, tmp );
        
          boolTypes1D[ i-1 ] = bool_type;
          intTypes1D[ i-1 ] = int_type;
          floatTypes1D[ i-1 ] = float_type;
          halfTypes1D[ i-1 ] = half_type;

          for( unsigned j = 1; j <= 4; ++j ) {
            char tmp[ 10 ];
            cpp_type *type;
            sprintf( tmp, "bool%dx%d", i, j );
            type = new cpp_type( cpp_type::kind_array, bool_type, j );
            makeTypedef( type, tmp );
            boolTypes2D[ i-1 ][ j-1 ] = type;

            sprintf( tmp, "int%dx%d", i, j );
            type = new cpp_type( cpp_type::kind_array, int_type, j );
            makeTypedef( type, tmp );
            intTypes2D[ i-1 ][ j-1 ] = type;

            sprintf( tmp, "float%dx%d", i, j );
            type = new cpp_type( cpp_type::kind_array, float_type, j );
            makeTypedef( type, tmp );
            floatTypes2D[ i-1 ][ j-1 ] = type;

            sprintf( tmp, "half%dx%d", i, j );
            type = new cpp_type( cpp_type::kind_array, half_type, j );
            makeTypedef( type, tmp );
            halfTypes2D[ i-1 ][ j-1 ] = type;
          }
        }
      
        // initialise anaonymous structure index
        structNumber = 0;
        numAbstract = 0;
        numTmpVars = 0;
      }

      void parse(const char *src) {
        preprocessor.begin(src);
        lexer.start(preprocessor.cur_line());
        dontReadLine = false;
        line_number = 1;
        getNext();

        while( (int)curToken != tok_end_of_source ) {
          if( !parseOuter() ) {
            return;
          }
        }
      }

    };
  }
}
