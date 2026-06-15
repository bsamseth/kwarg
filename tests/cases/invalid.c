#define KWARG_INCLUDE_IMPLEMENTATION
#include "kwarg.h"
#include <string.h>

int main(int argc, const char *argv[]) {
  kwarg_parser_t parser = kwarg_init(argc, argv);

  const char *mode = argc > 1 ? argv[1] : "";

  if (strcmp(mode, "no-name") == 0) {
    kwarg_flag(&parser, .short_name = 'x', .help = "missing name");
  } else if (strcmp(mode, "single-dash") == 0) {
    kwarg_str(&parser, .name = "-o", .help = "single dash name");
  } else if (strcmp(mode, "bad-short") == 0) {
    kwarg_str(&parser, .name = "--opt", .short_name = '=',
                 .help = "bad short");
  } else {
    kwarg_str(&parser, .name = "--mode", .help = "test mode selector");
  }

  kwarg_finish(&parser, .no_exit_on_failure = true,
                  .no_exit_on_help = true);
}
