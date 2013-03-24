#ifndef SEMANT_H_
#define SEMANT_H_

#include <assert.h>
#include <iostream>  
#include "cool-tree.h"
#include "stringtab.h"
#include "symtab.h"
#include "list.h"

#define TRUE 1
#define FALSE 0

class ClassTable;
typedef ClassTable *ClassTableP;

// This is a structure that may be used to contain the semantic
// information such as the inheritance graph.  You may use it or not as
// you like: it is only here to provide a container for the supplied
// methods.

class ClassTable {
private:
  int semant_errors;
  void install_basic_classes();
  ostream& error_stream;
  Classes _root;

  typedef Symbol Type;

  /* Symbol table for current method. */
  typedef SymbolTable<Symbol, Entry> MethodSymbolTable;
  MethodSymbolTable *currMethodST;
  
  /* Symbol table for each of classes. */
  typedef SymbolTable<Symbol, Entry> ClassSymbolTable;

  /* Symbol table for the global classes. */
  typedef SymbolTable<Symbol, ClassSymbolTable> GlobalSymbolTable;

  GlobalSymbolTable *_globalmap;

public:
  ClassTable(Classes);
  int errors() { return semant_errors; }

  void access_expr(Class_ c, Expression_class *e, ClassSymbolTable *t );
  void access_method(Class_ c, method_class *m, ClassSymbolTable *t);
  void access_attr(Class_ c, attr_class *attr, ClassSymbolTable *t);
  void access_features(Class_ c, Features fs, ClassSymbolTable *t);
  void access_tree_node(Classes class_, ClassTable *classtable);
  void access_class(tree_node *);
  void first_pass();
  
  
  ostream& semant_error();
  ostream& semant_error(Class_ c);
  ostream& semant_error(Symbol filename, tree_node *t);
};


#endif

