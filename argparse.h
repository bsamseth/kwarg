#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  bool no_help;
  bool no_args_shows_help;
} argparse_init_opts_t;

typedef struct {
  char *name;
  char short_name;
  char *help;
} argparse_argspec_t;

typedef struct {
  const size_t argc;
  const char **argv;
  bool *used;
  bool fail;
  bool help;
} argparse_parser_t;

#define argparse_init(argc, argv, ...)                                         \
  argparse_init_from_opts((argc), (argv), (argparse_init_opts_t){__VA_ARGS__})
#define argparse_flag(parser, ...)                                             \
  argparse_flag_from_opts((parser), (argparse_argspec_t){__VA_ARGS__})
#define argparse_str(parser, ...)                                              \
  argparse_str_from_opts((parser), (argparse_argspec_t){__VA_ARGS__})

static inline void _argparse_print_help_for_argspec(argparse_argspec_t opts) {
  // TODO: Make this smarter depending on possible null values.
  printf("%s, -%c\n\t%s\n", opts.name, opts.short_name, opts.help);
}

static inline const char *argparse_str_from_opts(argparse_parser_t *parser,
                                                 argparse_argspec_t opts) {
  if (parser->help) {
    _argparse_print_help_for_argspec(opts);
    return NULL;
  }

  bool is_option = opts.short_name || !strncmp(opts.name, "-", 1);

  for (size_t i = 1; i < parser->argc; ++i) {
    if (parser->used[i])
      continue;

    const char *arg = parser->argv[i];
    bool arg_is_option = arg[0] == '-';

    if (is_option != arg_is_option)
      continue;

    if (is_option) {
      size_t length = strlen(arg);
      if ((length == 2 && arg[0] == '-' && arg[1] == opts.short_name) ||
          (length > 2 && arg[0] == '-' && arg[1] == '-' &&
           !strcmp(arg, opts.name))) {

        if (i == parser->argc - 1) {
          parser->used[i] = true;
          parser->fail = true;
          fprintf(stderr, "Missing argument to %s\n", opts.name);
          return NULL;
        }
        parser->used[i] = parser->used[i + 1] = true;
        return parser->argv[i + 1];
      }
    } else {
      parser->used[i] = true;
      return parser->argv[i];
    }
  }

  return NULL;
}

static inline bool argparse_flag_from_opts(argparse_parser_t *parser,
                                           argparse_argspec_t opts) {
  if (parser->help) {
    _argparse_print_help_for_argspec(opts);
    return false;
  }

  for (size_t i = 1; i < parser->argc; ++i) {
    if (parser->used[i])
      continue;

    const char *arg = parser->argv[i];
    size_t length = strlen(arg);
    if ((length == 2 && arg[0] == '-' && arg[1] == opts.short_name) ||
        (length > 2 && arg[0] == '-' && arg[1] == '-' &&
         !strcmp(arg, opts.name))) {
      return parser->used[i] = true;
    }
  }
  return false;
}

static inline int argparse_finish(argparse_parser_t *parser) {
  if (parser->help) {
    // Fake parse out the help flag, so that it also gets added to the help.
    argparse_flag(parser, .name = "--help", .short_name = 'h',
                  .help = "Show this help message");
    return 1;
  }

  for (size_t i = 1; i < parser->argc; ++i) {
    if (!parser->used[i]) {
      fprintf(stderr, "Unexpected argument: %s\n", parser->argv[i]);
      return 1;
    }
  }

  free(parser->used);
  return parser->fail ? 1 : 0;
}

static inline argparse_parser_t
argparse_init_from_opts(int argc, const char *argv[static argc],
                        argparse_init_opts_t opts) {
  assert(argc > 0);
  assert(argv != NULL);
  argparse_parser_t parser = {
      .argc = argc, .argv = argv, .used = calloc(argc, 1)};
  assert(parser.used != NULL);

  if (!opts.no_help) {
    if ((opts.no_args_shows_help && argc == 1) ||
        argparse_flag(&parser, .name = "--help", .short_name = 'h')) {
      parser.help = true;
      printf("Start of help message\n");
    }
  }

  return parser;
}
