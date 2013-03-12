/*
*  cool.y
*              Parser definition for the COOL language.
*
*/
%{
  #include <iostream>
  #include "cool-tree.h"
  #include "stringtab.h"
  #include "utilities.h"
  
  extern char *curr_filename;
  
  /* Locations */
  #define YYLTYPE int              /* the type of locations */
  #define cool_yylloc curr_lineno  /* use the curr_lineno from the lexer
 /* for the location of tokens *\/ */
    
    extern int node_lineno;          /* set before constructing a tree node
    to whatever you want the line number
    for the tree node to be */
      
      
      #define YYLLOC_DEFAULT(Current, Rhs, N)         \
      Current = Rhs[1];                             \
      node_lineno = Current;
    
    
    #define SET_NODELOC(Current)  \
    node_lineno = Current;
    
    /* IMPORTANT NOTE ON LINE NUMBERS
    *********************************
    * The above definitions and macros cause every terminal in your grammar to 
    * have the line number supplied by the lexer. The only task you have to
    * implement for line numbers to work correctly, is to use SET_NODELOC()
    * before constructing any constructs from non-terminals in your grammar.
    * Example: Consider you are matching on the following very restrictive 
    * (fictional) construct that matches a plus between two integer constants. 
    * (SUCH A RULE SHOULD NOT BE  PART OF YOUR PARSER):
    
    plus_consts	: INT_CONST '+' INT_CONST 
    
    * where INT_CONST is a terminal for an integer constant. Now, a correct
    * action for this rule that attaches the correct line number to plus_const
    * would look like the following:
    
    plus_consts	: INT_CONST '+' INT_CONST 
    {
      // Set the line number of the current non-terminal:
      // ***********************************************
      // You can access the line numbers of the i'th item with @i, just
      // like you acess the value of the i'th exporession with $i.
      //
      // Here, we choose the line number of the last INT_CONST (@3) as the
      // line number of the resulting expression (@$). You are free to pick
      // any reasonable line as the line number of non-terminals. If you 
      // omit the statement @$=..., bison has default rules for deciding which 
      // line number to use. Check the manual for details if you are interested.
      @$ = @3;
      
      
      // Observe that we call SET_NODELOC(@3); this will set the global variable
      // node_lineno to @3. Since the constructor call "plus" uses the value of 
      // this global, the plus node will now have the correct line number.
      SET_NODELOC(@3);
      
      // construct the result node:
      $$ = plus(int_const($1), int_const($3));
    }
    
    */
    
    
    
    void yyerror(char *s);        /*  defined below; called for each parse error */
    extern int yylex();           /*  the entry point to the lexer  */
    
    /************************************************************************/
    /*                DONT CHANGE ANYTHING IN THIS SECTION                  */
    
    Program ast_root;	      /* the result of the parse  */
    Classes parse_results;        /* for use in semantic analysis */
    int omerrs = 0;               /* number of errors in lexing and parsing */
    %}
    
    /* A union of all the types that can be the result of parsing actions. */
    %union {
      Boolean boolean;
      Symbol symbol;
      Program program;
      Class_ class_;
      Classes classes;
      Feature feature;
      Features features;
      Formal formal;
      Formals formals;
      Case case_;
      Cases cases;
      Expression expression;
      Expressions expressions;
      char *error_msg;
    }
    
    /* 
    Declare the terminals; a few have types for associated lexemes.
    The token ERROR is never used in the parser; thus, it is a parse
    error when the lexer returns it.
    
    The integer following token declaration is the numeric constant used
    to represent that token internally.  Typically, Bison generates these
    on its own, but we give explicit numbers to prevent version parity
    problems (bison 1.25 and earlier start at 258, later versions -- at
    257)
    */

    %token CLASS 258 ELSE 259 FI 260 IF 261 IN 262 
    %token INHERITS 263 LET 264 LOOP 265 POOL 266 THEN 267 WHILE 268
    %token CASE 269 ESAC 270 OF 271 DARROW 272 NEW 273 ISVOID 274
    %token <symbol>  STR_CONST 275 INT_CONST 276 
    %token <boolean> BOOL_CONST 277
    %token <symbol>  TYPEID 278 OBJECTID 279 
    %token ASSIGN 280 NOT 281 LE 282 ERROR 283
    
    /*  DON'T CHANGE ANYTHING ABOVE THIS LINE, OR YOUR PARSER WONT WORK       */
    /**************************************************************************/
    
    /* Complete the nonterminal list below, giving a type for the semantic
    value of each non terminal. (See section 3.6 in the bison 
    documentation for details). */

    %left '+' '-'
    %left '*' '/'
    
    /* Declare types for the grammar's non-terminals. */
    %type <program> program
    %type <classes> class_list
    %type <class_> class
    %type <case_>  case_
    %type <cases>  case_list
    %type <feature> feature
    %type <features> feature_list
    %type <features> optional_feature_list
    %type <formal>   formal
    %type <formals>  formal_list
    %type <formals>  opt_formal_list
    %type <expression> expr
    %type <expression> opt_let_init_list let_init

    %type <expressions>  option_expr_list_semicdon opt_expr_list

    
    /* You will want to change the following line. */
    
    /* Precedence declarations go here. */
    
    
    %%
    /* 
    Save the root of the abstract syntax tree in a global variable.
    */
    program	: class_list	{ @$ = @1;         SET_NODELOC(@1); ast_root = program($1); }
    ;
    
    class_list
    : class			/* single class */
    {
            @$ = @1;
            SET_NODELOC(@1);
            $$ = single_Classes($1);
            parse_results = $$;
    }
    | class_list class	/* several classes */
    { @$ = @2;
        SET_NODELOC(@2);
        $$ = append_Classes($1,single_Classes($2)); 
        parse_results = $$;
    }
    | error
    ;
    
    /* If no parent is specified, the class inherits from the Object class. */
    class	: CLASS TYPEID '{' optional_feature_list '}' ';'
    {
        @$ = @4;
        SET_NODELOC(@4);
        $$ = class_($2,idtable.add_string("Object"),$4,
                stringtable.add_string(curr_filename));
    }
    | CLASS TYPEID INHERITS TYPEID '{' optional_feature_list '}' ';'
    {
            @$ = @6;
            SET_NODELOC(@6);
            $$ = class_($2,$4,$6,stringtable.add_string(curr_filename)); }
;

    /* Feature list may be empty, but no empty features in list. */
    optional_feature_list:		/* empty */
    { $$ = nil_Features(); }
    | feature_list
    { @$ = @1; SET_NODELOC(@1); $$ = $1; }
    ;

    feature_list: feature
    {  @$ = @1; SET_NODELOC(@1); $$ = single_Features($1); }
    | feature_list feature
    {  @$ = @2; SET_NODELOC(@2); $$ = append_Features($1, single_Features($2)); }
    | error feature
    {       @$ = @2; SET_NODELOC(@2); $$ = single_Features($2);
    }
    ;

    feature:  OBJECTID '(' opt_formal_list ')' ':' TYPEID '{' expr '}' ';'
    {   @$ = @8; SET_NODELOC(@8); $$ = method($1, $3, $6, $8);  }
    | OBJECTID ':' TYPEID ASSIGN expr ';'
    {   @$ = @5; SET_NODELOC(@5);  $$ = attr($1, $3, $5);  }
    | OBJECTID ':' TYPEID ';'
    {   @$ = @3; SET_NODELOC(@3); $$ =  attr($1, $3, no_expr()); }
    |  feature:  OBJECTID '(' opt_formal_list ')' ':' TYPEID '{' error '}' ';'
    ;

    formal: OBJECTID ':' TYPEID 
    { @$ = @3; SET_NODELOC(@3); $$ = formal($1, $3); }
    ;
    opt_formal_list:                /* empty */
    { $$ = nil_Formals();  }
    | formal_list
    ;

    formal_list: formal
    {  @$ = @1; SET_NODELOC(@1); $$ = single_Formals($1); }
    | formal ',' formal_list 
    {  @$ = @3; SET_NODELOC(@3); $$ = append_Formals(single_Formals($1), $3); }
    ;

    expr :
    OBJECTID ASSIGN expr
    {  @$ = @3; SET_NODELOC(@3); $$ = assign ($1, $3); }
    | expr '@' TYPEID '.' OBJECTID '(' expr opt_expr_list ')'
    { @$ = @8; SET_NODELOC(@8);
            $$ = static_dispatch($1, $3, $5,
                                 append_Expressions(single_Expressions($7), $8)); }
    | expr '.' OBJECTID '(' expr opt_expr_list ')'
    { @$ = @6; SET_NODELOC(@6); $$ = dispatch($1, $3,
                                              append_Expressions(single_Expressions($5), $6));  }
    | expr '.' OBJECTID '(' ')'
    { @$ = @5; SET_NODELOC(@5); $$ = dispatch($1, $3, nil_Expressions());  }
    | OBJECTID '(' expr opt_expr_list ')'
    { @$ = @4; SET_NODELOC(@4); $$ = dispatch(no_expr(), $1, append_Expressions(single_Expressions($3), $4)); }
    | OBJECTID '('  ')'
    { @$ = @1; SET_NODELOC(@1); $$ = dispatch(no_expr(), $1, nil_Expressions()); }
    | IF expr THEN expr ELSE expr FI
    { @$ = @7; SET_NODELOC(@7); $$ = cond($2, $4, $6);  }
    | WHILE expr LOOP expr POOL
    { @$ = @5; SET_NODELOC(@5); $$ = loop($2, $4); }
    | '{' option_expr_list_semicdon '}'
    {  @$ = @3; SET_NODELOC(@3); $$ = block($2);  }
    | '{' error '}'
/* TODO.... */
    | LET OBJECTID ':' TYPEID opt_let_init_list IN expr
    { @$ = @6; SET_NODELOC(@6);$$ = let($2, $4, $5, $7); }
    | LET OBJECTID ':' TYPEID ASSIGN  expr opt_let_init_list IN expr
    { @$ = @6; SET_NODELOC(@6);$$ = let($2, $4, $6, $9); }
    | LET OBJECTID ':' TYPEID IN expr
    { @$ = @6; SET_NODELOC(@6);$$ = let($2, $4, no_expr(), $6); }
    | CASE expr OF case_list ESAC
    {@$ = @5; SET_NODELOC(@5); $$  =  typcase($2, $4); }
    | NEW TYPEID
    {  @$ = @2; SET_NODELOC(@2); $$ = new_($2); }
    | expr '+' expr
    {  @$ = @3; SET_NODELOC(@3); $$ = plus($1, $3); }
    | expr '-' expr
    {  @$ = @3; SET_NODELOC(@3); $$ = sub($1, $3); }
    | expr '*' expr
    {  @$ = @3; SET_NODELOC(@3); $$ = mul($1, $3); }
    | expr '/' expr
    {  @$ = @3; SET_NODELOC(@3); $$ = divide($1, $3); }
    | '~' expr
    {  @$ = @2; SET_NODELOC(@2);$$ = neg($2); }
    | expr '<' expr
    {  @$ = @3; SET_NODELOC(@3); $$ = lt($1, $3); }
    | expr LE expr
    {  @$ = @3; SET_NODELOC(@3); $$ = leq($1, $3); }
    | expr '=' expr
    {  @$ = @3; SET_NODELOC(@3); $$ = eq($1, $3); }
    | NOT expr
    {  @$ = @2; SET_NODELOC(@2); $$ = comp($2); }           /*TODO, this maybe wrong...*/
    | '(' expr ')'
    {   @$ = @3;  SET_NODELOC(@3); $$ = $2; }
    | ISVOID expr
    {  @$ = @2; SET_NODELOC(@2); $$ = isvoid($2); }
    | OBJECTID
    {  @$ = @1; SET_NODELOC(@1); $$ = object($1); }
    | INT_CONST
    {  @$ = @1; SET_NODELOC(@1); $$ = int_const($1);}
     | STR_CONST
    {  @$ = @1; SET_NODELOC(@1); $$ = string_const($1); }
    | BOOL_CONST
    { @$ = @1; SET_NODELOC(@1); $$ = bool_const($1); }
    ;

option_expr_list_semicdon:              /* empty */
   {  $$ = nil_Expressions(); }
   | expr ';' option_expr_list_semicdon
   { @$ = @3;
     SET_NODELOC(@3);
     $$ = append_Expressions(single_Expressions($1), $3);
   }
   | expr
   { @$ = @1; SET_NODELOC(@1); $$ = single_Expressions($1); }
   | error ';' option_expr_list_semicdon
   ;

opt_expr_list:                      /*empty*/
   { $$ = nil_Expressions(); }
   |  opt_expr_list ',' expr
   { @$ = @3; SET_NODELOC(@3);
           $$ = append_Expressions($1, single_Expressions($3)); }
   | expr
   { @$ = @1; SET_NODELOC(@1);
           $$ = single_Expressions($1);
   }
   ;
        
   case_:  OBJECTID ':' TYPEID DARROW expr ';' 
   { @$ = @5; SET_NODELOC(@5); $$ = branch($1, $3, $5); }
   ;
   case_list : case_
   { @$ = @1; SET_NODELOC(@1); $$ = single_Cases($1); }
   | case_list case_
   { @$ = @2; SET_NODELOC(@2); $$ = append_Cases($1, single_Cases($2)); }
   | error
   ;


let_init:  OBJECTID ':' TYPEID
| OBJECTID ':' TYPEID ASSIGN expr
;

opt_let_init_list:                          /* empty */
    {$$ = no_expr(); }
    | opt_let_init_list ',' let_init
    { @$ = @2; SET_NODELOC(@2); $$ = no_expr(); }
     | error ',' opt_let_init_list
     { @$ = @3; SET_NODELOC(@3); $$ = no_expr(); }
    | let_init
    { @$ = @1; SET_NODELOC(@1); $$ = no_expr(); }
;

   /* opt_case_list :              /\*empty*\/ */
   /* { $$ = nil_Cases(); } */
   /* | opt_case_list case_list */
   /* { @$ = @2; $$ = append_Cases($1, $2); } */
   ;


// branch
// assign
// static dispatrch
// dispatch
// cond
// loop
// typecase

    
    /* end of grammar */
    %%
    
    /* This function is called automatically when Bison detects a parse error. */
    void yyerror(char *s)
    {
      extern int curr_lineno;
      
      cerr << "\"" << curr_filename << "\", line " << curr_lineno << ": " \
      << s << " at or near ";
      print_cool_token(yychar);
      cerr << endl;
      omerrs++;
      
      if(omerrs>50) {fprintf(stdout, "More than 50 errors\n"); exit(1);}
    }
    
    
