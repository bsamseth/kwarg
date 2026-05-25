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

// Public API.
/** Initialize an argument parser from the command line arguments. */
#define argparse_init(argc, argv, ...)                                         \
  argparse_init_from_opts((argc), (argv), (argparse_init_opts_t){__VA_ARGS__})
/** Parse a flag-style option that resolves to true if the flag is set. */
#define argparse_flag(parser, ...)                                             \
  argparse_flag_from_opts((parser), (argparse_argspec_t){__VA_ARGS__})
/** Parse a string argument/option. */
#define argparse_str(parser, ...)                                              \
  argparse_str_from_opts((parser), (argparse_argspec_t){__VA_ARGS__})
/** Finalize argument parsing, returning 0 if no errors occured. */
int argparse_finish(argparse_parser_t *parser);

// Backend of public API (usable if macros aren't desired).
argparse_parser_t argparse_init_from_opts(int argc,
                                          const char *argv[static argc],
                                          argparse_init_opts_t opts);
bool argparse_flag_from_opts(argparse_parser_t *parser,
                             argparse_argspec_t opts);
const char *argparse_str_from_opts(argparse_parser_t *parser,
                                   argparse_argspec_t opts);

// "Private" helper API
static void _argparse_print_help_for_argspec(argparse_argspec_t opts);
static bool _argparse_arg_matches_short_opt(const char arg[static 1],
                                            const argparse_argspec_t opts);

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
    if ((opts.no_args_shows_help && argc == 1) ||
        argparse_flag(&parser, .name = "--help", .short_name = 'h')) {
      parser.help = true;

      if (opts.tagline)
        printf("%s\n\n", opts.tagline);
      if (opts.description)
        printf("%s\n\n", opts.description);
      if (opts.explicit_usage)
        printf("Usage: %s %s\n\n", argv[0], opts.explicit_usage);
    }
  }

  return parser;
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

int argparse_finish(argparse_parser_t *parser) {
  if (parser->help) {
    free(parser->remaining_arg_uses);
    return 1;
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

  free(parser->remaining_arg_uses);
  _argparse_tas_free(parser->argspecs);
  return parser->fail ? 1 : 0;
}

static void _argparse_print_help_for_argspec(argparse_argspec_t opts) {
  // TODO: Make this smarter depending on possible null values.
  printf("%s, -%c\n\t%s\n", opts.name, opts.short_name, opts.help);
}

#endif
#endif
