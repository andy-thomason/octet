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
    class cpp_expr;
    class cpp_scope;
    class cpp_statement;
    class cpp_value;

    class cpp_value
    {
      cpp_type *valueType;
      string name;
      string semantic;
      unsigned offset;
      cpp_expr *init;
      //Value *llvmValue;
    
      // functions have the same name, but multiple types.
      // if we don't find a match for the first one, we can try another.
      cpp_value *nextPolymorphicValue;
    public:
      cpp_value( cpp_type *valueType_, const char *name_ ) : valueType( valueType_ ), name( name_ )
      {
        init = NULL;
        offset = 0;
        //llvmValue = NULL;
        nextPolymorphicValue = NULL;
      }
    
      cpp_value *getNextPolymorphic()
      {
        return nextPolymorphicValue;
      }

      void setNextPolymorphic( cpp_value *value )
      {
        nextPolymorphicValue = value;
      }

      const char *getName() const
      {
        return name.c_str();
      }

      void setSemantic( const char *name_ )
      {
        semantic = name_;
      }
    
      const char *getSemantic()
      {
        return name;
      }
    
      cpp_type *getType() const
      {
        return valueType;
      }
    
      void setType( cpp_type *type )
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
    
      void setInit( cpp_expr *value )
      {
        init = value;
      }

      cpp_expr *getInit()
      {
        return init;
      }
    
      #if 0
      void setLlvmValue( Value *value )
      {
        cpp_log("setLlvmValue" << name << " = " << *value << "\n";
        llvmValue = value;
      }
    
      Value *getLlvmValue()
      {
        return llvmValue;
      }
      #endif

      const char *toString(string &buf) {
        buf.format("");
        return buf.c_str();
      }
    };
  }
}

