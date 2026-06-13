#define ARGPARSE_INCLUDE_IMPLEMENTATION
#include "argparse.h"
#include <stdio.h>

int main(int argc, const char *argv[]) {
  argparse_parser_t parser = argparse_init(argc, argv);

  bool flag_a = argparse_flag(&parser, .name = "--alpha", .short_name = 'a',
                              .help = "Alpha flag");
  bool flag_b = argparse_flag(&parser, .name = "--beta", .short_name = 'b',
                              .help = "Beta flag");
  bool flag_c = argparse_flag(&parser, .name = "--gamma", .short_name = 'c',
                              .help = "Gamma flag");
  bool verbose = argparse_flag(&parser, .name = "--verbose",
                               .help = "Verbose output");
  const char *output = argparse_str(&parser, .name = "--output",
                                    .short_name = 'o', .help = "Output file");
  const char *format = argparse_str(&parser, .name = "--format",
                                    .help = "Output format");
  const char *name = argparse_str(&parser, .name = "--name",
                                  .short_name = 'n', .required = true,
                                  .help = "Your name");
  const char *file = argparse_str(&parser, .name = "FILE",
                                  .help = "Input file");

  argparse_result_t result = argparse_finish(&parser, .no_exit_on_failure = true,
                                             .no_exit_on_help = true);

  if (result == ARGPARSE_PARSE_OK) {
    printf("OK: a=%d b=%d c=%d verbose=%d output=%s format=%s name=%s file=%s\n",
           flag_a, flag_b, flag_c, verbose,
           output ? output : "(null)",
           format ? format : "(null)",
           name ? name : "(null)",
           file ? file : "(null)");
  }

  return result == ARGPARSE_PARSE_OK ? 0 : 1;
}
