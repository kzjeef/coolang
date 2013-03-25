

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <typeinfo>
#include "semant.h"
#include "utilities.h"


extern int semant_debug;
extern char *curr_filename;

//#define DDD

//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
static Symbol 
    arg,
    arg2,
    Bool,
    concat,
    cool_abort,
    copy,
    Int,
    in_int,
    in_string,
    IO,
    length,
    Main,
    main_meth,
    No_class,
    No_type,
    Object,
    out_int,
    out_string,
    prim_slot,
    self,
    SELF_TYPE,
    Str,
    str_field,
    substr,
    type_name,
    val;
//
// Initializing the predefined symbols.
//
static void initialize_constants(void)
{
    arg         = idtable.add_string("arg");
    arg2        = idtable.add_string("arg2");
    Bool        = idtable.add_string("Bool");
    concat      = idtable.add_string("concat");
    cool_abort  = idtable.add_string("abort");
    copy        = idtable.add_string("copy");
    Int         = idtable.add_string("Int");
    in_int      = idtable.add_string("in_int");
    in_string   = idtable.add_string("in_string");
    IO          = idtable.add_string("IO");
    length      = idtable.add_string("length");
    Main        = idtable.add_string("Main");
    main_meth   = idtable.add_string("main");
    //   _no_class is a symbol that can't be the name of any 
    //   user-defined class.
    No_class    = idtable.add_string("_no_class");
    No_type     = idtable.add_string("_no_type");
    Object      = idtable.add_string("Object");
    out_int     = idtable.add_string("out_int");
    out_string  = idtable.add_string("out_string");
    prim_slot   = idtable.add_string("_prim_slot");
    self        = idtable.add_string("self");
    SELF_TYPE   = idtable.add_string("SELF_TYPE");
    Str         = idtable.add_string("String");
    str_field   = idtable.add_string("_str_field");
    substr      = idtable.add_string("substr");
    type_name   = idtable.add_string("type_name");
    val         = idtable.add_string("_val");
}



// Clases -> Classes_class -> list_node<Class_> --> Class__class : public tree_node:tree.h


ClassTable::ClassTable(Classes classes) : semant_errors(0) , error_stream(cerr), _root(classes) {
    _globalmap = new GlobalSymbolTable();
    install_basic_classes();
    first_pass();
}

void ClassTable::install_basic_classes() {

    // The tree package uses these globals to annotate the classes built below.
   // curr_lineno  = 0;
    Symbol filename = stringtable.add_string("<basic class>");
    
    // The following demonstrates how to create dummy parse trees to
    // refer to basic Cool classes.  There's no need for method
    // bodies -- these are already built into the runtime system.
    
    // IMPORTANT: The results of the following expressions are
    // stored in local variables.  You will want to do something
    // with those variables at the end of this method to make this
    // code meaningful.

    // 
    // The Object class has no parent class. Its methods are
    //        abort() : Object    aborts the program
    //        type_name() : Str   returns a string representation of class name
    //        copy() : SELF_TYPE  returns a copy of the object
    //
    // There is no need for method bodies in the basic classes---these
    // are already built in to the runtime system.

    Class_ Object_class =
	class_(Object, 
	       No_class,
	       append_Features(
			       append_Features(
					       single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
					       single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
			       single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
	       filename);

    // 
    // The IO class inherits from Object. Its methods are
    //        out_string(Str) : SELF_TYPE       writes a string to the output
    //        out_int(Int) : SELF_TYPE            "    an int    "  "     "
    //        in_string() : Str                 reads a string from the input
    //        in_int() : Int                      "   an int     "  "     "
    //
    Class_ IO_class = 
	class_(IO, 
	       Object,
	       append_Features(
			       append_Features(
					       append_Features(
							       single_Features(method(out_string, single_Formals(formal(arg, Str)),
										      SELF_TYPE, no_expr())),
							       single_Features(method(out_int, single_Formals(formal(arg, Int)),
										      SELF_TYPE, no_expr()))),
					       single_Features(method(in_string, nil_Formals(), Str, no_expr()))),
			       single_Features(method(in_int, nil_Formals(), Int, no_expr()))),
	       filename);  

    //
    // The Int class has no methods and only a single attribute, the
    // "val" for the integer. 
    //
    Class_ Int_class =
	class_(Int, 
	       Object,
	       single_Features(attr(val, prim_slot, no_expr())),
	       filename);

    //
    // Bool also has only the "val" slot.
    //
    Class_ Bool_class =
	class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())),filename);

    //
    // The class Str has a number of slots and operations:
    //       val                                  the length of the string
    //       str_field                            the string itself
    //       length() : Int                       returns length of the string
    //       concat(arg: Str) : Str               performs string concatenation
    //       substr(arg: Int, arg2: Int): Str     substring selection
    //       
    Class_ Str_class =
	class_(Str, 
	       Object,
	       append_Features(
			       append_Features(
					       append_Features(
							       append_Features(
									       single_Features(attr(val, Int, no_expr())),
									       single_Features(attr(str_field, prim_slot, no_expr()))),
							       single_Features(method(length, nil_Formals(), Int, no_expr()))),
					       single_Features(method(concat, 
								      single_Formals(formal(arg, Str)),
								      Str, 
								      no_expr()))),
			       single_Features(method(substr, 
						      append_Formals(single_Formals(formal(arg, Int)), 
								     single_Formals(formal(arg2, Int))),
						      Str, 
						      no_expr()))),
	       filename);
}

////////////////////////////////////////////////////////////////////
//
// semant_error is an overloaded function for reporting errors
// during semantic analysis.  There are three versions:
//
//    ostream& ClassTable::semant_error()                
//
//    ostream& ClassTable::semant_error(Class_ c)
//       print line number and filename for `c'
//
//    ostream& ClassTable::semant_error(Symbol filename, tree_node *t)  
//       print a line number and filename
//
///////////////////////////////////////////////////////////////////

ostream& ClassTable::semant_error(Class_ c)
{                                                             
    return semant_error(c->get_filename(),c);
}

void ClassTable::access_attr(Class_ c,
                             attr_class *attr, ClassSymbolTable *t) {
#ifdef DDD
    cout << "attr: init: " << attr->get_init() << endl;
#endif
    // Because this is a init, we should record the attr 's name and
    // type in the type system.

    if (t->probe(attr->get_name()) != NULL) {
        semant_error(c);
    } else {
        Symbol type = attr->get_type_decl();
        t->addid(attr->get_name(), type);
    }

#ifdef DDD
    cout << "attr type: "  << attr->get_type_decl();
#endif

    if (!attr->get_init()->is_no_expr())
        attr->get_init()->set_type(attr->get_type_decl());
    
}

inline int comp_two_type(Symbol a, Symbol b)
{
    if (a == NULL || b == NULL)
        return false;

    return a->equal_string(b->get_string(), b->get_len());
}

inline int comp_two_type(Symbol a, Expression expr)
{
    if (expr->get_type() == NULL) {
        cout << "compare with no_type..., symbol:" << a << endl;
        return false;
    }
    return a->equal_string(expr->get_type()->get_string(), expr->get_type()->get_len());
}

Symbol ClassTable::access_expr(Class_ c, Expression_class *e, ClassSymbolTable *t )
{
    // Assign.
    if (typeid(*e) == typeid(assign_class)) {
        assign_class *ee = dynamic_cast<assign_class *>(e);
        if (ee == 0) {
            cout << "error" << endl;
        }

        Symbol ty = t->lookup(ee->get_name());
        if (ty == NULL) {
            // not found define.
            error_stream << "not found define:" << ee->get_name() << endl;
            semant_error(c);
            ee->set_type(Object);
            return Object;
        } else {
            // Needs access the expr first.
            access_expr(c, ee->get_expr(), t);
            if (comp_two_type(ty, ee->get_expr())) {
                ee->set_type(ee->get_expr()->get_type());
                return ee->get_expr()->get_type();
            } else {
                error_stream << "type not equal:" << ee->get_name() << endl;
                semant_error(c);
                ee->set_type(Object);
            }
            return Object;
        }
    }
    // Static dispatch

    // Dispatch
    
    // Cond
    // Loop
    // typecase
    // block
    else if (typeid(*e) == typeid(block_class)) {
        Expressions es = dynamic_cast<block_class *>(e)->get_body();
        Symbol type = NULL; 
        t->enterscope();
        for (int i = es->first(); es->more(i); i = es->next(i))
            type = access_expr(c, es->nth(i), t);
        t->exitscope();
        e->set_type(type);
        return type;
    }
    // Let

    // plus
    // sub_class
    // mul_class
    // divide class
    else if (typeid(*e) == typeid(plus_class)
             || typeid(*e) == typeid(sub_class)
             || typeid(*e) == typeid(mul_class)
             || typeid(*e) == typeid(divide_class)) {
        plus_class *ee = dynamic_cast<plus_class *>(e);
        access_expr(c, ee->get_e1(), t);
        access_expr(c, ee->get_e1(), t);
        // not same type, or not int is all error.
        if (ee->get_e1()->get_type() != ee->get_e2()->get_type()
            || !comp_two_type(Int, ee->get_e2())) {
            semant_error(c);
            e->set_type(Object);
            return Object;
        } else {
            e->set_type(ee->get_e2()->get_type());
            return ee->get_e2()->get_type();
        }
    }

    // neg class
    else if (typeid(*e) == typeid(neg_class)) {
        neg_class *ee = dynamic_cast<neg_class *>(e);
        Symbol tt = access_expr(c, ee->get_e1(), t);
        if (!comp_two_type(Int, ee->get_e1())) {
            semant_error(c);
            e->set_type(Object);
            return Object;
        } else {
            e->set_type(tt);
            return tt;
        }
    }
    // eq class
    if( typeid(*e) == typeid(eq_class)) {
        eq_class *ee = dynamic_cast<eq_class *>(e);
        Symbol t1 = access_expr(c, ee->get_e1(), t);
        Symbol t2 = access_expr(c, ee->get_e2(), t);

        if (comp_two_type(t1, t2) &&
            (comp_two_type(t1, Int)
             || comp_two_type(t1, Str)
             || comp_two_type(t1, Bool))) {
            e->set_type(Bool);
            return Bool;
        } else {
            semant_error(c);
            e->set_type(Object);
            return Object;
        }
    }

    // lt class
    else if (typeid(*e) == typeid(lt_class)) {
        lt_class *ee = dynamic_cast<lt_class *>(e);
        if (ee) {
            Symbol t1 = access_expr(c, ee->get_e1(), t);
            Symbol t2 = access_expr(c, ee->get_e2(), t);
            if (!comp_two_type(Int, t1)
                ||!comp_two_type(Int, t2)) {
                semant_error(c);
                e->set_type(Object);
                return Object;
            } else {
                e->set_type(Bool);
                return Bool;
            }
        }
    // leq class
    } else if (typeid(*e) == typeid(leq_class)) {
        leq_class *ee = dynamic_cast<leq_class *>(e);
        if (ee) {
            Symbol t1 = access_expr(c, ee->get_e1(), t);
            Symbol t2 = access_expr(c, ee->get_e2(), t);
            if (!comp_two_type(Int, t1)
                ||!comp_two_type(Int, t2)) {
                semant_error(c);
                e->set_type(Object);
                return Object;
            } else {
                e->set_type(Bool);
                return Bool;
            }
        }
    }
    
    // comp class
    else if (typeid(*e) == typeid(comp_class)) {
        comp_class *ee = dynamic_cast<comp_class *>(e);
        Symbol tt = access_expr(c, ee->get_e1(), t);
        if (!comp_two_type(Bool, tt)) {
            semant_error(c);
            e->set_type(Object);
            return Object;
        } else {
            e->set_type(Bool);
            return Bool;
        }
    }
    // int const class
    else if (typeid(*e) == typeid(int_const_class)) {
        e->set_type(Int);
        return Int;
    }
    // bool const class
    else if (typeid(*e) == typeid(bool_const_class)) {
        e->set_type(Bool);
        return Bool;
    }
    // string const class
    else if (typeid(*e) == typeid(string_const_class)) {
        e->set_type(Str);
        return Str;
    }
    // new class
    else if (typeid(*e) == typeid(new__class)) {

        Symbol tt = dynamic_cast<new__class *>(e)->get_type_name();
        if (comp_two_type(tt, SELF_TYPE)) {
            // ... TODO... how to return SELF-TYPE_c?
        }
        e->set_type(tt);
        return tt;
    }
    // isvoid class
    else if (typeid(*e) == typeid(isvoid_class)) {
        isvoid_class *ee = dynamic_cast<isvoid_class *>(e);
        access_expr(c, ee->get_e1(), t);
        e->set_type(Bool);
        return Bool;
    }
    // no expr
    // object class


    return 0;
}

void ClassTable::access_method(Class_ c, method_class *m, ClassSymbolTable *t)
{
    currMethodST = new MethodSymbolTable();
    currMethodST->enterscope();

    // process formals to init parameters.
    if (m->get_formals()->len() != 0) {
        Formals f = m->get_formals();
        for (int i = f->first(); f->more(i); i = f->next(i)) {
            formal_class *ff = dynamic_cast<formal_class *>(f->nth(i));
            currMethodST->addid(ff->get_name(), ff->get_type_decl());
        }
    }

    // Process the expr.
    Symbol tt = access_expr(c, m->get_expr(), t);

    // TODO: needs to check the result value is lum() of decleared return type.
//    if (tt != NULL)
//        m->set_return_type(tt);
//    else
//        m->set_return_type(Object);

    // Add method, set the return type to method's type.
    // FIXME: the symtab of method is seprated with var.   
//    t->addid(m->get_name(), );

    currMethodST->exitscope();
    delete currMethodST;
}


void ClassTable::access_features(Class_ c, Features fs, ClassSymbolTable *t)
{
    for (int i = fs->first(); fs->more(i); i = fs->next(i)) {
        Feature_class * f = fs->nth(i);

#ifdef DDD
        cout << typeid(*f).name() << endl;
#endif

        if (typeid(*f) == typeid(method_class)) {
            access_method(c, dynamic_cast<method_class *>(f), t);
        } else if (typeid(*f) == typeid(attr_class)) {
            access_attr(c, dynamic_cast<attr_class *>(f), t);
        }
    }
}

void ClassTable::access_class(tree_node* node)
{
    if (typeid(*node) == typeid(class__class)) {

        class__class *a = dynamic_cast<class__class *>(node);
        
        // Create a symbol table for this class.
        // If the class already exist, report an error.
        if (_globalmap->lookup(a->get_name()) != NULL) {
            semant_error(a->get_filename(), node);
        } else {
            _globalmap->addid(a->get_name(), new ClassSymbolTable());
        }

        ClassSymbolTable *t = _globalmap->probe(a->get_name());
        t->enterscope();
        access_features(a, a->get_features(), t);
        
#ifdef DDD
        cout << "name: " << a->get_name() << " parent " << a->get_parent() << "features:" << a->get_features() << endl;
#endif
    }
}

void ClassTable::access_tree_node(Classes class_, ClassTable *classtable)
{
    _globalmap->enterscope();
    for (int i = class_->first(); class_->more(i); i = class_->next(i)) {
        Class_ node = class_->nth(i);
#ifdef DDD
        cout << class_->nth(i) << endl;
#endif
        access_class(class_->nth(i));
    }
    _globalmap->exitscope();
}

void ClassTable::first_pass() {
#ifdef DDD
    cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
#endif
    access_tree_node(_root, this);
#ifdef DDD
    cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
#endif

    return;
}

ostream& ClassTable::semant_error(Symbol filename, tree_node *t)
{
    error_stream << filename << ":" << t->get_line_number() << ": ";
    return semant_error();
}

ostream& ClassTable::semant_error()                  
{                                                 
    semant_errors++;                            
    return error_stream;
} 


/*   This is the entry point to the semantic checker.

     Your checker should do the following two things:

     1) Check that the program is semantically correct
     2) Decorate the abstract syntax tree with type information
        by setting the `type' field in each Expression node.
        (see `tree.h')

     You are free to first do 1), make sure you catch all semantic
     errors. Part 2) can be done in a second stage, when you want
     to build mycoolc.
 */



void program_class::semant()
{
    initialize_constants();

    /* ClassTable constructor may do some semantic analysis */
    ClassTable *classtable = new ClassTable(classes);

    /* some semantic analysis code may go here */


    if (classtable->errors()) {
	cerr << "Compilation halted due to static semantic errors." << endl;
	exit(1);
    }
}


