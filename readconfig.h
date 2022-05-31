/* readconfig.h -- Functions to read configuration information from a file
   Copyright (C) 2003, 2006 Robin Hogan <r.j.hogan@reading.ac.uk> */

#ifndef _READCONFIG_H
#define _READCONFIG_H 1

#ifdef __cplusplus
extern "C" {
#endif

/* Get the "real" type */
#include "cloudgen.h"

/* Configuration information is stored as a singly linked list as a
   set of param-value pairs. */
typedef struct __rc_data rc_data;
struct __rc_data {
  char *param;
  char *value;
  rc_data *next;
};

/* Read configuration information from file called file_name and
   return a pointer to the rc_data structure, or NULL if an error
   occurred.  If err_file is not NULL, errors messages will be written
   to err_file. */
rc_data *rc_read(char *file_name, FILE *err_file);

/* Free an rc_data structure created with rc_read() - rc_data is a
   singly-linked list and its contents are recursively detelted. */
void rc_clear(rc_data *data);

/* Add a param-value pair to an existing rc_data structure,
   overwriting an existing param with the same name (case
   insensitive). Returns 1 on success and 0 on failure. */
int rc_register(rc_data *data, char *param, char *value);

/* Search the command-line arguments for param=value pairs and -param
   arguments and add them to the rc_data structure. Returns 1 on
   success and 0 on failure. */
int rc_register_args(rc_data *data, int argc, char **argv);

/* Find the first config file on the command line, returning an index
   to the argv array, and 0 if none is found. Basically the first
   argument that contains no "=" sign and doesn't start with "-" is
   returned. */
int rc_get_file(int argc, char **argv);

/* Print contents of rc_data structure to file. */
void rc_print(rc_data *data, FILE *file);

/* Return the contents of an rc_data structure as a string. Returns
   NULL on failure. Free with rc_free(). */
char *rc_sprint(rc_data *data);

/* Return 1 if param exists in data, 0 otherwise. */
int rc_exists(rc_data *data, char *param);

/* Find param in data, returning an element of hte rc_data list, or
   NULL if not present.  Notee that the search is case insensitive. */
rc_data * rc_find(rc_data *data, char *param);

/* Interpret the value associated with param as a boolean, returning 1
   if true and 0 if false. Note that 0 will be returned if param is
   not present, or if it exists and is "0", "no" or "false" (or any
   case insensitive variants). Any other scenario will result in 1
   being returned */
int rc_get_boolean(rc_data *data, char *param);

/* Interpret the value associated with param as an integer and return
   it. If successful *status is set to 1, but if either param is not
   present or the associated value cannot be interpreted as an
   integer, *status is set to 0. */
int rc_get_int(rc_data *data, char *param, int *status);

/* Interpret the value associated with param as a real. */
real rc_get_real(rc_data *data, char *param, int *status);

/* Return the value associated with param as a string. NULL is
   returned on failure or memory allocation error. The string should
   be deallocated with rc_free().  */
char *rc_get_string(rc_data *data, char *param);

/* If param exists, interpret the associated value as integers and
   return a pointer to an int array or NULL on failure. *length
   contains the number of integers assigned, 0 if none or memory
   allocation error. The array should be freed with rc_free(). */
int *rc_get_int_array(rc_data *data, char *param, int *length);

/* As rc_get_real_array() but with reals. */
real *rc_get_real_array(rc_data *data, char *param, int *length);

/* Free dynamically allocated data returned by rc_get_string(),
   rc_assign_string(), rc_assign_real_array(),
   rc_assign_real_array_default(), rc_get_int_array() and
   rc_get_real_array(). */
void rc_free_string(char *string);

/* If param exists in data, set *value to the value associated with
   param and return 1, otherwise leave *value untouched and return
   0. */
int rc_assign_int(rc_data *data, char *param, int *value);

/* As rc_assign_int() but with a real */
int rc_assign_real(rc_data *data, char *param, real *value);

/* If param exists in data, set *value to a pointer to a copy of the
   string associated with param and return 1, otherwise leave value
   untouched and return 0. Memory allocation error also results in 0
   being returned. The string should be freed with rc_free().  */
int rc_assign_string(rc_data *data, char *param, char **value);

/* If param exists in data and can be interpretted as 1 or more
   reals, assign *value to an array of reals, returning the number
   found. If fewer than min_length are present, the array will be
   padded with the last valid real found up to min_length. In this
   case the number returned is the number of valid reals found, not
   min_length. If no reals are found or if there is an error
   allocating memory, *value is untouched and 0 is returned. */
int rc_assign_real_array(rc_data *data, char *param,
			  real **value, int min_length);

/* As rc_assign_real_array() except that if no reals are found then
   an array of length "min_length" is returned containing
   default_value. This function always returns min_length, except if
   there was an error allocating memory in which case 0 is
   returned. */
int rc_assign_real_array_default(rc_data *data, char *param,
	  real **value, int min_length, real default_value);

/* Generate a base field from a configuration data. If an error occurs,
   the field is freed and NULL is returned. */
cg_field * rc_generate_base_field(rc_data * data);

#ifdef __cplusplus
}
#endif

#endif
