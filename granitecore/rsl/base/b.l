%option c++ noyywrap
%option yyclass="lexer_context"

%{

/*
 * b.l -- use with b.y
 *
 * RSL 2 lexer
 *
 * $Id: b.l,v 1.3 1998/11/24 14:13:19 holtrf Exp $
 */

// Redefine the YY_DECL to take an argument
#ifdef YY_DECL
#undef YY_DECL
#endif
#define YY_DECL lexer_context::blex(RSL_YYSTYPE *blval_v)

#include <stdio.h>
#include <string.h>

#include "lexer_context.h"
#include "b.h"
#include "res_param.h"
#include "rslMethod.h"

#include "b.tab.h"
#include "Resource.h"


extern char *malloc();

#define RSL_BLVAL ((YYSTYPE *) blval_v)
#define COPY_TEXT ( insertAlloc_free(RSL_BLVAL->sval = strdup(YYText())) )


#define parserInMethod 1

//#ifdef SMARTKEYWORDS
//#undef SMARTKEYWORDS
//#endif

// SMARTKEYWORDS - the lexer will recognize certain keywords only when the
// parser is in a state where that keyword is valid.
// KeywordState(tok,val) specifes that token 'tok' should be returned
// only if lexer_keystate does NOT contain 'val', where 'val' is the logical
// OR of those parser states (defined in lexer_aux.h) we want to ignore keywords, which
// means, consider it an identifier if the parser is in the state given
// (and return ID), otherwise consider it a keyword (and return 'tok').
// Eg, it's ok to name a method "class" or "while" if SMARTKEYWORDS is defined,
// (since it is illegal to use the keyword "class" inside a class definition
// or inside a method body), otherwise, a syntax error will result.
#ifdef SMARTKEYWORDS
#define KeywordState(tok, val) { \
		/* cout << "KeywordState: matched \"" << YYText() << "\";" \
			<< " current state is " << keystate << ", to reject is " \
			<< (val); */ \
		if (((val) | keystate) == (val)) /* cur. state is in list of rejects */ \
		{ COPY_TEXT; /* cout << " ==> ID\n" << flush; */ return ID; } \
		/* cout << " ==> keyword\n" << flush; */  return (tok); }
#else
#define KeywordState(tok) return (tok)
#endif

// int mylineno = 0;


%}
letter [a-zA-Z_]
dig [0-9]
ws [ \t\r]+
hexdig [a-fA-F0-9]
numid ({letter}|{dig})*
ident {letter}{numid}*
unary "++"|"--"
binCompare "&&"|"||"
assign "+="|"-="|"%="|"*="|"/="|"="|":="|"<<"|">>"
reference "<-"
methodsel "->"|"@"
binLTGT "<="|"<"|">"|"=="|">="|"!="
addOP "+"|"-"|"%"
mulOp "*"|"/"
qstring \"[^"\n]*\"
mqstring \"(\\\"|[^"\n])*\"
qchar \'[^'"]*\'
%%

{ws}	/* skip blanks and tabs */

\n#.*	mylineno++;	/* ignore shell-type comment IF it is the first char of a line! */
{mqstring}	{ COPY_TEXT; return QSTRING;}
\n		mylineno++;	/* keep track of current line number */
\/\/.*	/* skip C++ style to-EOL comments "//" */

		
"/**"	{	/* Documentation comment */
			int c;
			RWCString outstr;
			while((c = yyinput()) != 0)
			{
				if (c=='\n')
					mylineno++;
				else
				if (c == '*')
				{
					if ((c=yyinput()) == '/')
						break;
					
					unput(c);
					continue;
				}

				// this includes the newlines.
				outstr += char(c);
			}

			RSL_BLVAL->sval = strdup(outstr.data());
			
			// mark as free-able for later.
			insertAlloc_free(RSL_BLVAL->sval);

			return DOC_COMMENT;
		}

"/*"	{	/* Eat a C block comment */
			int c;
			while((c = yyinput()) != 0)
			{
				if (c=='\n')
					mylineno++;
				else
					if (c == '*')
					{
						if ((c=yyinput()) == '/')
							break;
						else
							unput(c);
					}
			}
		}
		
		
		
"*/"		{	/* end comment -- always an error, since matched pairs are eaten
				 *  by "/*" above! */
				berror("Mismatched \"*/\".\n");
			}

class     KeywordState(_CLASS,     lexer_context::InClass + lexer_context::InExpr)
exception KeywordState(_EXCEPTION, lexer_context::InClass + lexer_context::InExpr)
extends   KeywordState(_EXTENDS,   lexer_context::InClass + lexer_context::InExpr)
native    KeywordState(_NATIVE,    lexer_context::InExpr)
deprecated    KeywordState(_DEPRECATED,    lexer_context::InExpr)
abstract    KeywordState(_ABSTRACT,    lexer_context::InExpr)
in        { return _IN; }
import    KeywordState(_IMPORT,    lexer_context::InClass + lexer_context::InExpr)
global    KeywordState(_GLOBAL,    lexer_context::InClass + lexer_context::InExpr)
session   KeywordState(_SESSION,   lexer_context::InClass + lexer_context::InExpr)

public    KeywordState(_PUBLIC,    lexer_context::InExpr)
private   KeywordState(_PRIVATE,   lexer_context::InExpr)
protected KeywordState(_PROTECTED, lexer_context::InExpr)
const     KeywordState(_CONST,     lexer_context::InExpr)


if        KeywordState(_IF,        lexer_context::InExpr)
else      KeywordState(_ELSE,      lexer_context::InExpr)
while     KeywordState(_WHILE,     lexer_context::InExpr)
foreach   KeywordState(_FOREACH,   lexer_context::InExpr)
return    KeywordState(_RETURN,    0)
share     KeywordState(_SHARE,     0)
break     KeywordState(_BREAK,     lexer_context::InExpr)
continue  KeywordState(_CONTINUE,  lexer_context::InExpr)
exit      KeywordState(_EXIT,      lexer_context::InExpr)
branch    KeywordState(BRANCH,     lexer_context::InExpr)
case      KeywordState(CASE,       lexer_context::InExpr)
default   KeywordState(DEFAULT,    lexer_context::InExpr)

new       KeywordState(CMD_NEW,    0)
help      KeywordState(CMD_HELP,   lexer_context::InExpr + lexer_context::InClass)
print     KeywordState(CMD_PRINT,  lexer_context::InExpr + lexer_context::InClass)
html      KeywordState(CMD_HTML,   lexer_context::InExpr + lexer_context::InClass)
quit      KeywordState(CMD_QUIT,   lexer_context::InExpr + lexer_context::InClass)
destroy   KeywordState(CMD_DESTROY,lexer_context::InExpr + lexer_context::InClass)
list      KeywordState(CMD_LIST,   lexer_context::InExpr + lexer_context::InClass)
dump      KeywordState(CMD_DUMP,   lexer_context::InExpr + lexer_context::InClass)
stats     KeywordState(CMD_STATS,   lexer_context::InExpr + lexer_context::InClass)

true		return _BOOLTRUE;
false		return _BOOLFALSE;

-?{dig}+	{ COPY_TEXT; return INT; }
{ident}		{ COPY_TEXT; return ID; }
{numid}		{ COPY_TEXT; return NUMID; }
\%{hexdig}+	{ COPY_TEXT; return HEXVAL; }
{qchar}		{
#ifdef __cplusplus
				// don't want to mix new with malloc
				RSL_BLVAL->sval = new char[2];
				RSL_BLVAL->sval[0] = *(YYText());	// was yytext[1]  (?)
				RSL_BLVAL->sval[1] = '\0';
				insertAlloc_arraydelete(RSL_BLVAL->sval);
#else
				yylval.sval = (char *) malloc(sizeof(char)+1);
				*(yylval.sval) = yytext[0];
				yylval.sval[1] = '\0'; return QCHAR;
#endif
			}
"..."		return INFINITY;
"::"		return SCOPE;
{methodsel}	return AT_OP;

{binCompare} { COPY_TEXT; return BINCOMPARE_OP; }
{binLTGT} { COPY_TEXT; return BINLTGT_OP; }
{addOP} { COPY_TEXT; return ADD_OP; }
{mulOp} { COPY_TEXT; return MUL_OP; }
{assign} { COPY_TEXT; return ASSIGN_OP; }
{reference} { COPY_TEXT; return REFERENCE_OP; }
{unary}	{ COPY_TEXT; return UNARY_OP; }

"[]"	{ COPY_TEXT; return BRACKETS; }	// a weird one!



-?({dig}*\.{dig}+)([eE][-+]?{dig}+)?	{ COPY_TEXT; return FLOAT; }
\$-?{dig}*(\.{dig}*)?	{ COPY_TEXT; return MONEY; }
";"			return SEMI;

.			return yytext[0];	/* any other unmatched single char */

%%


/*
int yywrap()
{
	return 1;	// done -- no more files
}
*/

