/* Copyright 2022 Keith F. Prussing */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/errno.h>

#include "cloudgen.h"  /* NOLINT */
#include "readconfig.h"  /* NOLINT */

#include "check-field.h"  /* NOLINT */

void
usage(FILE * fd) {
    fprintf(fd,
        "usage: power-law [-h] [-g] CONFIG DATA\n\n"
        "Compare or generate phase data with CONFIG to DATA file\n\n"
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
  rc_data * config = rc_read(file, stderr);
  if (!config) {
    fprintf(stderr, "Bad configuration file\n");
    return EXIT_FAILURE;
  }

  cg_field * field = rc_generate_base_field(config);
  if (field == NULL) {
    fprintf(stderr, "Error creating the field\n");
    return EXIT_FAILURE;
  }
  cg_random_phase(field, 0);

  int is_size = rc_get_boolean(config, "size_variable_name");
  if (is_size) {
    real size_correlation;
    rc_assign_real(config, "size_correlation", &size_correlation);
    cg_correlated_phase(field, 1, 0, size_correlation);
  }

  real vertical_exponent, outer_scale;
  rc_assign_real(config, "vertical_exponent", &vertical_exponent);
  rc_assign_real(config, "outer_scale", &outer_scale);
  int  i;
  for (i = 0; i < is_size + 1; i++) {
    cg_power_law(field, i, outer_scale, vertical_exponent, 0.0);
  }

  /* Check if we are generating the file */
  length = (field->nx / 2 + 1) * field->ny * field->nz;
  file = argv[1];
  if (generate) {
    handle = fopen(file, "wb");
    if (handle == NULL) {
      fprintf(stderr, "Could open %s for writing\n", argv[2]);
      return EXIT_FAILURE;
    } else {
      errno = 0;
      fwrite((void *) field->p[0], sizeof (complex), length, handle);
      if (errno != 0) {
        fprintf(stderr, "Error writing to %s\n%d: %s\n", file,
                errno, strerror(errno));
        return EXIT_FAILURE;
      }
      fclose(handle);
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

  return EXIT_SUCCESS;
}
