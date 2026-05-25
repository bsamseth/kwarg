#include <stdio.h>

#ifndef ARGPARSE_INCLUDE_IMPLEMENTATION
#define ARGPARSE_INCLUDE_IMPLEMENTATION
#endif
#include "argparse.h"

int main(int argc, const char *argv[static argc]) {
  auto parser = argparse_init(
      argc, argv, .no_args_shows_help = true,
      .tagline = "Example CLI to showcase how to use the library.",
      .description = "This is where a longer description of the CLI would go. "
                     "Maybe the library should handle wrapping?",
      .explicit_usage = "[OPTIONS] FIZZ");

  bool foo = argparse_flag(&parser, .name = "--foo-mode", .short_name = 'f',
                           .help = "Enable foo mode");
  auto bar = argparse_str(&parser, .name = "--bar", .short_name = 'b',
                          .help = "What the bar?");
  auto fizz =
      argparse_str(&parser, .name = "fizz", .help = "Which fizz to use");

  if (argparse_finish(&parser) != 0)
    return 1;

  printf("foo-mode: %s\n", foo ? "on" : "off");
  printf("bar: %s\n", bar);
  printf("fizz: %s\n", fizz);

  return 0;
}
