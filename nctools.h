/* nctools.h -- useful NetCDF functions */

/* See nctools.c to see what each function does */

#define COMMENT_NAME "comment"

int nct_prepend_attribute(int ncid, int varid, char *attname, char *string, char *separator);
int nct_append_attribute(int ncid, int varid, char *attname, char *string, char *separator);
int nct_strip_parentheses(int ncid, int varid, char *attname);
int nct_add_history(int ncid, char *action, char *user);
int nct_add_command_line(int ncid, int argc, char **argv);
