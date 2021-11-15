%require "3.0.1"

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Flex details
int yylex(void);
extern FILE * yyin;
FILE * yyerr;
extern int line_num;

void yyerror(const char * s);

%}

%code requires {
#include "readconfig.h"
rc_data * parsed_data;
}

%union {
    char * text;
    rc_data * node;
}

// Define the tokens
%token <text> PARAM
%token SEPARATOR
%token <text> VALUE

// Map out the types
%type <node> file
%type <node> parameters
%type <node> parameter

%initial-action {
if (parsed_data != NULL) {
    rc_clear(parsed_data);
}
parsed_data = (rc_data *) malloc(sizeof (rc_data));
parsed_data->param = NULL;
parsed_data->value = NULL;
parsed_data->next = NULL;
}

%%

file:
    parameters
    {   rc_data * walker;
        walker = $1;
        while (walker != NULL) {
            rc_register(parsed_data, walker->param, walker->value);
            walker = walker->next;
        }
        rc_clear($1);
        $$ = parsed_data;
    }

parameters:
    parameter { $$ = $1; }
|   parameter parameters {
        $$ = $1;
        $$->next = $2;
}


parameter:
    PARAM {
        $$ = (rc_data *) malloc(sizeof (rc_data));
        if ($$ == NULL) {
            fprintf(yyerr, "Error allocating memory for %s\n", $1);
            free($1);
            return -1;
        }
        $$->param = strdup($1);
        $$->value = strdup("true\0");
        $$->next = NULL;
        free($1);
    }
|   PARAM SEPARATOR VALUE {
        $$ = (rc_data *) malloc(sizeof (rc_data));
        if ($$ == NULL) {
            fprintf(yyerr, "Error allocating memory for %s\n", $1);
            free($1);
            return -1;
        }
        $$->param = strdup($1);
        $$->value = strdup($3);
        $$->next = NULL;
        free($1);
        free($3);
    }

%%

void yyerror(const char * s)
{
    fprintf(yyerr, "Parsing error '%s' at line %d\n", s, line_num);
}