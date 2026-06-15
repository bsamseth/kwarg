#pragma once

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct kwarg_init_opts_t;
struct kwarg_parser_t;
struct kwarg_argspec_t;
struct kwarg_finish_opts_t;
typedef struct kwarg_init_opts_t kwarg_init_opts_t;
typedef struct kwarg_parser_t kwarg_parser_t;
typedef struct kwarg_argspec_t kwarg_argspec_t;
typedef struct kwarg_finish_opts_t kwarg_finish_opts_t;
typedef enum {
  KWARG_PARSE_OK,
  KWARG_HELP_INVOKED,
  KWARG_PARSE_FAILED = -1,
} kwarg_result_t;

///////////////////////////////////////////////////////////////////////////////
// Public API
///////////////////////////////////////////////////////////////////////////////

// Initialize an argument parser from the command line arguments.
//
// Optional named arguments (kwarg_init_opts_t):
//   - .no_help (bool): Disable the automatic --help/-h flag (default: false).
//   - .no_args_shows_help (bool): Show help when no arguments are given
//     (default: false).
//   - .tagline (const char *): Short tagline shown at the top of help output.
//   - .description (const char *): Description text shown after the tagline in
//     help output.
//   - .explicit_usage (const char *): Custom usage string in place of the
//     auto-generated one.
#define kwarg_init(argc, argv, ...)                                            \
  kwarg_init_from_opts((argc), (argv), (kwarg_init_opts_t){__VA_ARGS__})

// Parse a flag-style option that returns how many times the flag was provided.
//
// Optional named arguments (kwarg_argspec_t):
//   - .name (const char *): Long flag name, e.g. "--verbose".
//   - .short_name (char): Single-character short flag, e.g. 'v'.
//   - .help (const char *): Help text displayed for this flag.
#define kwarg_flag(parser, ...)                                                \
  kwarg_flag_from_opts((parser), (kwarg_argspec_t){__VA_ARGS__})

// Parse a string argument (positional) or string option (--name=value or
// -n value).
//
// A short_name and/or a name beginning with "-" makes this an option;
// otherwise it is treated as a positional argument.
//
// Optional named arguments (kwarg_argspec_t):
//   - .name (const char *): Long option name (e.g. "--output") or positional
//     argument name (e.g. "FILE").
//   - .short_name (char): Single-character short option, e.g. 'o'.
//   - .help (const char *): Help text displayed for this argument.
//   - .required (bool): Mark this option as required (only meaningful for
//     options, not positional arguments).
#define kwarg_str(parser, ...)                                                 \
  kwarg_str_from_opts((parser), (kwarg_argspec_t){__VA_ARGS__})

// Finalize argument parsing.
//
// By default this will call `exit(1)` if parsing failed, or `exit(0)` if
// --help was invoked. This can be disabled with the appropriate option.
//
// Optional named arguments (kwarg_finish_opts_t):
//   - .no_exit_on_failure (bool): Return KWARG_PARSE_FAILED instead of
//     calling exit(1) (default: false).
//   - .no_exit_on_help (bool): Return KWARG_HELP_INVOKED instead of
//     calling exit(0) (default: false).
//
// See `kwarg_finish_from_opts` for the return value.
#define kwarg_finish(parser, ...)                                              \
  kwarg_finish_from_opts((parser), (kwarg_finish_opts_t){__VA_ARGS__})

///////////////////////////////////////////////////////////////////////////////
// Backend of public API (usable if you don't want macros).
///////////////////////////////////////////////////////////////////////////////

/// Initialize an argument parser from the command line arguments.
///
/// See kwarg_init_opts_t for available options.
kwarg_parser_t kwarg_init_from_opts(int argc, const char *argv[static argc],
                                    kwarg_init_opts_t opts);
// Parse a flag-style option that returns how many times the flag was provided.
///
/// See kwarg_argspec_t for available options.
unsigned kwarg_flag_from_opts(kwarg_parser_t *parser, kwarg_argspec_t opts);
/// Parse a string argument (positional) or string option (--name=value or
/// -n value).
///
/// A short_name and/or a name beginning with "--" makes this an option;
/// otherwise it is treated as a positional argument.
///
/// See kwarg_argspec_t for available options.
const char *kwarg_str_from_opts(kwarg_parser_t *parser, kwarg_argspec_t opts);
// Finalize argument parsing.
//
// - Returns `KWARG_PARSE_OK` (0) if parsing was successful.
// - Returns `KWARG_HELP_INVOKED` (1) if --help was provided. In this case
//   parsing was skipped and all args have been zero-initialized. You should
//   exit the program if this is returned.
// - Returns `KWARG_PARSE_FAILED` (-1) if parsing failed in any way.
//   Diagnostics have been written to stderr and you should exit the program if
//   this was returned.
//
// See kwarg_finish_opts_t for available options.
kwarg_result_t kwarg_finish_from_opts(kwarg_parser_t *parser,
                                      kwarg_finish_opts_t opts);

///////////////////////////////////////////////////////////////////////////////
// Structures
///////////////////////////////////////////////////////////////////////////////

// Options for kwarg_init / kwarg_init_from_opts.
//
// Members:
//   - no_help: Disable the automatic --help/-h flag (default: false).
//   - no_args_shows_help: Show help when no arguments are given
//     (default: false).
//   - tagline: Short tagline shown at the top of help output.
//   - description: Description text shown after the tagline in help output.
//   - explicit_usage: Custom usage string in place of the auto-generated one.
struct kwarg_init_opts_t {
  bool no_help;
  bool no_args_shows_help;
  const char *tagline;
  const char *description;
  const char *explicit_usage;
};

// Options for kwarg_finish / kwarg_finish_from_opts.
//
// Members:
//   - no_exit_on_failure: Return KWARG_PARSE_FAILED instead of calling
//     exit(1) (default: false).
//   - no_exit_on_help: Return KWARG_HELP_INVOKED instead of calling exit(0)
//     (default: false).
struct kwarg_finish_opts_t {
  bool no_exit_on_failure;
  bool no_exit_on_help;
};

// Options for kwarg_flag / kwarg_str and their _from_opts backends.
//
// Members:
//   - name: Long name (e.g. "--verbose", "--output"). A name beginning with
//     "--" makes this an option; otherwise it is a positional argument.
//     This parameter _must_ be provided.
//   - short_name: Single-character short form (e.g. 'v', 'o'). Should only be
//     set if `.name` has two leading `-`, as otherwise it is ambiguous whether
//     or not this is a positional argument or an option.
//   - help: Help text displayed for this argument.
//   - required: Mark this option as required (only meaningful for
//     options, it is ignored for flags and positional arguments).
//
// NB: Do not set `-foo` as the name, using a single leading `-`. This will
// trigger an assertion, because it would be parsed as the joined form of
// `-f -o -o`, and would not work the way you want.
struct kwarg_argspec_t {
  char *name;
  char short_name;
  char *help;
  bool required;
};

enum _kwarg_arg_kind_t {
  _KWARG_POSITIONAL,
  _KWARG_OPTION,
  _KWARG_FLAG,
};

typedef struct {
  enum _kwarg_arg_kind_t kind;
  kwarg_argspec_t spec;
  int provided_in_argv;
} _kwarg_tagged_argspec_t;
// "[t]ag [a]rg [s]pecs. Defined by VECTOR_IMPL below.
typedef _kwarg_tagged_argspec_t *_kwarg_tas;

struct kwarg_parser_t {
  const size_t argc;
  const char **argv;
  _kwarg_tas argspecs;
  int16_t *remaining_arg_uses;
  kwarg_init_opts_t init_opts;
  bool fail;
  bool help;
};

///////////////////////////////////////////////////////////////////////////////
// "Private" helper API
///////////////////////////////////////////////////////////////////////////////
static size_t _kwarg_arg_matches_short_opt(const char arg[static 1],
                                           const kwarg_argspec_t opts);
void _kwarg_show_help(const kwarg_parser_t *parser, FILE *out);
void _kwarg_show_usage(const kwarg_parser_t *parser, FILE *out);
void _kwarg_show_arguments_help(const kwarg_parser_t *parser, FILE *out);
void _kwarg_show_options_help(const kwarg_parser_t *parser, FILE *out);
void _kwarg_validate_argspec(const kwarg_argspec_t opts);
void _kwarg_build_usage(const kwarg_parser_t *parser, FILE *out);

#ifdef KWARG_INCLUDE_IMPLEMENTATION
#ifndef KWARG_IMPLEMENTATION_INCLUDED
#define KWARG_IMPLEMENTATION_INCLUDED

// Forward declare the few vector.h APIs we need, so that vector.h can be
// inlined at the end of this file to avoid clutter.
static inline _kwarg_tas _kwarg_tas_init(size_t);
static inline size_t _kwarg_tas_len(const _kwarg_tas);
static inline _kwarg_tagged_argspec_t *_kwarg_tas_last_ptr(_kwarg_tas);
static inline bool _kwarg_tas_push(_kwarg_tas *, _kwarg_tagged_argspec_t);
static inline void _kwarg_tas_free(_kwarg_tas);

kwarg_parser_t kwarg_init_from_opts(int argc, const char *argv[static argc],
                                    kwarg_init_opts_t opts) {
  assert(argc > 0);
  assert(argv != NULL);
  kwarg_parser_t parser = {.argc = argc,
                           .argv = argv,
                           .remaining_arg_uses =
                               calloc(argc, sizeof(*parser.remaining_arg_uses)),
                           .argspecs = _kwarg_tas_init(argc),
                           .init_opts = opts};
  assert(parser.remaining_arg_uses != NULL);

  // Short opts like -abc can be combined and should be parsed as many times
  // as there are short opts in the argument.
  // Positional and long-form options should be used once.
  for (size_t i = 1; i < parser.argc; ++i) {
    const char *arg = argv[i];
    if (arg[0] != '-' || arg[1] == '-') {
      parser.remaining_arg_uses[i] = 1;
    } else {
      for (size_t j = 1; arg[j] != '\0' && arg[j] != '='; ++j) {
        ++parser.remaining_arg_uses[i];
      }
    }
  }

  if (!opts.no_help) {
    bool help = kwarg_flag(&parser, .name = "--help", .short_name = 'h',
                           .help = "Show this help message.");
    if (help || (opts.no_args_shows_help && argc == 1)) {
      parser.help = true;
    }
  }

  return parser;
}

void _kwarg_show_help(const kwarg_parser_t *parser, FILE *out) {
  if (out == NULL)
    out = stdout;

  if (parser->init_opts.tagline)
    fprintf(out, "%s\n\n", parser->init_opts.tagline);
  if (parser->init_opts.description)
    fprintf(out, "%s\n\n", parser->init_opts.description);
  if (parser->init_opts.explicit_usage)
    fprintf(out, "Usage: %s %s\n", parser->argv[0],
            parser->init_opts.explicit_usage);
  else
    _kwarg_show_usage(parser, out);

  fprintf(out, "\n");
  _kwarg_show_arguments_help(parser, out);
  fprintf(out, "\n");
  _kwarg_show_options_help(parser, out);
}

void _kwarg_show_usage(const kwarg_parser_t *parser, FILE *out) {
  if (out == NULL)
    out = stdout;

  fprintf(out, "Usage: %s ", parser->argv[0]);
  _kwarg_build_usage(parser, out);
  fprintf(out, "\n");
}

void _kwarg_show_arguments_help(const kwarg_parser_t *parser, FILE *out) {
  if (out == NULL)
    out = stdout;

  fprintf(out, "ARGUMENTS\n");
  for (size_t i = 0; i < _kwarg_tas_len(parser->argspecs); ++i) {
    auto tas = parser->argspecs[i];
    if (tas.kind != _KWARG_POSITIONAL)
      continue;

    fprintf(out, "\t%s\t%s%s\n", tas.spec.name, tas.spec.help,
            tas.spec.required ? "" : " (optional)");
  }
}

void _kwarg_show_options_help(const kwarg_parser_t *parser, FILE *out) {
  if (out == NULL)
    out = stdout;

  if (!parser->init_opts.no_help) {
    // Help is first option if not explicitly silenced.
    // Move it into the last position instead.
    auto last_ptr = _kwarg_tas_last_ptr(parser->argspecs);
    auto tmp = *last_ptr;
    *last_ptr = parser->argspecs[0];
    parser->argspecs[0] = tmp;
  }

  fprintf(out, "OPTIONS\n");
  for (size_t i = 0; i < _kwarg_tas_len(parser->argspecs); ++i) {
    auto tas = parser->argspecs[i];
    if (tas.kind == _KWARG_POSITIONAL)
      continue;

    if (tas.spec.name && tas.spec.short_name)
      fprintf(out, "\t-%c, %s", tas.spec.short_name, tas.spec.name);
    else if (tas.spec.short_name)
      fprintf(out, "\t-%c", tas.spec.short_name);
    else
      fprintf(out, "\t%s", tas.spec.name);

    if (tas.kind == _KWARG_OPTION) {
      const char *p = tas.spec.name;
      while (*p == '-')
        p++;
      fputc('=', out);
      for (; *p; p++)
        fputc(*p >= 'a' && *p <= 'z' ? *p - 32 : *p, out);
    }
    fprintf(out, "\t%s\n", tas.spec.help);
  }
}

const char *kwarg_str_from_opts(kwarg_parser_t *parser, kwarg_argspec_t opts) {
  _kwarg_validate_argspec(opts);
  bool is_option = opts.short_name || !strncmp(opts.name, "-", 1);
  const char *user_provided_argument = NULL;
  int provided_count = 0;

  for (size_t i = 1; i < parser->argc; ++i) {
    if (parser->remaining_arg_uses[i] <= 0)
      continue;
    const char *arg = parser->argv[i];
    bool arg_is_option = arg[0] == '-';

    if (is_option != arg_is_option)
      continue;
    else if (!arg_is_option) {
      parser->remaining_arg_uses[i] = 0;
      user_provided_argument = parser->argv[i];
      provided_count++;
      continue;
    } else { // arg is option
      size_t name_len = strlen(opts.name);

      // Short-form argument matching:
      size_t short_idx = _kwarg_arg_matches_short_opt(arg, opts);
      if (short_idx && arg[short_idx + 1] == '=') {
        parser->remaining_arg_uses[i]--;
        user_provided_argument = arg + short_idx + 2;
        provided_count++;
        continue;
      } else if (short_idx == strlen(arg) - 1) {
        parser->remaining_arg_uses[i]--;
        goto consume_next_arg_as_value;
      } else if (short_idx) {
        parser->fail = true;
        parser->remaining_arg_uses[i]--;
        fprintf(stderr,
                "Error: %s: Option -%c expects a value and must be the last "
                "argument in "
                "the group.\n",
                arg, opts.short_name);
        continue;
      }

      bool long_name_eq_match =
          !strncmp(arg, opts.name, name_len) && arg[name_len] == '=';
      if (long_name_eq_match) {
        parser->remaining_arg_uses[i] = 0;
        user_provided_argument = arg + name_len + 1;
        provided_count++;
        continue;
      }

      if (!strcmp(arg, opts.name)) {
        parser->remaining_arg_uses[i] = 0;
        goto consume_next_arg_as_value;
      }

      continue;

    consume_next_arg_as_value:
      // At this point, we matched either long form or short form, expecting the
      // next argument to be the value.
      //
      if (i == parser->argc - 1) {
        parser->fail = true;
        fprintf(stderr, "Error: %s: Missing value to %s (-%c)\n", arg,
                opts.name, opts.short_name);
        continue;
      }
      parser->remaining_arg_uses[i + 1] = 0;

      user_provided_argument = parser->argv[i + 1];
      provided_count++;
      continue;
    }
  }

  _kwarg_tas_push(&parser->argspecs,
                  (_kwarg_tagged_argspec_t){
                      .kind = is_option ? _KWARG_OPTION : _KWARG_POSITIONAL,
                      .spec = opts,
                      .provided_in_argv = provided_count});
  return user_provided_argument;
}

unsigned kwarg_flag_from_opts(kwarg_parser_t *parser, kwarg_argspec_t opts) {
  _kwarg_validate_argspec(opts);
  unsigned flag_set = 0;
  for (size_t i = 1; i < parser->argc; ++i) {
    if (parser->remaining_arg_uses[i] <= 0)
      continue;

    const char *arg = parser->argv[i];

    if (!strcmp(arg, opts.name)) {
      parser->remaining_arg_uses[i] = 0;
      flag_set++;
      continue;
    }

    if (arg[0] != '-' || arg[1] == '-')
      continue;

    for (size_t j = 1; arg[j] && arg[j] != '='; ++j) {
      if (arg[j] != opts.short_name)
        continue;

      if (arg[j + 1] == '=') {
        fprintf(stderr,
                "Error: %s: Unexpected value for flag -%c. Flags are boolean "
                "and don't "
                "take a value.\n",
                arg, opts.short_name);
        parser->fail = true;
        break;
      }

      parser->remaining_arg_uses[i]--;
      flag_set++;
    }
  }

  _kwarg_tas_push(&parser->argspecs,
                  (_kwarg_tagged_argspec_t){.kind = _KWARG_FLAG,
                                            .spec = opts,
                                            .provided_in_argv = flag_set});
  return flag_set;
}

static size_t _kwarg_arg_matches_short_opt(const char arg[static 1],
                                           const kwarg_argspec_t opts) {
  // Skip positional arguments and long-form options.
  if (arg[0] != '-' || arg[1] == '-')
    return 0;

  const size_t n_opts = strlen(arg);
  for (size_t i = 1; i < n_opts && arg[i] != '='; ++i) {
    if (arg[i] == opts.short_name)
      return i;
  }
  return 0;
}

kwarg_result_t kwarg_finish_from_opts(kwarg_parser_t *parser,
                                      kwarg_finish_opts_t opts) {
  if (parser->help) {
    _kwarg_show_help(parser, NULL);
    goto cleanup;
  }

  for (size_t i = 1; i < parser->argc; ++i) {
    if (parser->remaining_arg_uses[i] > 0) {
      parser->fail = true;
      const char *arg = parser->argv[i];

      // Unexpected positional and long-form option:
      if (arg[0] != '-' || arg[1] == '-') {
        fprintf(stderr, "Unexpected argument: %s\n", arg);
        continue;
      }

      // Unexpected short-form option.
      // We make sure to only output diagnostics for the short-form options
      // that aren't defined, in case of joined options like `-abc`. If `-a`
      // is defined, then diagnostics are only emitted for `-b` and `-c`.
      for (const char *opt = parser->argv[i] + 1; *opt && *opt != '='; ++opt) {
        bool opt_is_defined = false;
        for (size_t j = 0; j < _kwarg_tas_len(parser->argspecs); ++j) {
          auto spec = parser->argspecs[j];
          if (spec.spec.short_name == *opt) {
            opt_is_defined = true;
            break;
          }
        }
        if (!opt_is_defined) {
          fprintf(stderr, "Unknown short option '%c' in argument \"%s\"\n",
                  *opt, arg);
          if (--parser->remaining_arg_uses[i] == 0)
            break;
        }
      }
    }
  }

  for (size_t i = 0; i < _kwarg_tas_len(parser->argspecs); ++i) {
    auto spec = parser->argspecs[i];
    if (spec.kind == _KWARG_POSITIONAL && !spec.provided_in_argv &&
        spec.spec.required) {
      fprintf(stderr, "Error: Missing positional argument: %s\n",
              spec.spec.name);
      parser->fail = true;
    } else if (spec.kind == _KWARG_OPTION && !spec.provided_in_argv &&
               spec.spec.required) {
      fprintf(stderr, "Error: Missing required option: %s\n", spec.spec.name);
      parser->fail = true;
    }
  }

cleanup:
  if (!parser->help && parser->fail) {
    _kwarg_show_usage(parser, stderr);
  }
  free(parser->remaining_arg_uses);
  _kwarg_tas_free(parser->argspecs);

  kwarg_result_t result = parser->help   ? KWARG_HELP_INVOKED
                          : parser->fail ? KWARG_PARSE_FAILED
                                         : KWARG_PARSE_OK;
  switch (result) {
  case KWARG_PARSE_OK:
    return result;
  case KWARG_HELP_INVOKED:
    if (!opts.no_exit_on_help)
      exit(0);
    return KWARG_HELP_INVOKED;
  case KWARG_PARSE_FAILED:
    if (!opts.no_exit_on_failure)
      exit(1);
    return KWARG_PARSE_FAILED;
  default:
    unreachable();
  }
}

void _kwarg_debug_dump_argspec(const kwarg_argspec_t opts) {
  fprintf(stderr,
          "{\n\t.name = \"%s\",\n\t.short_name = "
          "'%c',\n\t.required = %s,\n\t.help = \"%s\"\n}\n",
          opts.name, opts.short_name, opts.required ? "true" : "false",
          opts.help);
}

void _kwarg_validate_argspec(const kwarg_argspec_t opts) {
  if (opts.name == NULL) {
    fprintf(stderr, "Invalid specification:\n");
    _kwarg_debug_dump_argspec(opts);
    fprintf(stderr,
            "You must specify the .name field for any options/flags.\n");
    exit(1);
  }
  if (opts.name[0] == '-' && opts.name[1] != '-') {
    fprintf(stderr, "Invalid specification:\n");
    _kwarg_debug_dump_argspec(opts);
    fprintf(stderr,
            "Argument names cannot start with a single `-`, they must either "
            "not start with a `-` or start with `--`.\n");
    exit(1);
  }
  if (opts.short_name == '-' || opts.short_name == '=') {
    fprintf(stderr, "Invalid specification:\n");
    _kwarg_debug_dump_argspec(opts);
    fprintf(stderr, "Short names cannot use the `-` or '=' characters.\n");
    exit(1);
  }
}

void _kwarg_build_usage(const kwarg_parser_t *parser, FILE *out) {
  bool first = true;

  // Flags with short_names, grouped together: e.g. [-abchv]
  {
    size_t n = 0;
    bool any_optional = false;
    for (size_t i = 0; i < _kwarg_tas_len(parser->argspecs); ++i) {
      auto tas = parser->argspecs[i];
      if (tas.kind != _KWARG_FLAG)
        continue;
      if (!tas.spec.short_name)
        continue;
      n++;
      if (!tas.spec.required)
        any_optional = true;
    }
    if (n > 0) {
      if (!first)
        fputc(' ', out);
      first = false;
      if (any_optional)
        fputc('[', out);
      fputc('-', out);
      for (size_t i = 0; i < _kwarg_tas_len(parser->argspecs); ++i) {
        auto tas = parser->argspecs[i];
        if (tas.kind != _KWARG_FLAG)
          continue;
        if (!tas.spec.short_name)
          continue;
        fputc(tas.spec.short_name, out);
      }
      if (any_optional)
        fputc(']', out);
    }
  }

  // Long-only flags (no short_name): printed individually
  for (size_t i = 0; i < _kwarg_tas_len(parser->argspecs); ++i) {
    auto tas = parser->argspecs[i];
    if (tas.kind != _KWARG_FLAG)
      continue;
    if (tas.spec.short_name)
      continue;
    if (!first)
      fputc(' ', out);
    first = false;
    if (!tas.spec.required)
      fputc('[', out);
    fputs(tas.spec.name, out);
    if (!tas.spec.required)
      fputc(']', out);
  }

  // Non-flag options (take a value): printed individually
  for (size_t i = 0; i < _kwarg_tas_len(parser->argspecs); ++i) {
    auto tas = parser->argspecs[i];
    if (tas.kind != _KWARG_OPTION)
      continue;
    if (!first)
      fputc(' ', out);
    first = false;
    if (!tas.spec.required)
      fputc('[', out);
    if (tas.spec.short_name) {
      fprintf(out, "-%c", tas.spec.short_name);
    } else {
      fputs(tas.spec.name, out);
    }
    fputc('=', out);
    const char *p = tas.spec.name;
    while (*p == '-')
      p++;
    for (; *p; p++)
      fputc(*p >= 'a' && *p <= 'z' ? *p - 32 : *p, out);
    if (!tas.spec.required)
      fputc(']', out);
  }

  // Positional arguments
  for (size_t i = 0; i < _kwarg_tas_len(parser->argspecs); ++i) {
    auto tas = parser->argspecs[i];
    if (tas.kind != _KWARG_POSITIONAL)
      continue;
    if (!first)
      fputc(' ', out);
    first = false;
    if (!tas.spec.required)
      fputc('[', out);
    fputs(tas.spec.name, out);
    if (!tas.spec.required)
      fputc(']', out);
  }
}

// =============================================================================
//
// The remainder of this file is a copy of vector.h from
// github.com/bsamseth/vector. It has been inlined into kwarg.h in order to not
// expose an implementation detail as another file you need to place in your
// project.
// =============================================================================
#include <stdbool.h>
#include <stddef.h>

/**
 * Instantiate a vector for the given element type.
 *
 * The second argument is the resulting vector type's name, which will be an
 * alias for a simple pointer to the element type, provided only for
 * readability.
 */
#define VECTOR_IMPL(element_type, typealias)                                   \
  typedef element_type *typealias;                                             \
                                                                               \
  [[maybe_unused]] static inline size_t typealias##_len(const typealias vec) { \
    return rawvec_len(vec) / sizeof(element_type);                             \
  }                                                                            \
  [[maybe_unused]] static inline size_t typealias##_capacity(                  \
      const typealias vec) {                                                   \
    return rawvec_capacity(vec) / sizeof(element_type);                        \
  }                                                                            \
  [[maybe_unused]] static inline size_t typealias##_remaining(                 \
      const typealias vec) {                                                   \
    return rawvec_remaining(vec) / sizeof(element_type);                       \
  }                                                                            \
  [[maybe_unused]] static inline typealias typealias##_spare_capacity(         \
      typealias vec) {                                                         \
    return &vec[typealias##_len(vec)];                                         \
  }                                                                            \
  [[maybe_unused]] static inline bool typealias##_is_empty(                    \
      const typealias vec) {                                                   \
    return rawvec_is_empty(vec);                                               \
  }                                                                            \
  [[maybe_unused]] static inline typealias typealias##_init(                   \
      size_t initial_capacity) {                                               \
    rawvec vec = rawvec_init(initial_capacity * sizeof(element_type));         \
    return (typealias)vec;                                                     \
  }                                                                            \
  [[maybe_unused]] static inline void typealias##_free(typealias ptr) {        \
    rawvec_free((rawvec)ptr);                                                  \
  }                                                                            \
  [[maybe_unused]] static inline bool typealias##_resize(typealias *ptr,       \
                                                         size_t capacity) {    \
    return rawvec_resize((rawvec *)ptr, capacity * sizeof(element_type));      \
  }                                                                            \
  [[maybe_unused]] static inline bool typealias##_reserve(typealias *ptr,      \
                                                          size_t additional) { \
    return rawvec_reserve((rawvec *)ptr, additional * sizeof(element_type));   \
  }                                                                            \
  [[maybe_unused]] static inline bool typealias##_shrink_to_fit(               \
      typealias *ptr) {                                                        \
    return rawvec_shrink_to_fit((rawvec *)ptr);                                \
  }                                                                            \
  [[maybe_unused]] static inline void typealias##_set_len(typealias ptr,       \
                                                          size_t len) {        \
    rawvec_set_len((rawvec)ptr, len * sizeof(element_type));                   \
  }                                                                            \
  [[maybe_unused]] static inline bool typealias##_memcpy(                      \
      typealias *ptr, size_t offset, const element_type *source, size_t n) {   \
    return rawvec_memcpy((rawvec *)ptr, offset * sizeof(element_type),         \
                         (void *)source, n * sizeof(element_type));            \
  }                                                                            \
  [[maybe_unused]] static inline bool typealias##_extend(                      \
      typealias *ptr, const element_type *source, size_t n) {                  \
    return rawvec_extend((rawvec *)ptr, (void *)source,                        \
                         n * sizeof(element_type));                            \
  }                                                                            \
  [[maybe_unused]] static inline bool typealias##_push(typealias *ptr,         \
                                                       element_type element) { \
    if (sizeof(element_type) == 1)                                             \
      return rawvec_push((rawvec *)ptr, *(char *)&element);                    \
    return typealias##_extend(ptr, &element, 1);                               \
  }                                                                            \
  [[maybe_unused]] static inline element_type typealias##_pop(typealias ptr) { \
    size_t length = typealias##_len(ptr);                                      \
    assert(length > 0);                                                        \
    element_type value = ptr[length - 1];                                      \
    typealias##_set_len(ptr, length - 1);                                      \
    return value;                                                              \
  }                                                                            \
  [[maybe_unused]] static inline element_type *typealias##_last_ptr(           \
      typealias ptr) {                                                         \
    size_t length = typealias##_len(ptr);                                      \
    assert(length > 0);                                                        \
    return &ptr[length - 1];                                                   \
  }                                                                            \
  [[maybe_unused]] static inline element_type typealias##_last(                \
      typealias ptr) {                                                         \
    return *typealias##_last_ptr(ptr);                                         \
  }                                                                            \
  [[maybe_unused]] static inline bool typealias##_extend_from_within(          \
      typealias *ptr, size_t offset, const element_type *source, size_t n) {   \
    return rawvec_extend_from_within(                                          \
        (rawvec *)ptr, offset * sizeof(element_type), (void *)source,          \
        n * sizeof(element_type));                                             \
  }

// A raw vector of bytes.
//
// This is just an alias for `char *`, and it can be used as such with two
// caveats:
//
// 1. Using a `char*` not obtained from `rawvec_init` in any `rawvec_*` API is
// undefined behaviour.
// 2. Calling `free` on a rawvec is undefined behaviour. Use `rawvec_free`
// instead.
//
// The functions in the rawvec API allow you to pull out additinal data such as
// the length of the vector, or its capacity, as well as dynamically extend it
// by pushing and copying data into the vector.
typedef char *rawvec;

// A raw vector is a vector whose element type is a single byte.
//
// The vector is implemented with this dynamically sized structure. The user
// will get a pointer to the `data` member in this struct, which lets the user
// use the vector as a plain pointer, but with magic powers if desired.
typedef struct {
  size_t count;
  size_t capacity;
  char data[];
} __rawvec_t;

// =============================================================================
// Private macros
// =============================================================================
//  Return a `rawvec` from a `__rawvec_t*`.
#define __rawvec_user_ptr_from_rawvec(vec) ((rawvec)((__rawvec_t *)(vec) + 1))
//  Return a `__rawvec_t*` from a `rawvec`.
#define __rawvec_from_user_ptr(ptr) ((__rawvec_t *)(ptr) - 1)

// =============================================================================
// Public macros
// =============================================================================
// Return the number of elements in the vector.
#define rawvec_len(ptr) __rawvec_from_user_ptr((ptr))->count
// Return the total number of elements the vector can hold without reallocating.
#define rawvec_capacity(ptr) __rawvec_from_user_ptr((ptr))->capacity
// Return the number of elements that can be pushed without reallocating.
#define rawvec_remaining(ptr) (rawvec_capacity(ptr) - rawvec_len(ptr))
// Return true if the vector is empty.
#define rawvec_is_empty(ptr) (rawvec_len(ptr) == 0)

// Initialize a rawvec with some initial capacity.
rawvec rawvec_init(size_t initial_capacity);

// Free the allocation associated with `ptr`.
void rawvec_free(rawvec ptr);

// Resize the vector so that its capacity is the new capacity.
//
// Returns true if the vector was moved during resizing.
bool rawvec_resize(rawvec *ptr, size_t capacity);

// Reserve capacity for at least `additional` more elements to be inserted.
//
// The implementation may allocate more than this, but after this call it is
// guaranteed that the capacity is at least `rawvec_len(*ptr) + additional`.
bool rawvec_reserve(rawvec *ptr, size_t additional);

// Set the number of initialized elements to `len`.
//
// Undefined if `len` exceeds the capacity. This will not initialize any
// elements if the length increases, it is the users responsibility to ensure
// any such elements are initialized before use.
void rawvec_set_len(rawvec ptr, size_t len);

// Push a new byte to the end of the vector.
//
// Returns true if the vector was moved during resizing.
bool rawvec_push(rawvec *ptr, char byte);

// Pop the last element from the vector.
//
// Returns the value that was present, and decreases the length of the vector by
// one. Undefined if the vector is empty.
char rawvec_pop(rawvec ptr);

// Like `memcpy(&ptr[offset], source, n)` while ensuring the vector has enough
// space.
//
// Undefined if offset exceeds the current length.
// Undefined if `[source, source+n)` overlaps with the vector's memory, as the
// vector might be moved if it needs to grow.
//
// Returns true if the vector was moved during resizing.
bool rawvec_memcpy(rawvec *ptr, size_t offset, const void *source, size_t n);

// Shorthand for `rawvec_memcpy` with `offset = rawvec_len(ptr)`.
bool rawvec_extend(rawvec *ptr, const void *source, size_t n);

// Shrink the capacity to match the current length.
//
// Returns true if the vector was moved during resizing, or false if the
// capacity already matches the length.
bool rawvec_shrink_to_fit(rawvec *ptr);

// Copy n bytes from `source` into the vector at `offset`, shifting later
// elements to the right.
//
// Undefined if `[source, source+n)` overlaps with the vector's memory, as the
// vector might be moved if it needs to grow.
//
// Returns true if the vector was moved during resizing.
bool rawvec_extend_from_within(rawvec *ptr, size_t offset, const void *source,
                               size_t n);

#ifndef VECTOR_IMPLEMENTATION_INCLUDED
#define VECTOR_IMPLEMENTATION_INCLUDED
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

rawvec rawvec_init(size_t initial_capacity) {
  __rawvec_t *vec = malloc(sizeof(__rawvec_t) + initial_capacity);
  assert(vec != NULL);
  vec->count = 0;
  vec->capacity = initial_capacity;
  return __rawvec_user_ptr_from_rawvec(vec);
}

void rawvec_free(rawvec ptr) {
  __rawvec_t *vec = __rawvec_from_user_ptr(ptr);
  free(vec);
}

bool rawvec_resize(rawvec *ptr, size_t capacity) {
  __rawvec_t *vec = __rawvec_from_user_ptr(*ptr);
  size_t old_count = vec->count;

  vec = realloc(vec, sizeof(__rawvec_t) + capacity);
  assert(vec != NULL);
  vec->capacity = capacity;
  if (old_count > capacity)
    vec->count = capacity;
  else
    vec->count = old_count;

  rawvec new_user_ptr = __rawvec_user_ptr_from_rawvec(vec);
  bool changed = *ptr != new_user_ptr;
  *ptr = new_user_ptr;

  return changed;
}

bool rawvec_reserve(rawvec *ptr, size_t additional) {
  __rawvec_t *vec = __rawvec_from_user_ptr(*ptr);

  if (additional == 0)
    return false;

  // First check if we already have enough room.
  if (additional > SIZE_MAX - vec->count) // Avoid overflow.
    return false;
  size_t required_capacity = vec->count + additional;
  if (required_capacity <= vec->capacity)
    return false;

  // Otherwise, determine the next size to grow to.
  // Growth strategy: Multiply by 1.625 until enough room.
  // If initial capacity is zero, start with the required capacity.
  size_t new_capacity = vec->capacity ? vec->capacity : required_capacity;
  while (new_capacity < required_capacity) {
    size_t old_capacity = new_capacity;
    new_capacity = (old_capacity * 13) / 8;
    if (new_capacity <= old_capacity) {
      new_capacity = required_capacity;
      break;
    }
  }

  return rawvec_resize(ptr, new_capacity);
}

void rawvec_set_len(rawvec ptr, size_t len) {
  __rawvec_t *vec = __rawvec_from_user_ptr(ptr);
  assert(len <= vec->capacity);
  vec->count = len;
}

bool rawvec_push(rawvec *ptr, char byte) {
  bool changed = rawvec_reserve(ptr, 1);
  __rawvec_t *vec = __rawvec_from_user_ptr(*ptr);
  (*ptr)[vec->count++] = byte;
  return changed;
}

char rawvec_pop(rawvec ptr) {
  __rawvec_t *vec = __rawvec_from_user_ptr(ptr);
  assert(vec->count > 0);
  return ptr[--vec->count];
}

bool rawvec_memcpy(rawvec *ptr, size_t offset, const void *source, size_t n) {
  __rawvec_t *vec = __rawvec_from_user_ptr(*ptr);
  assert(offset <= vec->count);
  size_t n_added = n - (vec->count - offset);
  bool changed = rawvec_reserve(ptr, n_added);

  memcpy((*ptr) + offset, source, n);
  if (changed)
    vec = __rawvec_from_user_ptr(*ptr);
  vec->count += n_added;
  return changed;
}

bool rawvec_extend(rawvec *ptr, const void *source, size_t n) {
  return rawvec_memcpy(ptr, rawvec_len(*ptr), source, n);
}

bool rawvec_shrink_to_fit(rawvec *ptr) {
  __rawvec_t *vec = __rawvec_from_user_ptr(*ptr);
  if (vec->capacity == vec->count)
    return false;
  return rawvec_resize(ptr, vec->count);
}

bool rawvec_extend_from_within(rawvec *ptr, size_t offset, const void *source,
                               size_t n) {
  __rawvec_t *vec = __rawvec_from_user_ptr(*ptr);
  assert(offset <= vec->count);
  bool changed = rawvec_reserve(ptr, n);
  size_t n_elements_from_offset = rawvec_len(*ptr) - offset;
  memmove(*ptr + offset + n, *ptr + offset, n_elements_from_offset);
  memcpy(*ptr + offset, source, n);
  if (changed)
    vec = __rawvec_from_user_ptr(*ptr);
  vec->count += n;
  return changed;
}
#endif

VECTOR_IMPL(_kwarg_tagged_argspec_t, _kwarg_tas)
#endif
#endif
