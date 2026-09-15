#ifndef HTILS_CLI_H
#define HTILS_CLI_H

/***********************************/

#include <htils/arena.h>
#include <htils/basictypes.h>
#include <htils/darray.h>
#include <htils/string.h>

/***********************************/

/**
 * @brief A cli option.
 *
 * @param name The name of the option
 * @param desc The description of the option
 * @param cli_short The short cli arg of the option
 * @param cli_long The long cli arg of the option
 * @param cli_short_short The short short cli arg of the option
 * @param has_arg Whether the option has another argument
 */
typedef struct cli_option {
  string *name;
  string *desc;
  string *cli_short;
  string *cli_long;
  u8 cli_short_short;
  b32 has_arg;
} htils_cli_option_t;

/**
 * @brief The cli state thats used during parsing.
 *
 * @param argc The number of arguments
 * @param argv The arguments
 *
 * @param cli_name The name of the cli
 * @param cli_desc The description of the cli
 * @param options The options of the cli
 *
 * @param arena The arena to use for allocating memory in the cli parser.
 * @param usage The usage function to use when printing the usage of the cli.
 *
 * @param optidx The index of the current option being parsed.
 * @param next_char The next character to parse.
 * @param optarg The argument of the current option being parsed.
 * @param optopt The option of the current option being parsed.
 */
typedef struct cli {
  i64 argc;
  cstr **argv;

  string *cli_name;
  string *cli_desc;
  htils_cli_option_t **options;

  arena_t *arena;
  void (*usage)(struct cli *cli, cstr **argv);

  i64 optidx;
  i16 next_char;
  cstr *optarg;
  i16 optopt;
} htils_cli_t;

/**
 * @brief Create a new cli.
 *
 * @details Allocates a new \ref htils_cli_t struct and initializes default
 * values.
 *
 * @pre
 * - @c arena, @c argv, @c name, and @c desc must be valid and not null.
 * - @c argc must be greater than 1.
 *
 * @param arena The arena to use for allocating memory in the cli parser.
 * @param argc The number of arguments
 * @param argv The arguments
 * @param name The name of the cli
 * @param desc The description of the cli
 *
 * @return A new \ref htils_cli_t struct.
 */
htils_cli_t *cli_new(arena_t *arena, i64 argc, cstr **argv, const string *name,
                     const string *desc);

//
//
//

/**
 * @brief Add an option to the cli.
 *
 * @details This will generate short and long option based on the name, and
 * provides a description for the option in the usage command, allowing marking
 * a option as having a required arg.
 *
 * @param cli The cli to add the option to.
 * @param name The name of the option.
 * @param desc The description of the option.
 * @param requires_arg Whether the option requires another argument afterwards.
 *
 * @pre @c cli, @c name, and @c desc must be valid and not null.
 */
void cli_add(htils_cli_t *cli, const string *name, const string *desc,
             b32 requires_arg);

//
//
//

/**
 * @brief Parse the next CLI argument.
 *
 * @details Intended to be called in a `while` loop with a `switch` on the
 * return. Returns the matched argument's code, or one of the error codes:
 * '?' for an unknown argument, ':' when a required argument was missing, and
 * ';' when an argument unexpectedly needs one.
 *
 * @pre @c cli must be valid and not null.
 *
 * @param cli The cli to parse.
 *
 * @return The matched argument, '?', ':', or ';'.
 */
i16 parse_cli(htils_cli_t *cli);

#endif // !HTILS_CLI_H
