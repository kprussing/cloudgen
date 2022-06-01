/* Helper tools for testing */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "check-field.h"

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
  size_t count;
  for (count = 1; count <= size; count++) {
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

/** @brief Check the full field
 */
int
check_field(cg_field * field, FILE * handle, real rtol, real atol) {
  int success = EXIT_SUCCESS;
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
  int i;
  for (i = 0; i < nvars; i++) {
    fprintf(stderr, "Checking field %d\n", i + 1);
    if (check_array(&line, size, field->field[i], handle, rtol, atol)
        != EXIT_SUCCESS) {
      success = EXIT_FAILURE;
      goto bail;
    }
  }

  fprintf(stderr, "Checking kx\n");
  if (check_array(&line, field->nx, field->kx, handle, rtol, atol)
      != EXIT_SUCCESS) {
    success = EXIT_FAILURE;
    goto bail;
  }
  fprintf(stderr, "Checking ky\n");
  if (check_array(&line, field->ny, field->ky, handle, rtol, atol)
      != EXIT_SUCCESS) {
    success = EXIT_FAILURE;
    goto bail;
  }
  fprintf(stderr, "Checking kz\n");
  if (check_array(&line, field->nz, field->kz, handle, rtol, atol)
      != EXIT_SUCCESS) {
    success = EXIT_FAILURE;
    goto bail;
  }

  fprintf(stderr, "Checking x\n");
  if (check_array(&line, field->nx, field->x, handle, rtol, atol)
      != EXIT_SUCCESS) {
    success = EXIT_FAILURE;
    goto bail;
  }
  fprintf(stderr, "Checking y\n");
  if (check_array(&line, field->ny, field->y, handle, rtol, atol)
      != EXIT_SUCCESS) {
    success = EXIT_FAILURE;
    goto bail;
  }
  fprintf(stderr, "Checking z\n");
  if (check_array(&line, field->nz, field->z, handle, rtol, atol)
      != EXIT_SUCCESS) {
    success = EXIT_FAILURE;
    goto bail;
  }

  fprintf(stderr, "Checking dx\n");
  if (check_array(&line, 1, &field->dx, handle, rtol, atol)
      != EXIT_SUCCESS) {
    success = EXIT_FAILURE;
    goto bail;
  }
  fprintf(stderr, "Checking dy\n");
  if (check_array(&line, 1, &field->dy, handle, rtol, atol)
      != EXIT_SUCCESS) {
    success = EXIT_FAILURE;
    goto bail;
  }
  fprintf(stderr, "Checking dz\n");
  if (check_array(&line, 1, &field->dz, handle, rtol, atol)
      != EXIT_SUCCESS) {
    success = EXIT_FAILURE;
    goto bail;
  }

  fprintf(stderr, "Checking dkx\n");
  if (check_array(&line, 1, &field->dkx, handle, rtol, atol)
      != EXIT_SUCCESS) {
    success = EXIT_FAILURE;
    goto bail;
  }
  fprintf(stderr, "Checking dky\n");
  if (check_array(&line, 1, &field->dky, handle, rtol, atol)
      != EXIT_SUCCESS) {
    success = EXIT_FAILURE;
    goto bail;
  }
  fprintf(stderr, "Checking dkz\n");
  if (check_array(&line, 1, &field->dkz, handle, rtol, atol)
      != EXIT_SUCCESS) {
    success = EXIT_FAILURE;
    goto bail;
  }

  bail:
  return success;
}
