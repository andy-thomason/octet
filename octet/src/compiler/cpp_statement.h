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
    class cpp_statement
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
        cpp_statement *statements;
        cpp_expr *expression;
      };
      cpp_statement *next;
      cpp_scope *scope;
    public:
      cpp_statement( kind_enum kind_ ) : kind( kind_ ), statements( NULL ), next( NULL )
      {
      }
    
      cpp_statement **getStatementsAddr()
      {
        return &statements;
      }

      cpp_statement *getStatements()
      {
        return statements;
      }

      cpp_expr *getExpression()
      {
        return expression;
      }

      void setExpression( cpp_expr *expr )
      {
        expression = expr;
      }
    
      cpp_statement **getNextAddr()
      {
        return &next;
      }

      cpp_statement *getNext()
      {
        return next;
      }

      cpp_scope *getScope()
      {
        return scope;
      }

      void setScope( cpp_scope *scope_ )
      {
        scope = scope_;
      }
    
      #if 0
      BasicBlock *translate( BasicBlock *bb, fc::default_context &context, cpp_type *functionType )
      {
        switch( kind )
        {
          case kind_compound:
          {
            if( !translate_scope( scope, bb, context ) )
            {
              goto crash_n_burn;
            }
            //cpp_log(context.push() << "kind_compound\n";
            for( cpp_statement *statement = statements; statement != NULL; statement = statement->next )
            {
              bb = statement->translate( bb, context, functionType );
              if( bb == NULL ) goto crash_n_burn;
            }
            //cpp_log(context.pop() << "/kind_compound\n";
          } break;
          case kind_expression:
          {
            //cpp_log(context.push() << "kind_expression\n";
            if( expression != NULL )
            {
              Value *expr = expression->translate_rvalue( bb, context, false );
              if( expr == NULL ) goto crash_n_burn;
            }
            //cpp_log(context.pop() << "/kind_expression\n";
          } break;
          case kind_declaration:
          {
            //cpp_log(context.push() << "kind_declaration\n";
            if( expression != NULL )
            {
              Value *expr = expression->translate_rvalue( bb, context, false );
              if( expr == NULL ) goto crash_n_burn;
            }
            //cpp_log(context.pop() << "/kind_declaration\n";
          } break;
          case kind_return:
          {
            //cpp_log(context.push() << "kind_return\n";
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
            //cpp_log(*ret << "\n";
            //cpp_log(context.pop() << "/kind_return\n";
          } break;
          case kind_discard:
          {
            //cpp_log(context.push() << "kind_discard\n";
            //cpp_log(context.pop() << "/kind_discard\n";
          } break;
          case kind_if:
          {
            //cpp_log(context.push() << "kind_if\n";
            //cpp_log(context.pop() << "/kind_if\n";
          } break;
          case kind_for:
          {
            //cpp_log(context.push() << "kind_for\n";
            //cpp_log(context.pop() << "/kind_for\n";
          } break;
          case kind_while:
          {
            //cpp_log(context.push() << "kind_while\n";
            //cpp_log(context.pop() << "/kind_while\n";
          } break;
          case kind_dowhile:
          {
            //cpp_log(context.push() << "kind_dowhile\n";
            //cpp_log(context.pop() << "/kind_dowhile\n";
          } break;
          default:
          {
            assert( 0 && "unknown statement kind" );
          } break;
        }
        return bb;
      crash_n_burn:
        //cpp_log(context.pop() << "error\n";
        return NULL;
      }
      #endif
    };
  
  }
}
