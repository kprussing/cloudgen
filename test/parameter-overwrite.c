// Copyright 2022 Keith F. Prussing
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cloudgen.h"  // NOLINT(build/include_subdir)
#include "readconfig.h"  // NOLINT(build/include_subdir)

extern FILE * yyin;
extern FILE * yyerr;
extern int yyparse();
extern rc_data * parsed_data;

int main(int argc, char * argv[]) {
  int success = EXIT_SUCCESS;

  if (argc < 2) {
    fprintf(stderr, "usage: parameter-overwrite <file>\n");
    return EXIT_FAILURE;
  }

  yyin = fopen(argv[1], "r");
  if (yyin == NULL) {
    fprintf(stderr, "Error opening %s\n", argv[1]);
    return EXIT_FAILURE;
  }
  yyerr = stderr;
  success = yyparse();
  fclose(yyin);

  if (success != EXIT_SUCCESS) {
    fprintf(stderr, "Error parsing %s\n", argv[1]);
    if (parsed_data != NULL) {
      rc_clear(parsed_data);
    }
    return EXIT_FAILURE;
  }

  if (parsed_data == NULL) {
    fprintf(stderr, "No information parsed\n");
    return EXIT_FAILURE;
  }

  if (argc > 2) {
    if (rc_register_args(parsed_data, argc, argv) == 0) {
    } else {
      fprintf(stderr, "rc_register_args returned non zeron\n");
    }
  }

  if (strncasecmp(parsed_data->param, "verbose", 7) != 0) {
    fprintf(stderr, "First parameter is not verbose.  Found %s\n",
            parsed_data->param);
    return EXIT_FAILURE;
  }

  const char * overwrite = "overwrite\0";
  if (strncmp(parsed_data->value, overwrite, strlen(overwrite)) != 0) {
    fprintf(stderr, "Invalid verbose option.\n\tExpected: %s\n\tFound: %s\n",
            overwrite, parsed_data->value);
    return EXIT_FAILURE;
  }

  return success;
}
