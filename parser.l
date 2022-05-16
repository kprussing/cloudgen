%{
#include <stdio.h>
#include <string.h>

#include "parser.h"

int line_num = 1;
%}

%option noyywrap noinput nounput

COMMENT #
EOL     \n
SPACE   [ \t\r]

%s payload

%%

<payload>{EOL}          { BEGIN(INITIAL); }
{EOL}                   { line_num++; }

{COMMENT}.*             ;

<payload>.*             { yylval.text = strdup(yytext);
                          return VALUE;
                        }

[A-Za-z_][A-Za-z0-9_]*  { yylval.text = strdup(yytext);
                          return PARAM;
                        }

{SPACE}+                { BEGIN(payload);
                          return SEPARATOR;
                        }

%%
