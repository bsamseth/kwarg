#include "argparse.h"
#include <stdio.h>

int main(int argc, const char *argv[static argc]) {
  auto parser = argparse_init(argc, argv, .no_args_shows_help = true);

  auto foo = argparse_flag(&parser, .name = "--foo-mode", .short_name = 'f',
                           .help = "Enable foo mode");
  auto bar = argparse_str(&parser, .name = "--bar", .short_name = 'b',
                          .help = "What the bar?");

  if (argparse_finish(&parser) != 0)
    return 1;

  printf("foo-mode: %s\n", foo ? "on" : "off");
  printf("bar: %s\n", bar);

  return 0;
}
