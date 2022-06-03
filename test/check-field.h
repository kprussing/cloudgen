/* Copyright 2022 Keith F. Prussing */
#ifndef TEST_CHECK_FIELD_H_
#define TEST_CHECK_FIELD_H_

#include <stdlib.h>

#include "cloudgen.h"  /* NOLINT */

/* Check an FFTW plan for consistency. Returns EXIT_SUCCESS if they are
   consistent otherwise EXIT_FAILURE. */
int check_plan(fftw_plan plan, FILE * handle, int * line);

/* Check the values in an array.  Compares the values using `abs(a - b)
   < atol + rtol * abs(b)`.  Returns EXIT_SUCCESS if all values pass;
   otherwise, it returns EXIT_FAILURE. */
int check_array(int * line, size_t size, real * expected, FILE * handle,
                real rtol, real atol);

/* Check all the values in a field.  Calls check_plan and check_array as
   appropriate to determine if the fields are consistent. */
int check_field(cg_field * field, FILE * handle, real rtol, real atol);

#endif  /* TEST_CHECK_FIELD_H_ */
