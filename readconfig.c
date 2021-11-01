/* readconfig.c -- Functions to read configuration information from a file
   Copyright (C) 2003 Robin Hogan <r.j.hogan@reading.ac.uk> */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "readconfig.h"

extern FILE * yyin;
extern FILE * yyerr;
extern int yyparse();
extern rc_data * parsed_data;

/* Move file pointer to the start of the next line, returning '\n' on
   success or EOF if the file ended first. */
static
int
__rc_skip_line(FILE *file)
{
  char c;
  do {
    c = fgetc(file);
  } while (c != '\n' && c != EOF);
  return c;
}

/* Move the file pointer over whitespace, returning the first
   non-whitespace character found, or '\n' if the line ended first, or
   EOF if the file ended first. */
static
int
__rc_skip_whitespace(FILE *file)
{
  int c;
  do {
    c = fgetc(file);
  } while (c <= ' ' && c != '\n' && c != EOF);
  return c;
}

/* Find param in data, returning an element of the rc_data list, or
   NULL if not present. Note that the search is case insensitive. */
static
rc_data *
__rc_find(rc_data *data, char *param)
{
  if (data->param) {
    if (strcasecmp(param, data->param) == 0) {
      return data;
    }
    else if (data->next) {
      return __rc_find(data->next, param);
    }
  }
  return NULL;
}

/* Add a param-value pair to an existing rc_data structure,
   overwriting an existing param with the same name (case
   insensitive). Note that param and value are the actual pointers
   added to the structure so they should not be freed after calling
   this function. Return 1 on success, 0 if memory allocation of the
   next element in the list failed. */
static
int
__rc_register(rc_data *data, char *param, char *value)
{
  while (data->param) {
    if (strcasecmp(param, data->param) == 0) {
      if (data->value) {
	free(data->value);
      }
      data->value = value;
      return 1;
    }
    data = data->next;
  }
  data->param = param;
  data->value = value;
  data->next = malloc(sizeof(rc_data));
  if (!data->next) {
    return 0;
  }
  else {
    data->next->param = data->next->value = NULL;
    data->next->next = NULL;
    return 1;
  }
}

/* Read configuration information from file called file_name and
   return a pointer to the rc_data structure, or NULL if an error
   occurred.  If file_name is NULL then an empty rc_data structure is
   returned. If err_file is not NULL, errors messages will be written
   to err_file. */
rc_data *
rc_read(char *file_name, FILE *err_file)
{
  FILE *file;
  rc_data *data;
  char *memory_error = "Error allocating memory for configuration information\n";
  int c;

  if (!file_name) {
    /* Assign an empty data structure */
    data = (rc_data *) malloc(sizeof(rc_data));
    if (!data) {
      if (err_file) {
	fprintf(err_file, "%s", memory_error);
      }
      return NULL;
    }
    data->param = NULL;
    data->value = NULL;
    data->next = NULL;
    return data;
  }

  file = fopen(file_name, "r");

  if (!file) {
    if (err_file) {
      fprintf(err_file, "Error opening %s\n", file_name);
    }
    return NULL;
  }

  yyin = file;
  yyerr = err_file;
  if (yyparse() != 0) {
    fprintf(err_file, "Error parsing %s\n", file_name);
    if (parsed_data != NULL) {
      free(parsed_data);
      return NULL;
    }
  }

  fclose(file);
  data = parsed_data;
  return data;
}

/* Free an rc_data structure created with rc_read() - rc_data is a
   singly-linked list and its contents are recursively detelted. */
void
rc_clear(rc_data *data)
{
  if (data->param) {
    free(data->param);
  }
  if (data->value) {
    free(data->value);
  }
  if (data->next) {
    rc_clear(data->next);
  }
  free(data);
}

/* Add a param-value pair to an existing rc_data structure,
   overwriting an existing param with the same name (case
   insensitive). Returns 1 on success and 0 on failure. */
int
rc_register(rc_data *data, char *param, char *value)
{
  char *newparam = strdup(param);
  if (value) {
    char *newvalue = strdup(value);
    return __rc_register(data, newparam, newvalue);
  }
  else {
    return __rc_register(data, newparam, NULL);
  }
}

/* Search the command-line arguments for param=value pairs and -param
   arguments and add them to the rc_data structure. */
int
rc_register_args(rc_data *data, int argc, char **argv)
{
  int i;
  for (i = 1; i < argc; i++) {
    /* Find an "=" sign in argument i */
    if (argv[i][0] == '-' && argv[i][1]) {
      char *param = strdup(argv[i]+1);
      if (!param) {
	return 0;
      }
      if (!__rc_register(data, param, NULL)) {
	return 0;
      }
    }
    else {
      char *c = argv[i];
      while (*c != '\0') {
	if (*c == '=') {
	  /* Found one */
	  int param_length = c-argv[i];
	  char *value = strdup(c+1);
	  char *param = malloc(param_length+1);
	  if (!value || !param) {
	    return 0;
	  }
	  strncpy(param, argv[i], param_length);
	  param[param_length] = '\0';
	  if (!__rc_register(data, param, value)) {
	    return 0;
	  }
	}
	c++;
      }
    }
  }
  return 1;
}

/* Find the first config file on the command line, returning an index
   to the argv array, and 0 if none is found. Basically the first
   argument that contains no "=" sign and doesn't start with "-" is
   returned. However, after a "--" argument, a filename begining with
   "-" could in principle be returned. */
int
rc_get_file(int argc, char **argv)
{
  int i;
  int ignore_hyphen = 0;
  for (i = 1; i < argc; i++) {
    /* Find an "=" sign in argument i */
    char *c = argv[i];
    if (!ignore_hyphen && c[0] == '-') {
      if (strcmp(c, "--")) {
	ignore_hyphen = 1;
      }
      continue;
    }

    while (*c != '\0') {
      if (*c == '=') {
	break;
      }
      c++;
    }
    if (*c != '=') {
      return i;
    }
  }
  return 0;
}

/* Print contents of rc_data structure to file. */
void
rc_print(rc_data *data, FILE *file)
{
  while (data->param) {
    fprintf(file, "%s=", data->param);
    if (data->value) {
      fprintf(file, "\"%s\"\n", data->value);
    }
    else {
      fprintf(file, "(no value)\n");
    }
    if (!(data = data->next)) {
      break;
    }   
  }
}

/* Return the contents of an rc_data structure as a string. Returns
   NULL on failure. Free with rc_free(). */
char *
rc_sprint(rc_data *data)
{
  int length = 0;
  char *out = NULL;
  while (data->param) {
    int param_length = strlen(data->param);
    out = realloc(out, length+param_length+2);
    if (!out) {
      return NULL;
    }
    strcpy(out+length, data->param);
    length += (param_length+1);

    if (data->value) {
      int value_length = strlen(data->value);
      out[length-1] = ' ';
      out = realloc(out, length+value_length+2);
      if (!out) {
	return NULL;
      }
      strcpy(out+length, data->value);
      length += (value_length+1);
    }

    out[length-1] = '\n';
    out[length] = '\0';

    if (!(data = data->next)) {
      break;
    }   
  }
  return out;
}

/* Return 1 if param exists in data, 0 otherwise. */
int
rc_exists(rc_data *data, char *param)
{
  return (__rc_find(data, param) != NULL);  
}

/* Interpret the value associated with param as a boolean, returning 1
   if true and 0 if false. Note that 0 will be returned if param is
   not present, or if it exists and is "0", "no" or "false" (or any
   case insensitive variants). Any other scenario will result in 1
   being returned */
int
rc_get_boolean(rc_data *data, char *param)
{
  data = __rc_find(data, param);
  if (!data) {
    return 0;
  }
  else if (!data->value) {
    return 1;
  }
  else if (strncasecmp(data->value, "false", 5) == 0
	   || strncasecmp(data->value, "no", 2) == 0) {
    return 0;
  }
  else {
    char *endptr = data->value;
    double val = strtod(data->value, &endptr);
    if (data->value == endptr || val != 0.0) {
      return 1;
    }
    else {
      return 0;
    }
  }
}

/* Interpret the value associated with param as an integer and return
   it. If successful *status is set to 1, but if either param is not
   present or the associated value cannot be interpreted as an
   integer, *status is set to 0. */
int
rc_get_int(rc_data *data, char *param, int *status)
{
  data = __rc_find(data, param);
  if (!data || !data->value) {
    *status = 0;
    return 0;
  }
  else {
    char *endptr;
    long val = strtol(data->value, &endptr, 10);
    if (data->value == endptr) {
      *status = 0;
      return 0;
    }
    else {
      *status = 1;
      return val;
    }
  }
}

/* If param exists in data, set *value to the value associated with
   param and return 1, otherwise leave *value untouched and return
   0. */
int
rc_assign_int(rc_data *data, char *param, int *value)
{
  int status;
  int val = rc_get_int(data, param, &status);
  if (status) {
    *value = val;
  }
  return status;
}

real
rc_get_real(rc_data *data, char *param, int *status)
{
  data = __rc_find(data, param);
  if (!data || !data->value) {
    *status = 0;
    return 0;
  }
  else {
    char *endptr;
    double val = strtod(data->value, &endptr);
    if (data->value == endptr) {
      *status = 0;
      return 0;
    }
    else {
      *status = 1;
      return val;
    }
  }
}

/* As rc_assign_int() but with a real */
int
rc_assign_real(rc_data *data, char *param, real *value)
{
  int status;
  real val = rc_get_real(data, param, &status);
  if (status) {
    *value = val;
  }
  return status;
}

/* If param exists in data and can be interpretted as 1 or more
   reals, assign *value to an array of reals, returning the number
   found. If fewer than min_length are present, the array will be
   padded with the last valid real found up to min_length. In this
   case the number returned is the number of valid reals found, not
   min_length. If no reals are found or if there is an error
   allocating memory, *value is untouched and 0 is returned. */
int
rc_assign_real_array(rc_data *data, char *param,
		      real **value, int min_length)
{
  int length;
  real *val = rc_get_real_array(data, param, &length);
  if (!val) {
    return 0;
  }
  else if (length < min_length) {
    int i;
    val = realloc(val, min_length*sizeof(real));
    if (!val) {
      return 0;
    }
    for (i = length; i < min_length; i++) {
      val[i] = val[length-1];
    }
  }
  *value = val;
  return length;
}

/* As rc_assign_real_array() except that if no reals are found then
   an array of length "min_length" is returned containing
   default_value. This function always returns min_length, except if
   there was an error allocating memory in which case 0 is
   returned. */
int
rc_assign_real_array_default(rc_data *data, char *param,
	      real **value, int min_length, real default_value)
{
  int length;
  real *val = rc_get_real_array(data, param, &length);
  if (!val) {
    int i;
    val = malloc(min_length*sizeof(real));
    if (!val) {
      return 0;
    }
    for (i = 0; i < min_length; i++) {
      val[i] = default_value;
    }
  }
  else if (length < min_length) {
    int i;
    val = realloc(val, min_length*sizeof(real));
    if (!val) {
      return 0;
    }
    for (i = length; i < min_length; i++) {
      val[i] = val[length-1];
    }
  }
  *value = val;
  return min_length;
}

/* Return the value associated with param as a string. NULL is
   returned on failure or memory allocation error. The string should
   be deallocated with rc_free().  */
char *
rc_get_string(rc_data *data, char *param)
{
  data = __rc_find(data, param);
  if (!data || !data->value) {
    return NULL;
  }
  else {
    char *value = strdup(data->value);
    if (value) {
      /* Remove trailing whitespace */
      char *ch = value + strlen(value) - 1;
      while (*ch <= ' ') {
	*ch = '\0';
	ch--;
      }
    }
    return value;
  }
}

/* Free dynamically allocated data returned by rc_get_string(),
   rc_assign_string(), rc_assign_real_array(),
   rc_assign_real_array_default(), rc_get_int_array() and
   rc_get_real_array(). */
void
rc_free(void *string)
{
  if (string)
    free(string);
}

/* If param exists in data, set *value to a pointer to a copy of the
   string associated with param and return 1, otherwise leave value
   untouched and return 0. Memory allocation error also results in 0
   being returned. The string should be freed with rc_free().  */
int
rc_assign_string(rc_data *data, char *param, char **value)
{
  char *str = rc_get_string(data, param);
  if (str) {
    if (*str) {
      /* A genuine non-empty string */
      *value = str;
      return 1;
    }
    else {
      /* An empty string */
      free(str);
    }
  }
  return 0;
}

/* If param exists, interpret the associated value as integers and
   return a pointer to an int array or NULL on failure. *length
   contains the number of integers assigned, 0 if none or memory
   allocation error. The array should be freed with rc_free(). */
int *
rc_get_int_array(rc_data *data, char *param, int *length)
{
  int *out = NULL;
  data = __rc_find(data, param);
  *length = 0;

  if (!data || !data->value) {
    return NULL;
  }
  else {
    char *endptr;
    char *c = data->value;
    while (*c) {
      long val = strtol(c, &endptr, 10);
      if (endptr != c) {
	out = realloc(out, (++*length)*sizeof(int));
	if (!out) {
	  *length = 0;
	  return NULL;
	}
	out[*length-1] = val;
	c = endptr;
      }
      else {
	break;
      }
    }
    return out;
  }
}

/* As rc_get_real_array() but with reals. */
real *
rc_get_real_array(rc_data *data, char *param, int *length)
{
  real *out = NULL;
  data = __rc_find(data, param);
  *length = 0;

  if (!data || !data->value) {
    return NULL;
  }
  else {
    char *endptr;
    char *c = data->value;
    while (*c) {
      double val = strtod(c, &endptr);
      if (endptr != c) {
	out = realloc(out, (++*length)*sizeof(real));
	if (!out) {
	  *length = 0;
	  return NULL;
	}
	out[*length-1] = val;
	c = endptr;
      }
      else {
	break;
      }
    }
    return out;
  }
}

