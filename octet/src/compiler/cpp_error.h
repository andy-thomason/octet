namespace octet
{
  namespace compiler
  {
    struct cpp_error
    {
      enum error_
      {
        none,
        stack_overflow,
        line_too_long,
        unterminated_comment,
        unknown,
        file_not_found,
        syntax,
        expect,
        internal,
      };
    };

  }

}
