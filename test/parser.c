#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cloudgen.h"
#include "readconfig.h"

extern FILE * yyin;
extern FILE * yyerr;
extern int yyparse();
extern rc_data * parsed_data;

/* Check the expected value of a boolean. Returns EXIT_SUCCESS or
   EXIT_FAILURE as appropriate. */
int
check_bool(rc_data * data, char * param) {
  rc_data * current;
  if (rc_get_boolean(data, param) == 1) {
    return EXIT_SUCCESS;
  }
  current = rc_find(data, param);
  if (current == NULL) {
    fprintf(stderr, "Parameter '%s' not in data\n", param);
  } else {
    fprintf(stderr, "Parameter '%s' has wrong value\n"
                    "\tExpected: true\n"
                    "\tReceived: %s\n", param, current->value);
  }
  return EXIT_FAILURE;
}

/* Check the expected value of a int. Returns EXIT_SUCCESS or
   EXIT_FAILURE as appropriate. */
int
check_int(rc_data * data, char * param, int value) {
  int status, found;
  rc_data * current;
  found = rc_get_int(data, param, &status);
  if (status == 1 && found == value) {
    return EXIT_SUCCESS;
  }
  current = rc_find(data, param);
  if (current == NULL) {
    fprintf(stderr, "Parameter '%s' not in data\n", param);
  } else {
    if (status == 1) {
      fprintf(stderr, "Parameter '%s' has wrong value\n"
                      "\tExpected: %d\n"
                      "\tReceived: %s\n",
                      param, value, current->value);
    } else {
      fprintf(stderr, "Parameter '%s' is not an integer\n"
                      "\tExpected: %d\n"
                      "\tReceived: %s\n",
                      param, value, current->value);
    }
  }
  return EXIT_FAILURE;
}

/* Check the expected value of a real. Returns EXIT_SUCCESS or
   EXIT_FAILURE as appropriate. */
int
check_real(rc_data * data, char * param, real value) {
  int status;
  real found;
  rc_data * current;
  found = rc_get_real(data, param, &status);
  if (status == 1 && fabs(value - found) < (1.0e-8 + 1e-5 * fabs(value))) {
    return EXIT_SUCCESS;
  }
  current = rc_find(data, param);
  if (current == NULL) {
    fprintf(stderr, "Parameter '%s' not in data\n", param);
  } else {
    if (status == 1) {
      fprintf(stderr, "Parameter '%s' has wrong value\n"
                      "\tExpected: %f\n"
                      "\tReceived: %s\n",
                      param, value, current->value);
    } else {
      fprintf(stderr, "Parameter '%s' is not an integer\n"
                      "\tExpected: %f\n"
                      "\tReceived: %s\n",
                      param, value, current->value);
    }
  }
  return EXIT_FAILURE;
}

/* Check the expected value of a string. Returns EXIT_SUCCESS or
   EXIT_FAILURE as appropriate. */
int
check_string(rc_data * data, char * param, char * value) {
  int success;
  char * found, * s, * back;
  rc_data * current;
  found = rc_get_string(data, param);
  if (found == NULL) {
    current = rc_find(data, param);
    if (current == NULL) {
      fprintf(stderr, "Parameter '%s' not in data\n", param);
    } else {
      fprintf(stderr, "Parameter '%s' has wrong value\n"
                      "\tExpected: %s\n"
                      "\tReceived: NULL\n", param, value);
    }
    free(found);
    return EXIT_FAILURE;
  }
  /* Trim leading and trailing white space. */
  s = found;
  while (isspace(*s)) s++;
  back = s + strlen(s) - 1;
  while (back != s && isspace(*back)) back--;
  *(back + 1) = '\0';
  if (strncmp(value, s, strlen(s)) == 0) {
    success = EXIT_SUCCESS;
  } else {
    current = rc_find(data, param);
    fprintf(stderr, "Parameter '%s' has wrong value\n"
                    "\tExpected: '%s'\n"
                    "\tReceived: '%s'\n"
                    "\tTrimmed: '%s'\n",
                    param, value, current->value, s);
    success = EXIT_FAILURE;
  }
  free(found);
  return success;
}

/* Check the expected value of a real array. Returns EXIT_SUCCESS or
   EXIT_FAILURE as appropriate. */
int
check_real_array(rc_data * data, char * param, real * value, int len) {
  real * found;
  int found_len, i, success;
  rc_data * current;
  found = rc_get_real_array(data, param, &found_len);
  if (found == NULL) {
    current = rc_find(data, param);
    if (current == NULL) {
      fprintf(stderr, "Parameter '%s' not in data\n", param);
    } else {
      fprintf(stderr, "Parameter '%s' has wrong value\n"
                      "\tExpected:", param);
      for (i = 0; i < len; i++) {
          fprintf(stderr, " %f", value[i]);
      }
      fprintf(stderr, "\n\tReceived: NULL\n");
    }
    free(found);
    return EXIT_FAILURE;
  }

  if (len != found_len) {
    fprintf(stderr, "Parameter '%s' has wrong length\n"
                    "\tExpected: %d\n"
                    "\tReceived: %d\n", param, len, found_len);
    free(found);
    return EXIT_FAILURE;
  }

  success = EXIT_SUCCESS;
  for (i = 0; i < len; i++) {
    success = fabs(value[i] - found[i]) < (1.0e-8 + 1.0e-5 * fabs(value[i]))
      ? success : EXIT_FAILURE;
  }
  if (success != EXIT_SUCCESS) {
    fprintf(stderr, "Parameter '%s' has wrong value\n"
                    "\tExpected:", param);
    for (i = 0; i < len; i++) {
        fprintf(stderr, " %f", value[i]);
    }
    fprintf(stderr, "\n\tReceived:");
    for (i = 0; i < len; i++) {
        fprintf(stderr, " %f", found[i]);
    }
    fprintf(stderr, "\n");
  }

  return success;
}

int
main(int argc, char * argv[])
{
  int success;
  real expected[7][8] = {
    /* interp_height */
    {7000.0, 7500.0, 8000.0, 8500.0, 9000.0, 9500.0, 10000.0, 10500.0},
    /* v_wind */
    {10.0, 15.0, 20.0, 25.0, 30.0, 35.0, 40.0, 40.0},
    /* u_wind */
    {5.0, 2.5, 0.0, -1.0, 0.0, 2.5, 5.0, 5.0},
    /* fall_speed */
    {1.0, 0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3},
    /* horizontal_exponent */
    {-1.667, -3.0, -2.5, -2.0, -1.667, -1.667, -1.667, -1.667},
    /* mean */
    {0.0, 0.06, 0.05, 0.04, 0.02, 0.01, 0.0001, 0.0},
    /* standard_deviation */
    {2.0, 1.0, 1.25, 1.5, 1.75, 2.0, 2.1, 2.2},
  };

  if (argc < 2) {
    fprintf(stderr, "usage: test-parser <file>\n");
    return EXIT_FAILURE;
  }

  yyin = fopen(argv[1], "r");
  if (!yyin) {
    fprintf(stderr, "Error opening %s\n", argv[1]);
    return EXIT_FAILURE;
  }
  yyerr = stderr;
  success = yyparse();
  fclose(yyin);

  if (success != 0) {
    fprintf(stderr, "Error parsing %s\n", argv[1]);
    if (parsed_data != NULL) {
      free(parsed_data);
    }
    return EXIT_FAILURE;
  }

  if (parsed_data == NULL) {
    fprintf(stdout, "No information parsed\n");
    return EXIT_FAILURE;
  }

  /* Check the boolean variables */
  success = check_bool(parsed_data, "verbose");
  success = check_bool(parsed_data, "anisotropic_mixing") == EXIT_SUCCESS
      ? success : EXIT_FAILURE;
  success = check_bool(parsed_data, "lognormal_distribution") == EXIT_SUCCESS
      ? success : EXIT_FAILURE;

  /* Check the scalar variables */
  success = check_real(parsed_data, "x_domain_size", 200000.0) == EXIT_SUCCESS
      ? success : EXIT_FAILURE;
  success = check_real(parsed_data, "z_domain_size", 3500.0) == EXIT_SUCCESS
      ? success : EXIT_FAILURE;
  success = check_int(parsed_data, "x_pixels", 256) == EXIT_SUCCESS
      ? success : EXIT_FAILURE;
  success = check_int(parsed_data, "z_pixels", 32) == EXIT_SUCCESS
      ? success : EXIT_FAILURE;
  success = check_real(parsed_data, "z_offset", 7000.0) == EXIT_SUCCESS
      ? success : EXIT_FAILURE;
  success = check_int(parsed_data, "seed", 2) == EXIT_SUCCESS
      ? success : EXIT_FAILURE;
  success = check_real(parsed_data, "vertical_exponent", -2.0) == EXIT_SUCCESS
      ? success : EXIT_FAILURE;
  success = check_real(parsed_data, "outer_scale", 50000.0) == EXIT_SUCCESS
      ? success : EXIT_FAILURE;
  success = check_real(parsed_data, "generating_level", 9500.0) == EXIT_SUCCESS
      ? success : EXIT_FAILURE;
  success = check_real(parsed_data, "threshold", 0.001) == EXIT_SUCCESS
      ? success : EXIT_FAILURE;
  success = check_real(parsed_data, "missing_value", -999.0) == EXIT_SUCCESS
      ? success : EXIT_FAILURE;

  /* Check some string values */
  success = check_string(parsed_data, "output_filename", "iwc.nc") == EXIT_SUCCESS
      ? success : EXIT_FAILURE;
  success = check_string(parsed_data, "variable_name", "iwc") == EXIT_SUCCESS
      ? success : EXIT_FAILURE;
  success = check_string(parsed_data, "long_name", "Ice water content") == EXIT_SUCCESS
      ? success : EXIT_FAILURE;
  success = check_string(parsed_data, "units", "g m-3") == EXIT_SUCCESS
      ? success : EXIT_FAILURE;
  success = check_string(parsed_data, "title", "3D stochastic field of ice water content") == EXIT_SUCCESS
      ? success : EXIT_FAILURE;

  /* Now check the arrays of reals */
  success = check_real_array(parsed_data, "interp_height", expected[0], 8) == EXIT_SUCCESS
     ? success : EXIT_FAILURE;
  success = check_real_array(parsed_data, "v_wind", expected[1], 8) == EXIT_SUCCESS
     ? success : EXIT_FAILURE;
  success = check_real_array(parsed_data, "u_wind", expected[2], 8) == EXIT_SUCCESS
     ? success : EXIT_FAILURE;
  success = check_real_array(parsed_data, "fall_speed", expected[3], 8) == EXIT_SUCCESS
     ? success : EXIT_FAILURE;
  success = check_real_array(parsed_data, "horizontal_exponent", expected[4], 8) == EXIT_SUCCESS
     ? success : EXIT_FAILURE;
  success = check_real_array(parsed_data, "mean", expected[5], 8) == EXIT_SUCCESS
     ? success : EXIT_FAILURE;
  success = check_real_array(parsed_data, "standard_deviation", expected[6], 8) == EXIT_SUCCESS
     ? success : EXIT_FAILURE;

  return success;
}
