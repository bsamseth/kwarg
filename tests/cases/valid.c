#define ARGPARSE_INCLUDE_IMPLEMENTATION
#include "argparse.h"
#include <stdio.h>

int main(int argc, const char *argv[]) {
  argparse_parser_t parser = argparse_init(argc, argv);

  bool verbose = argparse_flag(&parser, .name = "--verbose", .short_name = 'v',
                               .help = "Enable verbose output");
  bool quiet = argparse_flag(&parser, .name = "--quiet",
                             .help = "Suppress output");
  const char *output = argparse_str(&parser, .name = "--output",
                                    .short_name = 'o', .help = "Output file");
  const char *format = argparse_str(&parser, .name = "--format",
                                    .help = "Output format");
  const char *name = argparse_str(&parser, .name = "--name",
                                  .short_name = 'n', .required = true,
                                  .help = "Your name");
  const char *input = argparse_str(&parser, .name = "input",
                                   .help = "Input file");

  argparse_result_t result = argparse_finish(&parser, .no_exit_on_failure = true,
                                             .no_exit_on_help = true);

  if (result == ARGPARSE_PARSE_OK) {
    printf("OK: verbose=%d quiet=%d output=%s format=%s name=%s input=%s\n",
           verbose, quiet, output ? output : "(null)",
           format ? format : "(null)", name ? name : "(null)",
           input ? input : "(null)");
  }

  return result == ARGPARSE_PARSE_OK ? 0 : 1;
}
