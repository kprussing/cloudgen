#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/errno.h>

#include "cloudgen.h"
#include "readconfig.h"


void
usage(FILE * fd) {
    fprintf(fd,
        "usage: create-field [-h] [-g] CONFIG REFERENCE\n\n"
        "Compare or generate base field from CONFIG to REFERENCE file\n\n"
        "Options:\n"
        "  -h    Show this help\n"
        "  -g    Generate the file\n");
}


/** @brief Check an FFTW plan
 */
int
check_plan(fftw_plan plan, FILE * handle, int * line) {
  int success = EXIT_SUCCESS;
  char * _plan = fftw_sprint_plan(plan);
  char * left, * right = NULL;
  size_t size;

  left = _plan;
  left = strtok(left, "\n");
  while (left != NULL) {
    getline(&right, &size, handle);
    (*line)++;
    right[strlen(right) - 1] = '\0';
    if (strcmp(left, right) != 0) {
      fprintf(stderr, "Error on line %d:\n\tExpected: '%s'\n\tReceived: '%s'\n",
              *line, left, right);
      success = EXIT_FAILURE;
      break;
    }
    left = strtok(NULL, "\n");
  }
  free(_plan);
  if (right != NULL) {
    free(right);
  }
  return success;
}


/** @brief Check the values in an array.
 */
int
check_array(int * line, size_t size, real * expected, FILE * handle,
            real rtol, real atol) {
  int success = EXIT_SUCCESS;
  char * text = NULL, * tok;
  size_t len;
  getline(&text, &len, handle);
  (*line)++;
  tok = text;
  tok = strtok(tok, " ");
  for (size_t count = 1; count <= size; count++) {
    if (tok == NULL) {
      fprintf(stderr, "Invalid elements on line %d: Expected %zu, found %zu\n",
              *line, size, count);
      success = EXIT_FAILURE;
      break;
    }
    real value = atof(tok);
    if (fabs(expected[count-1] - value) > (atol + rtol * fabs(expected[count-1]))
        ) {
      fprintf(stderr,
              "Invalid element %zu on line %d:\n"
              "\tExpected: %f\n"
              "\tReceived: %f\n",
              count, *line, expected[count - 1], value);
      success = EXIT_FAILURE;
      break;
    }
    tok = strtok(NULL, " ");
  }
  if (text != NULL) {
    free(text);
  }
  return success;
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


  int success = EXIT_SUCCESS;
  handle = fopen(file, "r");
  if (handle == NULL) {
    fprintf(stderr, "Could not open %s for reading\n", file);
    goto bail;
  }

  int line = 0;
  fprintf(stderr, "Checking field->fft_plan\n");
  if (check_plan(field->fft_plan, handle, &line) != EXIT_SUCCESS) {
    success = EXIT_FAILURE;
  }
  fprintf(stderr, "Checking field->fft+plan_2d_1\n");
  if (check_plan(field->fft_plan_2d_1, handle, &line) != EXIT_SUCCESS) {
    success = EXIT_FAILURE;
  }
  fprintf(stderr, "Checking field->fft+plan_2d_2\n");
  if (check_plan(field->fft_plan_2d_2, handle, &line) != EXIT_SUCCESS) {
    success = EXIT_FAILURE;
  }

  char * _line = NULL;
  size_t size;
  getline(&_line, &size, handle);
  line++;
  int nx = atoi(_line);
  getline(&_line, &size, handle);
  line++;
  int ny = atoi(_line);
  getline(&_line, &size, handle);
  line++;
  int nz = atoi(_line);
  getline(&_line, &size, handle);
  line++;
  int nvars = atoi(_line);
  if (field->nx != nx ||
      field->ny != ny ||
      field->nz != nz ||
      field->nvars != nvars) {
    fprintf(stderr,
            "Invalid size:\n"
            "\tExpected: %dx%dx%dx%d\n"
            "\tReceived: %dx%dx%dx%d\n",
            field->nx, field->ny, field->nz, field->nvars,
            nx, ny, nz, nvars);
    success = EXIT_FAILURE;
    goto bail;
  }

  size = 2 * (nx / 2 + 1) * ny * nz;
  for (int i = 0; i < nvars; i++) {
    fprintf(stderr, "Checking field %d\n", i + 1);
    if (check_array(&line, size, field->field[i], handle, 1.0e-5, 1.0e-8)
        != EXIT_SUCCESS) {
      success = EXIT_FAILURE;
      goto bail;
    }
  }

  fprintf(stderr, "Checking kx\n");
  if (check_array(&line, field->nx, field->kx, handle, 1.0e-4, 1.0e-6)
      != EXIT_SUCCESS) {
    success = EXIT_FAILURE;
    goto bail;
  }
  fprintf(stderr, "Checking ky\n");
  if (check_array(&line, field->ny, field->ky, handle, 1.0e-4, 1.0e-6)
      != EXIT_SUCCESS) {
    success = EXIT_FAILURE;
    goto bail;
  }
  fprintf(stderr, "Checking kz\n");
  if (check_array(&line, field->nz, field->kz, handle, 1.0e-4, 1.0e-6)
      != EXIT_SUCCESS) {
    success = EXIT_FAILURE;
    goto bail;
  }

  fprintf(stderr, "Checking x\n");
  if (check_array(&line, field->nx, field->x, handle, 1.0e-4, 1.0e-6)
      != EXIT_SUCCESS) {
    success = EXIT_FAILURE;
    goto bail;
  }
  fprintf(stderr, "Checking y\n");
  if (check_array(&line, field->ny, field->y, handle, 1.0e-4, 1.0e-6)
      != EXIT_SUCCESS) {
    success = EXIT_FAILURE;
    goto bail;
  }
  fprintf(stderr, "Checking z\n");
  if (check_array(&line, field->nz, field->z, handle, 1.0e-4, 1.0e-6)
      != EXIT_SUCCESS) {
    success = EXIT_FAILURE;
    goto bail;
  }

  fprintf(stderr, "Checking dx\n");
  if (check_array(&line, 1, &field->dx, handle, 1.0e-4, 1.0e-6)
      != EXIT_SUCCESS) {
    success = EXIT_FAILURE;
    goto bail;
  }
  fprintf(stderr, "Checking dy\n");
  if (check_array(&line, 1, &field->dy, handle, 1.0e-4, 1.0e-6)
      != EXIT_SUCCESS) {
    success = EXIT_FAILURE;
    goto bail;
  }
  fprintf(stderr, "Checking dz\n");
  if (check_array(&line, 1, &field->dz, handle, 1.0e-4, 1.0e-6)
      != EXIT_SUCCESS) {
    success = EXIT_FAILURE;
    goto bail;
  }

  fprintf(stderr, "Checking dkx\n");
  if (check_array(&line, 1, &field->dkx, handle, 1.0e-4, 1.0e-6)
      != EXIT_SUCCESS) {
    success = EXIT_FAILURE;
    goto bail;
  }
  fprintf(stderr, "Checking dky\n");
  if (check_array(&line, 1, &field->dky, handle, 1.0e-4, 1.0e-6)
      != EXIT_SUCCESS) {
    success = EXIT_FAILURE;
    goto bail;
  }
  fprintf(stderr, "Checking dkz\n");
  if (check_array(&line, 1, &field->dkz, handle, 1.0e-4, 1.0e-6)
      != EXIT_SUCCESS) {
    success = EXIT_FAILURE;
    goto bail;
  }

  bail:
  if (handle != NULL) {
    fclose(handle);
  }
  return success;
}
