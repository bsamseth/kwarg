#define ARGPARSE_INCLUDE_IMPLEMENTATION
#include "argparse.h"
int main(int argc, const char *argv[static argc]) {
  auto parser = argparse_init(argc, argv);
  argparse_flag(&parser, .name = "--verbose", .short_name = 'v',
                .help = "Enable verbose output");
  argparse_str(&parser, .name = "--output", .short_name = 'o',
               .help = "Output file");
  argparse_str(&parser, .name = "input", .help = "Input file");
  argparse_finish(&parser, .no_exit_on_help = true,
                  .no_exit_on_failure = true);
  return 0;
}
