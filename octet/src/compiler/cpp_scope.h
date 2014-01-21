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
    class cpp_scope
    {
    public:
    
      typedef dynarray< cpp_value * > orderType;
    private:
      dynarray< cpp_value * > valuesByOrder;
      dictionary<cpp_value *> valuesByName;
      cpp_scope *parent;
    public:
      typedef dynarray< cpp_value * >::iterator iterator;

      cpp_scope( cpp_scope *parent_ ) : parent( parent_ )
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
    
      void addValue( cpp_value *value )
      {
        //valuesByName.insert( valuePair( value->getName(), value ) );
        valuesByName[value->getName()] = value;
        valuesByOrder.push_back( value );
      }
    
      cpp_value *getExistingValue( cpp_value *value )
      {
        int index = valuesByName.get_index(value->getName());
        return index == -1 ? NULL : valuesByName.get_value(index);
      }
    
      cpp_value *lookup( const char *name )
      {
        for( cpp_scope *scope = this; scope != NULL; scope = scope->parent )
        {
          int index = valuesByName.get_index(name);
          if (index != -1) return valuesByName.get_value(index);
        }
        return NULL;
      }

      #if 0
      void translateGlobal( Module *module, fc::default_context &context )
      {
        for( iterator i = begin(); i != end(); ++i )
        {
          cpp_value *value = *i;
          const Type *llvmType = value->getType()->translate( context );
          cpp_type::kind_enum kind = value->getType()->getKind();
          if( kind != cpp_type::kind_function )
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
        for( cpp_scope::iterator i = begin(); i != end(); ++i )
        {
          cpp_value *value = *i;

          do
          {        
            const Type *llvmType = value->getType()->translate( context );
            cpp_type::kind_enum kind = value->getType()->getKind();
          
            if( kind == cpp_type::kind_function )
            {
              const Type *returnType = value->getType()->getSubType()->translate( context );
              cpp_log("processing header " << *value << "\n";
              GlobalValue::LinkageTypes linkage = GlobalValue::ExternalLinkage;
              if( value->getInit() && value->getInit()->getKind() == cpp_expr::kind_statement )
              {
                linkage = GlobalValue::InternalLinkage;
              }

              string name = value->getName();
              if( name == "main" )
              {
                name = "$main";
                linkage = GlobalValue::ExternalLinkage;
              }

              Function *function = Function::Create( cast<FunctionType>( llvmType ), linkage, name, module );
              function->setCallingConv( CallingConv::C );
              value->setLlvmValue( function );
              cpp_log("  " << *function << "\n";
            }
            value = value->getNextPolymorphic();
          } while( value != NULL );
        }
        return true;
      }
    
      bool translateFunctionBodies( Module *module, fc::default_context &context )
      {
        for( cpp_scope::iterator i = begin(); i != end(); ++i )
        {
          cpp_value *value = *i;

          do
          {
            const Type *llvmType = value->getType()->translate( context );
            cpp_type::kind_enum kind = value->getType()->getKind();
            if( kind == cpp_type::kind_function )
            {
              cpp_log("processing function " << *value << "\n";
              Value *llvmValue = value->getLlvmValue();
              assert( llvmValue != NULL && "expected non-null value" );
              Function *function = cast<Function>( llvmValue );
              if( value->getInit() && value->getInit()->getKind() == cpp_expr::kind_statement )
              {
                BasicBlock *bb = BasicBlock::Create( "entry", function );
                cpp_scope *paramScope = value->getType()->getScope();
                paramScope->translateFormalParams( bb, context, value->getType(), function );
                if( !value->getInit()->getStatement()->translate( bb, context, value->getType() ) )
                {
                  return false;
                }
                if( bb->getTerminator() == NULL )
                {
                  const Type *returnType = function->getType()->getContainedType(0)->getContainedType(0);
                  cpp_log(*returnType << "\n";
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

      void translateFormalParams( BasicBlock *&bb, fc::default_context &context, cpp_type *cgFunctionType, Function *function )
      {
        Function::ArgumentListType &argList = function->getArgumentList();
        Function::ArgumentListType::iterator argp = argList.begin();
        iterator valuep = begin();
        for( ; argp != argList.end() && valuep != end(); ++valuep, ++argp )
        {
          cpp_value *cgValue = *valuep;
          cpp_type *cgType = cgValue->getType();
          const Type *llvmType = cgType->translate( context );
          Value *llvmFormal = (Value*)argp;
          if( trace_translate )
          {
            cpp_log("arg " << *cgValue << " llvmt: " << *llvmType << "\n";
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
          cpp_value *value = *i;
          const Type *llvmType = value->getType()->translate( context );
          cpp_type::kind_enum kind = value->getType()->getKind();
          Value *llvmValue = new AllocaInst( llvmType, value->getName(), bb );
          value->setLlvmValue( llvmValue );
        }
        return true;
      }
      #endif
      const char *toString(string &buf) {
        buf.format("");
        return buf.c_str();
      }
    };

  }

}
