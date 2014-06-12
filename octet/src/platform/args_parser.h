////////////////////////////////////////////////////////////////////////////////
//
// Rosalind sequencing tools (C) Andy Thomason 2014
//
// Command line tool argument parser
//
////////////////////////////////////////////////////////////////////////////////

namespace octet { namespace platform {
  /// argument parser class
  class args_parser {
    dynarray<const char *> opt_args;
    dynarray<const char *> free_args;
    dynarray<size_t> opt_lengths;
    const char * const *opts;
    const char *error;
    const char *error_arg;

    static const char *find_end_of_option(const char *str) {
      while (*str && *str != ' ' && *str != '<') ++str;
      return str;
    }

    static const char *find_end_of_arg(const char *str) {
      while (*str && *str != '=') ++str;
      return str;
    }
  
    bool opt_has_arg(int opt_idx) {
      const char *src = opts[opt_idx*2 + 1];
      return src[opt_lengths[opt_idx]] != 0;
    }

    int find_opt(const char *begin, const char *end) {
      size_t arg_len = end - begin;
      for (size_t i = 0; i != opt_lengths.size(); ++i) {
        const char *src = opts[i*2 + 1];
        size_t opt_len = opt_lengths[i];
        if (opt_len == arg_len && !memcmp(src, begin, opt_len)) {
          return (int)i;
        }
      }
      return -1;
    }

    void init(int argc, const char * const * argv, const char * const * _opts) {
      opts = _opts;
      error = 0;

      int num_opt_strings = 0;
      for (; opts[num_opt_strings]; ++num_opt_strings) {
      }

      if (num_opt_strings < 3 || (num_opt_strings & 1) == 0) {
        error = "wrong number of strings for args_parser()";
        return;
      }

      size_t num_opts = (num_opt_strings-1)/2;
      opt_args.resize(num_opts);
      opt_lengths.resize(num_opts);

      for (size_t i = 0; i != num_opts; ++i) {
        const char *src = opts[i*2 + 1];
        const char *end = find_end_of_option(src);
        opt_lengths[i] = end - src;
        opt_args[i] = "";
      }

      for (int argv_idx = 1; argv_idx != argc; ++argv_idx) {
        const char *arg = argv[argv_idx];
        error_arg = arg;
        if (arg[0] == '-') {
          const char *end = find_end_of_arg(arg);
          int opt_idx = find_opt(arg, end);
          if (opt_idx == -1) {
            error = "bad argument";
            return;
          }
          size_t opt_len = opt_lengths[opt_idx];
          bool has_equals = arg[opt_len] == '=';
          if (opt_has_arg(opt_idx)) {
            // option with argument
            if (has_equals) {
              opt_args[opt_idx] = arg + opt_len + 1;
            } else if (argv_idx != argc-1) {
              opt_args[opt_idx] = argv[++argv_idx];
            } else {
              error = "bad last argument";
              return;
            }
          } else {
            // option without argument (boolean)
            if (has_equals) {
              error = "not expecting =";
              return;
            } else {
              opt_args[opt_idx] = "y";
            }
          }
        } else {
          free_args.push_back(arg);
        }
      }
    }

  public:
    enum arg_kind {
      arg_bool,
      arg_string,
    };

    args_parser(int argc, const char * const * argv, const char * const * _opts) {
      init(argc, argv, _opts);
    }
  
    void usage() {
      puts(opts[0]);
      for (int i = 1; opts[i] && opts[i+1]; i+=2) {
        printf("%-20s %s\n", opts[i], opts[i+1]);
      }
    }
  
    const char *operator[](int index) {
      return (size_t)index < free_args.size() ? free_args[index] : 0;
    }

    const char *operator[](const char *opt_name) {
      int opt = find_opt(opt_name, find_end_of_option(opt_name));
      return opt < 0 ? "" : opt_args[opt];
    }

    const char *get_error() {
      return error;
    }

    const char *get_error_arg() {
      return error_arg;
    }
  };

} }
