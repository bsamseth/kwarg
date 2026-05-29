#define ARGPARSE_INCLUDE_IMPLEMENTATION
#include "argparse.h"
#include <string.h>

int main(int argc, const char *argv[]) {
  argparse_parser_t parser = argparse_init(argc, argv);

  const char *mode = argc > 1 ? argv[1] : "";

  if (strcmp(mode, "no-name") == 0) {
    argparse_flag(&parser, .short_name = 'x', .help = "missing name");
  } else if (strcmp(mode, "single-dash") == 0) {
    argparse_str(&parser, .name = "-o", .help = "single dash name");
  } else if (strcmp(mode, "bad-short") == 0) {
    argparse_str(&parser, .name = "--opt", .short_name = '=',
                 .help = "bad short");
  } else {
    argparse_str(&parser, .name = "--mode", .help = "test mode selector");
  }

  argparse_finish(&parser, .no_exit_on_failure = true,
                  .no_exit_on_help = true);
}
