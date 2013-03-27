//
// The following include files must come first.

#ifndef COOL_TREE_HANDCODE_H
#define COOL_TREE_HANDCODE_H

#include <iostream>
#include "tree.h"
#include "cool.h"
#include "stringtab.h"
#define yylineno curr_lineno;
extern int yylineno;

inline Boolean copy_Boolean(Boolean b) {return b; }
inline void assert_Boolean(Boolean) {}
inline void dump_Boolean(ostream& stream, int padding, Boolean b)
	{ stream << pad(padding) << (int) b << "\n"; }

void dump_Symbol(ostream& stream, int padding, Symbol b);
void assert_Symbol(Symbol b);
Symbol copy_Symbol(Symbol b);

class Program_class;
typedef Program_class *Program;
class Class__class;
typedef Class__class *Class_;
class Feature_class;
typedef Feature_class *Feature;
class Formal_class;
typedef Formal_class *Formal;
class Expression_class;
typedef Expression_class *Expression;
class Case_class;
typedef Case_class *Case;

typedef list_node<Class_> Classes_class;
typedef Classes_class *Classes;
typedef list_node<Feature> Features_class;
typedef Features_class *Features;
typedef list_node<Formal> Formals_class;
typedef Formals_class *Formals;
typedef list_node<Expression> Expressions_class;
typedef Expressions_class *Expressions;
typedef list_node<Case> Cases_class;
typedef Cases_class *Cases;

#define Program_EXTRAS                          \
virtual void semant() = 0;			\
virtual void dump_with_types(ostream&, int) = 0; 

#define public_accessor(type, mem) type get_##mem() { return mem; }

#define program_EXTRAS                          \
void semant();     				\
void dump_with_types(ostream&, int);            

#define Class__EXTRAS                   \
virtual Symbol get_filename() = 0;      \
virtual void dump_with_types(ostream&,int) = 0; 


#define class__EXTRAS                                 \
Symbol get_filename() { return filename; }             \
void dump_with_types(ostream&,int);                    

#define Class__SHARED_EXTRAS  \
        public_accessor(Symbol, name)  \
        public_accessor(Symbol, parent) \
        public_accessor(Features, features)

#define Feature_EXTRAS                                        \
virtual void dump_with_types(ostream&,int) = 0; 



#define Feature_SHARED_EXTRAS                                       \
        public_accessor(Symbol, name)                               \
        void dump_with_types(ostream&,int);

#define attr_EXTRAS                             \
        public_accessor(Expression, init)       \
        public_accessor(Symbol, type_decl)

#define method_EXTRAS                           \
        public_accessor(Formals, formals)       \
        public_accessor(Symbol,  return_type)   \
        public_accessor(Expression, expr)       \
        void set_return_type(Symbol t) { return_type = copy_Symbol(t); }

#define Formal_EXTRAS                              \
virtual void dump_with_types(ostream&,int) = 0;

#define Formal_SHARED_EXTRAS                    \
        public_accessor(Symbol, name)           \
        public_accessor(Symbol, type_decl)

#define no_expr_EXTRAS                          \
        virtual bool is_no_expr() { return true; }

#define formal_EXTRAS                           \
void dump_with_types(ostream&,int);

#define assign_EXTRAS                           \
        public_accessor(Symbol,     name)       \
        public_accessor(Expression, expr)

#define plus_EXTRAS                             \
        public_accessor(Expression, e1)         \
        public_accessor(Expression, e2)

#define sub_EXTRAS                             \
        public_accessor(Expression, e1)         \
        public_accessor(Expression, e2)

#define mul_EXTRAS                             \
        public_accessor(Expression, e1)         \
        public_accessor(Expression, e2)

#define divide_EXTRAS                             \
        public_accessor(Expression, e1)         \
        public_accessor(Expression, e2)

#define neg_EXTRAS                              \
        public_accessor(Expression, e1)
        
#define new__EXTRAS                             \
        public_accessor(Symbol, type_name)

#define dispatch_EXTRAS                         \
        public_accessor(Expression, expr)       \
        public_accessor(Symbol, name)           \
        public_accessor(Expressions, actual)

#define static_dispatch_EXTRAS                  \
        public_accessor(Expression, expr)       \
        public_accessor(Symbol, type_name)      \
        public_accessor(Symbol, name)           \
        public_accessor(Expressions, actual)


#define branch_EXTRAS                           \
        public_accessor(Symbol, name)           \
        public_accessor(Symbol, type_decl)      \
        public_accessor(Expression, expr)       \
        void dump_with_types(ostream& ,int);


#define cond_EXTRAS                             \
        public_accessor(Expression, pred)       \
        public_accessor(Expression, then_exp)   \
        public_accessor(Expression, else_exp)

#define loop_EXTRAS                             \
        public_accessor(Expression, pred)       \
        public_accessor(Expression, body)

#define typcase_EXTRAS                          \
        public_accessor(Expression, expr)       \
        public_accessor(Cases, cases)

#define let_EXTRAS                              \
        public_accessor(Symbol, identifier)     \
        public_accessor(Symbol, type_decl)      \
        public_accessor(Expression, init)       \
        public_accessor(Expression, body)

        
#define lt_EXTRAS                               \
        public_accessor(Expression, e1)         \
        public_accessor(Expression, e2)

#define eq_EXTRAS                               \
        public_accessor(Expression, e1)         \
                public_accessor(Expression, e2)

#define leq_EXTRAS                              \
        public_accessor(Expression, e1)         \
        public_accessor(Expression, e2)

#define comp_EXTRAS                             \
        public_accessor(Expression, e1)

#define bool_const_EXTRAS                       \
        public_accessor(Boolean, val)

#define isvoid_EXTRAS                           \
        public_accessor(Expression, e1)



#define Case_EXTRAS                             \
virtual void dump_with_types(ostream& ,int) = 0;

#define block_EXTRAS                            \
        public_accessor(Expressions,body)

#define object_EXTRAS                           \
        public_accessor(Symbol, name)


#define Expression_EXTRAS                    \
Symbol type;                                 \
Symbol get_type() { return type; }           \
Expression set_type(Symbol s) { type = s; return this; } \
virtual void dump_with_types(ostream&,int) = 0;  \
virtual bool is_no_expr() { return false; }      \
void dump_type(ostream&, int);               \
Expression_class() { type = (Symbol) NULL; }

#define Expression_SHARED_EXTRAS           \
void dump_with_types(ostream&,int); 

#endif
