

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

    int ret;
    classTreeRoot = new TreeNode(Object);
    ret = classTreeRoot->addchild(Str, Object);
    ret |= classTreeRoot->addchild(Bool, Object);
    ret |= classTreeRoot->addchild(Int, Object);
    ret |= classTreeRoot->addchild(IO, Object);

//#define TREENODE_SELF_TEST
#ifdef TREENODE_SELF_TEST
    cout << "add result: " << ret << endl;
    cout << "Str is Object subclass: " << classTreeRoot->isSubClass(Str, Object) << " expecting : True" << endl;
    cout << "IO is Object subclass: " << classTreeRoot->isSubClass(IO, Object) << " expecting : True" << endl;
    cout << "Object is IO subclass:" << classTreeRoot->isSubClass(Object, IO) << " expecting : False" << endl;
    cout << "Str is IO subclass:" << classTreeRoot->isSubClass(Str, IO) << " expecting: False" << endl;
#endif
    
    
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
    abort();
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

inline Symbol ClassTable::self_type_c(Class_ c) {
    class__class *cc = dynamic_cast<class__class *>(c);
    return cc->get_name();
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
    else if (typeid(*e) == typeid(static_dispatch_class)) {
        static_dispatch_class *ee = dynamic_cast<static_dispatch_class *>(e);
        Expressions es = ee->get_actual();

        Symbol t0 = access_expr(c, ee->get_expr(), t);

        if (comp_two_type(t0, ee->get_type_name()) || !classTreeRoot->isSubClass(t0, ee->get_type_name())) {
            cout << "#static dispatch class not good.";
            semant_error(c);
            e->set_type(Object);
            return Object;
        }

        // FIXME: here ignore some, like M(T0', f) = (T1', ..., Tn', Tn+1')
        Symbol tn;
        for (int i = es->first(); es->more(i); i = es->next(i)) {
            tn = access_expr(c, es->nth(i), t);
        }

        if (comp_two_type(tn, SELF_TYPE))
            tn = t0;

        e->set_type(tn);
        return tn;
    }
    // Dispatch
    else if (typeid(*e) == typeid(dispatch_class) ) {
        dispatch_class *ee = dynamic_cast<dispatch_class *>(e);
        Expressions es = ee->get_actual();

        Symbol t0 = access_expr(c, ee->get_expr(), t); 

        // FIXME: here ignore some, like M(T0', f) = (T1', ..., Tn', Tn+1')
        Symbol tn;
        for (int i = es->first(); es->more(i); i = es->next(i)) {
            tn = access_expr(c, es->nth(i), t);
        }

        if (comp_two_type(tn, SELF_TYPE))
            tn = t0;

        e->set_type(tn);
        return tn;
    }
    
    // Cond
    else if (typeid(*e) == typeid(cond_class)) {
        cond_class *ee = dynamic_cast<cond_class *>(e);
        Symbol tt = access_expr(c, ee->get_pred(), t);
        if (!comp_two_type(tt, Bool)) {
            semant_error(c);
            ee->set_type(Object);
        }
        Symbol t1 = access_expr(c, ee->get_then_exp(), t);
        Symbol t2 = access_expr(c, ee->get_else_exp(), t);
        Symbol tr = classTreeRoot->lct(t1, t2);
        ee->set_type(tr);
        return tr;
    }
    // Loop
    else if (typeid(*e) == typeid(loop_class)) {
        loop_class *ee = dynamic_cast<loop_class *>(e);
        Expression pred = ee->get_pred();
        Expression body = ee->get_body();
        Symbol type = access_expr(c, pred, t);
        if (!comp_two_type(type, Bool)) {
            semant_error(c);
            ee->set_type(Object);
        }
        access_expr(c, body, t);
        ee->set_type(Object);
        return Object;
    }
    // typcase
    else if (typeid(*e) == typeid(typcase_class)) {
        typcase_class *ee = dynamic_cast<typcase_class *>(e);
        Symbol t1 = access_expr(c, ee->get_expr(), t);
        Cases cs = ee->get_cases();
        for (int i = cs->first(); cs->more(i); i = cs->next(i)) {
            typcase_class *eee = dynamic_cast<typcase_class *>(cs->nth(i));
            t1 = classTreeRoot->lct(t1,
                                    access_expr(c, eee->get_expr(), t));
        }

        ee->set_type(t1);
        return t1;
    }
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
    else if (typeid(*e) == typeid(let_class)) {
        let_class *ee = dynamic_cast<let_class *>(e);
        // SELF
        Symbol t0 = ee->get_type_decl();
        if (comp_two_type(t0, SELF_TYPE)) {
            // FIXME: I think it's implement is not right about SELF_TYPE_c
            t0 = self_type_c(c);
        }
        Symbol t1 = access_expr(c, ee->get_init(), t);

        t->enterscope();
        t->addid(ee->get_identifier(), t0);
        
        if (t1) {
            // t1 must be a sub class or equal
            /*
class Main{main():Int{0};};

class A {
x:Int;

f(x:A):String {"ab"};
g(x:Bool):A {self};
h():Object{let x:String<-f(let x:Bool in g(x)) in x<-"ab"};
};----------------
              
             */
            // t1 : bool t0: stirng.
            // cout << "t1: " << t1 << "t0 " << t0 << endl;
            // if (!comp_two_type(t0, t1) && !classTreeRoot->isSubClass(t1, t0)) {
            //     semant_error(c);
            //     ee->set_type(Object);
            //     return Object;
            // }

        }

        Symbol rt = access_expr(c, ee->get_body(), t);

        t->exitscope();
        return rt;
    }
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
    else if (typeid(*e) == typeid(object_class)) {
        object_class *ee = dynamic_cast<object_class *>(e);

        if (comp_two_type(ee->get_name(), self)) {
            e->set_type(SELF_TYPE);
            return SELF_TYPE;
        } else {
            Symbol type = t->lookup(ee->get_name());
            if (type == NULL) {
                cout << "not find declear of : " << ee->get_name() << endl;
                e->set_type(Object);
                semant_error(c);
                return Object;
            }

            e->set_type(type);
            return type;
        }
    }


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
    if (tt != NULL) {

        if (comp_two_type(tt, SELF_TYPE)) {
            m->set_return_type(dynamic_cast<class__class *>(c)->get_name());
        } else if (!comp_two_type(tt, m->get_return_type())
            && !classTreeRoot->isSubClass(tt, m->get_return_type())) {

            cout << "two type not equal or less: a: " << tt
                 << "return type: " << m->get_return_type() << endl;
                semant_error(c);
                m->set_return_type(Object);
        } else 
            m->set_return_type(m->get_return_type());
    } else
        m->set_return_type(Object);

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

    typedef vector<Class__class *> vc;
    vc failed_first;

    for (int i = class_->first(); class_->more(i); i = class_->next(i)) {
        Class_ node = class_->nth(i);
        class__class *cc = dynamic_cast<class__class *>(node);
        Symbol p = cc->get_parent() == NULL ? Object : cc->get_parent();
        if (!classTreeRoot->addchild(cc->get_name(), p)) {
#ifdef DDD
            cout << "class : " << cc->get_name() << " with parent: " << p
                 << " failed to inherient" << endl;
#endif
            failed_first.push_back(cc);
        }
    }

    // Give another chance to declear... but still not fix the issue, because it better do topologicsort...
    for (vc::iterator i = failed_first.begin(); i != failed_first.end(); i++) {
        class__class *cc = dynamic_cast<class__class *>(*i);
        Symbol p = cc->get_parent() == NULL ? Object : cc->get_parent();
        if (!classTreeRoot->addchild(cc->get_name(), p)) {

#ifdef DDD
            cout << "2th: class : " << cc->get_name() << " with parent: " << p
                 << " failed to inherient" << endl;
#endif
            semant_error(*i);
        }
    }
    
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


