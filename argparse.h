#pragma once

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define VECTOR_INCLUDE_IMPLEMENTATION
#include "vector.h"

struct argparse_init_opts_t;
struct argparse_parser_t;
struct argparse_argspec_t;
struct argparse_finish_opts_t;
typedef struct argparse_init_opts_t argparse_init_opts_t;
typedef struct argparse_parser_t argparse_parser_t;
typedef struct argparse_argspec_t argparse_argspec_t;
typedef struct argparse_finish_opts_t argparse_finish_opts_t;
typedef enum {
  ARGPARSE_PARSE_OK,
  ARGPARSE_HELP_INVOKED,
  ARGPARSE_PARSE_FAILED = -1,
} argparse_result_t;

///////////////////////////////////////////////////////////////////////////////
// Public API
///////////////////////////////////////////////////////////////////////////////

// Initialize an argument parser from the command line arguments.
//
// Optional named arguments (argparse_init_opts_t):
//   - .no_help (bool): Disable the automatic --help/-h flag (default: false).
//   - .no_args_shows_help (bool): Show help when no arguments are given
//     (default: false).
//   - .tagline (const char *): Short tagline shown at the top of help output.
//   - .description (const char *): Description text shown after the tagline in
//     help output.
//   - .explicit_usage (const char *): Custom usage string in place of the
//     auto-generated one.
#define argparse_init(argc, argv, ...)                                         \
  argparse_init_from_opts((argc), (argv), (argparse_init_opts_t){__VA_ARGS__})

// Parse a flag-style option that resolves to true if the flag is set.
//
// Optional named arguments (argparse_argspec_t):
//   - .name (const char *): Long flag name, e.g. "--verbose".
//   - .short_name (char): Single-character short flag, e.g. 'v'.
//   - .help (const char *): Help text displayed for this flag.
#define argparse_flag(parser, ...)                                             \
  argparse_flag_from_opts((parser), (argparse_argspec_t){__VA_ARGS__})

// Parse a string argument (positional) or string option (--name=value or
// -n value).
//
// A short_name and/or a name beginning with "-" makes this an option;
// otherwise it is treated as a positional argument.
//
// Optional named arguments (argparse_argspec_t):
//   - .name (const char *): Long option name (e.g. "--output") or positional
//     argument name (e.g. "FILE").
//   - .short_name (char): Single-character short option, e.g. 'o'.
//   - .help (const char *): Help text displayed for this argument.
//   - .required (bool): Mark this option as required (only meaningful for
//     options, not positional arguments).
#define argparse_str(parser, ...)                                              \
  argparse_str_from_opts((parser), (argparse_argspec_t){__VA_ARGS__})

// Finalize argument parsing.
//
// By default this will call `exit(1)` if parsing failed, or `exit(0)` if
// --help was invoked. This can be disabled with the appropriate option.
//
// Optional named arguments (argparse_finish_opts_t):
//   - .no_exit_on_failure (bool): Return ARGPARSE_PARSE_FAILED instead of
//     calling exit(1) (default: false).
//   - .no_exit_on_help (bool): Return ARGPARSE_HELP_INVOKED instead of
//     calling exit(0) (default: false).
//
// See `argparse_finish_from_opts` for the return value.
#define argparse_finish(parser, ...)                                           \
  argparse_finish_from_opts((parser), (argparse_finish_opts_t){__VA_ARGS__})

///////////////////////////////////////////////////////////////////////////////
// Backend of public API (usable if you don't want macros).
///////////////////////////////////////////////////////////////////////////////

/// Initialize an argument parser from the command line arguments.
///
/// See argparse_init_opts_t for available options.
argparse_parser_t argparse_init_from_opts(int argc,
                                          const char *argv[static argc],
                                          argparse_init_opts_t opts);
/// Parse a flag-style option that resolves to true if the flag is set.
///
/// See argparse_argspec_t for available options.
bool argparse_flag_from_opts(argparse_parser_t *parser,
                             argparse_argspec_t opts);
/// Parse a string argument (positional) or string option (--name=value or
/// -n value).
///
/// A short_name and/or a name beginning with "--" makes this an option;
/// otherwise it is treated as a positional argument.
///
/// See argparse_argspec_t for available options.
const char *argparse_str_from_opts(argparse_parser_t *parser,
                                   argparse_argspec_t opts);
// Finalize argument parsing.
//
// - Returns `ARGPARSE_PARSE_OK` (0) if parsing was successful.
// - Returns `ARGPARSE_HELP_INVOKED` (1) if --help was provided. In this case
//   parsing was skipped and all args have been zero-initialized. You should
//   exit the program if this is returned.
// - Returns `ARGPARSE_PARSE_FAILED` (-1) if parsing failed in any way.
//   Diagnostics have been written to stderr and you should exit the program if
//   this was returned.
//
// See argparse_finish_opts_t for available options.
argparse_result_t argparse_finish_from_opts(argparse_parser_t *parser,
                                            argparse_finish_opts_t opts);

///////////////////////////////////////////////////////////////////////////////
// Structures
///////////////////////////////////////////////////////////////////////////////

// Options for argparse_init / argparse_init_from_opts.
//
// Members:
//   - no_help: Disable the automatic --help/-h flag (default: false).
//   - no_args_shows_help: Show help when no arguments are given
//     (default: false).
//   - tagline: Short tagline shown at the top of help output.
//   - description: Description text shown after the tagline in help output.
//   - explicit_usage: Custom usage string in place of the auto-generated one.
struct argparse_init_opts_t {
  bool no_help;
  bool no_args_shows_help;
  const char *tagline;
  const char *description;
  const char *explicit_usage;
};

// Options for argparse_finish / argparse_finish_from_opts.
//
// Members:
//   - no_exit_on_failure: Return ARGPARSE_PARSE_FAILED instead of calling
//     exit(1) (default: false).
//   - no_exit_on_help: Return ARGPARSE_HELP_INVOKED instead of calling exit(0)
//     (default: false).
struct argparse_finish_opts_t {
  bool no_exit_on_failure;
  bool no_exit_on_help;
};

// Options for argparse_flag / argparse_str and their _from_opts backends.
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
struct argparse_argspec_t {
  char *name;
  char short_name;
  char *help;
  bool required;
};

enum _argparse_arg_kind_t {
  _ARGPARSE_POSITIONAL,
  _ARGPARSE_OPTION,
  _ARGPARSE_FLAG,
};

typedef struct {
  enum _argparse_arg_kind_t kind;
  argparse_argspec_t spec;
  bool provided_in_argv;
} _argparse_tagged_argspec_t;
VECTOR_IMPL(_argparse_tagged_argspec_t,
            _argparse_tas) // "[t]ag [a]rg [s]pecs

struct argparse_parser_t {
  const size_t argc;
  const char **argv;
  _argparse_tas argspecs;
  int16_t *remaining_arg_uses;
  argparse_init_opts_t init_opts;
  bool fail;
  bool help;
};

///////////////////////////////////////////////////////////////////////////////
// "Private" helper API
///////////////////////////////////////////////////////////////////////////////
static size_t _argparse_arg_matches_short_opt(const char arg[static 1],
                                              const argparse_argspec_t opts);
void _argparse_show_help(const argparse_parser_t *parser, FILE *out);
void _argparse_show_usage(const argparse_parser_t *parser, FILE *out);
void _argparse_show_arguments_help(const argparse_parser_t *parser, FILE *out);
void _argparse_show_options_help(const argparse_parser_t *parser, FILE *out);
void _argparse_validate_argspec(const argparse_argspec_t opts);
void _argparse_build_usage(const argparse_parser_t *parser, FILE *out);

#ifdef ARGPARSE_INCLUDE_IMPLEMENTATION
#ifndef ARGPARSE_IMPLEMENTATION_INCLUDED
#define ARGPARSE_IMPLEMENTATION_INCLUDED

argparse_parser_t argparse_init_from_opts(int argc,
                                          const char *argv[static argc],
                                          argparse_init_opts_t opts) {
  assert(argc > 0);
  assert(argv != NULL);
  argparse_parser_t parser = {
      .argc = argc,
      .argv = argv,
      .remaining_arg_uses = calloc(argc, sizeof(*parser.remaining_arg_uses)),
      .argspecs = _argparse_tas_init(argc),
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
      for (size_t j = 1; arg[j] != '\0' && arg[j] != '=';
           ++j, ++parser.remaining_arg_uses[i])
        ;
    }
  }

  if (!opts.no_help) {
    bool help = argparse_flag(&parser, .name = "--help", .short_name = 'h',
                              .help = "Show this help message.");
    if (help || (opts.no_args_shows_help && argc == 1)) {
      parser.help = true;
    }
  }

  return parser;
}

void _argparse_show_help(const argparse_parser_t *parser, FILE *out) {
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
    _argparse_show_usage(parser, out);

  fprintf(out, "\n");
  _argparse_show_arguments_help(parser, out);
  fprintf(out, "\n");
  _argparse_show_options_help(parser, out);
}

void _argparse_show_usage(const argparse_parser_t *parser, FILE *out) {
  if (out == NULL)
    out = stdout;

  fprintf(out, "Usage: %s ", parser->argv[0]);
  _argparse_build_usage(parser, out);
  fprintf(out, "\n");
}

void _argparse_show_arguments_help(const argparse_parser_t *parser, FILE *out) {
  if (out == NULL)
    out = stdout;

  fprintf(out, "ARGUMENTS\n");
  for (size_t i = 0; i < _argparse_tas_len(parser->argspecs); ++i) {
    auto tas = parser->argspecs[i];
    if (tas.kind != _ARGPARSE_POSITIONAL)
      continue;

    fprintf(out, "\t%s\t%s%s\n", tas.spec.name, tas.spec.help,
            tas.spec.required ? "" : " (optional)");
  }
}

void _argparse_show_options_help(const argparse_parser_t *parser, FILE *out) {
  if (out == NULL)
    out = stdout;

  if (!parser->init_opts.no_help) {
    // Help is first option if not explicitly silenced.
    // Move it into the last position instead.
    auto last_ptr = _argparse_tas_last_ptr(parser->argspecs);
    auto tmp = *last_ptr;
    *last_ptr = parser->argspecs[0];
    parser->argspecs[0] = tmp;
  }

  fprintf(out, "OPTIONS\n");
  for (size_t i = 0; i < _argparse_tas_len(parser->argspecs); ++i) {
    auto tas = parser->argspecs[i];
    if (tas.kind == _ARGPARSE_POSITIONAL)
      continue;

    if (tas.spec.name && tas.spec.short_name)
      fprintf(out, "\t-%c, %s", tas.spec.short_name, tas.spec.name);
    else if (tas.spec.short_name)
      fprintf(out, "\t-%c", tas.spec.short_name);
    else
      fprintf(out, "\t%s", tas.spec.name);

    if (tas.kind == _ARGPARSE_OPTION) {
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

const char *argparse_str_from_opts(argparse_parser_t *parser,
                                   argparse_argspec_t opts) {
  _argparse_validate_argspec(opts);
  bool is_option = opts.short_name || !strncmp(opts.name, "-", 1);
  const char *user_provided_argument = NULL;

  // If --help was provided, skip parsing.
  for (size_t i = 1; !parser->help && i < parser->argc; ++i) {
    if (parser->remaining_arg_uses[i] <= 0)
      continue;

    const char *arg = parser->argv[i];
    bool arg_is_option = arg[0] == '-';

    if (is_option != arg_is_option)
      continue;
    else if (!arg_is_option) {
      parser->remaining_arg_uses[i] = 0;
      user_provided_argument = parser->argv[i];
      break;
    } else { // arg is option
      size_t name_len = strlen(opts.name);

      // Short-form argument matching:
      size_t short_idx = _argparse_arg_matches_short_opt(arg, opts);
      if (short_idx && arg[short_idx + 1] == '=') {
        parser->remaining_arg_uses[i]--;
        user_provided_argument = arg + short_idx + 2;
        break;
      } else if (short_idx == strlen(arg) - 1) {
        parser->remaining_arg_uses[i]--;
        goto consume_next_arg_as_value;
      } else if (short_idx) {
        parser->fail = true;
        parser->remaining_arg_uses[i]--;
        fprintf(stderr,
                "Option -%c expects a value and must be the last argument in "
                "the group: %s\n",
                opts.short_name, arg);
        break;
      }

      bool long_name_eq_match =
          !strncmp(arg, opts.name, name_len) && arg[name_len] == '=';
      if (long_name_eq_match) {
        parser->remaining_arg_uses[i] = 0;
        user_provided_argument = arg + name_len + 1;
        break;
      }

      if (!strcmp(arg, opts.name)) {
        parser->remaining_arg_uses[i] = 0;
        goto consume_next_arg_as_value;
      }

    consume_next_arg_as_value:
      // At this point, we matched either long form or short form, expecting the
      // next argument to be the value.
      //
      if (i == parser->argc - 1) {
        parser->fail = true;
        fprintf(stderr, "Missing argument to %s (-%c)\n", opts.name,
                opts.short_name);
        break;
      }
      parser->remaining_arg_uses[i + 1] = 0;

      user_provided_argument = parser->argv[i + 1];
      break;
    }
  }

  _argparse_tas_push(
      &parser->argspecs,
      (_argparse_tagged_argspec_t){
          .kind = is_option ? _ARGPARSE_OPTION : _ARGPARSE_POSITIONAL,
          .spec = opts,
          .provided_in_argv = user_provided_argument != NULL});
  return user_provided_argument;
}

bool argparse_flag_from_opts(argparse_parser_t *parser,
                             argparse_argspec_t opts) {
  _argparse_validate_argspec(opts);
  bool flag_set = false;
  for (size_t i = 1; !parser->help && i < parser->argc; ++i) {
    if (parser->remaining_arg_uses[i] <= 0)
      continue;

    const char *arg = parser->argv[i];

    if (!strcmp(arg, opts.name)) {
      parser->remaining_arg_uses[i] = 0;
      flag_set = true;
      break;
    }
    if (_argparse_arg_matches_short_opt(arg, opts)) {
      parser->remaining_arg_uses[i]--;
      flag_set = true;
      break;
    }
  }

  _argparse_tas_push(
      &parser->argspecs,
      (_argparse_tagged_argspec_t){
          .kind = _ARGPARSE_FLAG, .spec = opts, .provided_in_argv = flag_set});
  return flag_set;
}

static size_t _argparse_arg_matches_short_opt(const char arg[static 1],
                                              const argparse_argspec_t opts) {
  // Skip positional arguments and long-form options.
  if (arg[0] != '-' || arg[1] == '-')
    return 0;

  const size_t n_opts = strlen(arg);
  for (size_t i = 1; i < n_opts; ++i) {
    if (arg[i] == opts.short_name)
      return i;
  }
  return 0;
}

argparse_result_t argparse_finish_from_opts(argparse_parser_t *parser,
                                            argparse_finish_opts_t opts) {
  if (parser->help) {
    _argparse_show_help(parser, NULL);
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
      for (const char *opt = parser->argv[i] + 1; *opt; ++opt) {
        bool opt_is_defined = false;
        for (size_t j = 0; j < _argparse_tas_len(parser->argspecs); ++j) {
          auto spec = parser->argspecs[j];
          if (spec.spec.short_name == *opt) {
            opt_is_defined = true;
            break;
          }
        }
        if (!opt_is_defined) {
          fprintf(stderr, "Unexpected argument: -%c\n", *opt);
          if (--parser->remaining_arg_uses[i] == 0)
            break;
        }
      }
    }
  }

  for (size_t i = 0; i < _argparse_tas_len(parser->argspecs); ++i) {
    auto spec = parser->argspecs[i];
    if (spec.kind == _ARGPARSE_POSITIONAL && !spec.provided_in_argv &&
        spec.spec.required) {
      fprintf(stderr, "Missing positional argument: %s\n", spec.spec.name);
      parser->fail = true;
    } else if (spec.kind == _ARGPARSE_OPTION && !spec.provided_in_argv &&
               spec.spec.required) {
      fprintf(stderr, "Missing required option: %s\n", spec.spec.name);
      parser->fail = true;
    }
  }

cleanup:
  free(parser->remaining_arg_uses);
  _argparse_tas_free(parser->argspecs);

  argparse_result_t result = parser->help   ? ARGPARSE_HELP_INVOKED
                             : parser->fail ? ARGPARSE_PARSE_FAILED
                                            : ARGPARSE_PARSE_OK;
  switch (result) {
  case ARGPARSE_PARSE_OK:
    return result;
  case ARGPARSE_HELP_INVOKED:
    if (!opts.no_exit_on_help)
      exit(0);
    return ARGPARSE_HELP_INVOKED;
  case ARGPARSE_PARSE_FAILED:
    if (!opts.no_exit_on_failure)
      exit(1);
    return ARGPARSE_PARSE_FAILED;
  default:
    unreachable();
  }
}

void _argparse_debug_dump_argspec(const argparse_argspec_t opts) {
  fprintf(stderr,
          "{\n\t.name = \"%s\",\n\t.short_name = "
          "'%c',\n\t.required = %s,\n\t.help = \"%s\"\n}\n",
          opts.name, opts.short_name, opts.required ? "true" : "false",
          opts.help);
}

void _argparse_validate_argspec(const argparse_argspec_t opts) {
  if (opts.name == NULL) {
    fprintf(stderr, "Invalid specification:\n");
    _argparse_debug_dump_argspec(opts);
    fprintf(stderr,
            "You must specify the .name field for any options/flags.\n");
    exit(1);
  }
  if (opts.name[0] == '-' && opts.name[1] != '-') {
    fprintf(stderr, "Invalid specification:\n");
    _argparse_debug_dump_argspec(opts);
    fprintf(stderr,
            "Argument names cannot start with a single `-`, they must either "
            "not start with a `-` or start with `--`.\n");
    exit(1);
  }
  if (opts.short_name == '-' || opts.short_name == '=') {
    fprintf(stderr, "Invalid specification:\n");
    _argparse_debug_dump_argspec(opts);
    fprintf(stderr, "Short names cannot use the `-` or '=' characters.\n");
    exit(1);
  }
}

void _argparse_build_usage(const argparse_parser_t *parser, FILE *out) {
  bool first = true;

  for (size_t i = 0; i < _argparse_tas_len(parser->argspecs); ++i) {
    auto tas = parser->argspecs[i];
    if (tas.kind == _ARGPARSE_POSITIONAL)
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
    if (tas.kind == _ARGPARSE_OPTION) {
      fputc('=', out);
      const char *p = tas.spec.name;
      while (*p == '-')
        p++;
      for (; *p; p++)
        fputc(*p >= 'a' && *p <= 'z' ? *p - 32 : *p, out);
    }
    if (!tas.spec.required)
      fputc(']', out);
  }

  for (size_t i = 0; i < _argparse_tas_len(parser->argspecs); ++i) {
    auto tas = parser->argspecs[i];
    if (tas.kind != _ARGPARSE_POSITIONAL)
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
// Look at argparse.h. Don't consider any other files. It contains a command
// line argument parsing library. Please generate man-pages placed in
// `docs/man/man3/` with man-pages describing the public API of the library.
// Write it in the same style as the Linux libc man pages. I think there
// should be one page for `argparse_init`, one page for `argparse_finish`, one
// joint page for `argparse_flag`/`argparse_str`, and finally one page for
// `argparse` with an overview, explanation and examples. Each of the pages
// describing functions should include the "backend" functions as well as the
// function-like macros, e.g. the man page for `argparse_init` also documents
// `argparse_init_from_opts`.
