#pragma once

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define VECTOR_INCLUDE_IMPLEMENTATION
#include "vector.h"

typedef struct {
  bool no_help;
  bool no_args_shows_help;
  const char *tagline;
  const char *description;
  const char *explicit_usage;
} argparse_init_opts_t;

typedef struct {
  bool no_exit_on_failure;
  bool no_exit_on_help;
} argparse_finish_opts_t;

typedef struct {
  char *name;
  char short_name;
  char *help;
  bool required;
} argparse_argspec_t;

typedef enum {
  _ARGPARSE_POSITIONAL,
  _ARGPARSE_OPTION,
  _ARGPARSE_FLAG,
} _argparse_arg_kind_t;

typedef struct {
  _argparse_arg_kind_t kind;
  argparse_argspec_t spec;
  bool provided_in_argv;
} _argparse_tagged_argspec_t;
VECTOR_IMPL(_argparse_tagged_argspec_t, _argparse_tas) // "[t]ag [a]rg [s]pecs

typedef struct {
  const size_t argc;
  const char **argv;
  _argparse_tas argspecs;
  int16_t *remaining_arg_uses;
  argparse_init_opts_t init_opts;
  bool fail;
  bool help;
} argparse_parser_t;

typedef enum {
  ARGPARSE_PARSE_OK,
  ARGPARSE_HELP_INVOKED,
  ARGPARSE_PARSE_FAILED = -1,
} argparse_result_t;

///////////////////////////////////////////////////////////////////////////////
// Public API
///////////////////////////////////////////////////////////////////////////////

// Initialize an argument parser from the command line arguments.
#define argparse_init(argc, argv, ...)                                         \
  argparse_init_from_opts((argc), (argv), (argparse_init_opts_t){__VA_ARGS__})
// Parse a flag-style option that resolves to true if the flag is set.
#define argparse_flag(parser, ...)                                             \
  argparse_flag_from_opts((parser), (argparse_argspec_t){__VA_ARGS__})
// Parse a string argument/option.
#define argparse_str(parser, ...)                                              \
  argparse_str_from_opts((parser), (argparse_argspec_t){__VA_ARGS__})
// Finalize argument parsing.
//
// By default this will call `exit(1)` if parsing failed, or `exit(0)` if
// --help was invoked. This can be disabled with the appropriate option.
//
// See `argparse_finish_from_opts` for the return value.
#define argparse_finish(parser, ...)                                           \
  argparse_finish_from_opts((parser), (argparse_finish_opts_t){__VA_ARGS__})

///////////////////////////////////////////////////////////////////////////////
// Backend of public API (usable if macros aren't desired).
///////////////////////////////////////////////////////////////////////////////

/// Initialize an argument parser from the command line arguments.
argparse_parser_t argparse_init_from_opts(int argc,
                                          const char *argv[static argc],
                                          argparse_init_opts_t opts);
/// Parse a flag-style option that resolves to true if the flag is set.
bool argparse_flag_from_opts(argparse_parser_t *parser,
                             argparse_argspec_t opts);
/// Parse a string argument/option.
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
argparse_result_t argparse_finish_from_opts(argparse_parser_t *parser,
                                            argparse_finish_opts_t opts);

///////////////////////////////////////////////////////////////////////////////
// "Private" helper API
///////////////////////////////////////////////////////////////////////////////
static bool _argparse_arg_matches_short_opt(const char arg[static 1],
                                            const argparse_argspec_t opts);
void _argparse_show_help(const argparse_parser_t *parser, FILE *out);
void _argparse_show_usage(const argparse_parser_t *parser, FILE *out);
void _argparse_show_arguments_help(const argparse_parser_t *parser, FILE *out);
void _argparse_show_options_help(const argparse_parser_t *parser, FILE *out);

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
      parser.remaining_arg_uses[i] = strlen(arg) - 1;
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

  fprintf(out, "Usage: %s TODO\n", parser->argv[0]);
}

void _argparse_show_arguments_help(const argparse_parser_t *parser, FILE *out) {
  if (out == NULL)
    out = stdout;

  fprintf(out, "ARGUMENTS\n");
  for (size_t i = 0; i < _argparse_tas_len(parser->argspecs); ++i) {
    auto tas = parser->argspecs[i];
    if (tas.kind != _ARGPARSE_POSITIONAL)
      continue;

    fprintf(out, "\t%s\t%s\n", tas.spec.name, tas.spec.help);
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
      fprintf(out, "\t-%c, %s\t%s\n", tas.spec.short_name, tas.spec.name,
              tas.spec.help);
    else if (tas.spec.short_name)
      fprintf(out, "\t-%c\t%s\n", tas.spec.short_name, tas.spec.help);
    else
      fprintf(out, "\t%s\t%s\n", tas.spec.name, tas.spec.help);
  }
}

const char *argparse_str_from_opts(argparse_parser_t *parser,
                                   argparse_argspec_t opts) {
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
      if (!strcmp(arg, opts.name) ||
          _argparse_arg_matches_short_opt(arg, opts)) {
        parser->remaining_arg_uses[i]--;

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

static bool _argparse_arg_matches_short_opt(const char arg[static 1],
                                            const argparse_argspec_t opts) {
  // Skip positional arguments and long-form options.
  if (arg[0] != '-' || arg[1] == '-')
    return false;

  const size_t n_opts = strlen(arg);
  for (size_t i = 1; i < n_opts; ++i) {
    if (arg[i] == opts.short_name)
      return true;
  }
  return false;
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
      // We make sure to only output diagnostics for the short-form options that
      // aren't defined, in case of joined options like `-abc`. If `-a` is
      // defined, then diagnostics are only emitted for `-b` and `-c`.
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
    if (spec.kind == _ARGPARSE_POSITIONAL && !spec.provided_in_argv) {
      fprintf(stderr, "Missing positional argument: %s\n", spec.spec.name);
      parser->fail = true;
    } else if (!spec.provided_in_argv && spec.kind == _ARGPARSE_OPTION &&
               parser->argspecs[i].spec.required) {
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

#endif
#endif
