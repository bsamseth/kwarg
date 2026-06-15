#define KWARG_INCLUDE_IMPLEMENTATION
#include "kwarg.h"
#include <stdio.h>

int main(int argc, const char *argv[]) {
  kwarg_parser_t parser = kwarg_init(argc, argv);

  bool verbose = kwarg_flag(&parser, .name = "--verbose", .short_name = 'v',
                               .help = "Enable verbose output");
  bool quiet = kwarg_flag(&parser, .name = "--quiet",
                             .help = "Suppress output");
  const char *output = kwarg_str(&parser, .name = "--output",
                                    .short_name = 'o', .help = "Output file");
  const char *format = kwarg_str(&parser, .name = "--format",
                                    .help = "Output format");
  const char *name = kwarg_str(&parser, .name = "--name",
                                  .short_name = 'n', .required = true,
                                  .help = "Your name");
  const char *input = kwarg_str(&parser, .name = "input",
                                   .help = "Input file");

  kwarg_result_t result = kwarg_finish(&parser, .no_exit_on_failure = true,
                                             .no_exit_on_help = true);

  if (result == KWARG_PARSE_OK) {
    printf("OK: verbose=%d quiet=%d output=%s format=%s name=%s input=%s\n",
           verbose, quiet, output ? output : "(null)",
           format ? format : "(null)", name ? name : "(null)",
           input ? input : "(null)");
  }

  return result == KWARG_PARSE_OK ? 0 : 1;
}
