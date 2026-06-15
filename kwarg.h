#pragma once

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define VECTOR_INCLUDE_IMPLEMENTATION
#include "vector.h"

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
#define kwarg_init(argc, argv, ...)                                         \
  kwarg_init_from_opts((argc), (argv), (kwarg_init_opts_t){__VA_ARGS__})

// Parse a flag-style option that returns how many times the flag was provided.
//
// Optional named arguments (kwarg_argspec_t):
//   - .name (const char *): Long flag name, e.g. "--verbose".
//   - .short_name (char): Single-character short flag, e.g. 'v'.
//   - .help (const char *): Help text displayed for this flag.
#define kwarg_flag(parser, ...)                                             \
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
#define kwarg_str(parser, ...)                                              \
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
#define kwarg_finish(parser, ...)                                           \
  kwarg_finish_from_opts((parser), (kwarg_finish_opts_t){__VA_ARGS__})

///////////////////////////////////////////////////////////////////////////////
// Backend of public API (usable if you don't want macros).
///////////////////////////////////////////////////////////////////////////////

/// Initialize an argument parser from the command line arguments.
///
/// See kwarg_init_opts_t for available options.
kwarg_parser_t kwarg_init_from_opts(int argc,
                                          const char *argv[static argc],
                                          kwarg_init_opts_t opts);
// Parse a flag-style option that returns how many times the flag was provided.
///
/// See kwarg_argspec_t for available options.
unsigned kwarg_flag_from_opts(kwarg_parser_t *parser,
                                 kwarg_argspec_t opts);
/// Parse a string argument (positional) or string option (--name=value or
/// -n value).
///
/// A short_name and/or a name beginning with "--" makes this an option;
/// otherwise it is treated as a positional argument.
///
/// See kwarg_argspec_t for available options.
const char *kwarg_str_from_opts(kwarg_parser_t *parser,
                                   kwarg_argspec_t opts);
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
VECTOR_IMPL(_kwarg_tagged_argspec_t,
            _kwarg_tas) // "[t]ag [a]rg [s]pecs

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

kwarg_parser_t kwarg_init_from_opts(int argc,
                                          const char *argv[static argc],
                                          kwarg_init_opts_t opts) {
  assert(argc > 0);
  assert(argv != NULL);
  kwarg_parser_t parser = {
      .argc = argc,
      .argv = argv,
      .remaining_arg_uses = calloc(argc, sizeof(*parser.remaining_arg_uses)),
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

const char *kwarg_str_from_opts(kwarg_parser_t *parser,
                                   kwarg_argspec_t opts) {
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

  _kwarg_tas_push(
      &parser->argspecs,
      (_kwarg_tagged_argspec_t){.kind = is_option ? _KWARG_OPTION
                                                     : _KWARG_POSITIONAL,
                                   .spec = opts,
                                   .provided_in_argv = provided_count});
  return user_provided_argument;
}

unsigned kwarg_flag_from_opts(kwarg_parser_t *parser,
                                 kwarg_argspec_t opts) {
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

  _kwarg_tas_push(
      &parser->argspecs,
      (_kwarg_tagged_argspec_t){
          .kind = _KWARG_FLAG, .spec = opts, .provided_in_argv = flag_set});
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

#endif
#endif

// Prompt to generate man pages:
// Look at kwarg.h. Don't consider any other files. It contains a command
// line argument parsing library. Please generate man-pages placed in
// `docs/man/man3/` with man-pages describing the public API of the library.
// Write it in the same style as the Linux libc man pages. There
// should be one page for `kwarg_init`, one page for `kwarg_finish`, one
// joint page for `kwarg_flag`/`kwarg_str`, and finally one page for
// `kwarg` with an overview, explanation and examples. Each of the pages
// describing functions should include the "backend" functions as well as the
// function-like macros, e.g. the man page for `kwarg_init` also documents
// `kwarg_init_from_opts`.
