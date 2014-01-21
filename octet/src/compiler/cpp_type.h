////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// C/C++ language subset parser
//

namespace octet
{
  namespace compiler
  {
    class cpp_type
    {
    public:
      enum kind_enum
      {
        kind_none,
        kind_struct,
        kind_array,
        kind_ptr,
        kind_function,
        kind_float,
        kind_half,
        kind_bool,
        kind_void,
        kind_int,
        kind_cfloat,
        kind_cint,
        kind_sampler,
        kind_sampler1D,
        kind_sampler2D,
        kind_sampler3D,
        kind_samplerRECT,
        kind_samplerCUBE,
      };

      static const char *typeName( kind_enum kind )
      {
        switch( kind )
        {
          case kind_none: return "kind_none";
          case kind_struct: return "kind_struct";
          case kind_array: return "kind_array";
          case kind_ptr: return "kind_ptr";
          case kind_function: return "kind_function";
          case kind_float: return "kind_float";
          case kind_half: return "kind_half";
          case kind_bool: return "kind_bool";
          case kind_void: return "kind_void";
          case kind_int: return "kind_int";
          case kind_cfloat: return "kind_cfloat";
          case kind_cint: return "kind_cint";
          case kind_sampler: return "kind_sampler";
          case kind_sampler1D: return "kind_sampler1D";
          case kind_sampler2D: return "kind_sampler2D";
          case kind_sampler3D: return "kind_sampler3D";
          case kind_samplerRECT: return "kind_samplerRECT";
          case kind_samplerCUBE: return "kind_samplerCUBE";
          default: return "???";
        }
      }
    
      //typedef dynarray< cpp_value * > value_array;
      //typedef dynarray< cpp_value * >::iterator value_iterator;

    private:

      //const Type *llvmType;
      cpp_type *subType;
      cpp_scope *scope;

      kind_enum kind : 8;
      bool isIn : 1;
      bool isOut : 1;
      bool isUniform : 1;
      bool isConst : 1;
      bool isPacked : 1;
      bool isReturnByValue : 1;
    
      unsigned dimension : 16;
    
      void clear()
      {
        isIn = false;
        isOut = false;
        isUniform = false;
        isConst = false;
        isPacked = false;
        isReturnByValue = false;
        dimension = 0;
        //llvmType = NULL;
        subType = NULL;
        scope = NULL;
      }

    public:
      cpp_type( cpp_type::kind_enum kind_, cpp_type *subType_ = NULL ) : kind( kind_ )
      {
        clear();
        subType = subType_;
      }

      cpp_type( cpp_type::kind_enum kind_, unsigned dim, bool isPacked_=false ) : kind( kind_array )
      {
        clear();
        subType = new cpp_type( kind_ );
        dimension = dim;
        subType->isPacked = isPacked_;
      }

      cpp_type( cpp_type::kind_enum kind_, cpp_type *subType_, unsigned dim ) : kind( kind_ )
      {
        clear();
        subType = subType_;
        dimension = dim;
      }

      cpp_type *getSubType() const
      {
        return subType;
      }

      void setSubType( cpp_type *type )
      {
        subType = type;
      }

      cpp_scope *getScope() const
      {
        return scope;
      }

      void setScope( cpp_scope *scope_ )
      {
        scope = scope_;
      }

      kind_enum getKind() const { return kind; }
      //void setLlvmType( const Type *llvmType_ ) { llvmType = llvmType_; }
      //const Type *getLlvmType() const { return llvmType; }
      void setDimension( unsigned dimension_ ) { dimension = dimension_; }
      unsigned getDimension() const { return dimension; }
      void setIsConst( bool isConst_ ) { isConst = isConst_; }
      bool getIsConst() const { return isConst; }
      void setIsUniform( bool isUniform_ ) { isUniform = isUniform_; }
      bool getIsUniform() const { return isUniform; }
      void setIsIn( bool isIn_ ) { isIn = isIn_; }
      bool getIsIn() const { return isIn; }
      void setIsOut( bool isOut_ ) { isOut = isOut_; }
      bool getIsOut() const { return isOut; }
      void setIsPacked( bool isPacked_ ) { isPacked = isPacked_; }
      bool getIsPacked() const { return isPacked; }
      void setIsReturnByValue( bool isReturnByValue_ ) { isReturnByValue = isReturnByValue_; }
      bool getIsReturnByValue() const { return isReturnByValue; }

      bool getIsFloat()
      {
        return kind == kind_float || kind == kind_half || kind == kind_cfloat;
      }

      bool getIsInt()
      {
        return kind == kind_bool || kind == kind_int;
      }

      bool getIsScalar()
      {
        return kind == kind_bool || kind == kind_int || kind == kind_float || kind == kind_half || kind == kind_cfloat || kind == kind_cint || kind == kind_void;
      }

      bool getIsVector()
      {
        return kind == kind_array && subType->getIsScalar() && subType->getIsPacked();
      }

      bool getIsArray()
      {
        return kind == kind_array && !( subType->getIsScalar() && subType->getIsPacked() );
      }

      bool getIsPassByPtr()
      {
        return getIsOut() || getKind() == cpp_type::kind_struct || getIsArray();
      }

      #if 0
      const Type *translate( fc::default_context &context )
      {
        if( trace_translate )
        {
          cpp_log(context.push() << "type " << typeName( kind ) << "\n";
        }
        if( llvmType == NULL )
        {
          switch( kind )
          {
            case kind_ptr:
            {
              const Type *llvmSubType = subType->translate( context );
              if( llvmSubType == NULL ) goto crash_n_burn;
              llvmType = PointerType::get( llvmSubType, 0 );
            } break;
            case kind_struct:
            {
              dynarray< const Type * > params;
              getScopeTypes( params, scope, false, context );
              llvmType = StructType::get( params, false );
            } break;
            case kind_array:
            {
              if( getSubType()->getIsPacked() )
              {
                llvmType = VectorType::get( getSubType()->translate( context ), 4 );
              } else
              {
                llvmType = ArrayType::get( getSubType()->translate( context ), dimension );
              }
            } break;
            case kind_function:
            {
              // return paramaeter has been added, but no pointers have been made yet.
              dynarray< const Type * > params;
              getScopeTypes( params, scope, true, context ); // makes pointers for outs, structs and arrays
              llvmType = FunctionType::get( getSubType()->translate( context ), params, false );
              cpp_log(*llvmType << "<---\n";
            } break;
            case kind_float:
            {
              llvmType = Type::FloatTy;
            } break;
            case kind_half:
            {
              llvmType = Type::FloatTy;
            } break;
            case kind_bool:
            {
              llvmType = Type::Int1Ty;
            } break;
            case kind_void:
            {
              llvmType = Type::VoidTy;
            } break;
            case kind_int:
            {
              llvmType = Type::Int32Ty;
            } break;
            case kind_cfloat:
            {
              llvmType = Type::FloatTy;
            } break;
            case kind_cint:
            {
              llvmType = Type::Int32Ty;
            } break;
            case kind_sampler:
            case kind_sampler1D:
            case kind_sampler2D:
            case kind_sampler3D:
            case kind_samplerRECT:
            case kind_samplerCUBE:
            {
              dynarray< const Type * > params;
              llvmType = StructType::get( params, false );
            } break;
          }
          assert( llvmType != NULL && "llvm type not generated" );
        }

        if( llvmType != NULL )
        {
          if( trace_translate )
          {
            cpp_log(context.pop() << "type " << typeName( kind ) << " " << *llvmType << "\n";
          }
          return llvmType;
        }
      crash_n_burn:
        if( trace_translate )
        {
          cpp_log(context.pop() << "error\n";
        }
        return NULL;
      }
      #endif

      const char *toString(string &buf) {
        buf.format("");
        return buf.c_str();
      }
    
    };
  }
}
