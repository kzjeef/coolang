/*
 *  The scanner definition for COOL.
 */

/*
 *  Stuff enclosed in %{ %} in the first section is copied verbatim to the
 *  output, so headers and global definitions are placed here to be visible
 * to the code in the file.  Don't remove anything that was here initially
 */
%{
#include <cool-parse.h>
#include <stringtab.h>
#include <utilities.h>

/* The compiler assumes these identifiers. */
#define yylval cool_yylval
#define yylex  cool_yylex

/* Max size of string constants */
#define MAX_STR_CONST 1025
#define YY_NO_UNPUT   /* keep g++ happy */

extern FILE *fin; /* we read from this file */

/* define YY_INPUT so we read from the FILE fin:
 * This change makes it possible to use this scanner in
 * the Cool compiler.
 */
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
	if ( (result = fread( (char*)buf, sizeof(char), max_size, fin)) < 0) \
		YY_FATAL_ERROR( "read() in flex scanner failed");

char string_buf[MAX_STR_CONST]; /* to assemble string constants */
char *string_buf_ptr;

extern int curr_lineno;
extern int verbose_flag;

extern YYSTYPE cool_yylval;

/*
 *  Add Your own definitions here
 */

%}

%x COMMENT
%x STRING_START
%x LINE_COMMENT

 /*
 * Define names for regular expressions here.
 */

EOL             \n
DARROW          =>
ASSIGN          <-
LET             (?i:let)
ELSE            (?i:else)
 /* FALSE */
FI              (?i:fi)
IF              (?i:if)
IN              (?i:in)
INHERITS        (?i:inherits)
ISVOID          (?i:isvoid)
LOOP            (?i:loop)
POOL            (?i:pool)
THEN            (?i:then)
WHILE           (?i:while)
CASE            (?i:case)
ESAC            (?i:esac)
NEW             (?i:new)
OF              (?i:of)
NOT             (?i:not)
 /* TRUE            true                        */
 /* false */
TBOOL            (t[Rr][Uu][Ee])
FBOOL            (f[Aa][Ll][sS][eE])
DIGIT             [0-9]
OBJID                [a-z][a-zA-Z0-9_]*
TYPEID                [A-Z][a-zA-Z0-9_]*

WSE                   [ \t\r]

%%

 /*
  *  Nested comments
  */

"--"  BEGIN(LINE_COMMENT);
<LINE_COMMENT>\n              { ++curr_lineno; BEGIN(INITIAL); }
<LINE_COMMENT>.*                {};
<LINE_COMMENT><<EOF>>         {};

"(*"    BEGIN(COMMENT);
<COMMENT>[^"*"\n]*   /*eat anything but * */
<COMMENT>"*"[^"*)"\n]*   /*eat up * follow by ) */
<COMMENT>\n             {curr_lineno++;}
<COMMENT>"*"+")"        BEGIN(INITIAL);
<COMMENT><<EOF>>        {
                        cool_yylval.error_msg = strdup("EOF in comment");
                        return (ERROR);                        
                        }
"*)"                    {
                        cool_yylval.error_msg = strdup("Unmatched *)");
                        return (ERROR);
                        }         


 /*
  *  The multiple-character operators.
  */
{DARROW}		{ return (DARROW); }
{ASSIGN}                { return (ASSIGN); }


 /*
  * Keywords are case-insensitive except for the values true and false,
  * which must begin with a lower-case letter.
  */
{LET}        {return (LET); }    
(?i:class)      {return (CLASS);}
{ELSE}       {return (ELSE); }
{FI}         {return (FI); }
{IF}         {return (IF); }
{IN}         {return (IN); }
{INHERITS}   {return (INHERITS);}
{ISVOID}     {return (ISVOID); }
{LOOP}       {return (LOOP); }
{POOL}       {return (POOL); }
{THEN}       {return (THEN); }
{WHILE}      {return (WHILE); }
{CASE}       {return (CASE); }
{ESAC}       {return (ESAC); }
{NEW}        {return (NEW); }
{OF}         {return (OF); }
{NOT}        {return (NOT); }

{DIGIT}+ {
        cool_yylval.symbol = inttable.add_string(yytext);
        return INT_CONST;
}

{TBOOL} {
        cool_yylval.boolean = 1;
        return BOOL_CONST;
}

{FBOOL} {
        cool_yylval.boolean = 0;
        return BOOL_CONST;
}



 /* TODO: needs to check the table and declear */
{OBJID}    {cool_yylval.symbol = idtable.add_string(yytext); return (OBJECTID);}

{TYPEID}    {cool_yylval.symbol = idtable.add_string(yytext); return (TYPEID);}

 /*
  *  String constants (C syntax)
  *  Escape sequence \c is accepted for all characters c. Except for 
  *  \n \t \b \f, the result is c.
  *
  */
\"  BEGIN(STRING_START);
<STRING_START>[^\n^\b^\t^\f^\"]*\"    {
                  yytext[yyleng-1]= '\0';
                  cool_yylval.symbol = stringtable.add_string(yytext); 
                  BEGIN(INITIAL);
                  return (STR_CONST);
        }

<STRING_START>[^\n^\b^\t^\f^\"]*^\\\n { cool_yylval.error_msg = strdup("Unterminated string constant");
                                      curr_lineno++;
                                      BEGIN(INITIAL);
                                      return (ERROR);
                                      }
<STRING_START>EOF               {
        cool_yylval.error_msg = strdup("EOF in string constant");
        BEGIN(INITIAL);
        return (ERROR);
 }

 /* leave these charator along. */
 /* <- => ( ) , ; + - * / < = . @ ~ { } */

[ \t\r\f\v]+  /* eat whitespace */

^\n {
        return yytext[0];
  }
 /* . { */
 /*   cool_yylval.error_msg = yytext; */
 /*   return (ERROR); */
 /* } */

<<EOF>> {
        yyterminate();
    }
%%
