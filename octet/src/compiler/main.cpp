////////////////////////////////////////////////////////
//
// instance module for sncgc command line tool
//
////////////////////////////////////////////////////////

#include <stdio.h>
#include <vector>
#include <map>
#include <fstream>
#include <assert.h>
#include "fc_test.h"

#pragma warning ( disable : 4244 )
#pragma warning ( disable : 4800 )
#pragma warning ( disable : 4291 )
#pragma warning ( disable : 4146 )


#include "../llvm-2.3/include/llvm/Module.h"
#include "../llvm-2.3/include/llvm/DerivedTypes.h"
#include "../llvm-2.3/include/llvm/Constants.h"
#include "../llvm-2.3/include/llvm/Instructions.h"
#include "../llvm-2.3/include/llvm/ModuleProvider.h"
#include "../llvm-2.3/include/llvm/ParameterAttributes.h"
#include "../llvm-2.3/include/llvm/CallingConv.h"
#include "../llvm-2.3/include/llvm/GlobalVariable.h"
#include "../llvm-2.3/include/llvm/ADT/APFloat.h"
#include "../llvm-2.3/include/llvm/Bitcode/ReaderWriter.h"
#include "../llvm-2.3/include/llvm/Analysis/Verifier.h"
//#include "../llvm-2.3/lib/Target/PowerPC/PPCTargetMachine.h"
//#include "../llvm-2.3/lib/Target/CBackend/CTargetMachine.h"
#include "../llvm-2.3/lib/Target/CellSPU/SPUTargetMachine.h"
#include "../llvm-2.3/include/llvm/PassManagers.h"
#include "../llvm-2.3/include/llvm/Transforms/Scalar.h"
#include "../llvm-2.3/include/llvm/Transforms/IPO.h"
#include "../llvm-2.3/include/llvm/CodeGen/FileWriters.h"
#include "../llvm-2.3/include/llvm/Support/CommandLine.h"
#include "../llvm-2.3/include/llvm/Support/Debug.h"
#include "../llvm-2.3/include/llvm/Support/PassNameParser.h"

using namespace llvm;

/*static cl::opt<std::string> IncludePaths(
  cl::Positional, cl::desc( "Add include path" ),
  cl::init("-"), cl::value_desc( "path" )
);*/

static cl::list<const PassInfo*, bool, PassNameParser>
PassList(cl::desc("Optimizations available:"));

// Other command line options...
//
static cl::opt<std::string>
InputFilename(cl::Positional, cl::desc("<input bitcode file>"), 
    cl::init("-"), cl::value_desc("filename"));

static cl::opt<std::string>
OutputFilename("o", cl::desc("Override output filename"),
               cl::value_desc("filename"), cl::init("-"));

/*
static cl::opt<bool>
Force("f", cl::desc("Overwrite output files"));

static cl::opt<bool>
PrintEachXForm("p", cl::desc("Print module after each transformation"));

static cl::opt<bool>
NoOutput("disable-output",
         cl::desc("Do not write result bitcode file"), cl::Hidden);

static cl::opt<bool>
NoVerify("disable-verify", cl::desc("Do not verify result module"), cl::Hidden);

static cl::opt<bool>
VerifyEach("verify-each", cl::desc("Verify after each transform"));

static cl::opt<bool>
StripDebug("strip-debug",
           cl::desc("Strip debugger symbol info from translation unit"));

static cl::opt<bool>
DisableInline("disable-inlining", cl::desc("Do not run the inliner pass"));

static cl::opt<bool> 
DisableOptimizations("disable-opt", 
                     cl::desc("Do not run any optimization passes"));

static cl::opt<bool>
StandardCompileOpts("std-compile-opts", 
                   cl::desc("Include the standard compile time optimizations"));

static cl::opt<bool>
Quiet("q", cl::desc("Obsolete option"), cl::Hidden);

static cl::alias
QuietA("quiet", cl::desc("Alias for -q"), cl::aliasopt(Quiet));

static cl::opt<bool>
AnalyzeOnly("analyze", cl::desc("Only perform analysis, no optimization"));
*/

enum
{
  trace_tokens = 0,
  trace_parse = 1,
  trace_translate = 1,
};

/*struct cgcompiler_options
{
  fc::option filename;
  fc::option include;
  fc::option output_filename;
  
  cgcompiler_options() :
    filename( NULL, NULL, "file name", fc::option::type_text ),
    include( "-I", NULL, "extra include path", fc::option::type_text ),
    output_filename( "-o", NULL, "output file name", fc::option::type_text )
  {
  }

  bool parse( int argc, char *argv[] )
  {
    for( int i = 1; i < argc; )
    {
      //printf( "parse %d/%d: %s\n", i, argc, argv[ i ] );
      if(
        !filename.parse( argc, argv, i ) &&
        !include.parse( argc, argv, i ) &&
        !output_filename.parse( argc, argv, i )
      )
      {
        fprintf( stderr, "error: no option %s\n", argv[ i++ ] );
        return false;
      }
    }

    if( filename.text() == NULL )
    {
      fprintf( stderr, "error: no file name\n" );
      return false;
    }

    if( output_filename.text() == NULL )
    {
      char *tmp = (char*)alloca( strlen( filename.text() ) + 10 );
      strcpy( tmp, filename.text() );
      char *dot = strrchr( tmp, '.' );
      if( dot )
      {
        strcpy( dot, ".ps" );
      }
      output_filename.set_text( strdup( tmp ) );
    }

    return true;
  }
};*/

namespace sncgc
{
  class type_t;
  class expr_t;
  class scope_t;
  class stmt_t;
  class value_t;
  
  OStream &operator <<( OStream &in, const value_t &value );
  OStream &operator <<( OStream &in, const type_t &type );

  void getScopeTypes( std::vector< const Type * > &params, scope_t *scope, bool isParameterList, fc::default_context &context );
  unsigned doParametersMatch( std::vector< type_t * > &actualTypes, value_t *search );
  Value *translateCast( Value *lhs, type_t *srcType, type_t *destType, BasicBlock *bb, fc::default_context &context );
  bool operator == ( type_t &lhs, type_t &rhs );

  class value_t
  {
    type_t *valueType;
    std::string name;
    std::string semantic;
    unsigned offset;
    expr_t *init;
    Value *llvmValue;
    
    // functions have the same name, but multiple types.
    // if we don't find a match for the first one, we can try another.
    value_t *nextPolymorphicValue;
  public:
    value_t( type_t *valueType_, const std::string &name_ ) : valueType( valueType_ ), name( name_ )
    {
      init = NULL;
      offset = 0;
      llvmValue = NULL;
      nextPolymorphicValue = NULL;
    }
    
    value_t *getNextPolymorphic()
    {
      return nextPolymorphicValue;
    }

    void setNextPolymorphic( value_t *value )
    {
      nextPolymorphicValue = value;
    }

    const std::string &getName() const
    {
      return name;
    }

    void setSemantic( const std::string &name_ )
    {
      semantic = name_;
    }
    
    const std::string &getSemantic()
    {
      return name;
    }
    
    type_t *getType() const
    {
      return valueType;
    }
    
    void setType( type_t *type )
    {
      valueType = type;
    }
    
    void setOffset( unsigned value )
    {
      offset = value;
    }
    
    unsigned getOffset()
    {
      return offset;
    }
    
    void setInit( expr_t *value )
    {
      init = value;
    }

    expr_t *getInit()
    {
      return init;
    }
    
    void setLlvmValue( Value *value )
    {
      cerr << "setLlvmValue" << name << " = " << *value << "\n";
      llvmValue = value;
    }
    
    Value *getLlvmValue()
    {
      return llvmValue;
    }
  };

  class type_t
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
    
    //typedef std::vector< value_t * > value_array;
    //typedef std::vector< value_t * >::iterator value_iterator;

  private:

    const Type *llvmType;
    type_t *subType;
    scope_t *scope;

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
      llvmType = NULL;
      subType = NULL;
      scope = NULL;
    }

  public:
    type_t( type_t::kind_enum kind_, type_t *subType_ = NULL ) : kind( kind_ )
    {
      clear();
      subType = subType_;
    }

    type_t( type_t::kind_enum kind_, unsigned dim, bool isPacked_=false ) : kind( kind_array )
    {
      clear();
      subType = new type_t( kind_ );
      dimension = dim;
      subType->isPacked = isPacked_;
    }

    type_t( type_t::kind_enum kind_, type_t *subType_, unsigned dim ) : kind( kind_ )
    {
      clear();
      subType = subType_;
      dimension = dim;
    }

    type_t *getSubType() const
    {
      return subType;
    }

    void setSubType( type_t *type )
    {
      subType = type;
    }

    scope_t *getScope() const
    {
      return scope;
    }

    void setScope( scope_t *scope_ )
    {
      scope = scope_;
    }

    kind_enum getKind() const { return kind; }
    void setLlvmType( const Type *llvmType_ ) { llvmType = llvmType_; }
    const Type *getLlvmType() const { return llvmType; }
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
      return getIsOut() || getKind() == type_t::kind_struct || getIsArray();
    }

    const Type *translate( fc::default_context &context )
    {
      if( trace_translate )
      {
        cerr << context.push() << "type " << typeName( kind ) << "\n";
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
            std::vector< const Type * > params;
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
            std::vector< const Type * > params;
            getScopeTypes( params, scope, true, context ); // makes pointers for outs, structs and arrays
            llvmType = FunctionType::get( getSubType()->translate( context ), params, false );
            cerr << *llvmType << "<---\n";
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
            std::vector< const Type * > params;
            llvmType = StructType::get( params, false );
          } break;
        }
        assert( llvmType != NULL && "llvm type not generated" );
      }

      if( llvmType != NULL )
      {
        if( trace_translate )
        {
          cerr << context.pop() << "type " << typeName( kind ) << " " << *llvmType << "\n";
        }
        return llvmType;
      }
    crash_n_burn:
      if( trace_translate )
      {
        cerr << context.pop() << "error\n";
      }
      return NULL;
    }
    
  };
  
  BasicBlock *translate_statement( stmt_t *statement, BasicBlock *bb, fc::default_context &context, type_t *functionType );
  
  class expr_t
  {
  public:
    enum kind_enum
    {
      kind_nop,
      kind_statement,
      kind_value_ptr,
      kind_value,
      kind_dot,
      kind_call,
      kind_index,
      kind_swiz,
      kind_int_value,
      kind_double_value,
      kind_cast,
      kind_for,
      kind_init,
      kind_deref,

      kind_comma,
      kind_question,
      kind_or,
      kind_and,
      kind_xor,
      kind_lt,
      kind_gt,
      kind_le,
      kind_ge,
      kind_eq,
      kind_ne,
      kind_shift_left,
      kind_shift_right,
      kind_plus,
      kind_minus,
      kind_star,
      kind_divide,
      kind_mod,
      kind_or_or,
      kind_and_and,
      kind_equals,
      kind_times_equals,
      kind_divide_equals,
      kind_mod_equals,
      kind_plus_equals,
      kind_minus_equals,
      kind_shift_left_equals,
      kind_shift_right_equals,
      kind_and_equals,
      kind_xor_equals,
      kind_or_equals,
    };
    
    static const char *kindName( kind_enum kind )
    {
      switch( kind )
      {
        case kind_nop: return "kind_nop";
        case kind_statement: return "kind_statement";
        case kind_value_ptr: return "kind_value_ptr";
        case kind_value: return "kind_value";
        case kind_dot: return "kind_dot";
        case kind_call: return "kind_call";
        case kind_index: return "kind_index";
        case kind_swiz: return "kind_swiz";
        case kind_int_value: return "kind_int_value";
        case kind_double_value: return "kind_double_value";
        case kind_cast: return "kind_cast";
        case kind_for: return "kind_for";
        case kind_init: return "kind_init";
        case kind_deref: return "kind_deref";
        case kind_comma: return "kind_comma";
        case kind_question: return "kind_question";
        case kind_or: return "kind_or";
        case kind_and: return "kind_and";
        case kind_xor: return "kind_xor";
        case kind_lt: return "kind_lt";
        case kind_gt: return "kind_gt";
        case kind_le: return "kind_le";
        case kind_ge: return "kind_ge";
        case kind_eq: return "kind_eq";
        case kind_ne: return "kind_ne";
        case kind_shift_left: return "kind_shift_left";
        case kind_shift_right: return "kind_shift_right";
        case kind_plus: return "kind_plus";
        case kind_minus: return "kind_minus";
        case kind_star: return "kind_star";
        case kind_divide: return "kind_divide";
        case kind_mod: return "kind_mod";
        case kind_or_or: return "kind_or_or";
        case kind_and_and: return "kind_and_and";
        case kind_equals: return "kind_equals";
        case kind_times_equals: return "kind_times_equals";
        case kind_divide_equals: return "kind_divide_equals";
        case kind_mod_equals: return "kind_mod_equals";
        case kind_plus_equals: return "kind_plus_equals";
        case kind_minus_equals: return "kind_minus_equals";
        case kind_shift_left_equals: return "kind_shift_left_equals";
        case kind_shift_right_equals: return "kind_shift_right_equals";
        case kind_and_equals: return "kind_and_equals";
        case kind_xor_equals: return "kind_xor_equals";
        case kind_or_equals: return "kind_or_equals";
        default: return "???";
      }
    };
  private:
    kind_enum kind;
    expr_t *kid[ 3 ];
    union
    {
      stmt_t *statement;
      value_t *value;
      double double_value;
      long long int_value;
      unsigned char swiz[ 4 ];
    };
    type_t *type;
  public:
    expr_t( kind_enum kind_, type_t *type_, expr_t *lhs_ = 0, expr_t *rhs_ = 0, expr_t *rhs2_ = 0 ) : kind( kind_ )
    {
      type = type_;
      kid[ 0 ] = lhs_;
      kid[ 1 ] = rhs_;
      kid[ 2 ] = rhs2_;
    }
    
    expr_t( kind_enum kind_, expr_t *lhs_, value_t *value_ ) : kind( kind_ )
    {
      kid[ 0 ] = lhs_;
      kid[ 1 ] = NULL;
      kid[ 2 ] = NULL;
      value = value_;
      type = value_->getType();
    }
    
    expr_t( kind_enum kind_, value_t *value_ ) : kind( kind_ )
    {
      kid[ 0 ] = NULL;
      kid[ 1 ] = NULL;
      kid[ 2 ] = NULL;
      value = value_;
      //type = new type_t( type_t::kind_ptr, value_->getType() );
      type = value_->getType();
    }
    
    expr_t( kind_enum kind_, type_t *type_, double value ) : kind( kind_ )
    {
      kid[ 0 ] = NULL;
      kid[ 1 ] = NULL;
      kid[ 2 ] = NULL;
      double_value = value;
      type = type_;
    }
    
    expr_t( kind_enum kind_, type_t *type_, long long value ) : kind( kind_ )
    {
      kid[ 0 ] = NULL;
      kid[ 1 ] = NULL;
      kid[ 2 ] = NULL;
      int_value = value;
      type = type_;
    }
    
    expr_t( kind_enum kind_, stmt_t *statement_ ) : kind( kind_ )
    {
      kid[ 0 ] = NULL;
      kid[ 1 ] = NULL;
      kid[ 2 ] = NULL;
      statement = statement_;
      type = NULL;
    }
    
    type_t *getType()
    {
      return type;
    }
    
    void setType( type_t *t )
    {
      type = t;
    }
    
    expr_t *getKid( size_t n )
    {
      return kid[ n ];
    }

    kind_enum getKind()
    {
      return kind;
    }

    stmt_t *getStatement()
    {
      return statement;
    }

    value_t *getValue()
    {
      return value;
    }
    
    void setValue( value_t *v )
    {
      value = v;
    }

    double getDoubleValue()
    {
      return double_value;
    }

    unsigned long long getIntValue()
    {
      return int_value;
    }
    
    unsigned char *getSwiz()
    {
      return swiz;
    }
    
    // generate an llvm pointer to this cg expression
    Value *translate_lvalue( BasicBlock *&bb, fc::default_context &context, bool isConstant )
    {
      Value *result = NULL, *lhs = NULL;
      if( trace_translate )
      {
        cerr << context.push() << "lvalue " << kindName( kind ) << "\n";
      }
      
      switch( kind )
      {
        case kind_value:
        {
          if( !isConstant )
          {
            result = value->getLlvmValue();
          }
        } break;
        case kind_dot:
        {
          lhs = getKid( 0 )->translate_lvalue( bb, context, isConstant );
          if( lhs == NULL ) goto crash_n_burn;
          Value *indices[ 2 ];
          indices[ 0 ] = ConstantInt::get( Type::Int32Ty, 0 );
          indices[ 1 ] = ConstantInt::get( Type::Int32Ty, value->getOffset() );
          cerr << *lhs->getType() << "\n";
          if( isConstant )
          {
            result = ConstantExpr::getGetElementPtr( cast<Constant>( lhs ), (Constant * const *)indices, 2 );
          } else
          {
            result = GetElementPtrInst::Create( lhs, indices, indices + 2, "", bb );
          }
          //result = GetElementPtrInst::Create( lhs, ConstantInt::get( Type::Int32Ty, value->getOffset() ), "", bb );
        } break;
        case kind_index:
        {
          lhs = getKid( 0 )->translate_lvalue( bb, context, isConstant );
          if( lhs == NULL ) goto crash_n_burn;
          Value *rhs = getKid( 1 )->translate_rvalue( bb, context, isConstant );
          if( rhs == NULL ) goto crash_n_burn;
          Value *indices[ 2 ];
          indices[ 0 ] = ConstantInt::get( Type::Int32Ty, 0 );
          indices[ 1 ] = rhs;
          if( isConstant )
          {
            result = ConstantExpr::getGetElementPtr( cast<Constant>( lhs ), (Constant * const *)indices, 2 );
          } else
          {
            result = GetElementPtrInst::Create( lhs, indices, indices + 2, "", bb );
          }
        } break;
      }

      if( result == NULL ) goto crash_n_burn;

      const Type *llvmType = PointerType::getUnqual( type ? type->translate( context ) : Type::VoidTy );
      if( llvmType == NULL )
      {
        cerr << context.indent() << "type failed\n";
        goto crash_n_burn;
      }
      if( trace_translate )
      {
        cerr << context.pop() << "/lvalue " << kindName( kind ) << "" << *result << " [" << *llvmType << "/" << *result->getType() << "]\n";
      }
      assert( llvmType == result->getType() );
      return result;
    crash_n_burn:
      if( trace_translate )
      {
        cerr << context.pop() << "error\n";
      }
      return NULL;
    }

    // generate an llvm Value for this cg expression.
    Value *translate_rvalue( BasicBlock *&bb, fc::default_context &context, bool isConstant )
    {
      Value *result = NULL;
      Value *lhs = 0, *rhs = 0, *rhs2 = 0;
      const Type *llvmType = type ? type->translate( context ) : Type::VoidTy;
      Instruction::BinaryOps op;
      
      if( trace_translate )
      {
        cerr << context.push() << "rvalue " << kindName( kind ) << "\n";
      }
      
      if( llvmType == NULL )
      {
        cerr << context.indent() << "type failed\n";
        goto crash_n_burn;
      }

      switch( kind )
      {
        ICmpInst::Predicate ipred;
        FCmpInst::Predicate fpred;
        case kind_nop:
        {
          return ConstantInt::get( Type::Int32Ty, 0 );
        } break;
        //case kind_statement:
        {
          //bb = translate_statement( statement, bb, context,  );
        } break;
        case kind_value_ptr:
        {
          result = value->getLlvmValue();
          assert( result != NULL && "value has null type" );
        } break;
        case kind_value:
        {
          result = new LoadInst( value->getLlvmValue(), false, bb );
        } break;
        case kind_index:
        case kind_dot:
        {
          lhs = translate_lvalue( bb, context, isConstant );
          if( lhs == NULL ) goto crash_n_burn;
          result = new LoadInst( lhs, false, bb );
        } break;
        case kind_call:
        {
          // only works with static functions
          //assert( getKid( 0 )->getKind() == expr_t::kind_deref && getKid( 0 )->getKid( 0 )->getKind() == expr_t::kind_value_ptr );
          assert( getKid( 0 )->getKind() == expr_t::kind_value );
          
          value_t *firstValue = getKid( 0 )->getValue();
          assert( firstValue->getType()->getKind() == type_t::kind_function );
          cerr << context.indent() << "translating call to function " << firstValue->getName().data() << "\n";

          std::vector< expr_t * > valuePtrs;
          std::vector< Value * > llvmActuals;
          std::vector< const Type * > llvmTypes;
          
          std::vector< type_t * > actualTypes;
          if( getKid( 1 )->getKind() != expr_t::kind_comma )
          {
            actualTypes.push_back( getKid( 1 )->getType() );
            valuePtrs.push_back( getKid( 1 ) );
          } else
          {
            for( expr_t *expr = getKid( 1 ); ; expr = expr->getKid( 0 ) )
            {
              if( expr->getKind() == expr_t::kind_comma )
              {
                actualTypes.push_back( expr->getKid( 1 )->getType() );
                valuePtrs.push_back( expr->getKid( 1 ) );
              } else
              {
                actualTypes.push_back( expr->getType() );
                valuePtrs.push_back( expr );
                break;
              }
            }
          }
          
          for( std::vector< type_t * >::iterator i = actualTypes.begin(); i != actualTypes.end(); ++i )
          {
            const Type *type = (*i)->translate( context );
            if( type == NULL ) goto crash_n_burn;
            llvmTypes.push_back( type );
          }
          
          value_t *value = NULL;
          for( value_t *search = firstValue; search != NULL; search = search->getNextPolymorphic() )
          {
            unsigned score = doParametersMatch( actualTypes, search );
            if( score == 2 )
            {
              // exact match. ignore any others.
              value = search;
              break;
            }
            if( score == 1 )
            {
              // partial match, will need casts.
              if( value != NULL )
              {
                cerr << context.errorText() << "more than one function matches parameters\n";
                goto crash_n_burn;
              }
              value = search;
            }
          }
          
          if( value == NULL )
          {
            cerr << context.errorText() << "none of the functions called " << firstValue->getName() << " match the parameters\n";
            goto crash_n_burn;
          }
          
          type_t *functionType = value->getType();
          
          Value *llvmFunction = value->getLlvmValue();
          assert( llvmFunction != NULL );
          
          type_t *returnType = functionType->getSubType();
          Value *returnPtr = NULL;

          if( returnType->getIsPassByPtr() )
          {
            const Type *llvmType = returnType->translate( context );
            if( llvmType == NULL ) goto crash_n_burn;
            returnPtr = new AllocaInst( llvmType, "retValue", bb );
            llvmActuals.push_back( returnPtr );
          }
          
          std::vector< type_t * >::iterator actualType = actualTypes.begin();
          std::vector< expr_t * >::iterator valuePtr = valuePtrs.begin();
          std::vector< Value * > llvmLoads;
          //scope_t::iterator valueIterator = functionType->getScope()->begin();
          
          while( actualType != actualTypes.end() )
          {
            type_t *cgActualType = actualType[0];
            const Type *llvmType = cgActualType->translate( context );
            if( llvmType == NULL ) goto crash_n_burn;
            if( cgActualType->getIsPassByPtr() )
            {
              Value *ptr = new AllocaInst( llvmType, "type", bb );
              llvmActuals.push_back( ptr );
              if( cgActualType->getIsIn() || !cgActualType->getIsOut() )
              {
                Value *actual = valuePtr[0]->translate_rvalue( bb, context, isConstant );
                // todo insert cast here
                Value *store = new StoreInst( actual, ptr, false, 16, bb );
                cerr << *store << "\n";
              }
              if( actualType[0]->getIsOut() )
              {
                llvmLoads.push_back( ptr );
              }
            } else
            {
              llvmActuals.push_back( valuePtr[0]->translate_rvalue( bb, context, isConstant ) );
            }
            ++actualType;
            ++valuePtr;
          }
          
          Value *call = CallInst::Create( llvmFunction, llvmActuals.begin(), llvmActuals.end(), "", bb );
          cerr << *call << "\n";
          
          if( returnType->getIsPassByPtr() )
          {
            if( returnType != NULL )
            {
              const Type *llvmType = returnType->translate( context );
              if( llvmType == NULL ) goto crash_n_burn;
              result = new LoadInst( returnPtr, false, bb );
            }
          } else
          {
            result = call;
          }
        } break;
        //case kind_index:
        {
        } break;
        case kind_swiz:
        {
          // to do, insert swizzle operator.
          // make a cast operation.
          lhs = getKid( 0 )->translate_rvalue( bb, context, isConstant );
          if( lhs == NULL ) goto crash_n_burn;

          std::vector< Constant * >indices;
          unsigned i, destDim = type->getDimension();
          
          // single swizzle is a scalar
          if( type->getIsScalar() )
          {
            if( isConstant )
            {
              result = ConstantExpr::getExtractElement( cast<Constant>( lhs ), ConstantInt::get( Type::Int32Ty, swiz[ 0 ] ) );
            } else
            {
              result = new ExtractElementInst( lhs, ConstantInt::get( Type::Int32Ty, swiz[ 0 ] ), "", bb );
            }
          } else
          {
            for( i = 0; i != destDim; ++i )
            {
              indices.push_back( ConstantInt::get( Type::Int32Ty, swiz[ i ] ) );
            }
            for( ; i != 4; ++i )
            {
              indices.push_back( ConstantInt::get( Type::Int32Ty, swiz[ destDim-1 ] ) );
            }
            result = new ShuffleVectorInst( lhs, lhs, ConstantVector::get( indices ), "", bb );
          }
        } break;
        case kind_int_value:
        {
          result = ConstantInt::get( Type::Int32Ty, int_value );
        } break;
        case kind_double_value:
        {
          result = ConstantFP::get( Type::FloatTy, double_value );
        } break;
        case kind_cast:
        {
          lhs = getKid( 0 )->translate_rvalue( bb, context, isConstant );
          if( lhs == NULL ) goto crash_n_burn;
          
          cerr << "lhs " << *lhs << "\n";
          cerr << "this " << *this->getType() << "\n";
          cerr << "kid0 " << *kid[0]->getType() << "\n";
          
          result = translateCast( lhs, getKid( 0 )->getType(), type, bb, context );
        } break;
        //case kind_for:
        {
        } break;
        //case kind_init:
        {
        } break;
        /*case kind_deref:
        {
          lhs = getKid( 0 )->translate( bb, context );
          if( lhs == NULL ) goto crash_n_burn;

          cerr << *lhs->getType() << "\n";
          result = new LoadInst( lhs, false, bb );
        } break;*/
        case kind_comma:
        {
          getKid( 0 )->translate_rvalue( bb, context, isConstant );
          result = getKid( 1 )->translate_rvalue( bb, context, isConstant );
        } break;
        case kind_question:
        {
          lhs = getKid( 0 )->translate_rvalue( bb, context, isConstant );
          if( lhs == NULL ) goto crash_n_burn;
          rhs = getKid( 1 )->translate_rvalue( bb, context, isConstant );
          if( rhs == NULL ) goto crash_n_burn;
          rhs2 = getKid( 2 )->translate_rvalue( bb, context, isConstant );
          if( rhs2 == NULL ) goto crash_n_burn;
          result = SelectInst::Create( lhs, rhs, rhs2, "", bb );
          /*if( getKid( 0 )->getType()->getKind() == type_t::kind_bool )
          {
            result = new SelectInst( rhs, rhs2, lhs, "", bb );
          } else if( getKid( 0 )->getType()->getIsVector() )
          {
          }*/
        } break;
        case kind_lt: ipred = ICmpInst::ICMP_SLT; fpred = FCmpInst::FCMP_OLT; goto relop;
        case kind_gt: ipred = ICmpInst::ICMP_SGT; fpred = FCmpInst::FCMP_OGT; goto relop;
        case kind_le: ipred = ICmpInst::ICMP_SLE; fpred = FCmpInst::FCMP_OLE; goto relop;
        case kind_ge: ipred = ICmpInst::ICMP_SGE; fpred = FCmpInst::FCMP_OGE; goto relop;
        case kind_eq: ipred = ICmpInst::ICMP_EQ;  fpred = FCmpInst::FCMP_OEQ; goto relop;
        case kind_ne: ipred = ICmpInst::ICMP_NE;  fpred = FCmpInst::FCMP_ONE; goto relop;
        relop:
        {
          lhs = getKid( 0 )->translate_rvalue( bb, context, isConstant );
          if( lhs == NULL ) goto crash_n_burn;
          rhs = getKid( 1 )->translate_rvalue( bb, context, isConstant );
          if( rhs == NULL ) goto crash_n_burn;
          if(
            getKid( 0 )->getType()->getIsScalar() && getKid( 0 )->getType()->getIsInt() ||
            getKid( 0 )->getType()->getIsVector() && getKid( 0 )->getType()->getSubType()->getIsInt()
          )
          {
            result = new ICmpInst( ipred, lhs, rhs, "", bb );
          } else if(
            getKid( 0 )->getType()->getIsScalar() && getKid( 0 )->getType()->getIsFloat() ||
            getKid( 0 )->getType()->getIsVector() && getKid( 0 )->getType()->getSubType()->getIsFloat()
          )
          {
            result = new FCmpInst( fpred, lhs, rhs, "", bb );
          } else
          {
            cerr << context.errorText() << "unsupported compare\n";
            goto crash_n_burn;
          }
        } break;
        //case kind_shift_left:
        {
        } break;
        //case kind_shift_right:
        {
        } break;
        case kind_plus: op = BinaryOperator::Add; goto binop;
        case kind_minus: op = BinaryOperator::Sub; goto binop;
        case kind_or: op = BinaryOperator::Or; goto binop;
        case kind_and: op = BinaryOperator::And; goto binop;
        case kind_xor: op = BinaryOperator::Xor; goto binop;
        case kind_star: op = BinaryOperator::Mul; goto binop;
        case kind_divide: op = BinaryOperator::FDiv; goto binop;
        case kind_mod: op = BinaryOperator::FRem; goto binop;
        binop:
        {
          lhs = getKid( 0 )->translate_rvalue( bb, context, isConstant );
          if( lhs == NULL ) goto crash_n_burn;
          rhs = getKid( 1 )->translate_rvalue( bb, context, isConstant );
          if( rhs == NULL ) goto crash_n_burn;
          if( isConstant )
          {
            Constant *clhs = cast<Constant>( lhs );
            Constant *crhs = cast<Constant>( rhs );
            switch( kind )
            {
              case kind_plus: result = ConstantExpr::getAdd( clhs, crhs ); break;
              case kind_minus: result = ConstantExpr::getSub( clhs, crhs ); break;
              case kind_or: result = ConstantExpr::getOr( clhs, crhs ); break;
              case kind_and: result = ConstantExpr::getAnd( clhs, crhs ); break;
              case kind_xor: result = ConstantExpr::getXor( clhs, crhs ); break;
              case kind_star: result = ConstantExpr::getMul( clhs, crhs ); break;
              case kind_divide: result = ConstantExpr::getFDiv( clhs, crhs ); break;
              case kind_mod: result = ConstantExpr::getFRem( clhs, crhs ); break;
            }
          } else
          {
            result = BinaryOperator::create( op, lhs, rhs, "", bb );
          }
        } break;
        //case kind_or_or:
        {
        } break;
        //case kind_and_and:
        {
        } break;
        case kind_equals:
        {
          if( isConstant )
          {
            cerr << context.errorText() << "assigment can't be used in constant expression\n";
            return NULL;
          }
          rhs = getKid( 1 )->translate_rvalue( bb, context, isConstant );
          if( rhs == NULL ) goto crash_n_burn;

          if( getKid( 0 )->getKind() == kind_swiz )
          {
            expr_t *swiz = getKid( 0 );
            Value *ptr = swiz->getKid( 0 )->translate_lvalue( bb, context, false );
            if( ptr == NULL ) goto crash_n_burn;
            cerr << *ptr << "\n";
            
            Value *insert = NULL;
            if( swiz->getType()->getIsScalar() )
            {
              Value *load = new LoadInst( ptr, "", bb );
              cerr << *load << "\n";
              insert = InsertElementInst::Create( load, rhs, ConstantInt::get( Type::Int32Ty, swiz->getSwiz()[0] ), "", bb );
            } else if( swiz->getType()->getIsVector() )
            {
              unsigned dim = swiz->getType()->getDimension();
              Value *load = new LoadInst( ptr, "", bb );
              cerr << *load << "\n";
              std::vector< Constant * > indices;
              for( unsigned i = 0; i != 4; ++i )
              {
                indices.push_back( ConstantInt::get( Type::Int32Ty, i ) );
              }
              for( unsigned i = 0; i != dim; ++i )
              {
                indices[ swiz->getSwiz()[ i ] & 3 ] = ConstantInt::get( Type::Int32Ty, i + 4 );
              }
              insert = new ShuffleVectorInst( load, rhs, ConstantVector::get( indices ), "", bb );
            } else
            {
              cerr << context.errorText() << "only swizzle of scalar or vector supported\n";
              goto crash_n_burn;
            }
            cerr << *insert << "\n";
            Value *store = new StoreInst( insert, ptr, false, 16, bb );
            cerr << *store << "\n";
            result = rhs;
          } else if( getKid( 0 )->getKind() == kind_cast )
          {
            // todo
            result = rhs;
          } else
          {
            Value *ptr = getKid( 0 )->translate_lvalue( bb, context, false );
            if( ptr == NULL )
            {
              goto crash_n_burn;
            }
            Value *store = new StoreInst( rhs, ptr, false, 16, bb );
            cerr << *store << "\n";
            result = rhs;
          }
        } break;
        default:
        {
          cerr << context.errorText() << "not implemented\n";
          return NULL;
        }
      }
      if( result != NULL )
      {
        if( trace_translate )
        {
          cerr << context.pop() << "/rvalue " << kindName( kind ) << "" << *result << " [" << *llvmType << "/" << *result->getType() << "]\n";
        }
        assert( llvmType == result->getType() );
        return result;
      }
    crash_n_burn:
      if( trace_translate )
      {
        cerr << context.pop() << "error\n";
      }
      return NULL;
    }
  };
  
  inline bool translate_scope( scope_t *scope, BasicBlock *&bb, fc::default_context &context );
  
  void copy( BasicBlock *bb, fc::default_context &context, Value *dest, Value *src )
  {
    const Type *srcType = src->getType();
    const Type *destType = dest->getType();
    assert( destType == srcType );
    if( isa< ArrayType >( srcType->getContainedType(0) ) )
    {
      assert( 0 );
    } else if( isa< StructType >( srcType->getContainedType(0) ) )
    {
      unsigned numElements = srcType->getContainedType(0)->getNumContainedTypes();
      for( unsigned i = 0; i != numElements; ++i )
      {
        Value *indices[ 2 ];
        indices[ 0 ] = ConstantInt::get( Type::Int32Ty, 0 );
        indices[ 1 ] = ConstantInt::get( Type::Int32Ty, i );
        Value *srcPtr = GetElementPtrInst::Create( src, indices, indices + 2, "", bb );
        Value *destPtr = GetElementPtrInst::Create( dest, indices, indices + 2, "", bb );
        copy( bb, context, destPtr, srcPtr );
      }
    } else
    {
      Value *load = new LoadInst( src, "", bb );
      new StoreInst( load, dest, false, bb );
    }
  }

  
  class stmt_t
  {
  public:
    enum kind_enum
    {
      kind_compound,
      kind_expression,
      kind_declaration,
      kind_return,
      kind_discard,
      kind_if,
      kind_for,
      kind_while,
      kind_dowhile,
    };
  private:
    kind_enum kind;
    union
    {
      stmt_t *statements;
      expr_t *expression;
    };
    stmt_t *next;
    scope_t *scope;
  public:
    stmt_t( kind_enum kind_ ) : kind( kind_ ), statements( NULL ), next( NULL )
    {
    }
    
    stmt_t **getStatementsAddr()
    {
      return &statements;
    }

    stmt_t *getStatements()
    {
      return statements;
    }

    expr_t *getExpression()
    {
      return expression;
    }

    void setExpression( expr_t *expr )
    {
      expression = expr;
    }
    
    stmt_t **getNextAddr()
    {
      return &next;
    }

    stmt_t *getNext()
    {
      return next;
    }

    scope_t *getScope()
    {
      return scope;
    }

    void setScope( scope_t *scope_ )
    {
      scope = scope_;
    }
    
    BasicBlock *translate( BasicBlock *bb, fc::default_context &context, type_t *functionType )
    {
      switch( kind )
      {
        case kind_compound:
        {
          if( !translate_scope( scope, bb, context ) )
          {
            goto crash_n_burn;
          }
          //cerr << context.push() << "kind_compound\n";
          for( stmt_t *statement = statements; statement != NULL; statement = statement->next )
          {
            bb = statement->translate( bb, context, functionType );
            if( bb == NULL ) goto crash_n_burn;
          }
          //cerr << context.pop() << "/kind_compound\n";
        } break;
        case kind_expression:
        {
          //cerr << context.push() << "kind_expression\n";
          if( expression != NULL )
          {
            Value *expr = expression->translate_rvalue( bb, context, false );
            if( expr == NULL ) goto crash_n_burn;
          }
          //cerr << context.pop() << "/kind_expression\n";
        } break;
        case kind_declaration:
        {
          //cerr << context.push() << "kind_declaration\n";
          if( expression != NULL )
          {
            Value *expr = expression->translate_rvalue( bb, context, false );
            if( expr == NULL ) goto crash_n_burn;
          }
          //cerr << context.pop() << "/kind_declaration\n";
        } break;
        case kind_return:
        {
          //cerr << context.push() << "kind_return\n";
          Value *ret = NULL;
          if( expression != NULL )
          {
            if( functionType->getIsReturnByValue() )
            {
              Value *src = expression->translate_lvalue( bb, context, false );
              Argument *arg = bb->getParent()->getArgumentList().begin();
              Value *dest = cast< Value>( arg );
              copy( bb, context, dest, src );
              ret = ReturnInst::Create( NULL, bb );
            } else
            {
              Value *expr = expression->translate_rvalue( bb, context, false );
              if( expr == NULL ) goto crash_n_burn;
              ret = ReturnInst::Create( expr, bb );
            }
          } else
          {
            ret = ReturnInst::Create( NULL, bb );
          }
          //cerr << *ret << "\n";
          //cerr << context.pop() << "/kind_return\n";
        } break;
        case kind_discard:
        {
          //cerr << context.push() << "kind_discard\n";
          //cerr << context.pop() << "/kind_discard\n";
        } break;
        case kind_if:
        {
          //cerr << context.push() << "kind_if\n";
          //cerr << context.pop() << "/kind_if\n";
        } break;
        case kind_for:
        {
          //cerr << context.push() << "kind_for\n";
          //cerr << context.pop() << "/kind_for\n";
        } break;
        case kind_while:
        {
          //cerr << context.push() << "kind_while\n";
          //cerr << context.pop() << "/kind_while\n";
        } break;
        case kind_dowhile:
        {
          //cerr << context.push() << "kind_dowhile\n";
          //cerr << context.pop() << "/kind_dowhile\n";
        } break;
        default:
        {
          assert( 0 && "unknown statement kind" );
        } break;
      }
      return bb;
    crash_n_burn:
      //cerr << context.pop() << "error\n";
      return NULL;
    }
  };
  
  inline BasicBlock *translate_statement( stmt_t *statement, BasicBlock *bb, fc::default_context &context, type_t *functionType )
  {
    return statement->translate( bb, context, functionType );
  }
  
  class scope_t
  {
  public:
    typedef std::map< std::string, value_t * > valuesType;
    typedef valuesType::value_type valuePair;
    typedef valuesType::iterator valueIterator;
    typedef std::vector< value_t * > orderType;
  private:
    orderType valuesByOrder;
    valuesType valuesByName;
    scope_t *parent;
  public:
    typedef std::vector< value_t * >::iterator iterator;

    scope_t( scope_t *parent_ ) : parent( parent_ )
    {
    }
    
    size_t size()
    {
      return valuesByOrder.size();
    }
    
    iterator begin()
    {
      return valuesByOrder.begin();
    }
    
    iterator end()
    {
      return valuesByOrder.end();
    }
    
    void addValue( value_t *value )
    {
      valuesByName.insert( valuePair( value->getName(), value ) );
      valuesByOrder.push_back( value );
    }
    
    value_t *getExistingValue( value_t *value )
    {
      valueIterator i = valuesByName.find( value->getName() );
      return i != valuesByName.end() ? i->second : NULL;
    }
    
    value_t *lookup( const std::string &name )
    {
      for( scope_t *scope = this; scope != NULL; scope = scope->parent )
      {
        valueIterator i = scope->valuesByName.find( name );
        if( i != scope->valuesByName.end() )
        {
          return i->second;
        }
      }
      return NULL;
    }

    void translateGlobal( Module *module, fc::default_context &context )
    {
      for( iterator i = begin(); i != end(); ++i )
      {
        value_t *value = *i;
        const Type *llvmType = value->getType()->translate( context );
        type_t::kind_enum kind = value->getType()->getKind();
        if( kind != type_t::kind_function )
        {
          GlobalVariable *var = new GlobalVariable( llvmType, value->getType()->getIsConst(), GlobalValue::InternalLinkage, 0, value->getName(), module, false, 0 );
          value->setLlvmValue( var );
          if( value->getInit() )
          {
            BasicBlock *bb = NULL;
            Constant *constValue = cast< Constant >( value->getInit()->translate_rvalue( bb, context, true ) );
            var->setInitializer( constValue );
          } else
          {
            var->setInitializer( Constant::getNullValue( var->getType()->getElementType() ) );
          }
        }
      }
    }

    bool translateFunctionHeaders( Module *module, fc::default_context &context )
    {
      for( scope_t::iterator i = begin(); i != end(); ++i )
      {
        value_t *value = *i;

        do
        {        
          const Type *llvmType = value->getType()->translate( context );
          type_t::kind_enum kind = value->getType()->getKind();
          
          if( kind == type_t::kind_function )
          {
            const Type *returnType = value->getType()->getSubType()->translate( context );
            cerr << "processing header " << *value << "\n";
            GlobalValue::LinkageTypes linkage = GlobalValue::ExternalLinkage;
            if( value->getInit() && value->getInit()->getKind() == expr_t::kind_statement )
            {
              linkage = GlobalValue::InternalLinkage;
            }

            std::string name = value->getName();
            if( name == "main" )
            {
              name = "$main";
              linkage = GlobalValue::ExternalLinkage;
            }

            Function *function = Function::Create( cast<FunctionType>( llvmType ), linkage, name, module );
            function->setCallingConv( CallingConv::C );
            value->setLlvmValue( function );
            cerr << "  " << *function << "\n";
          }
          value = value->getNextPolymorphic();
        } while( value != NULL );
      }
      return true;
    }
    
    bool translateFunctionBodies( Module *module, fc::default_context &context )
    {
      for( scope_t::iterator i = begin(); i != end(); ++i )
      {
        value_t *value = *i;

        do
        {
          const Type *llvmType = value->getType()->translate( context );
          type_t::kind_enum kind = value->getType()->getKind();
          if( kind == type_t::kind_function )
          {
            cerr << "processing function " << *value << "\n";
            Value *llvmValue = value->getLlvmValue();
            assert( llvmValue != NULL && "expected non-null value" );
            Function *function = cast<Function>( llvmValue );
            if( value->getInit() && value->getInit()->getKind() == expr_t::kind_statement )
            {
              BasicBlock *bb = BasicBlock::Create( "entry", function );
              scope_t *paramScope = value->getType()->getScope();
              paramScope->translateFormalParams( bb, context, value->getType(), function );
              if( !value->getInit()->getStatement()->translate( bb, context, value->getType() ) )
              {
                return false;
              }
              if( bb->getTerminator() == NULL )
              {
                const Type *returnType = function->getType()->getContainedType(0)->getContainedType(0);
                cerr << *returnType << "\n";
                if( returnType == Type::VoidTy )
                {
                  ReturnInst::Create( bb );
                } else
                {
                  ReturnInst::Create( Constant::getNullValue( returnType ), bb );
                }
              }
            }
          }
          value = value->getNextPolymorphic();
        } while( value != NULL );
      }
      return true;
    }

    void translateFormalParams( BasicBlock *&bb, fc::default_context &context, type_t *cgFunctionType, Function *function )
    {
      Function::ArgumentListType &argList = function->getArgumentList();
      Function::ArgumentListType::iterator argp = argList.begin();
      iterator valuep = begin();
      for( ; argp != argList.end() && valuep != end(); ++valuep, ++argp )
      {
        value_t *cgValue = *valuep;
        type_t *cgType = cgValue->getType();
        const Type *llvmType = cgType->translate( context );
        Value *llvmFormal = (Value*)argp;
        if( trace_translate )
        {
          cerr << "arg " << *cgValue << " llvmt: " << *llvmType << "\n";
        }
        if( cgType->getIsPassByPtr() )
        {
          // llvm arg type will be a pointer, cg arg type is an out, struct or array
          // we use the pointer itself.
          assert( llvmType == argp->getType()->getContainedType(0) && "wrong type" );
          cgValue->setLlvmValue( argp );
        } else
        {
          // llvm arg type is same as cg arg type. we create a variable
          // and store the formal arg in it.
          assert( llvmType == argp->getType() && "wrong type" );
          Value *result = new AllocaInst( llvmType, cgValue->getName(), bb );
          Value *store = new StoreInst( llvmFormal, result, false, 16, bb );
          cgValue->setLlvmValue( result );
        }
      }
      assert( argp == argList.end() && valuep == end() && "translateFormalParams" );
    }

    bool translateLocal( BasicBlock *&bb, fc::default_context &context )
    {
      for( iterator i = begin(); i != end(); ++i )
      {
        value_t *value = *i;
        const Type *llvmType = value->getType()->translate( context );
        type_t::kind_enum kind = value->getType()->getKind();
        Value *llvmValue = new AllocaInst( llvmType, value->getName(), bb );
        value->setLlvmValue( llvmValue );
      }
      return true;
    }
  };

  inline bool translate_scope( scope_t *scope, BasicBlock *&bb, fc::default_context &context )
  {
    return scope->translateLocal( bb, context );
  }
  
  // a very large can of worms that converts one type into another.
  Value *translateCast( Value *llvmSrc, type_t *srcType, type_t *destType, BasicBlock *bb, fc::default_context &context )
  {
    Value *result = NULL;
    const Type *llvmSrcType = srcType->translate( context );
    const Type *llvmDestType = destType->translate( context );
    assert( llvmSrcType == llvmSrc->getType() );

    if( *srcType == *destType )
    {
      // no work to do.
      result = llvmSrc;
    } else if( srcType->getIsScalar() && destType->getIsVector() )
    {
      const Type *scalarType = destType->getSubType()->translate( context );
      std::vector< Constant * > indices;
      
      // cast to destination's scalar type first. eg. int -> float -> float4
      if( scalarType != llvmSrcType )
      {
        llvmSrc = translateCast( llvmSrc, srcType, destType->getSubType(), bb, context );
      }

      if( isa<Constant>( llvmSrc ) )
      {
        for( unsigned i = 0; i != 4; ++i )
        {
          indices.push_back( cast<Constant>( llvmSrc ) );
        }
        result = ConstantVector::get( indices );
      } else
      {
        Value *insert = InsertElementInst::Create( Constant::getNullValue( llvmDestType ), llvmSrc, Constant::getNullValue( Type::Int32Ty ), "", bb );
        result = new ShuffleVectorInst( insert, insert, Constant::getNullValue( VectorType::get( Type::Int32Ty, 4 ) ), "", bb );
      }
    } else if( srcType->getIsVector() && destType->getIsScalar() )
    {
      if( srcType->getDimension() != 1 )
      {
        cerr << context.errorText() << "cast from vector to scalar\n";
        result = NULL;
      } else
      {
        result = new ExtractElementInst( llvmSrc, Constant::getNullValue( Type::Int32Ty ), "", bb );
      }
    } else if( srcType->getIsScalar() && destType->getIsScalar() )
    {
      // scalar to scalar conversion
      Instruction::CastOps op = CastInst::getCastOpcode( llvmSrc, true, llvmDestType, true );
      result = CastInst::create( op, llvmSrc, llvmDestType, "", bb );
    } else if( srcType->getIsVector() && destType->getIsVector() )
    {
      // eg. vec3 -> vec4  0123
      //     vec4 -> vec3  0122
      //     vec1 -> vec4  0000
      //     vec2 -> vec4  0111
      //     vec4 -> vec2  0111
      std::vector< Constant * > indices;
      int srcDim = srcType->getDimension();
      int destDim = destType->getDimension();
      if( srcDim >= destDim )
      {
        result = llvmSrc;
      } else
      {
        // smear last element
        for( int i = 0; i != 4; ++i )
        {
          int index = i < srcDim ? i : srcDim;
          indices.push_back( ConstantInt::get( Type::Int32Ty, index ) );
        }
        result = new ShuffleVectorInst( llvmSrc, llvmSrc, ConstantVector::get( indices ), "", bb );
      }
    } else
    {
      cerr << "unable to cast from " << *srcType << " to " << *destType << "\n";
      result = NULL;
    }
    return result;
  }

  inline void getScopeTypes( std::vector< const Type * > &params, scope_t *scope, bool isParameterList, fc::default_context &context )
  {
    for( scope_t::iterator i = scope->begin(); i != scope->end(); ++i )
    {
      type_t *type = i[0]->getType();
      const Type *llvmParam = type->translate( context );
      if( isParameterList && type->getIsPassByPtr() )
      {
        params.push_back( PointerType::get( llvmParam, 0 ) );
      } else
      {
        params.push_back( llvmParam );
      }
    }
  }

  // type equivalence
  bool operator == ( type_t &lhs, type_t &rhs )
  {
    if( &lhs == &rhs )
    {
      return true;
    }
    if( lhs.getKind() != rhs.getKind() )
    {
      return false;
    }

    scope_t *lscope = lhs.getScope();
    scope_t *rscope = rhs.getScope();
    type_t *lsub = lhs.getSubType();
    type_t *rsub = rhs.getSubType();
    if( lhs.getKind() == type_t::kind_array )
    {
      if( lhs.getDimension() != rhs.getDimension() )
      {
        return false;
      }
      if( lsub->getIsPacked() != rsub->getIsPacked() )
      {
        return false;
      }
    } else if( lscope != NULL || rscope != NULL )
    {
      if( lscope == NULL || rscope == NULL || lscope->size() != rscope->size() )
      {
        return false;
      }
      if( lhs.getKind() == type_t::kind_struct )
      {
        if( lscope != rscope )
        {
          return false;
        }
        if( lhs.getIsPacked() != rhs.getIsPacked() )
        {
          return false;
        }
      } else
      {
        // check all the types in the scope. ie. function params
        scope_t::iterator l = lscope->begin();
        scope_t::iterator r = rscope->begin();
        while( l != lscope->end() )
        {
          if( !( *l[0]->getType() == *r[0]->getType() ) )
          {
            return false;
          }
          l++;
          r++;
        }
      }
    }

    if( lsub != NULL || rsub != NULL )
    {
      if( lsub == NULL || rsub == NULL )
      {
        return false;
      }
      if( !( *lsub == *rsub ) )
      {
        return false;
      }
    }
    
    return true;
  }
  
  // can cast rhs to lhs?
  bool canCastTo( type_t *lhs, type_t *rhs )
  {
    if( *lhs == *rhs )
    {
      return true;
    }
    
    return lhs->getIsScalar() && rhs->getIsScalar();
  }

  // debug  
  OStream &operator <<( OStream &in, const type_t &type )
  {
    if( type.getIsConst() )
    {
      in << "const ";
    }
    if( type.getIsPacked() )
    {
      in << "packed ";
    }
    if( type.getIsUniform() )
    {
      in << "uniform ";
    }
    if( type.getIsIn() )
    {
      if( type.getIsOut() )
      {
        in << "inout ";
      } else
      {
        in << "in ";
      }
    }
    if( type.getIsOut() )
    {
      in << "out ";
    }

    switch( type.getKind() )
    {
      case type_t::kind_struct:
        in << "struct { } ";
        break;
      case type_t::kind_array:
        in << *type.getSubType() << "[ " << type.getDimension() << " ] ";
        break;
      case type_t::kind_function:
      {
        in << *type.getSubType();
        scope_t *scope = type.getScope();
        char *comma = "( ";
        for( scope_t::iterator i = scope->begin(); i != scope->end(); ++i )
        {
          in << comma << **i;
          comma = ", ";
        }
        in << " )";
      } break;
      case type_t::kind_float: in << "float "; break;
      case type_t::kind_half: in << "half "; break;
      case type_t::kind_bool: in << "bool "; break;
      case type_t::kind_void: in << "void "; break;
      case type_t::kind_int: in << "int "; break;
      case type_t::kind_cfloat: in << "cfloat "; break;
      case type_t::kind_cint: in << "cint "; break;
      case type_t::kind_sampler: in << "sampler "; break;
      case type_t::kind_sampler1D: in << "sampler1D "; break;
      case type_t::kind_sampler2D: in << "sampler2D "; break;
      case type_t::kind_sampler3D: in << "sampler3D "; break;
      case type_t::kind_samplerRECT: in << "samplerRECT "; break;
      case type_t::kind_samplerCUBE: in << "samplerCUBE "; break;
      default: in << "type(" << type.getKind() << ")"; break;
    }
    return in;
  }
  
  OStream &operator <<( OStream &in, const value_t &value )
  {
    return in << *value.getType() << value.getName();
  }

  inline OStream &operator <<( OStream &in, scope_t &scope )
  {
    in << "{";
    const char *semi = " ";
    for( scope_t::orderType::iterator i = scope.begin(); i != scope.end(); ++i )
    {
      in << semi << **i;
      semi = "; ";
    }
    in << "}";
    return in;
  }

  unsigned doParametersMatch( std::vector< type_t * > &actualTypes, value_t *search )
  {
    type_t *searchType = search->getType();
    assert( searchType->getKind() == type_t::kind_function );
    cerr << *searchType << "\n";
    if( searchType->getScope()->size() != actualTypes.size() )
    {
      return 0;
    }

    scope_t *scope = searchType->getScope();
    //cerr << "num params ok\n";
    std::vector< type_t * >::iterator actualType = actualTypes.begin();
    
    size_t numExact = 0;
    size_t numPartial = 0;
    for( scope_t::iterator formal = scope->begin(); formal != scope->end(); ++formal, ++actualType )
    {
      type_t *formalType = (*formal)->getType();
      //cerr << *formalType << " vs " << **actualType << "\n";
      if( *formalType == **actualType )
      { 
        numExact++;
      } else if( canCastTo( formalType, *actualType ) )
      {
        numPartial++;
      }
    }
    //cerr << numExact << " exact    " << numPartial << "partial\n";
    return numExact == actualTypes.size() ? 2 : numPartial != 0 ? 1 : 0;
  }

  class compiler : public fc::default_context, public fc::cplusplus_token_type
  {
    fc::preprocessor< default_context > preprocessor;
    fc::lexer< default_context, fc::cplusplus_tokens > lexer;
    int line_number;
    unsigned curToken;
    scope_t *invariantScope;
    scope_t *curScope;
    unsigned structNumber;
    unsigned numTmpVars;
    unsigned numAbstract;
    
    //Module *curModule;
    //Function *curFunction;
    //BasicBlock *curBasicBlock;
    //type_t *curFunctionType;

    type_t *voidType;
    type_t *boolType;
    type_t *intType;
    type_t *floatType;
    type_t *doubleType;
    type_t *halfType;
    type_t *cintType;
    type_t *cfloatType;
    type_t *boolTypes1D[ 4 ];
    type_t *boolTypes2D[ 4 ][ 4 ];
    type_t *intTypes1D[ 4 ];
    type_t *intTypes2D[ 4 ][ 4 ];
    type_t *floatTypes1D[ 4 ];
    type_t *floatTypes2D[ 4 ][ 4 ];
    type_t *halfTypes1D[ 4 ];
    type_t *halfTypes2D[ 4 ][ 4 ];
    
    struct intrinsic_t
    {
      const char name[ 16 ];
      const char *params;
    };
    
    typedef std::map< std::string, type_t * > typedefType;
    typedef typedefType::value_type typedefPair;
    typedef typedefType::iterator typedefIterator;
    
    typedef std::map< std::string, value_t * > valuesType;
    typedef valuesType::value_type valuePair;
    typedef valuesType::iterator valueIterator;
    
    valuesType values;
    typedefType typedefs;
    typedefType tags;

    type_t *findTypedef( const std::string &name )
    {
      typedefIterator i = typedefs.find( name );
      if( i != typedefs.end() )
      {
        return i->second;
      }
      return 0;
    }
    
    type_t *makeTypedef( type_t *type, const std::string &name )
    {
      typedefPair pair( name, type );
      typedefs.insert( pair );
      return type;
    }
    
    type_t *findTag( const std::string &name )
    {
      typedefIterator i = tags.find( name );
      if( i != tags.end() )
      {
        return i->second;
      }
      return 0;
    }
    
    type_t *makeTag( type_t *type, const std::string &name )
    {
      typedefPair pair( name, type );
      tags.insert( pair );
      return type;
    }

    typedef fc::cplusplus_tokens tokens;
    
    unsigned idToKeyword( uint64_type value_t )
    {
      //fprintf( stderr, "[%llx]\n", value_t );
      switch( value_t )
      {
        case 0x61736dll: return tok_asm;
        case 0x667261676d656e74ll: return !strcmp( lexer.id(), "asm_fragment" ) ? tok_asm_fragment : !strcmp( lexer.id(), "pixelfragment" ) ? tok_pixelfragment : !strcmp( lexer.id(), "vertexfragment" ) ? tok_vertexfragment : tok_identifier;
        case 0x6175746fll: return tok_auto;
        case 0x626f6f6cll: return tok_bool;
        case 0x627265616bll: return tok_break;
        case 0x63617365ll: return tok_case;
        case 0x6361746368ll: return tok_catch;
        case 0x63686172ll: return tok_char;
        case 0x636c617373ll: return tok_class;
        case 0x636f6c756d6ell: return tok_column;
        case 0x6d616a6f72ll: return tok_major;
        case 0x636f6d70696c65ll: return tok_compile;
        case 0x636f6e7374ll: return tok_const;
        case 0x6e73745f63617374ll: return !strcmp( lexer.id(), "const_cast" ) ? tok_const_cast : tok_identifier;
        case 0x636f6e74696e7565ll: return !strcmp( lexer.id(), "continue" ) ? tok_continue : tok_identifier;
        case 0x6465636cll: return tok_decl;
        case 0x64656661756c74ll: return tok_default;
        case 0x64656c657465ll: return tok_delete;
        case 0x64697363617264ll: return tok_discard;
        case 0x646fll: return tok_do;
        case 0x646f75626c65ll: return tok_double;
        case 0x64776f7264ll: return tok_dword;
        case 0x6d69635f63617374ll: return !strcmp( lexer.id(), "dynamic_cast" ) ? tok_dynamic_cast : tok_identifier;
        case 0x656c7365ll: return tok_else;
        case 0x656d6974ll: return tok_emit;
        case 0x656e756dll: return tok_enum;
        case 0x6578706c69636974ll: return !strcmp( lexer.id(), "explicit" ) ? tok_explicit : tok_identifier;
        case 0x65787465726ell: return tok_extern;
        case 0x66616c7365ll: return tok_false;
        case 0x6669786564ll: return tok_fixed;
        case 0x666c6f6174ll: return tok_float;
        case 0x666f72ll: return tok_for;
        case 0x667269656e64ll: return tok_friend;
        case 0x676574ll: return tok_get;
        case 0x676f746fll: return tok_goto;
        case 0x68616c66ll: return tok_half;
        case 0x6966ll: return tok_if;
        case 0x696ell: return tok_in;
        case 0x696e6c696e65ll: return tok_inline;
        case 0x696e6f7574ll: return tok_inout;
        case 0x696e74ll: return tok_int;
        case 0x6e74657266616365ll: return !strcmp( lexer.id(), "interface" ) ? tok_interface : tok_identifier;
        case 0x6c6f6e67ll: return tok_long;
        case 0x6d6174726978ll: return tok_matrix;
        case 0x6d757461626c65ll: return tok_mutable;
        case 0x616d657370616365ll: return !strcmp( lexer.id(), "namespace" ) ? tok_namespace : tok_identifier;
        case 0x6e6577ll: return tok_new;
        case 0x6f70657261746f72ll: return !strcmp( lexer.id(), "operator" ) ? tok_operator : tok_identifier;
        case 0x6f7574ll: return tok_out;
        case 0x7061636b6564ll: return tok_packed;
        case 0x70617373ll: return tok_pass;
        case 0x656c736861646572ll: return !strcmp( lexer.id(), "pixelshader" ) ? tok_pixelshader : tok_identifier;
        case 0x70726976617465ll: return tok_private;
        case 0x726f746563746564ll: return !strcmp( lexer.id(), "protected" ) ? tok_protected : tok_identifier;
        case 0x7075626c6963ll: return tok_public;
        case 0x7265676973746572ll: return !strcmp( lexer.id(), "register" ) ? tok_register : tok_identifier;
        case 0x7265745f63617374ll: return !strcmp( lexer.id(), "reinterpret_cast" ) ? tok_reinterpret_cast : tok_identifier;
        case 0x72657475726ell: return tok_return;
        case 0x726f77ll: return tok_row;
        case 0x73616d706c6572ll: return tok_sampler;
        case 0x65725f7374617465ll: return !strcmp( lexer.id(), "sampler_state" ) ? tok_sampler_state : tok_identifier;
        case 0x616d706c65723144ll: return !strcmp( lexer.id(), "sampler1D" ) ? tok_sampler1D : tok_identifier;
        case 0x616d706c65723244ll: return !strcmp( lexer.id(), "sampler2D" ) ? tok_sampler2D : tok_identifier;
        case 0x616d706c65723344ll: return !strcmp( lexer.id(), "sampler3D" ) ? tok_sampler3D : tok_identifier;
        case 0x706c657243554245ll: return !strcmp( lexer.id(), "samplerCUBE" ) ? tok_samplerCUBE : tok_identifier;
        case 0x706c657252454354ll: return !strcmp( lexer.id(), "samplerRECT" ) ? tok_samplerRECT : tok_identifier;
        case 0x736861726564ll: return tok_shared;
        case 0x73686f7274ll: return tok_short;
        case 0x7369676e6564ll: return tok_signed;
        case 0x73697a656f66ll: return tok_sizeof;
        case 0x737461746963ll: return tok_static;
        case 0x7469635f63617374ll: return !strcmp( lexer.id(), "static_cast" ) ? tok_static_cast : tok_identifier;
        case 0x737472696e67ll: return tok_string;
        case 0x737472756374ll: return tok_struct;
        case 0x737769746368ll: return tok_switch;
        case 0x6563686e69717565ll: return !strcmp( lexer.id(), "technique" ) ? tok_technique : tok_identifier;
        case 0x74656d706c617465ll: return !strcmp( lexer.id(), "template" ) ? tok_template : tok_identifier;
        /*case 0x74657874757265ll: return tok_texture;
        case 0x6578747572653144ll: return !strcmp( lexer.id(), "texture1D" ) ? tok_texture1D : tok_identifier;
        case 0x6578747572653244ll: return !strcmp( lexer.id(), "texture2D" ) ? tok_texture2D : tok_identifier;
        case 0x6578747572653344ll: return !strcmp( lexer.id(), "texture3D" ) ? tok_texture3D : tok_identifier;
        case 0x7475726543554245ll: return !strcmp( lexer.id(), "textureCUBE" ) ? tok_textureCUBE : tok_identifier;
        case 0x7475726552454354ll: return !strcmp( lexer.id(), "textureRECT" ) ? tok_textureRECT : tok_identifier;*/
        case 0x74686973ll: return tok_this;
        case 0x7468726f77ll: return tok_throw;
        case 0x74727565ll: return tok_true;
        case 0x747279ll: return tok_try;
        case 0x74797065646566ll: return tok_typedef;
        case 0x747970656964ll: return tok_typeid;
        case 0x747970656e616d65ll: return !strcmp( lexer.id(), "typename" ) ? tok_typename : tok_identifier;
        case 0x756e69666f726dll: return tok_uniform;
        case 0x756e696f6ell: return tok_union;
        case 0x756e7369676e6564ll: return !strcmp( lexer.id(), "unsigned" ) ? tok_unsigned : tok_identifier;
        case 0x7573696e67ll: return tok_using;
        case 0x766563746f72ll: return tok_vector;
        case 0x6578736861646572ll: return !strcmp( lexer.id(), "vertexshader" ) ? tok_vertexshader : tok_identifier;
        case 0x7669727475616cll: return tok_virtual;
        case 0x766f6964ll: return tok_void;
        case 0x766f6c6174696c65ll: return !strcmp( lexer.id(), "volatile" ) ? tok_volatile : tok_identifier;
        case 0x7768696c65ll: return tok_while;
        default: return tok_identifier;
      }
    }

    enum toks  {      tok_asm = tok_last,tok_asm_fragment,tok_auto,tok_bool,tok_break,tok_case,tok_catch,tok_char,tok_class,tok_column,tok_major,tok_compile,tok_const,tok_const_cast,tok_continue,tok_decl,tok_default,tok_delete,tok_discard,tok_do,tok_double,tok_dword,tok_dynamic_cast,tok_else,tok_emit,tok_enum,tok_explicit,tok_extern,tok_false,tok_fixed,tok_float,tok_for,tok_friend,tok_get,tok_goto,tok_half,tok_if,tok_in,tok_inline,tok_inout,tok_int,tok_interface,tok_long,tok_matrix,tok_mutable,tok_namespace,tok_new,tok_operator,tok_out,tok_packed,tok_pass,tok_pixelfragment,tok_pixelshader,tok_private,tok_protected,tok_public,tok_register,tok_reinterpret_cast,tok_return,tok_row,tok_sampler,tok_sampler_state,tok_sampler1D,tok_sampler2D,tok_sampler3D,tok_samplerCUBE,tok_samplerRECT,tok_shared,tok_short,tok_signed,tok_sizeof,tok_static,tok_static_cast,tok_string,tok_struct,tok_switch,tok_technique,tok_template,tok_texture,tok_texture1D,tok_texture2D,tok_texture3D,tok_textureCUBE,tok_textureRECT,tok_this,tok_throw,tok_true,tok_try,tok_typedef,tok_typeid,tok_typename,tok_uniform,tok_union,tok_unsigned,tok_using,tok_vector,tok_vertexfragment,tok_vertexshader,tok_virtual,tok_void,tok_volatile,tok_while, tok_lastlast };
    const char *getTokenName( unsigned tok )
    {
      const char *tok_names[] = { "asm",    "asm_fragment",  "auto",  "bool",  "break",  "case",  "catch",  "char",  "class",  "column",  "major",  "compile",  "const",  "const_cast",  "continue",  "decl",  "default",  "delete",  "discard",  "do",  "double",  "dword",  "dynamic_cast",  "else",  "emit",  "enum",  "explicit",  "extern",  "false",  "fixed",  "float",  "for",  "friend",  "get",  "goto",  "half",  "if",  "in",  "inline",  "inout",  "int",  "interface",  "long",  "matrix",  "mutable",  "namespace",  "new",  "operator",  "out",  "packed",  "pass",  "pixelfragment",  "pixelshader",  "private",  "protected",  "public",  "register",  "reinterpret_cast",  "return",  "row",  "sampler",  "sampler_state",  "sampler1D",  "sampler2D",  "sampler3D",  "samplerCUBE",  "samplerRECT",  "shared",  "short",  "signed",  "sizeof",  "static",  "static_cast",  "string",  "struct",  "switch",  "technique",  "template",  "texture",  "texture1D",  "texture2D",  "texture3D",  "textureCUBE",  "textureRECT",  "this",  "throw",  "true",  "try",  "typedef",  "typeid",  "typename",  "uniform",  "union",  "unsigned",  "using",  "vector",  "vertexfragment",  "vertexshader",  "virtual",  "void",  "volatile",  "while", 0 };
      return tok < tok_last ? lexer.token_name( tok ) : tok_names[ tok - tok_last ];
    }
    
    unsigned char tokenToPrecidence[ tok_lastlast ];
    unsigned char tokenIsRightGrouping[ tok_lastlast ];
    
    bool dontReadLine;

    void getNext()
    {
      lexer.lex_token();
      for(;;)
      {
        if( lexer.type() == tok_newline )
        {
          if( dontReadLine || preprocessor.next_line() == NULL )
          {
            curToken = tok_end_of_source;
            return;
          }
          lexer.start( preprocessor.cur_line() );
          lexer.lex_token();
          if( trace_tokens )
          {
            cerr << indent() << "> " << (char*)preprocessor.cur_line() << "\n";
          }
        } else if( lexer.type() == tok_hash )
        {
          lexer.lex_token();
          if( lexer.type() == tok_int_constant )
          {
            line_number = (int)lexer.value() - 1;
          }
          while( lexer.type() != tok_newline )
          {
            lexer.lex_token();
          }
        } else if( lexer.type() == tok_identifier )
        {
          curToken = idToKeyword( lexer.value() );
          if( trace_tokens )
          {
            if( curToken == tok_identifier )
            {
              cerr << indent() << "# tok (" << lexer.id() << ")\n";
            } else
            {
              cerr << indent() << "# tok " << getTokenName( curToken ) << "\n";
            }
          }
          return;
        } else
        {
          curToken = lexer.type();
          if( trace_tokens )
          {
            cerr << indent() << "# tok " << getTokenName( curToken ) << "\n";
          }
          return;
        }
      }
    }

    unsigned lineNumber()
    {
      return 0;
    }
  
    const char *fileName()
    {
      return "file";
    }
    
    bool expect( unsigned type )
    {
      if( curToken != type )
      {
        cerr << errorText() << "expecting " << getTokenName( type ) << " got " <<  getTokenName( curToken ) << "\n";
        return false;
      }
      return true;
    }
    
    expr_t *makeIntType( expr_t *lhs )
    {
      if( lhs->getType()->getKind() != type_t::kind_int )
      {
	      return new expr_t( expr_t::kind_cast, intType, lhs );
      } else
      {
        return lhs;
	    }
    }
    
    expr_t *makeBoolType( expr_t *lhs )
    {
      if( lhs->getType()->getKind() != type_t::kind_bool )
      {
	      expr_t *zero = new expr_t( expr_t::kind_cast, intType, (long long)0 );
	      zero = new expr_t( expr_t::kind_cast, lhs->getType(), zero );
	      return new expr_t( expr_t::kind_ne, boolType, lhs, zero );
      } else
      {
        return lhs;
	    }
    }

    expr_t *makeVectorBoolType( expr_t *lhs )
    {
      if( lhs->getType()->getIsPacked() )
      {
        int dim = lhs->getType()->getDimension();
        expr_t *zero = new expr_t( expr_t::kind_cast, intTypes1D[ dim-1 ], (long long)0 );
        zero = new expr_t( expr_t::kind_cast, lhs->getType(), zero );
        return new expr_t( expr_t::kind_ne, boolTypes1D[ dim-1 ], lhs, zero );
      } else
      {
        return makeBoolType( lhs );
	    }
    }

    // generate code to cast src to destType    
    expr_t *makeCast( expr_t *src, type_t *destType )
    {
      type_t *srcType = src->getType();
      return new expr_t( expr_t::kind_cast, destType, src );
    }

    // called twice in the case of binary operators to convert the two types into a common type.
    expr_t *makeSameType( expr_t *lhs, expr_t *rhs )
    {
      type_t *lhsType = lhs->getType();
      type_t *rhsType = rhs->getType();
      if( *lhsType == *rhsType )
      {
        return lhs;
      } else
      {
        bool lhsVector = lhsType->getIsVector();
        bool rhsVector = rhsType->getIsVector();
        bool lhsScalar = lhsType->getIsScalar();
        bool rhsScalar = rhsType->getIsScalar();
        if( lhsScalar && rhsScalar )
        {
          // scalars go to a bigger, floatier kind
          unsigned lhsKind = lhsType->getKind();
          unsigned rhsKind = rhsType->getKind();
          bool hasCfloat = lhsKind == type_t::kind_cfloat || rhsKind == type_t::kind_cfloat;
          bool hasFloat = lhsKind == type_t::kind_float || rhsKind == type_t::kind_float;
          bool hasHalf = lhsKind == type_t::kind_half || rhsKind == type_t::kind_half;
          if( hasCfloat || hasFloat || hasHalf )
          {
  	        return makeCast( lhs, hasCfloat || hasFloat ? floatType : halfType );
          } else
          {
  	        return makeCast( lhs, intType );
          }
        } else if( lhsScalar && rhsVector )
        {
          // scalars get cast to vectors first
	        return makeCast( lhs, rhsType );
        } else if( lhsVector && rhsScalar )
        {
          // but vectors don't get cast to scalars (it will be done the other way round)
	        return lhs;
        } else
        {
          return makeCast( lhs, rhsType );
        }
	    }
    }
    
    expr_t *parseDeclarators( type_t *type, bool allowFunctionBodies, unsigned finalToken, bool allowAbstract )
    {
      expr_t *expr = NULL;
      for(;;)
      {
        value_t *value = parseDeclarator( type, allowAbstract );
        if( !value )
        {
          return NULL;
        }
        bool isFunction = value->getType()->getKind() == type_t::kind_function;
        if( value->getType()->getIsIn() || value->getType()->getIsOut() )
        {
          cerr << errorText() << "in/out not allowed here\n";
          return NULL;
        }
        if( value_t *found = curScope->getExistingValue( value ) )
        {
          if( isFunction && allowFunctionBodies )
          {
            value_t *search = found;
            for(;;)
            {
              cerr << "checking " << *search->getType() << " == " << *value->getType() << "?\n";
              if( *search->getType() == *value->getType() )
              {
                cerr << "*** same function!\n";
                // get existing function, but update the function type (parameter names may differ)
                search->setType( value->getType() );
                break;
              }
              value_t *prev = search;
              search = search->getNextPolymorphic();
              if( search == NULL )
              {
                cerr << "*** new function!\n";
                // insert new function at end
                prev->setNextPolymorphic( value );
                break;
              }
            }
          } else
          {
            cerr << errorText() << "'" << value->getName() << "' has been redefined\n";
            return NULL;
          }
        } else
        {
          curScope->addValue( value );
        }
        if( expr_t *init = value->getInit() )
        {
          if( isFunction )
          {
            cerr << errorText() << "function '" << value->getName() << "' has been redefined\n";
            return NULL;
          }
          expr_t *val = new expr_t( expr_t::kind_value, value );
          expr_t *rhs = makeCast( init, value->getType() );
          expr_t *assign = new expr_t( expr_t::kind_equals, value->getType(), val, rhs );
          expr = expr ? new expr_t( expr_t::kind_comma, assign->getType(), expr, assign ) : assign;
        } else if( curToken == tok_lbrace && isFunction )
        {
          scope_t *saveScope = curScope;
          curScope = value->getType()->getScope();
          cerr << "parsing body " << *curScope << "\n";
          stmt_t *functionBody = parseStatement();
          curScope = saveScope;
          if( !functionBody )
          {
            return NULL;
          }
          cerr << indent() << "done function " << value->getName() << "\n";
          
          //functionBody->end()
          
          value->setInit( new expr_t( expr_t::kind_statement, functionBody ) );
          finalToken = 0;
          break;
        }
        if( curToken != tok_comma )
        {
          break;
        }
        getNext();
      }

      if( finalToken != 0 )
      {
        if( !expect( finalToken ) )
        {
          return NULL;
        }
        getNext();
      }

      if( expr == NULL )
      {
        expr = new expr_t( expr_t::kind_nop, voidType );
      }
      return expr;
    }

    
    value_t *parseDeclarator( type_t *type, bool allowAbstract )
    {
      std::string name;

      if( curToken == tok_lparen )
      {
        // eg. int (x[5])();
        getNext();
        value_t *value = parseDeclarator( type, allowAbstract );
        if( !expect( tok_rparen ) )
        {
          return NULL;
        }
        getNext();
        name = value->getName();
        type = value->getType();
      } else if( allowAbstract && ( curToken == tok_rparen || curToken == tok_comma ) )
      {
        // eg. float f( int, int, float );
        char tmp[ 32 ];
        sprintf( tmp, "__%d", numAbstract++ );
        name = tmp;
      } else if( curToken == tok_identifier )
      {
        name = lexer.id();
        getNext();
        
        type_t *typeDef = findTypedef( name );
        if( typeDef != NULL )
        {
          cerr << errorText() << "not expecting a typedef here\n";
          return NULL;
        }
      } else
      {
        cerr << errorText() << "expecting a declarator here\n";
        return NULL;
      }
      
      // many arrays allowed in type
      type_t *returned_type = type;
      type_t *working_type = (type_t *)type;
      bool first = true;
      while( curToken == tok_lbracket )
      {
        getNext();
        expr_t *dim = parseExpression( 0 );
        if( !expect( tok_rbracket ) )
        {
          return NULL;
        }
        getNext();
        
        // translate immediately.
        BasicBlock *bb = NULL;
        Value *cexpr = dim->translate_rvalue( bb, *this, true );
        if( cexpr == NULL || !isa< ConstantInt >( cexpr ) )
        {
          cerr << errorText() << "expecting constant int in []\n";
          return NULL;
        }
        
        uint64_t dimension = cast< ConstantInt >( cexpr )->getValue().getLimitedValue();

        if( first )
        {
          type_t *new_type = new type_t( type_t::kind_array, type, dimension );
          returned_type = working_type = new_type;
          first = false;
        } else
        {
          type_t *new_type = new type_t( type_t::kind_array, type, dimension );
          working_type->setSubType( new_type );
          working_type = new_type;
        }
      }
      
      // only one function allowed in type      
      if( curToken == tok_lparen )
      {
        scope_t *saveScope = curScope;
        curScope = new scope_t( curScope );
        
        type_t *new_type = new type_t( type_t::kind_function );
        new_type->setScope( curScope );
        getNext();
        
        unsigned paramOffset = 0;
        
        if( returned_type->getIsPassByPtr() )
        {
          type_t *rtype = new type_t( *returned_type );
          rtype->setIsOut( true );
          value_t *value = new value_t( rtype, "$return" );
          value->setOffset( paramOffset++ );
          curScope->addValue( value );
          new_type->setSubType( voidType );
          new_type->setIsReturnByValue( true );
        } else
        {
          new_type->setSubType( returned_type );
        }
        returned_type = new_type;
        
        while( curToken != tok_rparen )
        {
          type_t *type = parseDeclspec();
          if( !type )
          {
            if( curToken == tok_identifier )
            {
              cerr << errorText() << "type " << lexer.id() << " has not been defined\n";
            } else
            {
              cerr << errorText() << "expecting type here";
            }
            return NULL;
          }

          if( type->getKind() == type_t::kind_function )
          {
            cerr << errorText() << "functions not allowed here\n";
            return NULL;
          }

          value_t *value = parseDeclarator( type, true );
          if( !value )
          {
            return NULL;
          }
          value->setOffset( paramOffset++ );
          curScope->addValue( value );

          if( curToken != tok_comma )
          {
            break;
          }
          getNext();
        }
        
        curScope = saveScope;

        if( !expect( tok_rparen ) )
        {
          return false;
        }
        getNext();
      }

      if( !verifyType( returned_type ) )
      {
        return NULL;
      }

      value_t *value = new value_t( returned_type, name );

      if( curToken == tok_colon )
      {
        getNext();
        if( curToken == tok_register )
        {
          getNext();
          if( !expect( tok_lparen ) )
          {
            return NULL;
          }
          getNext();
          if( !expect( tok_identifier ) )
          {
            return NULL;
          }
          value->setSemantic( lexer.id() );
          getNext();
          if( !expect( tok_rparen ) )
          {
            return NULL;
          }
          getNext();
        } else if( expect( tok_identifier ) )
        {
          value->setSemantic( lexer.id() );
          getNext();
        } else
        {
          return NULL;
        }
      } else if( curToken == tok_equals )
      {
        getNext();
        if( curToken == tok_lbrace )
        {
          int level = 0;
          do
          {
            level += curToken == tok_lbrace;
            level -= curToken == tok_rbrace;
            getNext();
          } while( level != 0 && curToken != tok_end_of_source );
        } else
        {
          expr_t *expr = parseExpression( 1 );
          value->setInit( makeCast( expr, value->getType() ) );
          //value->setInit( expr );
        }
      }
      return value;
    }
    
    bool verifyType( type_t *type )
    {
      if( type->getKind() == type_t::kind_array )
      {
        if( type->getIsVector() && type->getDimension() > 4 )
        {
          cerr << errorText() << "packed types have a maximum dimension of 4\n";
        }
      }
      return true;
    }
    
    type_t *parseDeclspec()
    {
      type_t *thisType = NULL;
      type_t *newType = NULL;
      bool isIn = false;
      bool isOut = false;
      bool isUniform = false;
      bool isConst = false;
      bool isPacked = false;
   
      for(;;)
      {
        switch( curToken )
        {
          case tok_identifier:
          {
            type_t *typeDef = findTypedef( lexer.id() );
            if( typeDef == NULL )
            {
              goto finish;
            }
            if( thisType != NULL )
            {
              cerr << errorText() << "not expecting a typedef here\n";
              return NULL;
            } else
            {
              thisType = typeDef;
            }
            getNext();
          } break;

          case tok_struct:
          {
            getNext();
            
            if( curToken != tok_lbrace )
            {
              expect( tok_identifier );
              std::string structId = lexer.id();
              getNext();
              
              if( type_t *tagType = findTag( structId ) )
              {
                printf( "*** old tag %s\n", structId.data() );
                thisType = tagType;
              } else
              {
                printf( "*** new tag %s\n", structId.data() );
                thisType = new type_t( type_t::kind_struct );
                thisType->setScope( NULL );
                makeTypedef( thisType, structId );
                makeTag( thisType, structId );
              }
            } else
            {
              char tmp[ 20 ];
              sprintf( tmp, "__anon%d", structNumber++ );
              std::string structId = tmp;
              thisType = new type_t( type_t::kind_struct );
              thisType->setScope( NULL );
              makeTypedef( thisType, structId );
              makeTag( thisType, structId );
            }
            
            if( curToken == tok_lbrace )
            {
              if( thisType->getScope() != NULL )
              {
                cerr << errorText() << "struct already defined\n";
                return NULL;
              }
              getNext();

              scope_t *scope = new scope_t( NULL );
              thisType->setScope( scope );
              unsigned structOffset = 0;

              while( curToken != tok_rbrace )
              {
                type_t *type = parseDeclspec();
                if( !type )
                {
                  cerr << errorText() << "expected type here\n";
                  return NULL;
                }
                for(;;)
                {
                  value_t *value = parseDeclarator( type, false );
                  if( !value )
                  {
                    return NULL;
                  }
                  value->setOffset( structOffset++ );
                  scope->addValue( value );
                  if( curToken != tok_comma )
                  {
                    break;
                  }
                  getNext();
                }
                
                if( !expect( tok_semicolon ) )
                {
                  break;
                }
                getNext();
              }
              
              if( curToken == tok_rbrace )
              {
                getNext();
              }
            }
          } break;
          case tok_void: newType = voidType; goto typeCommon;
          case tok_bool: newType = boolType; goto typeCommon;
          case tok_int: newType = intType; goto typeCommon;
          case tok_float: newType = floatType; goto typeCommon;
          case tok_half: newType = floatType; goto typeCommon;
          case tok_sampler: newType = new type_t( type_t::kind_sampler ); goto typeCommon;
          case tok_sampler1D: newType = new type_t( type_t::kind_sampler1D ); goto typeCommon;
          case tok_sampler2D: newType = new type_t( type_t::kind_sampler2D ); goto typeCommon;
          case tok_sampler3D: newType = new type_t( type_t::kind_sampler3D ); goto typeCommon;
          case tok_samplerRECT: newType = new type_t( type_t::kind_samplerRECT ); goto typeCommon;
          case tok_samplerCUBE: newType = new type_t( type_t::kind_samplerCUBE ); goto typeCommon;
          typeCommon:
          {
            if( thisType )
            {
              cerr << errorText() << "more than one type specified\n";
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
      if( thisType == NULL )
      {
        if( isIn | isOut | isUniform| isConst| isPacked )
        {
          cerr << errorText() << "qualifier without type\n";
        }
      } else
      {
        thisType->setIsConst( isConst );
        thisType->setIsUniform( isUniform );
        thisType->setIsIn( isIn );
        thisType->setIsOut( isOut );
        thisType->setIsPacked( isPacked );
      }
      return thisType;
    }
    
    stmt_t *parseStatement()
    {
      stmt_t *result = NULL;
      if( curToken == tok_lbrace )
      {
        scope_t *saveScope = curScope;
        curScope = new scope_t( curScope );
        
        result = new stmt_t( stmt_t::kind_compound );
        result->setScope( curScope );
        getNext();
        stmt_t **prev = result->getStatementsAddr();
        while( curToken != tok_rbrace )
        {
          stmt_t *statement = parseStatement();
          if( statement == NULL )
          {
            result = NULL;
            break;
          }
          *prev = statement;
          prev = statement->getNextAddr();
        }
        *prev = 0;
        curScope = saveScope;
        
        if( !expect( tok_rbrace ) )
        {
          return NULL;
        }
        getNext();
        return result;
      } else if( curToken == tok_return )
      {
        getNext();
        if( curToken != tok_semicolon )
        {
          expr_t *expr = parseExpression();
          if( expr == NULL )
          {
            return NULL;
          }
          result = new stmt_t( stmt_t::kind_return );
          result->setExpression( expr );
        } else
        {
          result = new stmt_t( stmt_t::kind_return );
        }
        if( !expect( tok_semicolon ) )
        {
          return NULL;
        }
        getNext();
      } else if( curToken == tok_discard )
      {
        getNext();
        result = new stmt_t( stmt_t::kind_return );
        if( !expect( tok_semicolon ) )
        {
          return NULL;
        }
        getNext();
      } else if( curToken == tok_for || curToken == tok_if || curToken == tok_while )
      {
        bool isFor = curToken == tok_for;
        bool isIf = curToken == tok_if;
        bool isWhile = curToken == tok_while;

        getNext();
        if( !expect( tok_lparen ) )
        {
          return NULL;
        }
        getNext();

        scope_t *saveScope = curScope;
        curScope = new scope_t( curScope );

        result = new stmt_t( isFor ? stmt_t::kind_for : isIf ? stmt_t::kind_if : stmt_t::kind_while );
        result->setScope( curScope );
        
        expr_t *expr = NULL;
        
        if( !isFor || curToken != tok_semicolon )
        {
          if( type_t *type = parseDeclspec() )
          {
            expr = parseDeclarators( type, false, 0, false );
          } else
          {
            expr = parseExpression();
          }
          if( !expr )
          {
            return NULL;
          }
        }

        if( isFor )
        {
          if( !expect( tok_semicolon ) )
          {
            return NULL;
          }
          getNext();
          expr_t *expr2 = parseExpression();
          if( !expect( tok_semicolon ) )
          {
            return NULL;
          }
          getNext();
          expr_t *expr3 = parseExpression();
          expr = new expr_t( expr_t::kind_for, NULL, expr, expr2, expr3 );
        }
        
        result->setExpression( expr );

        if( !expect( tok_rparen ) )
        {
          return NULL;
        }
        getNext();

        stmt_t *stmt = parseStatement();
        if( stmt == NULL )
        {
          return NULL;
        }
        *result->getStatementsAddr() = stmt;
        
        if( isIf && curToken == tok_else )
        {
          getNext();
          stmt_t *else_stmt = parseStatement();
          if( else_stmt == NULL )
          {
            return NULL;
          }
          *result->getNextAddr() = else_stmt;
        }
        
        curScope = saveScope;
        return result;
      } else if( curToken == tok_do )
      {
        getNext();

        result = new stmt_t( stmt_t::kind_dowhile );

        stmt_t *stmt = parseStatement();
        if( stmt == NULL || !expect( tok_while ) )
        {
          return NULL;
        }
        *result->getStatementsAddr() = stmt;
        getNext();

        if( !expect( tok_lparen ) )
        {
          return NULL;
        }

        expr_t *expr = parseExpression();
        if( expr == NULL )
        {
          return NULL;
        }
        result->setExpression( expr );
        /*if( !expect( tok_semicolon ) )
        {
          return NULL;
        }
        getNext();*/
        return result;
      } else if( type_t *type = parseDeclspec() )
      {
        result = new stmt_t( stmt_t::kind_declaration );
        expr_t *init = parseDeclarators( type, false, tok_semicolon, false );
        if( init == NULL )
        {
          return NULL;
        }
        result->setExpression( init );
      } else if( expr_t *expr = parseExpression() )
      {
        result = new stmt_t( stmt_t::kind_expression );
        result->setExpression( expr );
        if( !expect( tok_semicolon ) )
        {
          return NULL;
        }
        getNext();
      } else
      {
        cerr << errorText() << "expected statement";
      }

      return result;
    }
    
    expr_t *parseExpression( unsigned minPrecidence=0 )
    {
    	expr_t *result = 0;
	    if( trace_parse )
	    {
      	cerr << push() << "expr\n";
      }
	    if( curToken == tok_minus || curToken == tok_plus )
	    {
	      unsigned op = curToken;
	      getNext();
	      expr_t * rhs = parseExpression( 100 );
	      if( rhs == NULL )
	      {
	        return result;
	      }
	      expr_t *zero = new expr_t( expr_t::kind_int_value, intType, (long long)0 );
	      zero = new expr_t( expr_t::kind_cast, rhs->getType(), zero );
	      result = new expr_t( curToken == tok_minus ? expr_t::kind_minus : expr_t::kind_plus, rhs->getType(), zero, rhs );
	    } else if( curToken == tok_not )
	    {
	      unsigned op = curToken;
	      getNext();
	      expr_t * rhs = parseExpression( 100 );
	      if( rhs == NULL )
	      {
	        return result;
	      }
	      rhs = makeBoolType( rhs );
	      expr_t *one = new expr_t( expr_t::kind_int_value, intType, (long long)1 );
	      one = new expr_t( expr_t::kind_cast, rhs->getType(), one );
	      result = new expr_t( expr_t::kind_xor, rhs->getType(), one, rhs );
	    } else if( curToken == tok_tilda )
	    {
	      unsigned op = curToken;
	      getNext();
	      expr_t * rhs = parseExpression( 100 );
	      if( rhs == NULL )
	      {
	        return result;
	      }
	      rhs = makeIntType( rhs );
	      expr_t *one = new expr_t( expr_t::kind_int_value, intType, (long long)-1 );
	      one = new expr_t( expr_t::kind_cast, rhs->getType(), one );
	      result = new expr_t( expr_t::kind_xor, rhs->getType(), one, rhs );
	    } else if( curToken == tok_plus_plus || curToken == tok_minus_minus )
	    {
	      unsigned op = curToken;
	      getNext();
	      expr_t *rhs = parseExpression( 100 );
	      if( rhs == NULL )
	      {
	        return result;
	      }
	      expr_t *inc = new expr_t( expr_t::kind_int_value, intType, (long long)1 );
	      inc = new expr_t( expr_t::kind_cast, rhs->getType(), inc );
        result = new expr_t( curToken == tok_plus_plus ? expr_t::kind_plus_equals : expr_t::kind_minus_equals, rhs->getType(), rhs, inc );
	    } else if( curToken == tok_identifier )
	    {
	      std::string name( lexer.id() );
	      type_t *type = findTypedef( name );
	      if( type )
	      {
	        getNext();
	        if( !expect( tok_lparen ) )
	        {
	          return NULL;
	        }
	        expr_t *init = parseExpression( 100 );
	        result = new expr_t( expr_t::kind_init, type, init );
	      } else
	      {
	        value_t *value = curScope->lookup( name );
	        if( value == NULL && buildIntrinsic( name.data() ) )
	        {
	          value = curScope->lookup( name );
	        }
	        if( value == NULL )
	        {
            cerr << errorText() << "undefined symbol ' " << name.data() << "'\n";
	          return NULL;
	        }
          getNext();
	        result = new expr_t( expr_t::kind_value, value );
	      }
	    } else if( curToken == tok_int_constant || curToken == tok_int64_constant || curToken == tok_uint_constant || curToken == tok_uint64_constant )
	    {
        //result = (int64_type)lexer_.value();
        result = new expr_t( expr_t::kind_int_value, cintType, (long long)lexer.value() );
        getNext();
	    } else if( curToken == tok_float_constant || curToken == tok_double_constant )
	    {
        result = new expr_t( expr_t::kind_double_value, cfloatType, lexer.double_value() );
        //result = (int64_type)lexer_.value();
        getNext();  
	    } else if( curToken == tok_lparen )
	    {
        getNext();
        type_t *type = parseDeclspec();
        if( type )
        {
          if( !expect( tok_rparen ) )
          {
            return NULL;
          }
          getNext();
  	      expr_t * rhs = parseExpression( 100 );
          result = new expr_t( expr_t::kind_cast, type, rhs );
        } else
        {
          result = parseExpression();
          expect( tok_rparen );
          getNext();
        }
	    } else if( curToken == tok_rparen )
	    {
        cerr << errorText() << "missing '(' in expression\n";
        return NULL;
	    } else
	    {
        cerr << errorText() << "expected expression, got " << getTokenName( curToken ) << "\n";
        return NULL;
	    }
	    
	    // handle  xxx[ n ] xxx( a, b ) xxx++ and xxx--
	    for(;;)
	    {
        if( curToken == tok_dot )
        {
          getNext();
          if( curToken != tok_identifier )
          {
            cerr << errorText() << "expected member name after '.', got " << getTokenName( curToken ) << " instead\n";
            return NULL;
          }
          if( result->getType()->getIsVector() )
          {
            type_t *newType = new type_t( *result->getType() );
            expr_t *newExpr = new expr_t( expr_t::kind_swiz, newType, result );
            unsigned char *swiz = newExpr->getSwiz();
            unsigned numSwiz = 0;
            bool bad = false;
            const char choices[] = "xyzwrgba";
            for( char *p = lexer.id(); *p; ++p )
            {
              bad |= numSwiz >= 4;
              const char *which = strchr( choices, *p );
              bad |= which == NULL;
              if( !bad )
              {
                swiz[ numSwiz++ ] = (char)( which - choices ) & 3;
              }
            }
            if( bad )
            {
              cerr << errorText() << " '" << lexer.id() << "' is not a valid swizzle\n";
              return NULL;
            }
            
            // single swizzles make scalars
            cerr << *newType << "\n";
            if( numSwiz == 1 )
            {
              *newType = *newType->getSubType();
            } else
            {
              newType->setDimension( numSwiz );
            }
            cerr << "..." << *newType << "\n";
            getNext();
            result = newExpr;
          } else if( result->getType()->getKind() != type_t::kind_struct )
          {
            cerr << errorText() << "left hand size of '.' should be a packed array or structure";
            return NULL;
          } else
          {
            std::string name( lexer.id() );
            getNext();
            value_t *value = result->getType()->getScope()->lookup( name );
            if( value == NULL )
            {
              cerr << errorText() << "structure does not have a member '" << name.data() << "'.\n";
              return NULL;
            }
            result = new expr_t( expr_t::kind_dot, result, value );
          }
        } else if( curToken == tok_lparen )
        {
          getNext();
          
          if( result->getKind() == expr_t::kind_int_value )
          {
            expr_t *rhs = parseExpression( 0 );
            if( !expect( tok_rparen ) )
            {
              return NULL;
            }
            getNext();
            result = new expr_t( expr_t::kind_int_value, cintType, (long long )0 );
          } else
          {
            if( result->getKind() != expr_t::kind_value || result->getType()->getKind() != type_t::kind_function )
            {
              cerr << errorText() << "calling something that is not a function.";
              return NULL;
            }
            expr_t *rhs = parseExpression( 0 );

            if( !expect( tok_rparen ) )
            {
              return NULL;
            }
            getNext();

            std::vector< type_t * > actualTypes;
            if( rhs->getKind() != expr_t::kind_comma )
            {
              actualTypes.push_back( rhs->getType() );
            } else
            {
              for( expr_t *expr = rhs; ; expr = expr->getKid( 0 ) )
              {
                if( expr->getKind() == expr_t::kind_comma )
                {
                  actualTypes.push_back( expr->getKid( 1 )->getType() );
                } else
                {
                  actualTypes.push_back( expr->getType() );
                  break;
                }
              }
            }

            value_t *value = NULL;
            value_t *firstValue = result->getValue();
            for( value_t *search = firstValue; search != NULL; search = search->getNextPolymorphic() )
            {
              unsigned score = doParametersMatch( actualTypes, search );
              if( score == 2 )
              {
                // exact match. ignore any others.
                value = search;
                break;
              }
              if( score == 1 )
              {
                // partial match, will need casts.
                if( value != NULL )
                {
                  cerr << errorText() << "more than one function matches parameters\n";
                  result = NULL;
                  break;
                }
                value = search;
              }
            }

            if( result != NULL )
            {
              if( value == NULL )
              {
                //error( "", 0, "no matching function found\n" );
                cerr << "no matching function found: " << firstValue->getName();
                const char *comma = "( ";
                for( std::vector< type_t * >::iterator i = actualTypes.begin(); i != actualTypes.end(); ++i )
                {
                  cerr << comma << **i;
                  comma = ", ";
                }
                cerr << ")\n";
                for( value_t *search = firstValue; search != NULL; search = search->getNextPolymorphic() )
                {
                  cerr << *search << "\n";
                }
                result = NULL;
              } else
              {
                result->setType( value->getType() );
                result->setValue( value );
                result = new expr_t( expr_t::kind_call, result->getType()->getSubType(), result, rhs );
              }
            }
          }
	      } else if( curToken == tok_lbracket )
	      {
          getNext();
          expr_t * rhs = parseExpression();
          expect( tok_rbracket );
          getNext();
          if( result->getType()->getKind() != type_t::kind_array )
          {
            cerr << errorText() << "trying to use [] on non-array";
            return NULL;
          }
          result = new expr_t( expr_t::kind_index, result->getType()->getSubType(), result, rhs );
	      } else if( curToken == tok_plus_plus || curToken == tok_minus_minus )
	      {
	        getNext();
	        char name[ 32 ];
	        sprintf( name, "$tmp%d", numTmpVars++ );
          value_t *tmp = new value_t( result->getType(), name );
	        expr_t *inc = new expr_t( expr_t::kind_int_value, intType, (long long)1 );
	        inc = new expr_t( expr_t::kind_cast, result->getType(), inc );
          inc = new expr_t( curToken == tok_plus_plus ? expr_t::kind_plus_equals : expr_t::kind_minus_equals, result->getType(), result, inc );
          expr_t *tmpExpr = new expr_t( expr_t::kind_value, tmp );
          expr_t *tmpAssign = new expr_t( expr_t::kind_equals, result->getType(), tmpExpr, result );
          result = new expr_t( expr_t::kind_comma, result->getType(), inc, tmpExpr );
        } else
        {
          break;
        }
      }      
	    //if( debug_expressions ) cerr << indent() << "thisPrecidence=%d minPrecidence=%d tok=%s %s\n", thisPrecidence, minPrecidence, getTokenName( curToken ), lexer_.src() );
	    
	    // filter out operators with lower precidence than minPrecidence
	    //unsigned thisPrecidence = tokenToPrecidence[ curToken ];
	    //unsigned thisGrouping = tokenIsRightGrouping[ curToken ];
      unsigned op = curToken;
	    while( tokenToPrecidence[ curToken ] != 0 && tokenToPrecidence[ curToken ] > minPrecidence )
	    {
	      unsigned op = curToken;
		    getNext();

        // left grouping operators will parse like ( ( a + b ) + c ) + d    so rhs will accept fewer tokens
        // right grouping operators will parse like a = ( b = ( c = d ) )   so rhs will accept more tokens
	      expr_t *rhs = parseExpression( tokenToPrecidence[ op ] - tokenIsRightGrouping[ curToken ] );
	      if( rhs == NULL )
	      {
	        return NULL;
	      }
	      switch( op )
	      {
	        expr_t::kind_enum kind;

	        case tok_comma:
	        {
	          result = new expr_t( expr_t::kind_comma, rhs->getType(), result, rhs );
	        } break;
	        case tok_question:
	        {
	          if( !expect( tok_colon ) )
	          {
	            return NULL;
	          }
   	        getNext();
    	      expr_t *rhs2 = parseExpression( tokenToPrecidence[ op ] - tokenIsRightGrouping[ curToken ] );
    	      
            rhs = makeSameType( rhs, rhs2 );
            if( result == NULL )
            {
              cerr << errorText() << "unable to convert types\n";
              return NULL;
            }
            rhs2 = makeSameType( rhs2, rhs );
            if( rhs == NULL )
            {
              cerr << errorText() << "unable to convert types\n";
              return NULL;
            }

	          result = new expr_t( expr_t::kind_question, rhs->getType(), makeVectorBoolType( result ), rhs, rhs2 );
	        } break;
	        case tok_or: kind = expr_t::kind_or; goto binop;
	        case tok_and: kind = expr_t::kind_and; goto binop;
	        case tok_xor: kind = expr_t::kind_xor; goto binop;
	        case tok_shift_left: kind = expr_t::kind_shift_left; goto binop;
	        case tok_shift_right: kind = expr_t::kind_shift_right; goto binop;
	        case tok_plus: kind = expr_t::kind_plus; goto binop;
	        case tok_minus: kind = expr_t::kind_minus; goto binop;
	        case tok_star: kind = expr_t::kind_star; goto binop;
	        case tok_divide: kind = expr_t::kind_divide; goto binop;
	        case tok_mod: kind = expr_t::kind_mod; goto binop;
	        case tok_or_or: kind = expr_t::kind_or_or; goto binop;
	        case tok_and_and: kind = expr_t::kind_and_and; goto binop;
	        binop:
	        {
            result = makeSameType( result, rhs );
            if( result == NULL )
            {
              cerr << errorText() << "unable to convert types\n";
              return NULL;
            }
            rhs = makeSameType( rhs, result );
            if( rhs == NULL )
            {
              cerr << errorText() << "unable to convert types\n";
              return NULL;
            }
	          result = new expr_t( kind, rhs->getType(), result, rhs );
	        } break;

	        case tok_lt: kind = expr_t::kind_lt; goto relop;
	        case tok_gt: kind = expr_t::kind_gt; goto relop;
	        case tok_le: kind = expr_t::kind_le; goto relop;
	        case tok_ge: kind = expr_t::kind_ge; goto relop;
	        case tok_eq: kind = expr_t::kind_eq; goto relop;
	        case tok_ne: kind = expr_t::kind_ne; goto relop;
	        relop:
	        {
            result = makeSameType( result, rhs );
            if( result == NULL )
            {
              cerr << errorText() << "unable to convert types\n";
              return NULL;
            }
            rhs = makeSameType( rhs, result );
            if( rhs == NULL )
            {
              cerr << errorText() << "unable to convert types\n";
              return NULL;
            }
	          result = new expr_t( kind, boolType, result, rhs );
	        } break;

	        case tok_equals:
	        {
            rhs = makeCast( rhs, result->getType() );
	          result = new expr_t( expr_t::kind_equals, rhs->getType(), result, rhs );
	        } break;
	        case tok_divide_equals: kind = expr_t::kind_divide_equals; goto assignop;
	        case tok_mod_equals: kind = expr_t::kind_mod_equals; goto assignop;
	        case tok_plus_equals: kind = expr_t::kind_plus_equals; goto assignop;
	        case tok_minus_equals: kind = expr_t::kind_minus_equals; goto assignop;
	        case tok_shift_left_equals: kind = expr_t::kind_shift_left_equals; goto assignop;
	        case tok_shift_right_equals: kind = expr_t::kind_shift_right_equals; goto assignop;
	        case tok_and_equals: kind = expr_t::kind_and_equals; goto assignop;
	        case tok_xor_equals: kind = expr_t::kind_xor_equals; goto assignop;
	        case tok_or_equals: kind = expr_t::kind_or_equals; goto assignop;
	        case tok_times_equals: kind = expr_t::kind_times_equals; goto assignop;
	        assignop:
	        {
            rhs = makeCast( rhs, result->getType() );
	          result = new expr_t( expr_t::kind_equals, rhs->getType(), result, new expr_t( kind, rhs->getType(), result, rhs ) );
	        } break;
	        default:
	        {
	          assert( 0 );
	        } break;
	      }
	    }
	    if( trace_parse )
	    {
	      if( result != NULL )
	      {
      	  cerr << pop() << *result->getType() << " /expr\n";
      	}
      }
	    return result;
    }

    bool parseOuter()
    {
      if( curToken == tok_semicolon )
      {
        getNext();
        return true;
      }
      if( curToken == tok_typedef )
      {
        getNext();
        type_t *type = parseDeclspec();
        if( !type )
        {
          cerr << errorText() << "expecting type\n";
          return false;
        }
        value_t *value = parseDeclarator( type, false );
        if( !value )
        {
          cerr << errorText() << "expecting declarator\n";
          return false;
        }
        makeTypedef( value->getType(), value->getName() );
        if( !expect( tok_semicolon ) )
        {
          return false;
        }
        getNext();
        return true;
      }
      type_t *type = parseDeclspec();
      if( !type )
      {
        cerr << errorText() << "expecting typedef or declaration\n";
        return false;
      }
      if( curToken == tok_semicolon )
      {
        getNext();
      } else
      {
        expr_t *expr = parseDeclarators( type, true, tok_semicolon, false );
        if( expr == NULL )
        {
          return false;
        }
      }
      
      return true;
    }

  public:
    compiler() : preprocessor( getContext() ), lexer( getContext() )
    {
      /*FILE *toks = fopen( "toks.txt", "w" );
      for( unsigned i = tok_asm; getTokenName( i ); ++i )
      {
        const char *tn = getTokenName( i );
        
        {
          uint64_type value_t = 0;
          for( const char *p = tn; *p; ++p )
          {
            value_t = value_t * 256 + *p;
          }
          if( strlen( tn ) > 7 )
          {
            fprintf( toks, "case 0x%llxll: return !strcmp( lexer.id(), \"%s\" ) ? tok_%s : tok_identifier;\n", value_t, tn, tn );
          } else
          {
            fprintf( toks, "case 0x%llxll: return tok_%s;\n", value_t, tn );
          }
        }
      }
      fclose( toks );
      exit( 0 );*/
      
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

      invariantScope = new scope_t( NULL );
      
      dontReadLine = false;
      
      voidType = makeTypedef( new type_t( type_t::kind_void ), "void" );
      intType = makeTypedef( new type_t( type_t::kind_int ), "int" );
      floatType = makeTypedef( new type_t( type_t::kind_float ), "float" );
      doubleType = makeTypedef( new type_t( type_t::kind_float ), "double" );
      halfType = makeTypedef( new type_t( type_t::kind_half ), "half" );
      boolType = makeTypedef( new type_t( type_t::kind_bool ), "bool" );
      cintType = new type_t( type_t::kind_cint );
      cfloatType = new type_t( type_t::kind_cfloat );
      
      for( unsigned i = 1; i <= 4; ++i )
      {
        char tmp[ 10 ];

        sprintf( tmp, "bool%d", i );
        type_t *bool_type = new type_t( type_t::kind_bool, i, true );
        makeTypedef( bool_type, tmp );

        sprintf( tmp, "int%d", i );
        type_t *int_type = new type_t( type_t::kind_int, i, true );
        makeTypedef( int_type, tmp );

        sprintf( tmp, "float%d", i );
        type_t *float_type = new type_t( type_t::kind_float, i, true );
        makeTypedef( float_type, tmp );

        sprintf( tmp, "half%d", i );
        type_t *half_type = new type_t( type_t::kind_half, i, true );
        makeTypedef( half_type, tmp );
        
        boolTypes1D[ i-1 ] = bool_type;
        intTypes1D[ i-1 ] = int_type;
        floatTypes1D[ i-1 ] = float_type;
        halfTypes1D[ i-1 ] = half_type;

        for( unsigned j = 1; j <= 4; ++j )
        {
          char tmp[ 10 ];
          type_t *type;
          sprintf( tmp, "bool%dx%d", i, j );
          type = new type_t( type_t::kind_array, bool_type, j );
          makeTypedef( type, tmp );
          boolTypes2D[ i-1 ][ j-1 ] = type;

          sprintf( tmp, "int%dx%d", i, j );
          type = new type_t( type_t::kind_array, int_type, j );
          makeTypedef( type, tmp );
          intTypes2D[ i-1 ][ j-1 ] = type;

          sprintf( tmp, "float%dx%d", i, j );
          type = new type_t( type_t::kind_array, float_type, j );
          makeTypedef( type, tmp );
          floatTypes2D[ i-1 ][ j-1 ] = type;

          sprintf( tmp, "half%dx%d", i, j );
          type = new type_t( type_t::kind_array, half_type, j );
          makeTypedef( type, tmp );
          halfTypes2D[ i-1 ][ j-1 ] = type;
        }
      }
      
      // initialise anaonymous structure index
      structNumber = 0;
      numAbstract = 0;
      numTmpVars = 0;

      buildIntrinsic( NULL );
    }
    
    // we don't want to pre-parse the intrinsics otherwise they will take up lots of space.
    // so we parse them on demand.
    bool buildIntrinsic( const char *name )
    {
      static const intrinsic_t intrinsics[] =
      {
        // fundamental intrinsics. Everything else is built from these
        { "__RSQ", "float4 __RSQ( float4 );" },
        { "__POW", "float4 __POW( float4, float4 );" },
        { "__DP3", "float4 __DP3( float4, float4 );" },
        { "__DP4", "float4 __DP4( float4, float4 );" },

        // derived intrinsics.
        { "mul", "" },
        { "dot", "float dot(float3 a,float3 b){return __DP3(a.xyzz,b.xyzz).x;}" },
        { "distance", "" },
        { "lit", "" },
        { "min", "float max(float a,float b){return a<b?a:b;}" },
        { "max", "float max(float a,float b){return a>b?a:b;}" },
        { "sin", "" },
        { "cos", "" },
        { "abs", "" }, // abs(x)
        { "absacos", "" }, // absacos(x)
        { "all", "" }, // all(x)
        { "any", "" }, // any(x)
        { "asin", "" }, // asin(x)
        { "atan", "" }, // atan(x)
        { "atan2", "" }, // atan2(y, x)
        { "ceil", "" }, // ceil(x)
        { "clamp", "" }, // clamp(x, a, b)
        { "cos", "" }, // cos(x)
        { "cosh", "" }, // cosh(x)
        { "cross", "" }, // cross(a, b)
        { "degress", "" }, // degress(x)
        { "determinant", "" }, // determinant(M)
        { "dot", "float dot(float3 a,float3 b){return a.x*b.x+a.y*b.y+a.z*b.z;}" },
        { "exp", "" }, // exp(x)
        { "exp2", "" }, // exp2(x)
        { "floor", "" }, // floor(x)
        { "fmod", "" }, // fmod(x, y)
        { "frac", "" }, // frac(x)
        { "frexp", "" }, // frexp(x, out exp)
        { "isfinite", "" }, // isfinite(x)
        { "isinf", "" }, // isinf(x)
        { "isnan", "" }, // isnan(x)
        { "ldexp", "" }, // ldexp(x, n)
        { "lerp", "" }, // lerp(a, b, f)
        { "lit", "" }, // lit(ndotl, ndoth, m)
        { "log", "" }, // log(x)
        { "log2", "" }, // log2(x)
        { "log10", "" }, // log10(x)
        { "max", "" }, // max(a, b)
        { "min", "" }, // min(a, b)
        { "modf", "" }, // modf(x, out ip)
        { "mul", "" }, // mul(M, N)
        { "noise", "" }, // noise(x)
        { "pow", "float pow(float a,float b) {return __POW((float4)a,(float4)b).x;}" }, // pow(x, y)
        { "radians", "" }, // radians(x)
        { "round", "" }, // round(x)
        { "rsqrt", "" },
        { "saturate", "" }, // saturate(x)
        { "sign", "" }, // sign(x)
        { "sin", "" }, // sin(x)
        { "sincos", "" }, // sincos(float x, out s, out c)
        { "sinh", "" }, // sinh(x)
        { "smoothstep", "" }, // smoothstep(min, max, x)
        { "step", "" }, // step(a, x)
        { "sqrt", "" }, // sqrt(x)
        { "tan", "" }, // tan(x)
        { "tanh", "" }, // tanh(x)
        { "transpose", "" }, // transpose(M)
        { "distance", "" }, // distance(pt1, pt2)
        { "faceforward", "" }, // faceforward(N, I, Ng)
        { "length", "" },
        { "normalize",
          "float normalize(float a){return a;}"
          "float2 normalize(float2 a){return a*__RSQ( (float4)( a.x*a.x+a.y*a.y ) ).x;}"
          "float3 normalize(float3 a){return a*__RSQ( __DP3( (float4)a,(float4)a ) ).x;}"
          "float4 normalize(float4 a){return a*__RSQ( __DP4( a, a ) ).x;}"
        },
        { "reflect", "" }, // reflect(i, n)
        { "refract", "" }, // refract(i, n, eta)
        { "tex1D", "" }, // tex1D(sampler1D tex, float s)
        { "tex1D", "" }, // tex1D(sampler1D tex, float s, float dsdx, float dsdy)
        { "tex1D", "" }, // tex1D(sampler1D tex, float2 sz)
        { "tex1D", "" }, // tex1D(sampler1D tex, float2 sz, float dsdx, float dsdy)
        { "tex1Dproj", "" }, // tex1Dproj(sampler1D tex, float2 sq)
        { "tex1Dproj", "" }, // tex1Dproj(sampler1D tex, float3 szq)
        { "tex2D", "" }, // tex2D(sampler2D tex, float2 s)
        { "tex2D", "" }, // tex2D(sampler2D tex, float2 s, float2 dsdx, float2 dsdy)
        { "tex2D", "" }, // tex2D(sampler2D tex, float3 sz)
        { "tex2D", "" }, // tex2D(sampler2D tex, float3 sz, float2 dsdx, float2 dsdy)
        { "tex2Dproj", "" }, // tex2Dproj(sampler2D tex, float3 sq)
        { "tex2Dproj", "" }, // tex2Dproj(sampler2D tex, float4 szq)
        { "texRECT", "" }, // texRECT(samplerRECT tex, float2 s)
        { "texRECT", "" }, // texRECT(samplerRECT tex, float2 s, float2 dsdx, float2 dsdy)
        { "texRECT", "" }, // texRECT(samplerRECT tex, float3 sz)
        { "texRECT", "" }, // texRECT(samplerRECT tex, float3 sz, float2 dsdx, float2 dsdy)
        { "texRECTproj", "" }, // texRECTproj(samplerRECT tex, float3 sq)
        { "texRECTproj", "" }, // texRECTproj(samplerRECT tex, float3 szq)
        { "tex3D", "" }, // tex3D(sampler3D tex, float3 s)
        { "tex3D", "" }, // tex3D(sampler3D tex, float3 s, float3 dsdx, float3 dsdy)
        { "tex3Dproj", "" }, // tex3Dproj(sampler3D tex, float4 szq)
        { "texCUBE", "" }, // texCUBE(samplerCUBE tex, float3 s)
        { "texCUBE", "" }, // texCUBE(samplerCUBE tex, float3 s, float3 dsdx, float3 dsdy)
        { "texCUBEproj", "" }, // texCUBEproj(samplerCUBE tex, float4 sq)
        { "h2tex1D", "" }, // tex1D(sampler1D tex, float s)
        { "h2tex1D", "" }, // tex1D(sampler1D tex, float s, float dsdx, float dsdy)
        { "h2tex1D", "" }, // tex1D(sampler1D tex, float2 sz)
        { "h2tex1D", "" }, // tex1D(sampler1D tex, float2 sz, float dsdx, float dsdy)
        { "h2tex1Dproj", "" }, // tex1Dproj(sampler1D tex, float2 sq)
        { "h2tex1Dproj", "" }, // tex1Dproj(sampler1D tex, float3 szq)
        { "h2tex2D", "" }, // tex2D(sampler2D tex, float2 s)
        { "h2tex2D", "" }, // tex2D(sampler2D tex, float2 s, float2 dsdx, float2 dsdy)
        { "h2tex2D", "" }, // tex2D(sampler2D tex, float3 sz)
        { "h2tex2D", "" }, // tex2D(sampler2D tex, float3 sz, float2 dsdx, float2 dsdy)
        { "h2tex2Dproj", "" }, // tex2Dproj(sampler2D tex, float3 sq)
        { "h2tex2Dproj", "" }, // tex2Dproj(sampler2D tex, float4 szq)
        { "h2texRECT", "" }, // texRECT(samplerRECT tex, float2 s)
        { "h2texRECT", "" }, // texRECT(samplerRECT tex, float2 s, float2 dsdx, float2 dsdy)
        { "h2texRECT", "" }, // texRECT(samplerRECT tex, float3 sz)
        { "h2texRECT", "" }, // texRECT(samplerRECT tex, float3 sz, float2 dsdx, float2 dsdy)
        { "h2texRECTproj", "" }, // texRECTproj(samplerRECT tex, float3 sq)
        { "h2texRECTproj", "" }, // texRECTproj(samplerRECT tex, float3 szq)
        { "h2tex3D", "" }, // tex3D(sampler3D tex, float3 s)
        { "h2tex3D", "" }, // tex3D(sampler3D tex, float3 s, float3 dsdx, float3 dsdy)
        { "h2tex3Dproj", "" }, // tex3Dproj(sampler3D tex, float4 szq)
        { "h2texCUBE", "" }, // texCUBE(samplerCUBE tex, float3 s)
        { "h2texCUBE", "" }, // texCUBE(samplerCUBE tex, float3 s, float3 dsdx, float3 dsdy)
        { "h2texCUBEproj", "" }, // texCUBEproj(samplerCUBE tex, float4 sq)
        { "ddx", "" }, // ddx(a)
        { "ddy", "" }, // ddy(a)
        // fp30
        { "pack_2half", "" },
        { "pack_4byte", "" },
        { "pack_4ubyte", "" },
        { "pack_2ushort", "" },
        { "pack_2short", "" },
        { "unpack_2half", "" },
        { "unpack_4byte", "" },
        { "unpack_4ubyte", "" },
        { "unpack_2ushort", "" },
        { "unpack_2short", "" },
        { "" }
      };
      
      bool doneSomething = false;
      if( name == NULL )
      {
        for( unsigned i = 0; intrinsics[ i ].name[0] != 0; ++i )
        {
          if( intrinsics[ i ].name[ 0 ] == '_' )
          {
            lexer.start( intrinsics[ i ].params );
            dontReadLine = true;
            getNext();
            curScope = invariantScope;
            while( curToken != tok_end_of_source )
            {
              if( !parseOuter() )
              {
                return false;
              }
            }
            dontReadLine = false;
          }
        }
      } else
      {
        if( !dontReadLine )
        {
          for( unsigned i = 0; intrinsics[ i ].name[0] != 0; ++i )
          {
            if( !strcmp( intrinsics[ i ].name, name ) )
            {
              scope_t *saveScope = curScope;
              const char *savePos = lexer.getSource();
              lexer.start( intrinsics[ i ].params );
              dontReadLine = true;
              getNext();
              curScope = invariantScope;
              while( curToken != tok_end_of_source )
              {
                if( !parseOuter() )
                {
                  return false;
                }
              }
              curScope = saveScope;
              lexer.start( savePos );
              dontReadLine = false;
              doneSomething = true;
              break;
            }
          }
        }
      }
      return doneSomething;
    }

    bool execute()
    {
      /*if( include.text() != NULL )
      {
        file_mapper::addIncludePath( include.text() );
      }*/
      //stderr = fopen( output_filename.text(), "w" );
      fprintf( stderr, "# %s\n", InputFilename.getValue().data() );

      curScope = new scope_t( invariantScope );

      char *tmp = (char*)alloca( strlen( (char_type*)InputFilename.getValue().data() ) + 100 );
      sprintf( tmp, "#include \"%s\"\n", InputFilename.getValue().data() );
      //puts( tmp );
      preprocessor.begin( tmp );
      line_number = 1;
      unsigned num_opcodes = 0;


      if( preprocessor.cur_line() )
      {
        //puts( (char*)preprocessor.cur_line() );
        lexer.start( preprocessor.cur_line() );
        getNext();

        while( curToken != tok_end_of_source )
        {
          if( !parseOuter() )
          {
            return false;
          }
        }
      }

      Module *module = new Module( OutputFilename.getValue() );
      module->setDataLayout("e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32");
      //module->setTargetTriple("spu-linux-gnu");

      curScope->translateGlobal( module, *this );
      
      if( !invariantScope->translateFunctionHeaders( module, *this ) )
      {
        return false;
      }
      if( !curScope->translateFunctionHeaders( module, *this ) )
      {
        return false;
      }
      if( !invariantScope->translateFunctionBodies( module, *this ) )
      {
        return false;
      }
      if( !curScope->translateFunctionBodies( module, *this ) )
      {
        return false;
      }
      
      for( typedefType::iterator i = typedefs.begin(); i != typedefs.end(); ++i )
      {
        if( i->second->getLlvmType() != NULL && !i->second->getIsScalar() )
        {
          module->addTypeName( i->first, i->second->getLlvmType() );
        }
      }

      //module->dump();
 
      if( verifyModule( *module, PrintMessageAction ) )
      {
        module->dump();
        cerr <<  "failed to verify\n";
      } else
      {
        //CTargetMachine target( *module, "" );
        SPUTargetMachine target( *module, "" );
        //PPC64TargetMachine target( *CurModule, "" );
        
        {
          PassManager passes;
          passes.add( new TargetData( *target.getTargetData() ) );
          
          passes.add( createCFGSimplificationPass() );
          passes.add( createPromoteMemoryToRegisterPass() );
          passes.add( createFunctionInliningPass() );
          passes.add( createInstructionCombiningPass() );
          passes.add( createScalarReplAggregatesPass() );
          //passes.add( createLoopUnrollPass() );
          passes.run( *module );
        }
        
        module->dump();

        /*
        {        
          std::ofstream out( "test.c" );
          PassManager passes;
          passes.add( new TargetData( *target.getTargetData() ) );
          if( !target.addPassesToEmitWholeFile( passes, out, TargetMachine::AssemblyFile, false ) )
          {
            passes.run( *module );
          }
        }
        */

        {
          FunctionPassManager passes( new ExistingModuleProvider( module ) );
          passes.add( new TargetData( *target.getTargetData() ) );
          
          std::ofstream out( "test.s" );
          FileModel::Model model = target.addPassesToEmitFile( passes, out, TargetMachine::AssemblyFile, true );

          MachineCodeEmitter *emitter =
            model == FileModel::MachOFile ? AddMachOWriter( passes, out, target ) :
            model == FileModel::ElfFile ? AddELFWriter( passes, out, target ) :
            0
          ;

          if( !target.addPassesToEmitFileFinish( passes, emitter, true ) )
          {
            passes.doInitialization();
          
            for( Module::iterator i = module->begin(); i != module->end(); ++i )
            {
              if( !i->isDeclaration() )
              {
                passes.run( *i );
              }
            }
            
            passes.doFinalization();
          }
        }
        
      }
      return true;
    }
  };
}

int main( int argc, char *argv[] )
{
  sncgc::compiler cgcompiler;
  /*if( !cgcompiler.parse( argc, argv ) )
  {
    return 1;
  }*/
  cl::ParseCommandLineOptions(
    argc,
    argv,
    "sncgc cg compiler\n"
  );
  if( !cgcompiler.execute() )
  {
    exit( 1 );
  }
  return 0;
}

/*
        addPass(PM, createGlobalOptimizerPass());      // Optimize out global vars
        addPass(PM, createGlobalDCEPass());            // Remove unused fns and globs
        addPass(PM, createIPConstantPropagationPass());// IP Constant Propagation
        addPass(PM, createDeadArgEliminationPass());   // Dead argument elimination
        addPass(PM, createInstructionCombiningPass()); // Clean up after IPCP & DAE
        addPass(PM, createCFGSimplificationPass());    // Clean up after IPCP & DAE

        addPass(PM, createPruneEHPass());              // Remove dead EH info

        if (!DisableInline)
          addPass(PM, createFunctionInliningPass());   // Inline small functions
        addPass(PM, createArgumentPromotionPass());    // Scalarize uninlined fn args

        addPass(PM, createTailDuplicationPass());      // Simplify cfg by copying code
        addPass(PM, createSimplifyLibCallsPass());     // Library Call Optimizations
        addPass(PM, createInstructionCombiningPass()); // Cleanup for scalarrepl.
        addPass(PM, createJumpThreadingPass());        // Thread jumps.
        addPass(PM, createCFGSimplificationPass());    // Merge & remove BBs
        addPass(PM, createScalarReplAggregatesPass()); // Break up aggregate allocas
        addPass(PM, createInstructionCombiningPass()); // Combine silly seq's
        addPass(PM, createCondPropagationPass());      // Propagate conditionals

        addPass(PM, createTailCallEliminationPass());  // Eliminate tail calls
        addPass(PM, createCFGSimplificationPass());    // Merge & remove BBs
        addPass(PM, createReassociatePass());          // Reassociate expressions
        addPass(PM, createLoopRotatePass());
        addPass(PM, createLICMPass());                 // Hoist loop invariants
        addPass(PM, createLoopUnswitchPass());         // Unswitch loops.
        addPass(PM, createLoopIndexSplitPass());       // Index split loops.
        addPass(PM, createInstructionCombiningPass()); // Clean up after LICM/reassoc
        addPass(PM, createIndVarSimplifyPass());       // Canonicalize indvars
        addPass(PM, createLoopUnrollPass());           // Unroll small loops
        addPass(PM, createInstructionCombiningPass()); // Clean up after the unroller
        addPass(PM, createGVNPass());                  // Remove redundancies
        addPass(PM, createMemCpyOptPass());            // Remove memcpy / form memset
        addPass(PM, createSCCPPass());                 // Constant prop with SCCP

        // Run instcombine after redundancy elimination to exploit opportunities
        // opened up by them.
        addPass(PM, createInstructionCombiningPass());
        addPass(PM, createCondPropagationPass());      // Propagate conditionals

        addPass(PM, createDeadStoreEliminationPass()); // Delete dead stores
        addPass(PM, createAggressiveDCEPass());        // SSA based 'Aggressive DCE'
        addPass(PM, createCFGSimplificationPass());    // Merge & remove BBs
        addPass(PM, createStripDeadPrototypesPass());  // Get rid of dead prototypes
        addPass(PM, createDeadTypeEliminationPass());  // Eliminate dead types
        addPass(PM, createConstantMergePass());        // Merge dup global constants
*/
// C:\depot\bt\sdks\ps3\2_4_0.411\cell\samples\tutorial\PhysicsEffects\PhysicsFramework\shader\fs_mesh.cg
