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
    class cpp_type;
    class cpp_statement;
    class cpp_scope;
    class cpp_value;

    class cpp_expr
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
      cpp_expr *kid[ 3 ];
      union
      {
        cpp_statement *statement;
        cpp_value *value;
        double double_value;
        long long int_value;
        unsigned char swiz[ 4 ];
      };
      cpp_type *type;
    public:
      cpp_expr( kind_enum kind_, cpp_type *type_, cpp_expr *lhs_ = 0, cpp_expr *rhs_ = 0, cpp_expr *rhs2_ = 0 ) : kind( kind_ )
      {
        type = type_;
        kid[ 0 ] = lhs_;
        kid[ 1 ] = rhs_;
        kid[ 2 ] = rhs2_;
      }
    
      cpp_expr( kind_enum kind_, cpp_expr *lhs_, cpp_value *value_ ) : kind( kind_ )
      {
        kid[ 0 ] = lhs_;
        kid[ 1 ] = NULL;
        kid[ 2 ] = NULL;
        value = value_;
        type = value_->getType();
      }
    
      cpp_expr( kind_enum kind_, cpp_value *value_ ) : kind( kind_ )
      {
        kid[ 0 ] = NULL;
        kid[ 1 ] = NULL;
        kid[ 2 ] = NULL;
        value = value_;
        //type = new cpp_type( cpp_type::kind_ptr, value_->getType() );
        type = value_->getType();
      }
    
      cpp_expr( kind_enum kind_, cpp_type *type_, double value ) : kind( kind_ )
      {
        kid[ 0 ] = NULL;
        kid[ 1 ] = NULL;
        kid[ 2 ] = NULL;
        double_value = value;
        type = type_;
      }
    
      cpp_expr( kind_enum kind_, cpp_type *type_, long long value ) : kind( kind_ )
      {
        kid[ 0 ] = NULL;
        kid[ 1 ] = NULL;
        kid[ 2 ] = NULL;
        int_value = value;
        type = type_;
      }
    
      cpp_expr( kind_enum kind_, cpp_statement *statement_ ) : kind( kind_ )
      {
        kid[ 0 ] = NULL;
        kid[ 1 ] = NULL;
        kid[ 2 ] = NULL;
        statement = statement_;
        type = NULL;
      }
    
      cpp_type *getType()
      {
        return type;
      }
    
      void setType( cpp_type *t )
      {
        type = t;
      }
    
      cpp_expr *getKid( size_t n )
      {
        return kid[ n ];
      }

      kind_enum getKind()
      {
        return kind;
      }

      cpp_statement *getStatement()
      {
        return statement;
      }

      cpp_value *getValue()
      {
        return value;
      }
    
      void setValue( cpp_value *v )
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
    
      #if 0
      // generate an llvm pointer to this cg expression
      Value *translate_lvalue( BasicBlock *&bb, fc::default_context &context, bool isConstant )
      {
        Value *result = NULL, *lhs = NULL;
        if( trace_translate )
        {
          cpp_log(context.push() << "lvalue " << kindName( kind ) << "\n";
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
            cpp_log(*lhs->getType() << "\n";
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
          cpp_log(context.indent() << "type failed\n";
          goto crash_n_burn;
        }
        if( trace_translate )
        {
          cpp_log(context.pop() << "/lvalue " << kindName( kind ) << "" << *result << " [" << *llvmType << "/" << *result->getType() << "]\n";
        }
        assert( llvmType == result->getType() );
        return result;
      crash_n_burn:
        if( trace_translate )
        {
          cpp_log(context.pop() << "error\n";
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
          cpp_log(context.push() << "rvalue " << kindName( kind ) << "\n";
        }
      
        if( llvmType == NULL )
        {
          cpp_log(context.indent() << "type failed\n";
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
            //assert( getKid( 0 )->getKind() == cpp_expr::kind_deref && getKid( 0 )->getKid( 0 )->getKind() == cpp_expr::kind_value_ptr );
            assert( getKid( 0 )->getKind() == cpp_expr::kind_value );
          
            cpp_value *firstValue = getKid( 0 )->getValue();
            assert( firstValue->getType()->getKind() == cpp_type::kind_function );
            cpp_log(context.indent() << "translating call to function " << firstValue->getName().c_str() << "\n";

            dynarray< cpp_expr * > valuePtrs;
            dynarray< Value * > llvmActuals;
            dynarray< const Type * > llvmTypes;
          
            dynarray< cpp_type * > actualTypes;
            if( getKid( 1 )->getKind() != cpp_expr::kind_comma )
            {
              actualTypes.push_back( getKid( 1 )->getType() );
              valuePtrs.push_back( getKid( 1 ) );
            } else
            {
              for( cpp_expr *expr = getKid( 1 ); ; expr = expr->getKid( 0 ) )
              {
                if( expr->getKind() == cpp_expr::kind_comma )
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
          
            for( dynarray< cpp_type * >::iterator i = actualTypes.begin(); i != actualTypes.end(); ++i )
            {
              const Type *type = (*i)->translate( context );
              if( type == NULL ) goto crash_n_burn;
              llvmTypes.push_back( type );
            }
          
            cpp_value *value = NULL;
            for( cpp_value *search = firstValue; search != NULL; search = search->getNextPolymorphic() )
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
                  cpp_log(context.errorText() << "more than one function matches parameters\n";
                  goto crash_n_burn;
                }
                value = search;
              }
            }
          
            if( value == NULL )
            {
              cpp_log(context.errorText() << "none of the functions called " << firstValue->getName() << " match the parameters\n";
              goto crash_n_burn;
            }
          
            cpp_type *functionType = value->getType();
          
            Value *llvmFunction = value->getLlvmValue();
            assert( llvmFunction != NULL );
          
            cpp_type *returnType = functionType->getSubType();
            Value *returnPtr = NULL;

            if( returnType->getIsPassByPtr() )
            {
              const Type *llvmType = returnType->translate( context );
              if( llvmType == NULL ) goto crash_n_burn;
              returnPtr = new AllocaInst( llvmType, "retValue", bb );
              llvmActuals.push_back( returnPtr );
            }
          
            dynarray< cpp_type * >::iterator actualType = actualTypes.begin();
            dynarray< cpp_expr * >::iterator valuePtr = valuePtrs.begin();
            dynarray< Value * > llvmLoads;
            //cpp_scope::iterator valueIterator = functionType->getScope()->begin();
          
            while( actualType != actualTypes.end() )
            {
              cpp_type *cgActualType = actualType[0];
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
                  cpp_log(*store << "\n";
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
            cpp_log(*call << "\n";
          
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

            dynarray< Constant * >indices;
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
          
            cpp_log("lhs " << *lhs << "\n";
            cpp_log("this " << *this->getType() << "\n";
            cpp_log("kid0 " << *kid[0]->getType() << "\n";
          
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

            cpp_log(*lhs->getType() << "\n";
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
            /*if( getKid( 0 )->getType()->getKind() == cpp_type::kind_bool )
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
              cpp_log(context.errorText() << "unsupported compare\n";
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
              cpp_log(context.errorText() << "assigment can't be used in constant expression\n";
              return NULL;
            }
            rhs = getKid( 1 )->translate_rvalue( bb, context, isConstant );
            if( rhs == NULL ) goto crash_n_burn;

            if( getKid( 0 )->getKind() == kind_swiz )
            {
              cpp_expr *swiz = getKid( 0 );
              Value *ptr = swiz->getKid( 0 )->translate_lvalue( bb, context, false );
              if( ptr == NULL ) goto crash_n_burn;
              cpp_log(*ptr << "\n";
            
              Value *insert = NULL;
              if( swiz->getType()->getIsScalar() )
              {
                Value *load = new LoadInst( ptr, "", bb );
                cpp_log(*load << "\n";
                insert = InsertElementInst::Create( load, rhs, ConstantInt::get( Type::Int32Ty, swiz->getSwiz()[0] ), "", bb );
              } else if( swiz->getType()->getIsVector() )
              {
                unsigned dim = swiz->getType()->getDimension();
                Value *load = new LoadInst( ptr, "", bb );
                cpp_log(*load << "\n";
                dynarray< Constant * > indices;
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
                cpp_log(context.errorText() << "only swizzle of scalar or vector supported\n";
                goto crash_n_burn;
              }
              cpp_log(*insert << "\n";
              Value *store = new StoreInst( insert, ptr, false, 16, bb );
              cpp_log(*store << "\n";
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
              cpp_log(*store << "\n";
              result = rhs;
            }
          } break;
          default:
          {
            cpp_log(context.errorText() << "not implemented\n";
            return NULL;
          }
        }
        if( result != NULL )
        {
          if( trace_translate )
          {
            cpp_log(context.pop() << "/rvalue " << kindName( kind ) << "" << *result << " [" << *llvmType << "/" << *result->getType() << "]\n";
          }
          assert( llvmType == result->getType() );
          return result;
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

