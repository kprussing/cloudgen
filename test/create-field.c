#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/errno.h>

#include "cloudgen.h"
#include "readconfig.h"

#include "check-field.h"

void
usage(FILE * fd) {
    fprintf(fd,
        "usage: create-field [-h] [-g] CONFIG REFERENCE\n\n"
        "Compare or generate base field from CONFIG to REFERENCE file\n\n"
        "Options:\n"
        "  -h    Show this help\n"
        "  -g    Generate the file\n");
}


int
main(int argc, char * argv[]) {
  FILE * handle;
  bool generate = false;
  int length;
  char arg;
  char * file;

  /* Default settings */
  cg_field * field;
  rc_data * config;

  while (true) {
    arg = getopt(argc, argv, "gh");
    if (arg == -1) {
        break;
    }
    switch (arg) {
        case 'g':
            generate = true;
            break;
        case 'h':
            usage(stdout);
            return EXIT_SUCCESS;
            break;
        case '?':
            fprintf(stderr, "Bad option %c\n", arg);
            usage(stderr);
            return EXIT_FAILURE;
            break;
    }
  }
  /* Shift over the necessary arguments including the command */
  argc -= optind;
  argv += optind;

  if (argc < 2) {
    fprintf(stderr, "File paths missing!\n");
    usage(stderr);
    return EXIT_FAILURE;
  }

  file = argv[0];
  config = rc_read(file, stderr);
  if (config == NULL) {
    fprintf(stderr, "Bad configuration file\n");
    return EXIT_FAILURE;
  }

  field = rc_generate_base_field(config);
  if (field == NULL) {
    fprintf(stderr, "Error creating field\n");
    return EXIT_FAILURE;
  }

  /* Check if we are generating the file */
  file = argv[1];
  if (generate) {
    if (strcmp("-\0", file) == 0) {
      handle = stdout;
    } else {
      handle = fopen(file, "w");
    }
    if (handle == NULL) {
      fprintf(stderr, "Could open %s for writing\n", file);
      return EXIT_FAILURE;
    } else {
      errno = 0;
      cg_dump_field(handle, field);
      if (errno != 0) {
        fprintf(stderr, "Error writing to %s\n%d: %s\n", file,
                errno, strerror(errno));
        return EXIT_FAILURE;
      }
      if (handle != stdout) {
        fclose(handle);
      }
      return EXIT_SUCCESS;
    }
  }

  handle = fopen(file, "r");
  if (handle == NULL) {
    fprintf(stderr, "Could not open %s for reading\n", file);
    return EXIT_FAILURE;
  }
  int success = check_field(field, handle, 1.0e-4, 1.0e-6);
  if (handle != NULL) {
    fclose(handle);
  }
  return success;
}
