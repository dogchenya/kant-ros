%{
#include <iostream>
#include <memory>
#include <cassert>

using namespace std;

#define YYSTYPE GrammarBasePtr

#include "parse.h"
#define YYDEBUG 1
#define YYINITDEPTH 10000
%}

%defines
%debug

//keyword token
%token KANT_VOID
%token KANT_STRUCT
%token KANT_BOOL
%token KANT_BYTE
%token KANT_SHORT
%token KANT_INT
%token KANT_DOUBLE
%token KANT_FLOAT
%token KANT_LONG
%token KANT_STRING
%token KANT_VECTOR
%token KANT_MAP
%token KANT_NAMESPACE
%token KANT_INTERFACE
%token KANT_IDENTIFIER
%token KANT_OUT
%token KANT_OP
%token KANT_KEY
%token KANT_ROUTE_KEY
%token KANT_REQUIRE
%token KANT_OPTIONAL
%token KANT_CONST_INTEGER
%token KANT_CONST_FLOAT
%token KANT_FALSE
%token KANT_TRUE
%token KANT_STRING_LITERAL
%token KANT_SCOPE_DELIMITER
%token KANT_CONST
%token KANT_ENUM
%token KANT_UNSIGNED
%token BAD_CHAR

%%
start: definitions
;

// ----------------------------------------------------------------------
definitions
// ----------------------------------------------------------------------
: definition
{
}
';' definitions
| error ';'
{
    yyerrok;
}
definitions
| definition
{
    g_parse->error("`;' missing after definition");
}
|
{
}
;

// ----------------------------------------------------------------------
definition
// ----------------------------------------------------------------------
: namespace_def
{
    assert($1 == 0 || std::dynamic_pointer_cast<Namespace>($1));
}
| interface_def
{
    assert($1 == 0 || std::dynamic_pointer_cast<Interface>($1));
}
| struct_def
{
    assert($1 == 0 || std::dynamic_pointer_cast<Struct>($1));
}
| key_def
{
}
| enum_def
{
    assert($1 == 0 || std::dynamic_pointer_cast<Enum>($1));
}
| const_def
{
    assert($1 == 0 || std::dynamic_pointer_cast<Const>($1));
}
;

// ----------------------------------------------------------------------
enum_def
// ----------------------------------------------------------------------
: enum_id
{
    $$ = $1;
}
'{' enumerator_list '}'
{
    if($3)
    {
        g_parse->popContainer();
        $$ = $3;
    }
    else
    {
        $$ = 0;
    }

    $$ = $2;
}
;

// ----------------------------------------------------------------------
enum_id
// ----------------------------------------------------------------------
: KANT_ENUM KANT_IDENTIFIER
{
    NamespacePtr c = std::dynamic_pointer_cast<Namespace>(g_parse->currentContainer());
    if(!c)
    {
        g_parse->error("enum must define in namespace");
    }
    StringGrammarPtr ident  = std::dynamic_pointer_cast<StringGrammar>($2);
    EnumPtr e = c->createEnum(ident->v);
    g_parse->pushContainer(e);

    $$ = e;
}
| KANT_ENUM keyword
{
    StringGrammarPtr ident = std::dynamic_pointer_cast<StringGrammar>($2);
    g_parse->error("keyword `" + ident->v + "' cannot be used as enumeration name");
    $$ = $2;
}
;

// ----------------------------------------------------------------------
enumerator_list
// ----------------------------------------------------------------------
: enumerator ',' enumerator_list
{
    $$ = $2;
}
| enumerator
{
}
;

// ----------------------------------------------------------------------
enumerator
// ----------------------------------------------------------------------
: KANT_IDENTIFIER
{
    TypePtr type        = std::dynamic_pointer_cast<Type>(g_parse->createBuiltin(Builtin::KindLong));
    StringGrammarPtr ident  = std::dynamic_pointer_cast<StringGrammar>($1);
    TypeIdPtr tPtr      = std::make_shared<TypeId>(type, ident->v);
    tPtr->disableDefault();
    EnumPtr e = std::dynamic_pointer_cast<Enum>(g_parse->currentContainer());
    assert(e);
    e->addMember(tPtr);
    $$ = e;
}
| keyword
{
    StringGrammarPtr ident = std::dynamic_pointer_cast<StringGrammar>($1);
    g_parse->error("keyword `" + ident->v + "' cannot be used as enumerator");
}
| KANT_IDENTIFIER  '=' const_initializer 
{
    TypePtr type        = std::dynamic_pointer_cast<Type>(g_parse->createBuiltin(Builtin::KindLong));
    StringGrammarPtr ident  = std::dynamic_pointer_cast<StringGrammar>($1);
    TypeIdPtr tPtr      = std::make_shared<TypeId>(type, ident->v);
    ConstGrammarPtr sPtr    = std::dynamic_pointer_cast<ConstGrammar>($3);
    g_parse->checkConstValue(tPtr, sPtr->t);
    tPtr->setDefault(sPtr->v);
    EnumPtr e = std::dynamic_pointer_cast<Enum>(g_parse->currentContainer());
    assert(e);
    e->addMember(tPtr);
    $$ = e;
}
| 
{
}
;

// ----------------------------------------------------------------------
namespace_def
// ----------------------------------------------------------------------
: KANT_NAMESPACE KANT_IDENTIFIER
{
    StringGrammarPtr ident  = std::dynamic_pointer_cast<StringGrammar>($2);
    ContainerPtr c      = g_parse->currentContainer();
    NamespacePtr n      = c->createNamespace(ident->v);
    if(n)
    {
        g_parse->pushContainer(n);
        $$ = std::dynamic_pointer_cast<GrammarBase>(n);
    }
    else
    {
        $$ = 0;
    }
}
'{' definitions '}'
{
    if($3)
    {
        g_parse->popContainer();
        $$ = $3;
    }
    else
    {
        $$ = 0;
    }
}
;


//key------------------------------------------------------
key_def
// ----------------------------------------------------------------------
: KANT_KEY '[' scoped_name ','
{
    StringGrammarPtr ident = std::dynamic_pointer_cast<StringGrammar>($3);
    StructPtr sp = std::dynamic_pointer_cast<Struct>(g_parse->findUserType(ident->v));
    if(!sp)
    {
        g_parse->error("struct '" + ident->v + "' undefined!");
    }

    g_parse->setKeyStruct(sp);
}
key_members ']'
{
}
;

//key------------------------------------------------------
key_members
// ----------------------------------------------------------------------
: KANT_IDENTIFIER
{
    StringGrammarPtr ident = std::dynamic_pointer_cast<StringGrammar>($1);
    StructPtr np = g_parse->getKeyStruct();
    if(np)
    {
        np->addKey(ident->v);
    }
    else
    {
        $$ = 0;
    }
}
| key_members ',' KANT_IDENTIFIER
{
    StringGrammarPtr ident = std::dynamic_pointer_cast<StringGrammar>($3);
    StructPtr np = g_parse->getKeyStruct();
    if(np)
    {
        np->addKey(ident->v);
    }
    else
    {
        $$ = 0;
    }   
}
;


// ----------------------------------------------------------------------
interface_def
// ----------------------------------------------------------------------
: interface_id
{
    StringGrammarPtr ident = std::dynamic_pointer_cast<StringGrammar>($1);

    NamespacePtr c = std::dynamic_pointer_cast<Namespace>(g_parse->currentContainer());

    InterfacePtr cl = c->createInterface(ident->v);
    if(cl)
    {
        g_parse->pushContainer(cl);
        $$ = std::dynamic_pointer_cast<GrammarBase>(cl);
    }
    else
    {
        $$ = 0;
    }
}
'{' interface_exports '}'
{
    if($2)
    {
       g_parse->popContainer();
       $$ = std::dynamic_pointer_cast<GrammarBase>($2);
    }
    else
    {
       $$ = 0;
    }
}
;

// ----------------------------------------------------------------------
interface_id
// ----------------------------------------------------------------------
: KANT_INTERFACE KANT_IDENTIFIER
{
    $$ = $2;
}
| KANT_INTERFACE keyword
{
    StringGrammarPtr ident = std::dynamic_pointer_cast<StringGrammar>($2);
    g_parse->error("keyword `" + ident->v + "' cannot be used as interface name");
    $$ = $2;
}
;

// ----------------------------------------------------------------------
interface_exports
// ----------------------------------------------------------------------
: interface_export ';' interface_exports
{
}
| error ';' interface_exports
{
}
| interface_export
{
    g_parse->error("`;' missing after definition");
}
|
{
}
;

// ----------------------------------------------------------------------
interface_export
// ----------------------------------------------------------------------
: operation
;

// ----------------------------------------------------------------------
operation
// ----------------------------------------------------------------------
: operation_preamble parameters ')'
{
    if($1)
    {
        g_parse->popContainer();
        $$ = std::dynamic_pointer_cast<GrammarBase>($1);
    }
    else
    {
        $$ = 0;
    }
}
;

// ----------------------------------------------------------------------
operation_preamble
// ----------------------------------------------------------------------
: return_type KANT_OP
{
    TypePtr returnType = std::dynamic_pointer_cast<Type>($1);
    StringGrammarPtr ident = std::dynamic_pointer_cast<StringGrammar>($2);
    string name        = ident->v;
    InterfacePtr cl    = std::dynamic_pointer_cast<Interface>(g_parse->currentContainer());
    if(cl)
    {
         OperationPtr op = cl->createOperation(name, returnType);
         if(op)
         {
             g_parse->pushContainer(op);
             $$ = std::dynamic_pointer_cast<GrammarBase>(op);
         }
         else
         {
             $$ = 0;
         }
    }
    else
    {
        $$ = 0;
    }
}
;

// ----------------------------------------------------------------------
return_type
// ----------------------------------------------------------------------
: type
| KANT_VOID
{
    $$ = 0;
}
;


// ----------------------------------------------------------------------
parameters
// ----------------------------------------------------------------------
: // empty
{
}
| type_id
{
    TypeIdPtr  tsp         = std::dynamic_pointer_cast<TypeId>($1);

    OperationPtr op = std::dynamic_pointer_cast<Operation>(g_parse->currentContainer());
    assert(op);
    if(op)
    {
        op->createParamDecl(tsp, false, false);
    }
}
| parameters ',' type_id
{
    TypeIdPtr  tsp         = std::dynamic_pointer_cast<TypeId>($3);

    OperationPtr op = std::dynamic_pointer_cast<Operation>(g_parse->currentContainer());
    assert(op);
    if(op)
    {
        op->createParamDecl(tsp, false, false);
    }
}
| out_qualifier type_id
{
    BoolGrammarPtr isOutParam  = std::dynamic_pointer_cast<BoolGrammar>($1);
    TypeIdPtr  tsp         = std::dynamic_pointer_cast<TypeId>($2);

    OperationPtr op = std::dynamic_pointer_cast<Operation>(g_parse->currentContainer());
    assert(op);
    if(op)
    {
        op->createParamDecl(tsp, isOutParam->v, false);
    }
}
| parameters ',' out_qualifier type_id
{
    BoolGrammarPtr isOutParam  = std::dynamic_pointer_cast<BoolGrammar>($3);
    TypeIdPtr  tsp         = std::dynamic_pointer_cast<TypeId>($4);

    OperationPtr op = std::dynamic_pointer_cast<Operation>(g_parse->currentContainer());
    assert(op);
    if(op)
    {
        op->createParamDecl(tsp, isOutParam->v, false);
    }
}
| routekey_qualifier type_id
{
    BoolGrammarPtr isRouteKeyParam  = std::dynamic_pointer_cast<BoolGrammar>($1);
    TypeIdPtr  tsp              = std::dynamic_pointer_cast<TypeId>($2);

    OperationPtr op = std::dynamic_pointer_cast<Operation>(g_parse->currentContainer());
    assert(op);
    if(op)
    {
         op->createParamDecl(tsp, false, isRouteKeyParam->v);
    }
}
| parameters ',' routekey_qualifier type_id
{
    BoolGrammarPtr isRouteKeyParam = std::dynamic_pointer_cast<BoolGrammar>($3);
    TypeIdPtr  tsp             = std::dynamic_pointer_cast<TypeId>($4);

    OperationPtr op = std::dynamic_pointer_cast<Operation>(g_parse->currentContainer());
    assert(op);
    if(op)
    {
         op->createParamDecl(tsp, false, isRouteKeyParam->v);
    }
}
| out_qualifier 
{
    g_parse->error("'out' must be defined with a type");
}
| routekey_qualifier 
{
    g_parse->error("'routekey' must be defined with a type");
}
;

// ----------------------------------------------------------------------
routekey_qualifier
// ----------------------------------------------------------------------
: KANT_ROUTE_KEY
{
    BoolGrammarPtr routekey = std::make_shared<BoolGrammar>();
    routekey->v = true;
    $$ = std::dynamic_pointer_cast<GrammarBase>(routekey);
}
;

// ----------------------------------------------------------------------
out_qualifier
// ----------------------------------------------------------------------
: KANT_OUT
{
    BoolGrammarPtr out = std::make_shared<BoolGrammar>();
    out->v = true;
    $$ = std::dynamic_pointer_cast<GrammarBase>(out);
}
;

// struct--------------------------------------------------------------
struct_def
// ----------------------------------------------------------------------
: struct_id
{
    StringGrammarPtr ident = std::dynamic_pointer_cast<StringGrammar>($1);
    NamespacePtr np = std::dynamic_pointer_cast<Namespace>(g_parse->currentContainer());
    if(np)
    {
         StructPtr sp = np->createStruct(ident->v);
         if(sp)
         {
             g_parse->pushContainer(sp);
             $$ = std::dynamic_pointer_cast<GrammarBase>(sp);
         }
         else
         {
             $$ = 0;
         }
    }
    else
    {
       g_parse->error("struct '" + ident->v + "' must definition in namespace");
    }
}
'{' struct_exports '}'
{
    if($2)
    {
        g_parse->popContainer();
    }
    $$ = $2;

    StructPtr st = std::dynamic_pointer_cast<Struct>($$);
    assert(st);
    if(st->getAllMemberPtr().size() == 0)
    {
        g_parse->error("struct `" + st->getSid() + "' must have at least one member");
    }
}
;

// struct name----------------------------------------------------------
struct_id
// ----------------------------------------------------------------------
: KANT_STRUCT KANT_IDENTIFIER
{
    $$ = $2;
}
| KANT_STRUCT keyword
{
    StringGrammarPtr ident = std::dynamic_pointer_cast<StringGrammar>($2);

    g_parse->error("keyword `" + ident->v + "' cannot be used as struct name");
}
| KANT_STRUCT error
{
    g_parse->error("abstract declarator '<anonymous struct>' used as declaration");
}
;

// struct members------------------------------------------------------
struct_exports
// ----------------------------------------------------------------------
: data_member ';' struct_exports
{

}
| data_member
{
   g_parse->error("';' missing after definition");
}
|
{
}
;

// 
// 
// data member--------------------------------------------------------------
data_member
// ----------------------------------------------------------------------
: struct_type_id
{
    $$ = std::dynamic_pointer_cast<GrammarBase>($1);
}
;

// struct member id--------------------------------------------------------------
struct_type_id
// ----------------------------------------------------------------------
: KANT_CONST_INTEGER KANT_REQUIRE type_id
{
    StructPtr np = std::dynamic_pointer_cast<Struct>(g_parse->currentContainer());
    if(np)
    {
        IntergerGrammarPtr iPtr = std::dynamic_pointer_cast<IntergerGrammar>($1);
        g_parse->checkTag(iPtr->v);

        TypeIdPtr tPtr  = std::dynamic_pointer_cast<TypeId>($3);
        tPtr->setRequire(iPtr->v);
        np->addTypeId(tPtr);
        $$ = std::dynamic_pointer_cast<GrammarBase>($3);
    }
    else
    {
        $$ = 0;
    }
}
| KANT_CONST_INTEGER KANT_REQUIRE type_id '=' const_initializer
{
    StructPtr np = std::dynamic_pointer_cast<Struct>(g_parse->currentContainer());
    if(np)
    {
        IntergerGrammarPtr iPtr = std::dynamic_pointer_cast<IntergerGrammar>($1);
        g_parse->checkTag(iPtr->v);

        TypeIdPtr tPtr   = std::dynamic_pointer_cast<TypeId>($3);
        ConstGrammarPtr sPtr = std::dynamic_pointer_cast<ConstGrammar>($5);
        g_parse->checkConstValue(tPtr, sPtr->t);

        tPtr->setRequire(iPtr->v);
        tPtr->setDefault(sPtr->v);
        np->addTypeId(tPtr);
        $$ = std::dynamic_pointer_cast<GrammarBase>($3);
    }
    else
    {
        $$ = 0;
    }
}
| KANT_CONST_INTEGER KANT_OPTIONAL type_id '=' const_initializer
{
    StructPtr np = std::dynamic_pointer_cast<Struct>(g_parse->currentContainer());
    if(np)
    {
        IntergerGrammarPtr iPtr = std::dynamic_pointer_cast<IntergerGrammar>($1);
        g_parse->checkTag(iPtr->v);

        TypeIdPtr tPtr   = std::dynamic_pointer_cast<TypeId>($3);
        ConstGrammarPtr sPtr = std::dynamic_pointer_cast<ConstGrammar>($5);
        g_parse->checkConstValue(tPtr, sPtr->t);

        tPtr->setOptional(iPtr->v);
        tPtr->setDefault(sPtr->v);
        np->addTypeId(tPtr);
        $$ = std::dynamic_pointer_cast<GrammarBase>($3);
    }
    else
    {
        $$ = 0;
    }
}
| KANT_CONST_INTEGER KANT_OPTIONAL type_id
{
    StructPtr np = std::dynamic_pointer_cast<Struct>(g_parse->currentContainer());
    if(np)
    {
        IntergerGrammarPtr iPtr = std::dynamic_pointer_cast<IntergerGrammar>($1);
        g_parse->checkTag(iPtr->v);
        TypeIdPtr tPtr = std::dynamic_pointer_cast<TypeId>($3);
        tPtr->setOptional(iPtr->v);
        np->addTypeId(tPtr);
        $$ = std::dynamic_pointer_cast<GrammarBase>($3);
    }
    else
    {
        $$ = 0;
    }
}
| KANT_REQUIRE type_id
{
    g_parse->error("struct member need 'tag'");
}
| KANT_OPTIONAL type_id
{
    g_parse->error("struct member need 'tag'");
}
| KANT_CONST_INTEGER type_id
{
    g_parse->error("struct member need 'require' or 'optional'");
}
| type_id
{
    g_parse->error("struct member need 'tag' or 'require' or 'optional'");
}
;

// ----------------------------------------------------------------------
const_initializer
// ----------------------------------------------------------------------
: KANT_CONST_INTEGER
{
    IntergerGrammarPtr intVal = std::dynamic_pointer_cast<IntergerGrammar>($1);
    ostringstream sstr;
    sstr << intVal->v;
    ConstGrammarPtr c = std::make_shared<ConstGrammar>();
    c->t = ConstGrammar::VALUE;
    c->v = sstr.str();
    $$ = c;
}
| KANT_CONST_FLOAT
{
    FloatGrammarPtr floatVal = std::dynamic_pointer_cast<FloatGrammar>($1);
    ostringstream sstr;
    sstr << floatVal->v;
    ConstGrammarPtr c = std::make_shared<ConstGrammar>();
    c->t = ConstGrammar::VALUE;
    c->v = sstr.str();
    $$ = c;
}
| KANT_STRING_LITERAL
{
    StringGrammarPtr ident = std::dynamic_pointer_cast<StringGrammar>($1);
    ConstGrammarPtr c = std::make_shared<ConstGrammar>();
    c->t = ConstGrammar::STRING;
    c->v = ident->v;
    $$ = c;
}
| KANT_FALSE
{
    StringGrammarPtr ident = std::dynamic_pointer_cast<StringGrammar>($1);
    ConstGrammarPtr c = std::make_shared<ConstGrammar>();
    c->t = ConstGrammar::BOOL;
    c->v = ident->v;
    $$ = c;
}
| KANT_TRUE
{
    StringGrammarPtr ident = std::dynamic_pointer_cast<StringGrammar>($1);
    ConstGrammarPtr c = std::make_shared<ConstGrammar>();
    c->t = ConstGrammar::BOOL;
    c->v = ident->v;
    $$ = c;
}
| KANT_IDENTIFIER
{
    StringGrammarPtr ident = std::dynamic_pointer_cast<StringGrammar>($1);

    if (g_parse->checkEnum(ident->v) == false)
    {
        g_parse->error("error enum default value, not defined yet");
    }
    ConstGrammarPtr c = std::make_shared<ConstGrammar>();
    c->t = ConstGrammar::ENUM;
    c->v = ident->v;
    $$ = c;
}
| scoped_name KANT_SCOPE_DELIMITER KANT_IDENTIFIER
{

    StringGrammarPtr scoped = std::dynamic_pointer_cast<StringGrammar>($1);
    StringGrammarPtr ident  = std::dynamic_pointer_cast<StringGrammar>($3);
    
    if (g_parse->checkEnum(ident->v) == false)
    {
        g_parse->error("error enum default value, not defined yet");
    }
    ConstGrammarPtr c = std::make_shared<ConstGrammar>();
    c->t = ConstGrammar::ENUM;
    c->v = scoped->v + "::" + ident->v;
    $$ = c;
}
;

// const--------------------------------------------------------------
const_def
// ----------------------------------------------------------------------
: KANT_CONST type_id '=' const_initializer
{
    NamespacePtr np = std::dynamic_pointer_cast<Namespace>(g_parse->currentContainer());
    if(!np)
    {
        g_parse->error("const type must define in namespace");
    }

    TypeIdPtr t   = std::dynamic_pointer_cast<TypeId>($2);
    ConstGrammarPtr c = std::dynamic_pointer_cast<ConstGrammar>($4);
    ConstPtr cPtr = np->createConst(t, c);
    $$ = cPtr;
}
;

// type--------------------------------------------------------------
type_id
// ----------------------------------------------------------------------
: type KANT_IDENTIFIER
{
    TypePtr type = std::dynamic_pointer_cast<Type>($1);
    StringGrammarPtr ident = std::dynamic_pointer_cast<StringGrammar>($2);

    TypeIdPtr typeIdPtr = std::make_shared<TypeId>(type, ident->v);

    $$ = std::dynamic_pointer_cast<GrammarBase>(typeIdPtr);
}
|type KANT_IDENTIFIER  '[' KANT_CONST_INTEGER ']'
{
    TypePtr type = g_parse->createVector(std::dynamic_pointer_cast<Type>($1));
    IntergerGrammarPtr iPtrSize = std::dynamic_pointer_cast<IntergerGrammar>($4);
    g_parse->checkArrayVaid(type,iPtrSize->v);
    type->setArray(iPtrSize->v);
    StringGrammarPtr ident = std::dynamic_pointer_cast<StringGrammar>($2);
    TypeIdPtr typeIdPtr = std::make_shared<TypeId>(type, ident->v);
    $$ = std::dynamic_pointer_cast<GrammarBase>(typeIdPtr);
}
|type '*' KANT_IDENTIFIER  
{
    TypePtr type = g_parse->createVector(std::dynamic_pointer_cast<Type>($1));
    //IntergerGrammarPtr iPtrSize = std::dynamic_pointer_cast<IntergerGrammar>($4);
    g_parse->checkPointerVaid(type);
    type->setPointer(true);
    StringGrammarPtr ident = std::dynamic_pointer_cast<StringGrammar>($3);
    TypeIdPtr typeIdPtr = std::make_shared<TypeId>(type, ident->v);
    $$ = std::dynamic_pointer_cast<GrammarBase>(typeIdPtr);
}
|type KANT_IDENTIFIER  ':' KANT_CONST_INTEGER 
{
    TypePtr type = std::dynamic_pointer_cast<Type>($1);
    StringGrammarPtr ident = std::dynamic_pointer_cast<StringGrammar>($2);
    TypeIdPtr typeIdPtr = std::make_shared<TypeId>(type, ident->v);
    IntergerGrammarPtr iPtrSize = std::dynamic_pointer_cast<IntergerGrammar>($3);
    g_parse->checkArrayVaid(type,iPtrSize->v);
    $$ = std::dynamic_pointer_cast<GrammarBase>(typeIdPtr);
}
| type keyword
{
    StringGrammarPtr ident = std::dynamic_pointer_cast<StringGrammar>($2);
    g_parse->error("keyword `" + ident->v + "' cannot be used as data member name");
}
| type
{
    g_parse->error("missing data member name");
}
| error
{
    g_parse->error("unkown type");
}
;

// type------------------------------------------------------------------
type
// ----------------------------------------------------------------------
: type_no ':' KANT_CONST_INTEGER 
{

    TypePtr type = std::dynamic_pointer_cast<Type>($1);
    IntergerGrammarPtr iPtrSize = std::dynamic_pointer_cast<IntergerGrammar>($3);
    g_parse->checkArrayVaid(type,iPtrSize->v);
    type->setArray(iPtrSize->v);
    $$ = type;
}
| type_no
{
    $$ = $1;
}
| type_no ':' error
{
   g_parse->error("array missing size");
}
;

// type------------------------------------------------------------------
type_no
// ----------------------------------------------------------------------
: KANT_BOOL
{
    $$ = g_parse->createBuiltin(Builtin::KindBool);
}
| KANT_BYTE
{
    $$ = g_parse->createBuiltin(Builtin::KindByte);
}
| KANT_UNSIGNED KANT_BYTE //unsigned char -> short
{
    $$ = g_parse->createBuiltin(Builtin::KindShort,true);
}
| KANT_SHORT
{
    $$ = g_parse->createBuiltin(Builtin::KindShort);
}
| KANT_UNSIGNED KANT_SHORT
{
    $$ = g_parse->createBuiltin(Builtin::KindInt,true);
}
| KANT_INT
{
    $$ = g_parse->createBuiltin(Builtin::KindInt);
}
| KANT_UNSIGNED KANT_INT
{
    $$ = g_parse->createBuiltin(Builtin::KindLong,true);
}
| KANT_LONG
{
    $$ = g_parse->createBuiltin(Builtin::KindLong);
}
| KANT_FLOAT
{
    $$ = g_parse->createBuiltin(Builtin::KindFloat);
}
| KANT_DOUBLE
{
    $$ = g_parse->createBuiltin(Builtin::KindDouble);
}
| KANT_STRING
{
    $$ = g_parse->createBuiltin(Builtin::KindString);
}
| vector
{
   $$ = std::dynamic_pointer_cast<GrammarBase>($1);
}
| map
{
   $$ = std::dynamic_pointer_cast<GrammarBase>($1);
}
| scoped_name
{
    StringGrammarPtr ident = std::dynamic_pointer_cast<StringGrammar>($1);
    TypePtr sp = g_parse->findUserType(ident->v);
    if(sp)
    {
        $$ = std::dynamic_pointer_cast<GrammarBase>(sp);
    }
    else
    {
        g_parse->error("'" + ident->v + "' undefined!");
    }
}
;

// vector----------------------------------------------------------------
vector
// ----------------------------------------------------------------------
: KANT_VECTOR '<' type '>'
{
   $$ = std::dynamic_pointer_cast<GrammarBase>(g_parse->createVector(std::dynamic_pointer_cast<Type>($3)));
}
| KANT_VECTOR '<' error
{
   g_parse->error("vector error");
}
| KANT_VECTOR '<' type error
{
   g_parse->error("vector missing '>'");
}
| KANT_VECTOR error
{
   g_parse->error("vector missing type");
}
;

// map----------------------------------------------------------------
map
// ----------------------------------------------------------------------
: KANT_MAP '<' type ',' type '>'
{
   $$ = std::dynamic_pointer_cast<GrammarBase>(g_parse->createMap(std::dynamic_pointer_cast<Type>($3), std::dynamic_pointer_cast<Type>($5)));
}
| KANT_MAP '<' error
{
   g_parse->error("map error");
}
;

// ----------------------------------------------------------------------
scoped_name
// ----------------------------------------------------------------------
: KANT_IDENTIFIER
{
}
| KANT_SCOPE_DELIMITER KANT_IDENTIFIER
{
    StringGrammarPtr ident = std::dynamic_pointer_cast<StringGrammar>($2);
    ident->v = "::" + ident->v;
    $$ = std::dynamic_pointer_cast<GrammarBase>(ident);
}
| scoped_name KANT_SCOPE_DELIMITER KANT_IDENTIFIER
{
    StringGrammarPtr scoped = std::dynamic_pointer_cast<StringGrammar>($1);
    StringGrammarPtr ident  = std::dynamic_pointer_cast<StringGrammar>($3);
    scoped->v += "::";
    scoped->v += ident->v;
    $$ = std::dynamic_pointer_cast<GrammarBase>(scoped);
}
;

// key----------------------------------------------------------------
keyword
// ----------------------------------------------------------------------
: KANT_STRUCT
{
}
| KANT_VOID
{
}
| KANT_BOOL
{
}
| KANT_BYTE
{
}
| KANT_SHORT
{
}
| KANT_INT
{
}
| KANT_FLOAT
{
}
| KANT_DOUBLE
{
}
| KANT_STRING
{
}
| KANT_VECTOR
{
}
| KANT_KEY
{
}
| KANT_MAP
{
}
| KANT_NAMESPACE
{
}
| KANT_INTERFACE
{
}
| KANT_OUT
{
}
| KANT_REQUIRE
{
}
| KANT_OPTIONAL
{
}
| KANT_CONST_INTEGER
{
}
| KANT_CONST_FLOAT
{
}
| KANT_FALSE
{
}
| KANT_TRUE
{
}
| KANT_STRING_LITERAL
{
}
| KANT_CONST
{
}
| KANT_ENUM
{
}
| KANT_UNSIGNED
{
}
;

%%


