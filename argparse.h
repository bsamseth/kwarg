#pragma once

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  bool no_help;
  bool no_args_shows_help;
  const char *tagline;
  const char *description;
  const char *usage;
} argparse_init_opts_t;

typedef struct {
  char *name;
  char short_name;
  char *help;
} argparse_argspec_t;

typedef struct {
  const size_t argc;
  const char **argv;
  int16_t *remaining_arg_uses;
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

// Public help string utilities.
/** Insert a section header into the help message. */
void argparse_help_section(argparse_parser_t *parser, const char *section_name);
/** Insert help for the help option. */
#define argparse_help_help(parser)                                             \
  argparse_flag((parser), .name = "--help", .short_name = 'h',                 \
                .help = "Show this help message.")
void argparse_derived_usage(
    argparse_parser_t *parser); // TODO: Implement automatic usage derivation.

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

static void _argparse_print_help_for_argspec(argparse_argspec_t opts) {
  // TODO: Make this smarter depending on possible null values.
  printf("%s, -%c\n\t%s\n", opts.name, opts.short_name, opts.help);
}

const char *argparse_str_from_opts(argparse_parser_t *parser,
                                   argparse_argspec_t opts) {
  if (parser->help) {
    _argparse_print_help_for_argspec(opts);
    return NULL;
  }

  bool is_option = opts.short_name || !strncmp(opts.name, "-", 1);

  for (size_t i = 1; i < parser->argc; ++i) {
    if (parser->remaining_arg_uses[i] <= 0)
      continue;

    const char *arg = parser->argv[i];
    bool arg_is_option = arg[0] == '-';

    if (is_option != arg_is_option)
      continue;
    else if (!arg_is_option) {
      parser->remaining_arg_uses[i] = 0;
      return parser->argv[i];
    } else { // arg is option
      if (!strcmp(arg, opts.name) ||
          _argparse_arg_matches_short_opt(arg, opts)) {
        parser->remaining_arg_uses[i]--;

        if (i == parser->argc - 1) {
          parser->fail = true;
          fprintf(stderr, "Missing argument to %s\n", opts.name);
          return NULL;
        }
        parser->remaining_arg_uses[i + 1] = 0;

        return parser->argv[i + 1];
      }
    }
  }

  return NULL;
}

bool argparse_flag_from_opts(argparse_parser_t *parser,
                             argparse_argspec_t opts) {
  if (parser->help) {
    _argparse_print_help_for_argspec(opts);
    return false;
  }

  for (size_t i = 1; i < parser->argc; ++i) {
    if (parser->remaining_arg_uses[i] <= 0)
      continue;

    const char *arg = parser->argv[i];

    if (!strcmp(arg, opts.name)) {
      parser->remaining_arg_uses[i] = 0;
      return true;
    }
    if (_argparse_arg_matches_short_opt(arg, opts)) {
      parser->remaining_arg_uses[i]--;
      return true;
    }
  }
  return false;
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
      parser->fail = 1;
      fprintf(stderr, "Unexpected argument: %s\n", parser->argv[i]);
    }
  }

  free(parser->remaining_arg_uses);
  return parser->fail ? 1 : 0;
}

argparse_parser_t argparse_init_from_opts(int argc,
                                          const char *argv[static argc],
                                          argparse_init_opts_t opts) {
  assert(argc > 0);
  assert(argv != NULL);
  argparse_parser_t parser = {
      .argc = argc,
      .argv = argv,
      .remaining_arg_uses = calloc(argc, sizeof(*parser.remaining_arg_uses))};
  assert(parser.remaining_arg_uses != NULL);

  for (size_t i = 1; i < parser.argc; ++i) {
    const char *arg = argv[i];
    if (arg[0] != '-' || arg[1] == '-') {
      parser.remaining_arg_uses[i] = 1;
    } else {
      // Short opts like -abc can be combined and should be parsed as many times
      // as there are short opts in the argument.
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
      if (opts.usage)
        printf("Usage: %s %s\n\n", argv[0], opts.usage);
    }
  }

  return parser;
}

void argparse_help_section(argparse_parser_t *parser,
                           const char *section_name) {
  if (!parser->help)
    return;
  printf("\n%s\n", section_name);
}

#endif
#endif
