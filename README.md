# kwarg - Single-header command-line argument parsing in C

This is a _procedural_ command-line argument parsing library for C23.

## Features

Two things are "notable" about this argument parser.

1. Arguments are declared with a Python-argparse-style keyword argument syntax using
   designated initializers.
2. Arguments are parsed immediately as they are declared. No argument struct or
   large macro magic; you get back the users argument or `0/nullptr`. This means
   you can conditionally declare arguments based on the values of earlier
   arguments, making sub-commands natural to write.

```c
// Define this before including, controlling where you'd like the _actual_ implementation to be.
#define KWARG_INCLUDE_IMPLEMENTATION
#include "kwarg.h"

int main(int argc, const char *argv[static argc]) {
    auto parser = kwarg_init(argc, argv,
        .description = "A sample program.",
        .tagline = "greet v1.0");

    unsigned verbose = kwarg_flag(&parser,
        .name = "--verbose", .short_name = 'v',
        .help = "Increase verbosity.");

    const char *name = kwarg_str(&parser,
        .name = "--name", .short_name = 'n',
        .help = "Name to greet.", .required = true);

    const char *file = kwarg_str(&parser,
        .name = "FILE", .help = "File to process.");

    kwarg_finish(&parser);

    printf("verbosity: %u\n", verbose);
    printf("name:      %s\n", name);
    printf("file:      %s\n", file);
    return 0;
}
```

```console
$ ./greet --name=world file.txt
verbosity: 0
name:      world
file:      file.txt

$ ./greet -h
greet v1.0

A sample program.

Usage: ./a.out [-hv] -n=NAME [FILE]

ARGUMENTS
        FILE    File to process. (optional)

OPTIONS
        -v, --verbose   Increase verbosity.
        -n, --name=NAME Name to greet.
        -h, --help      Show this help message.
```

### Argument types

Three kinds of arguments are supported:

- **Flags**: Flags are optional arguments that don't take a value. E.g.
  `--verbose/-v` could be defined to control the verbosity, and you'd get back
  the number of times it was given (0 or more). Short flags can be combined:
  `-abc` is equivalent to `-a -b -c`.
- **Options**: Options are optional or required key-value pairs that can be
  specified at any location in the arguments. All these forms are allowed:
  `--name=value`, `--name value`, `-n=value` and `-n value`. Can be marked
  required with *.required = true*.
- **Positional arguments**: Positional arguments are defined as options without
  leading dashes in the name. Matches the next non-option word in declaration order.

## Implementation

Every public API exists in two forms: a *macro* that accepts keyword
arguments, and a *`_from_opts` backend* that takes an explicit struct. The
macro is just syntax sugar that expands to the backend call with a compound literal:

```c
#define kwarg_flag(parser, ...)                                                \
  kwarg_flag_from_opts((parser), (kwarg_argspec_t){__VA_ARGS__})
```

This means the macro form and the backend form are interchangeable. Use
whichever you prefer:

```c
// Macro form — ergonomic keyword arguments:
const char *name = kwarg_str(&p,
    .name = "--name", .short_name = 'n',
    .help = "Name to greet.", .required = true);

// Backend form — explicit struct (useful when spec is computed):
kwarg_argspec_t spec = {
    .name = "--name", .short_name = 'n',
    .help = "Name to greet.", .required = true,
};
const char *name = kwarg_str_from_opts(&p, spec);
```

Arguments are parsed eagerly. When you call `kwarg_flag` or `kwarg_str`, the
function loops over `argv` immediately and returns the value. This lets you
write code like this without a special subcommand mechanism:

```c
auto parser = kwarg_init(argc, argv, .no_args_shows_help = true);

// Parse the subcommand first.
auto cmd = kwarg_str(&parser, .name = "cmd");

if (!cmd) {
    // No subcommand — show help.
    kwarg_finish(&parser);
} else if (!strcmp(cmd, "build")) {
    // Subcommand-specific arguments.
    auto target = kwarg_str(&parser,
        .name = "--target", .short_name = 't',
        .help = "Build target.");
    auto clean = kwarg_flag(&parser,
        .name = "--clean", .short_name = 'c',
        .help = "Clean before building.");
    kwarg_finish(&parser);
    // ...
}
```

Internally, the library tracks which `argv` entries have been consumed, so
later arguments cannot accidentally consume tokens belonging to earlier ones.

## Alternatives

Probably many. But this is C, where reinventing the wheel is always a good
idea. Especially if you make it a squircle.
