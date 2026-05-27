#define ARGPARSE_INCLUDE_IMPLEMENTATION
#include "argparse.h"
int main(int argc, const char *argv[static argc]) {
  auto parser = argparse_init(argc, argv);
  argparse_flag(&parser, .name = "--verbose", .help = "Verbose mode");
  argparse_str(&parser, .name = "--output", .help = "Output path");
  argparse_str(&parser, .name = "src", .help = "Source path");
  argparse_finish(&parser, .no_exit_on_help = true,
                  .no_exit_on_failure = true);
  return 0;
}
