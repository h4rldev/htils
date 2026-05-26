#include <stdio.h>
#include <string.h>

//
//
//

#include <htils/arena.h>
#include <htils/cli.h>
#include <htils/darray.h>
#include <htils/string.h>

//
//
//

/**
 * @brief Print the usage of the cli, auttomatically generated from the options
 * in the cli.
 *
 * @details This will print the name, description, and usage of the cli, each
 * option being provided.
 * @note TODO: add footer customization.
 *
 * @param cli The cli to print the usage of.
 * @param argv The argv of the cli, for simply referencing argv[0].
 *
 * @pre @c cli and @c argv must be valid and not null.
 */
void cli_usage(htils_cli_t *cli, cstr **argv) {
  fprintf(stderr, "%" print_string "\n", print_string_arg(cli->cli_name));
  fprintf(stderr, "%" print_string "\n", print_string_arg(cli->cli_desc));
  fprintf(stderr, "Usage: %s [options]\n", argv[0]);

  fprintf(stderr, "Options:\n");
  for (u64 i = 0; i < da_len(cli->options); i++) {
    fprintf(stderr, "  %" print_string ",\t",
            print_string_arg(cli->options[i]->cli_long));
    fprintf(stderr, "%" print_string "\t",
            print_string_arg(cli->options[i]->cli_short));
    fprintf(stderr, " %" print_string "\n",
            print_string_arg(cli->options[i]->desc));
  }
}

htils_cli_t *cli_new(arena_t *arena, i64 argc, cstr **argv, const string *name,
                     const string *desc) {
  htils_cli_t *cli = arena_alloc(arena, htils_cli_t, 1);
  cli->argc = argc;
  cli->argv = argv;
  cli->cli_name = (string *)name;
  cli->cli_desc = (string *)desc;
  da_new(arena, cli->options, 1);
  cli->arena = arena;
  cli->usage = cli_usage;

  cli->optidx = 1;
  cli->next_char = -1;
  cli->optarg = null;
  cli->optopt = 0;

  return cli;
}

/**
 * @brief Generate a short cli arg for an option.
 *
 * @details This will generate a short cli arg for an option, based on the name,
 * by simply getting the first letter of the name, if the name contains '_' or
 * '-' it the short name will be 2 letters.
 * @note TODO: make it generate short arguments for option names longer than 2
 * words.
 *
 * @param arena The arena to use for allocating memory in the cli parser.
 * @param name The name of the option.
 *
 * @pre @c arena and @c name must be valid and not null.
 *
 * @return A new \ref string containing the short cli arg.
 */
static string *_cli_create_short(arena_t *arena, const string *name) {
  htils_assert(name != null && "Name cannot be null.");
  htils_assert(name->len > 0 && "Name cannot be empty.");
  htils_assert(name->base[0] != '-' &&
               "Name shouldn't have dashes in the beginning.");

  string *short_name;

  if (string_findc((string *)name, '-') != -1 ||
      string_findc((string *)name, '_') != -1) {
    short_name = string_new(arena, 3);
    short_name->base[0] = '-';
    if (string_findc((string *)name, '_') != -1) {
      i64 idx = string_findc((string *)name, '_');
      short_name->base[1] = name->base[0];
      short_name->base[2] = name->base[idx + 1];
    } else {
      i64 idx = string_findc((string *)name, '-');
      short_name->base[1] = name->base[0];
      short_name->base[2] = name->base[idx + 1];
    }

    return short_name;
  }

  short_name = string_new(arena, 2);
  short_name->base[0] = '-';
  short_name->base[1] = name->base[0];
  return short_name;
}

/**
 * @brief Generate a long cli arg for an option.
 *
 * @details This will generate a long cli arg for an option, based on the name,
 * by simply prefacing the name with '--'.
 *
 * @param arena The arena to use for allocating memory in the cli parser.
 * @param name The name of the option.
 *
 * @pre @c arena and @c name must be valid and not null.
 *
 * @return A new \ref string containing the long cli arg.
 */
static string *_cli_create_long(arena_t *arena, const string *name) {
  htils_assert(name != null && "Name cannot be null.");
  htils_assert(name->len > 0 && "Name cannot be empty.");
  htils_assert(name->base[0] != '-' &&
               "Name shouldn't have dashes in the beginning.");

  string *long_name = string_new(arena, name->len + 2);
  long_name->base[0] = '-';
  long_name->base[1] = '-';
  memcpy(long_name->base + 2, name->base, name->len);
  return long_name;
}

/**
 * @brief Find a short cli arg for an option.
 *
 * @param cli The cli to find the short cli arg for.
 * @param c The character to find the short cli arg for.
 *
 * @pre
 * - @c cli must be valid and not null.
 * - @c c must be a valid character.
 *
 * @return The \ref htils_cli_option_t that contains the short cli arg, or null.
 */
static htils_cli_option_t *cli_find_short(htils_cli_t *cli, u8 c) {
  for (u64 i = 0; i < da_len(cli->options); i++)
    if (cli->options[i]->cli_short_short == c)
      return cli->options[i];

  return null;
}

/**
 * @brief Find a long cli arg for an option.
 *
 * @param cli The cli to find the long cli arg for.
 * @param name The name of the option to find the long cli arg for.
 *
 * @pre @c cli and @c name must be valid and not null.
 *
 * @return The \ref htils_cli_option_t that contains the long cli arg, or null.
 */
static htils_cli_option_t *cli_find_long(htils_cli_t *cli, const string *name) {
  for (u64 i = 0; i < da_len(cli->options); i++) {
    string *ln = cli->options[i]->cli_long;
    if (ln->len >= 2 && (ln->len - 2 == name->len) &&
        memcmp(ln->base + 2, name->base, name->len) == 0)
      return cli->options[i];
  }

  return null;
}

//
//
//

void cli_add(htils_cli_t *cli, const string *name, const string *desc,
             b32 requires_arg) {
  htils_cli_option_t *option = arena_alloc(cli->arena, htils_cli_option_t, 1);
  option->name = string_dup(cli->arena, name);
  option->desc = string_dup(cli->arena, desc);
  option->cli_short = _cli_create_short(cli->arena, name);
  option->cli_long = _cli_create_long(cli->arena, name);
  option->cli_short_short = name->base[0];
  option->has_arg = requires_arg;

  da_append(cli->arena, cli->options, option);
}

i16 parse_cli(htils_cli_t *cli) {
  for (;;) {
    if (cli->next_char != -1) {
      cstr *current_arg = cli->argv[cli->optidx];
      u8 c = current_arg[cli->next_char++];

      if (c == '\0') {
        cli->next_char = -1;
        cli->optidx++;
        continue;
      }

      htils_cli_option_t *option = cli_find_short(cli, c);
      if (!option) {
        cli->optopt = c;
        return '?';
      }

      if (option->has_arg) {
        if (current_arg[cli->next_char] != '\0') {
          cli->optopt = c;
          return ';';
        }

        if (cli->optidx + 1 >= cli->argc) {
          cli->optopt = c;
          return ':';
        }

        cli->optarg = cli->argv[cli->optidx++];
        cli->next_char = -1;
        return option->cli_short_short;
      } else {
        return option->cli_short_short;
      }
    }

    if (cli->optidx >= cli->argc)
      return -1;

    char *arg = cli->argv[cli->optidx];
    if (arg[0] == '-' && arg[1] == '-' && arg[2] == '\0') {
      cli->optidx++;
      return -1;
    }

    if (arg[0] == '-' && arg[1] == '-') {
      cstr *name_start = arg + 2;
      cstr *eq = strchr(name_start, '=');

      u64 name_len;
      cstr *value = null;

      if (eq) {
        name_len = eq - name_start;
        value = eq + 1;
      } else
        name_len = strlen(name_start);

      string *name = string_new(cli->arena, name_len);
      memcpy(name->base, name_start, name_len);
      htils_cli_option_t *option = cli_find_long(cli, name);
      if (!option) {
        cli->optopt = name->base[0];
        cli->optidx++;
        return '?';
      }

      if (option->has_arg) {
        if (!eq) {
          cli->optopt = name->base[0];
          cli->optidx++;
          return ':';
        }

        cli->optarg = value;
        cli->optidx++;
        return option->cli_short_short;
      } else {
        if (eq) {
          cli->optopt = name->base[0];
          cli->optidx++;
          return ';';
        }

        cli->optidx++;
        return option->cli_short_short;
      }
    }

    if (arg[0] == '-' && arg[1] != '\0') {
      cli->next_char = 1;
      continue;
    }

    return -1;
  }
}
