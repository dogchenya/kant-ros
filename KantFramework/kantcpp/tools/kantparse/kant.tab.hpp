/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_KANT_TAB_HPP_INCLUDED
# define YY_YY_KANT_TAB_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    KANT_VOID = 258,
    KANT_STRUCT = 259,
    KANT_BOOL = 260,
    KANT_BYTE = 261,
    KANT_SHORT = 262,
    KANT_INT = 263,
    KANT_DOUBLE = 264,
    KANT_FLOAT = 265,
    KANT_LONG = 266,
    KANT_STRING = 267,
    KANT_VECTOR = 268,
    KANT_MAP = 269,
    KANT_NAMESPACE = 270,
    KANT_INTERFACE = 271,
    KANT_IDENTIFIER = 272,
    KANT_OUT = 273,
    KANT_OP = 274,
    KANT_KEY = 275,
    KANT_ROUTE_KEY = 276,
    KANT_REQUIRE = 277,
    KANT_OPTIONAL = 278,
    KANT_CONST_INTEGER = 279,
    KANT_CONST_FLOAT = 280,
    KANT_FALSE = 281,
    KANT_TRUE = 282,
    KANT_STRING_LITERAL = 283,
    KANT_SCOPE_DELIMITER = 284,
    KANT_CONST = 285,
    KANT_ENUM = 286,
    KANT_UNSIGNED = 287,
    BAD_CHAR = 288
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_KANT_TAB_HPP_INCLUDED  */
