#define ARGPARSE_INCLUDE_IMPLEMENTATION
#include "argparse.h"
int main(int argc, const char *argv[static argc]) {
  auto parser = argparse_init(argc, argv);
  argparse_str(&parser, .name = "--name", .short_name = 'n',
               .help = "Your name", .required = true);
  argparse_str(&parser, .name = "file", .help = "File to process");
  argparse_finish(&parser, .no_exit_on_help = true,
                  .no_exit_on_failure = true);
  return 0;
}
