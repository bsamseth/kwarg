#ifndef ARGPARSE_INCLUDE_IMPLEMENTATION
#define ARGPARSE_INCLUDE_IMPLEMENTATION
#endif
#include "argparse.h"

int main(int argc, const char *argv[static argc]) {
  auto parser = argparse_init(
      argc, argv, .no_args_shows_help = true,
      .tagline = "Example CLI to showcase how to use the library.",
      .description = "This is where a longer description of the CLI would go. "
                     "Maybe the library should handle wrapping?");

  bool foo = argparse_flag(&parser, .name = "--foo-mode", .short_name = 'f',
                           .help = "Enable foo mode");
  int verbose =
      argparse_flag(&parser, .name = "--verbose", .short_name = 'v',
                    .help = "Enable verbose mode, can be set multiple times.");
  auto bar = argparse_str(&parser, .name = "--bar", .short_name = 'b',
                          .required = true, .help = "What the bar?");
  auto fizz = argparse_str(&parser, .name = "fizz", .required = true,
                           .help = "Which fizz to use");
  auto bazz = argparse_str(&parser, .name = "bazz",
                           .help = "Which baz to use (defaults to fizz)");
  if (!bazz)
    bazz = fizz;

  argparse_finish(&parser);

  printf("foo = %s\n", foo ? "true" : "false");
  printf("verbose = %d\n", verbose);
  printf("bar = %s\n", bar);
  printf("fizz = %s\n", fizz);
  printf("bazz = %s\n", bazz);

  return 0;
}
