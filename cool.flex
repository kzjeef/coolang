/*
 *  The scanner definition for COOL.
 */

/*
 *  Stuff enclosed in %{ %} in the first section is copied verbatim to the
 *  output, so headers and global definitions are placed here to be visible
 * to the code in the file.  Don't remove anything that was here initially
 */

L			[a-zA-Z_]

%{
#include <cool-parse.h>
#include <stringtab.h>
#include <utilities.h>

int input(void);        
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
int  string_has_zero = 0;
int  string_has_escape_null = 0;

extern int curr_lineno;
extern int verbose_flag;
int comment_depth = 0;

extern YYSTYPE cool_yylval;
void count();
int is_valid_char(char c);

/* #define DEBUG_COMMENT */
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
LE              <=
TBOOL            (t[Rr][Uu][Ee])
FBOOL            (f[Aa][Ll][sS][eE])
DIGIT             [0-9]
OBJID                [a-z][a-zA-Z0-9_]*
TYPEID                [A-Z][a-zA-Z0-9_]*
SPC            [ \t\r\f\v]

%%

 /*
  *  Nested comments
  */

"--"  BEGIN(LINE_COMMENT);
<LINE_COMMENT>\n              {++curr_lineno; BEGIN(INITIAL); }
<LINE_COMMENT>.*
<LINE_COMMENT><<EOF>>         {BEGIN(INITIAL);yyterminate();}

"(*"                          {
        if (comment_depth++ == 0) {
#ifdef DEBUG_COMMENT
                printf("BEGIN\n");
#endif
                BEGIN(COMMENT);
        }
}
<COMMENT>[^*\n]*"("+"*"     {
#ifdef DEBUG_COMMENT
        printf("BEGIN2\n");
#endif
        comment_depth++;
 }
<COMMENT>[^*\n]*   /*eat anything but * */
<COMMENT>"*"[^"*)"\n]*  {}  /*eat up * follow by ) */
<COMMENT>\n             {curr_lineno++;}
<COMMENT>"*"+")"        {
        if (--comment_depth == 0) {
#ifdef DEBUG_COMMENT
                printf("FINISH\n");
#endif
                BEGIN(INITIAL);}
 }
<COMMENT><<EOF>>        {
#ifdef DEBUG_COMMENT
        printf("EOF Comment?\n");
#endif
                        cool_yylval.error_msg = strdup("EOF in comment");

                        BEGIN(INITIAL);
                        return (ERROR);
                        yyterminate();
                        }
"*)"                    {
        cool_yylval.error_msg = strdup("unmatch *)");
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
{LE}         {return (LE); }
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
{OBJID}    {
        cool_yylval.symbol = idtable.add_string(yytext);
        return (OBJECTID);
}

{TYPEID}    {
        cool_yylval.symbol = idtable.add_string(yytext);
        return (TYPEID);
}

 /*
  *  String constants (C syntax)
  *  Escape sequence \c is accepted for all characters c. Except for 
  *  \n \t \b \f, the result is c.
  *
  */



 /* a string is a " and zero or more eith an escapted any thing, \\.
     or a non-quota charactor, and finally the termination quote
     put together:
     got following this.
 */
 /* TODO: check the backslash2 test case... */

\"     {
        string_buf_ptr = string_buf;
        string_has_zero = 0;
        string_has_escape_null = 0;
        BEGIN(STRING_START);
}

<STRING_START>\"          {     /*the end string quote.*/

        if (string_has_zero) {
                cool_yylval.error_msg = "String contains null character.";
                BEGIN(INITIAL);
                return ERROR;
        }

        if (string_has_escape_null) {
                cool_yylval.error_msg = "String contains escaped null character.";
                BEGIN(0);
                return ERROR;
        }

        if (string_buf_ptr - string_buf >= sizeof(string_buf)) {
                cool_yylval.error_msg = "String constant too long";
                BEGIN(INITIAL);
                return ERROR;
        }
        *string_buf_ptr = '\0';
        cool_yylval.symbol = stringtable.add_string(string_buf);
        BEGIN(INITIAL);
        return (STR_CONST);
 }
<STRING_START>\n {
                cool_yylval.error_msg = "Unterminated string constant";
                BEGIN(0);
                curr_lineno++;
                return (ERROR);
 }

<STRING_START>\\[0-9] {
        if (string_buf_ptr > (string_buf + sizeof(string_buf))) {
                cool_yylval.error_msg = "String constant too long";
                BEGIN(INITIAL);
                return (ERROR);
        }
        *string_buf_ptr++ = yytext[1];
 }

<STRING_START>\\n   {
        if (string_buf_ptr > (string_buf + sizeof(string_buf))) {
                cool_yylval.error_msg = "String constant too long";
                BEGIN(INITIAL);
                return (ERROR);
        }
        *string_buf_ptr++ = '\n';
 }

<STRING_START>\\t {
        if (string_buf_ptr > (string_buf + sizeof(string_buf))) {
                cool_yylval.error_msg = "String constant too long";
                BEGIN(INITIAL);
                return (ERROR);
        }
        *string_buf_ptr++ = '\t';
 }

<STRING_START>\\\t {
        if (string_buf_ptr > (string_buf + sizeof(string_buf))) {
                cool_yylval.error_msg = "String constant too long";
                BEGIN(INITIAL);
                return (ERROR);
        }
        *string_buf_ptr++ = '\t';
 }

<STRING_START>\\\\ {
        if (string_buf_ptr > (string_buf + sizeof(string_buf))) {
                cool_yylval.error_msg = "String constant too long";
                BEGIN(INITIAL);
                return (ERROR);
        }
        *string_buf_ptr++ = '\\';
 }

<STRING_START>\\f {
        if (string_buf_ptr > (string_buf + sizeof(string_buf))) {
                cool_yylval.error_msg = "String constant too long";
                BEGIN(INITIAL);
                return (ERROR);
        }
        *string_buf_ptr++ = '\f';
 }

<STRING_START>\\b {
        if (string_buf_ptr > (string_buf + sizeof(string_buf))) {
                cool_yylval.error_msg = "String constant too long";
                BEGIN(INITIAL);
                return (ERROR);
        }
        *string_buf_ptr++ = '\b';
}

<STRING_START>\\[arcdge] {
        if (string_buf_ptr > (string_buf + sizeof(string_buf))) {
                cool_yylval.error_msg = "String constant too long";
                BEGIN(INITIAL);
                return (ERROR);
        }
        *string_buf_ptr++ = yytext[1];
 }


<STRING_START>\\\" {
        if (string_buf_ptr > (string_buf + sizeof(string_buf))) {
                cool_yylval.error_msg = "String constant too long";
                BEGIN(INITIAL);
                return (ERROR);
        }
        *string_buf_ptr++ = '\"';
}

<STRING_START>\\\0 {
        string_has_escape_null = 1;
}

<STRING_START>\\\n {
        if (string_buf_ptr > (string_buf + sizeof(string_buf))) {
                cool_yylval.error_msg = "String constant too long";
                BEGIN(INITIAL);
                return (ERROR);
        }
        curr_lineno++;
        *string_buf_ptr++ = '\n';
 }

 /* 
   <STRING_START> {
        if (string_buf_ptr > (string_buf + sizeof(string_buf))) {
                cool_yylval.error_msg = "String constant too long";
                BEGIN(INITIAL);
                return (ERROR);
        }
        "\\n"  *string_buf_ptr++ = '\n';
        "\\r"  *string_buf_ptr++ = '\r';
        "\\b", *string_buf_ptr++ = '\b';
        "\\f", *string_buf_ptr++ = 'f';
        "\\0"  cool_yylval.error_msg = "String contains null character.";  return (ERROR);
    }
 */

<STRING_START>[.|\n]    {
        if (string_buf_ptr > (string_buf + sizeof(string_buf))) {
                cool_yylval.error_msg = "String constant too long";
                BEGIN(INITIAL);
                return (ERROR);
        }
        *string_buf_ptr++ = yytext[1];
 }

<STRING_START>[^\\\n\"]+ {
        char *yptr = yytext;
        int i = 0;
        for (; i < yyleng
                     && (string_buf_ptr - string_buf) <= sizeof(string_buf);
             i++) {
                if (*yptr == '\0')
                        string_has_zero = 1;
                
                *string_buf_ptr++ = *yptr++;
        }



        if (string_buf_ptr - string_buf > sizeof(string_buf)) {
                cool_yylval.error_msg = "String constant too long";
                BEGIN(INITIAL);
                return (ERROR);
        }
 }

<STRING_START><<EOF>> {
        cool_yylval.error_msg = "EOF in string constant";
        BEGIN(INITIAL);
//        yyterminate();
        return (ERROR);
 }

 /* leave these charator along. */
 /* <- => ( ) , ; + - * / < = . @ ~ { } */

{SPC}  {} /* eat whitespace */
[ \t] {}
\n    {curr_lineno++;}
. {

        if (!((yytext[0] > 'a' && yytext[0] < 'z')
            || (yytext[0] > 'A' && yytext[0] < 'Z')
           || (yytext[0] > '1' && yytext[0] < '9')
            || is_valid_char(yytext[0]))) {
              cool_yylval.error_msg = strdup(yytext);
              return ERROR;
        }
        if (yytext[0] == '_' || yytext[0] == '\0') {
              cool_yylval.error_msg = strdup(yytext);
              return ERROR;
        }

        if (yytext[0] != ' ')
                return yytext[0];
  }

<<EOF>> {
        yyterminate();
    }
%%


int is_valid_char(char c) {

        char *chars = "(),;:+-*/<=.@~{}";
        for (int i = 0; i < strlen(chars); i++) {
                if (c == chars[i])
                   return 1;
        }
        return 0;
}



/*   \"(\\.|[^"])*\" { */
/*   //  cool_yylval.symbol = stringtable.add_string(str); */
/*         int i; */

/*         char *str = (char *)malloc(MAX_STR_CONST+1); */
/*         memset(str, 0, MAX_STR_CONST+1); */
/*         int backslash = 0; */
/*         int stringleng = 0; */
/*         int charcount = 0; */
/*         for (i = 1; yytext[i] != '\0' && charcount < MAX_STR_CONST+1; i++,charcount++) { */

/*                 /\* if (yytext[i] == '\0') { *\/ */
/*                 /\*                cool_yylval.error_msg = "String contains null character."; *\/ */
/*                 /\*                 free(str); *\/ */
/*                 /\*                 /\\* BEGIN(INITIAL); *\\/ *\/ */
/*                 /\*                 return (ERROR); *\/ */
/*                 /\*           } *\/ */
/*                 if (yytext[i] != '\\') { */
/*                         if (yytext[i] == '\n') { */
/*                                 curr_lineno++; */

/*                                 if (yytext[i-1] == '\\') { */
/*                                         str[stringleng++] = '\n'; */
/*                                         charcount--; */
/*                                 } else { */
/*                                         cool_yylval.error_msg = "Unterminated string constant"; */
/*                                         free(str); */
/*                                         BEGIN(INITIAL); */
/*                                         yyless(i); */
/*                                         return (ERROR); */
/*                                 } */
/*                         } else { */
/*                                 str[stringleng++] = yytext[i]; */
/*                         } */
/*                 } else { */
/*                         if (yytext[i+1] != '\0' && yytext[i+1] != '\n') { */
/*                                 switch(yytext[i+1]) { */
/*                                 case 'n': str[stringleng++] = '\n'; break; */
/*                                 case 't': str[stringleng++] = '\t'; break; */
/*                                 case 'b': str[stringleng++] = '\b'; break; */
/*                                 case 'f': str[stringleng++] = '\f'; break; */
/*                                 case '\\':str[stringleng++] = '\\'; break; */
/*                                 case '"': str[stringleng++] = '"'; break; */
/*                                 default: str[stringleng++] = yytext[i+1];  break; */
/*                                 } */
/*                                 i++; */
/*                         } */

/*                 } */
/*         } */

/* /\*       printf("********** i:%d stringleng:%d char:%d lastc:%c\n", i, stringleng, charcount, yytext[i]);  *\/ */
/*         if (charcount > MAX_STR_CONST) { */
/*                 /\* if run here, it means the string is too long... *\/ */
/*                 cool_yylval.error_msg = "String constant too long"; */
/*                 free(str); */
/*                 BEGIN(INITIAL); */
/*                 return (ERROR); */
/*         } else { */
/*                 str[stringleng - 1] = '\0'; */
/*                 cool_yylval.symbol = stringtable.add_string(str); */
/*                 free(str); */
/*                 BEGIN(INITIAL); */
/*                 return (STR_CONST); */
/*         } */
/* } */
