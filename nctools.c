/* nctools.c -- useful NetCDF functions */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include <netcdf.h>

#define MAX_STRING_LENGTH 512

/* Prepend "string" to the attribute "attname" of the variable with ID
 * "varid" (where NC_GLOBAL indicates a global attribute) of NetCDF
 * dataset with ID "ncid", using separator "separator". */
int 
nct_prepend_attribute(int ncid, int varid, char *attname, char *string, char *separator)
{ 
  size_t oldlen, newlen = strlen(string);
  int status;

  status = nc_inq_attlen(ncid, varid, attname, &oldlen);

  if (status == NC_ENOTATT) {
    /* New attribute */
    return nc_put_att_text(ncid, varid, attname, newlen, string);
  }
  else if (status == NC_NOERR) {
    /* Existing attribute */
    size_t fulllen = oldlen + newlen + strlen(separator) + 1;
    char *newstring = malloc(fulllen * sizeof(char));
    if (!newstring) {
      return NC_ENOMEM;
    }
    sprintf(newstring, "%s%s", string, separator);
    status = nc_get_att_text(ncid, varid, attname, newstring+strlen(newstring));
    if (status != NC_NOERR) {
      free(newstring);
      return status;
    }
    newstring[fulllen-1] = '\0';
    status = nc_put_att_text(ncid, varid, attname, strlen(newstring), newstring);
    free(newstring);
    return status;
  }
  else {
    /* An error occurred */
    return status;
  }
}

/* Append "string" to the attribute "attname" of the variable with ID
 * "varid" (where NC_GLOBAL indicates a global attribute) of NetCDF
 * dataset with ID "ncid", using separator "separator". */
int
nct_append_attribute(int ncid, int varid, char *attname, char *string, char *separator)
{
  size_t oldlen, newlen = strlen(string);
  int status;

  status = nc_inq_attlen(ncid, varid, attname, &oldlen);

  if (status == NC_ENOTATT) {
    /* New attribute */
    return nc_put_att_text(ncid, varid, attname, newlen, string);
  }
  else if (status == NC_NOERR) {
    /* Existing attribute */
    size_t fulllen = oldlen + newlen + strlen(separator) + 1;
    char *newstring = malloc(fulllen * sizeof(char));
    if (!newstring) {
      return NC_ENOMEM;
    }
    status = nc_get_att_text(ncid, varid, attname, newstring);
    newstring[oldlen] = '\0';
    sprintf(newstring+strlen(newstring), "%s%s", separator, string);

    if (status != NC_NOERR) {
      free(newstring);
      return status;
    }
    status = nc_put_att_text(ncid, varid, attname, strlen(newstring), newstring);
    free(newstring);
    return status;
  }
  else {
    /* An error occurred */
    return status;
  }
}

/* Append information to the "history" global attribute of a NetCDF
 * dataset.  The information is of the form "$action at $time by $user
 * on $host", where action and user are given as arguments (a value of
 * NULL for user causes the username to be used), time is the current
 * time and host is the name of the machine. Histories are separated
 * by semicolons. */
int
nct_add_history(int ncid, char *action, char *user)
{
  struct timeval tv;
  char *name;
  char hostname[MAX_STRING_LENGTH] = "unknown";
  char history[MAX_STRING_LENGTH] = "";

  if (gettimeofday(&tv, NULL)) {
    name = "";
  }
  else {
    name = ctime(&tv.tv_sec);
    name[24] = ' '; /* Get rid of the newline */
  }

  gethostname(hostname, MAX_STRING_LENGTH);

  if (!user) {
    /* user string is NULL; get the username instead */
    if (!(user = getenv("LOGNAME"))) {
      if (!(user = getenv("USER"))) {
	user = "anonymous";
      }    
    }
  }
  
  snprintf(history, MAX_STRING_LENGTH, "%s- %s by %s on %s", name, action, user, hostname);
  history[MAX_STRING_LENGTH-1] = '\0';

  return nct_append_attribute(ncid, NC_GLOBAL, "history", history, "\n");
}

/* Append the full command line to the global attribute "command_line"
 * of a NetCDF file, using semicolons as separators if several
 * programs write to the same file. */
#define COMPARE_ARG_LENGTH 4
#define NUM_SIMILAR_ARGS 5
int
nct_add_command_line(int ncid, int argc, char **argv)
{
  int i, j, len = 0;
  char *cmdline;
  char lastarg[COMPARE_ARG_LENGTH+1];
  int status;
  int numsimilarargs = 1;
  
  if (argv == NULL) {
    return NC_EINVAL;
  }
  for (i = 0; i < argc; i++) {
    char *c = argv[i];
    char need_quotes = 0;
    for (j = 0; c[j] != '\0'; j++) {
      if (c[j] <= ' ') {
	need_quotes = 1;
      }
    }
    len += (1 + j + need_quotes * 2);
  }

  cmdline = malloc(len * sizeof(char));
  if (cmdline == NULL) {
    return NC_ENOMEM;
  }

  len = 0;
  lastarg[0] = '\0';
  for (i = 0; i < argc; i++) {
    char *c = argv[i];
    char need_quotes = 0;
    for (j = 0; c[j] != '\0'; j++) {
      if (c[j] <= ' ') {
	need_quotes = 1;
      }
    }
    /* This next bit of code ensures that if more than
       NUM_SIMILAR_ARGS are found that share the same first
       COMPARE_ARG_LENGTH characters, then an elipsis (...) will be
       output.  This saves on excessively long command lines being
       saved. */
    if (j > COMPARE_ARG_LENGTH) {
      if (c[0] != '-' && strncmp(lastarg, c, COMPARE_ARG_LENGTH) == 0) {
	numsimilarargs++;
	if (numsimilarargs == NUM_SIMILAR_ARGS+1) {
	  c = "...";
	  j = 3;
	}
	else if (numsimilarargs > NUM_SIMILAR_ARGS+1) {
	  continue;
	}
      }
      else {
	snprintf(lastarg, COMPARE_ARG_LENGTH+1, "%s", c);
	lastarg[COMPARE_ARG_LENGTH] = '\0';
	numsimilarargs = 1;
      }
    }
    else {
      numsimilarargs = 1;
    }

    if (need_quotes) {
      sprintf(cmdline + len, "\"%s\"", c);
    }
    else {
      sprintf(cmdline + len, "%s", c);
    }
    len += (1 + j + need_quotes * 2);
    cmdline[len -1] = ' ';
  }
  cmdline[len-1] = '\0';

  status = nct_append_attribute(ncid, NC_GLOBAL, "command_line", cmdline, "\n");
  free(cmdline);

  return status;
}

/* Remove trailing abreviations in parantheses from an attribute */
int
nct_strip_parentheses(int ncid, int varid, char *attname)
{
  size_t len;
  int status;

  status = nc_inq_attlen(ncid, varid, attname, &len);

  if (status == NC_NOERR) {
    char *str = malloc(len * sizeof(char));
    char *ch;
    if (!str) {
      return NC_ENOMEM;
    }
    status = nc_get_att_text(ncid, varid, attname, str);
    if (status != NC_NOERR) {
      return status;
    }
    ch = str + len - 1;
    if (*ch == '\0' && len > 4) {
      --ch;
    }
    if (*ch == ')') {
      while ((--ch) > str) {
	if (*ch == '(') {
	  ch--;
	  if (*ch == ' ') {
	    *ch = '\0';
	  }
	  break;
	}
      }
    }
    if (*ch == '\0') {
      nc_put_att_text(ncid, varid, attname, strlen(str), str);
    }
    free(str);
    return NC_NOERR;
  }
  else {
    /* An error occurred */
    return status;
  }
  
}
