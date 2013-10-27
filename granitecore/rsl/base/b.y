

%{

/* b.y
 * RSL 2 grammar
 * Includes class definitions with method prototypes,
 * argument defaults and value matching (pdecl, param, sparam)
 *
 * Russell, Oct 2-4 1996
 *
 * !!!!!! REQUIRES bison 1.24, flex++ 2.5.2 !!!!!!
 *
 * $Id: b.y,v 1.6 1999/01/22 20:51:43 toddm Exp $
 */

// Define the name of the parameter to be passed to the yyparse
// function. It's a lexer_context object to enable reentrancy.
// Also define a convenience casting macro since it's passed
// as a void *.
#define YYPARSE_PARAM currentContext

// lexer_context.h defines CURRCONTEXTP and CURRCONTEXT
#include "lexer_context.h"

#define BGROUPOPEXPR (CURRCONTEXTP->bGroupOpExpr)

// bison error verbosity
// #define YYERROR_VERBOSE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>	// bzero
#include <strstream.h>


#include "res_class.h"
#include "b.h"
#include "rslEvents.h"
#include "R_String.h"
#include "R_Integer.h"
#include "R_Boolean.h"
#include "runtime.h"
#include "rslMethod.h"
#include "iteration.h"
	
#include "valdecl.h"

#include "killevents.h"

extern unsigned int theIDHash(const char *s0);

int parse_it(lexer_context &lexc);
int NextSessionID = 1;

// global RTF
extern runtimeRSL runtimeStuff;

// statistics (runtime.cc)
extern int nProgramResources;

// whether the system is in operation or not (runtime.cc)
// ie, this will be FALSE if we are parsing files, but
// TRUE when we are actively doing ECI communication.
// This is used here to determine whether we should Remember()
// events that we create with c++ new; startup events are
// forgotten for now because we do not yet need to clean them up.
extern bool Running;   // should be a runtimeRSL class variable

#define ENDQNL "\"\n"

// ********************************************************************
// Redefine the yyerror function call to invoke the
// error reporting method of the current lexer object.
// CURRCONTEXT etc. defined in lexer_context.h
#ifdef yyerror	// "berror" by default
#undef yyerror   
#endif
#define yyerror CURRCONTEXTP->berror

// ********************************************************************
// Here's a nice kludge. I want to have yylex (ie lexer_context::blex)
// take a YYSTYPE * argument, but I can't have a YYSTYPE * in
// lexer_context.h because bison creates that typedef _after_
// lexer_context.h is included (I have no control).  So I make it
// take a void * and define a macro cast this to a YYSTYPE * in the
// rules section when I need to access it. But now, I need to change
// the call semantics that bison uses, to cast the argument to a void *.
// I can control the definition of the macro yylex, and bison
// makes the call "yylex(&yylval)", where yylval is a YYSTYPE.
// So I just define yylex to be a macro with an argument, which I
// cast to void *.
#ifdef yylex	// "blex" by default
#undef yylex
#endif

#define yylex(X) CURRCONTEXTP->blex((void *) X)

%}

%pure_parser

%union {
	char *sval;
	int ival;
	event *eptr;
	Request *requestptr;
	elist *elistptr;
	param *paramptr;
	decl *declptr;
	method_decl *methodptr;
	data_decl *dataptr;
	res_class_decl *classdeclptr;
	res_class *resclassptr;
	Resource *rsrcptr;
	rslMethod *rslM;
	Argument *argptr;
}

%token <sval> ID, NUMID, QSTRING, INT, FLOAT, HEXVAL, DOC_COMMENT, _BOOLTRUE, _BOOLFALSE, QCHAR, MONEY
%token <sval> REFERENCE_OP, ASSIGN_OP, BINCOMPARE_OP, BINLTGT_OP, ADD_OP, MUL_OP, BRACKETS, UNARY_OP
%token CMD_PRINT CMD_HTML CMD_HELP CMD_QUIT CMD_NEW CMD_DESTROY CMD_LIST CMD_DUMP CMD_STATS
%token  EQ LOP ROP CASE DEFAULT NOT_OP AT_OP
%token INFINITY _CLASS _EXTENDS _PUBLIC _PRIVATE _PROTECTED _EXCEPTION _NATIVE _DEPRECATED _ABSTRACT
%token _BEGIN _END SEMI A_FILE _IF _ELSE _SHARE SCOPE _IN _IMPORT _GLOBAL _SESSION _CONST
%token _WHILE _RETURN _BREAK _CONTINUE _EXIT BRANCH _FOREACH

%left REFERENCE_OP
%left ASSIGN_OP
%left AT_OP
%left NOT_OP
%left BINCOMPARE_OP
%left BINLTGT_OP
%left ADD_OP
%left MUL_OP
%left BRACKETS
%left UMINUS

%nonassoc _ELSE
%nonassoc LOWER_THAN_ELSE

%type <ival> decl_flags, decl_flag
%type <sval> NumOrIntID, method_name, docComment
%type <paramptr> idl_param, sparam
%type <methodptr> idl_pdecl, idl_method
%type <declptr> idl_decl, qualified_decl, uproc
%type <dataptr> idl_data, idl_data_list
%type <classdeclptr> idl_decl_list, idl_classcontents
%type <rsrcptr> literal
%type <requestptr> methodreq, objreq
%type <eptr> term, expr, stmt, sblock
%type <elistptr> alist, slist
%type <rslM> proc
%type <argptr> ar, argvalue
 
%%

system: things
		{
			if (CURRCONTEXT.type == lexer_context::prompt)	// interactive + prompt
				CURRCONTEXT.Out() << "RSL> " << flush;
		}
	| system things
		{
			if (CURRCONTEXT.type == 2)	// interactive + prompt
				CURRCONTEXT.Out() << "RSL> " << flush;
		}
	;

things: ntclass
	| proc
		{
			if ($1)
				runtimeStuff.AddMethod($1);
		}
	| _EXCEPTION proc
		{
			if ($2)
				runtimeStuff.AddMethod($2);
		}
	| ID { CURRCONTEXT.SetKeyState(lexer_context::InExpr); } '#' NumOrIntID '.' method_name '#' NumOrIntID '(' alist ')' SEMI
		{
			CURRCONTEXT.SetKeyState(lexer_context::Everywhere);
			AuditRequest *areq = new AuditRequest($1, $4, $6, $8);

			elist *oldargs=NULL;
			if (areq)
			{
				areq->SetArgs($10);
				areq->kind |= event::dynamicRequestKind;

#ifdef RSLERR
				cerr << "Read ECI request in lexer_context #"
					<< CURRCONTEXT.lexer_number << endl;
#endif

				CURRCONTEXT.done = lexer_context::GotECIReq;
				CURRCONTEXT.dynamic_request = areq;
				YYACCEPT;
			}
		}
	| _IMPORT { CURRCONTEXT.SetKeyState(lexer_context::InExpr); } ID SEMI
		{
			CURRCONTEXT.SetKeyState(lexer_context::Everywhere);
			runtimeStuff.AddImport($3);
		}
	| CMD_HELP
		{
			// could this be a pass through to the admin
			// session? so that "help" is based on the current
			// "application" ???

			CURRCONTEXT.Out() << "ECI syntax:\n"
				<< "\tobject # session . method # audit ( args ) ;\n"
				<< "Other commands are help, print, quit.\n\n"
				<< flush;
		}
	| CMD_PRINT
		{
			if (CURRCONTEXT.type == 1)	// interactive
				runtimeStuff.print(CURRCONTEXT.Out());
		}
	| CMD_HTML
		{
			if (CURRCONTEXT.type == 1)	// interactive
				// runtimeStuff.html(CURRCONTEXT.Out());
				runtimeStuff.htmlClasses(cout)
		}
	| CMD_DESTROY NumOrIntID
	  	{
			if (runtimeStuff.KillSession($2))
				CURRCONTEXT.Out() << "session `" << $2 << "' not found.\n" << flush;
		}
	| CMD_LIST
		{
			// List the active sessions in RSL "List" inline resource form,
			// eg, ActiveSessions: List { "A", "B", "C" };
			RWTValHashSet<ResReference> *pLocals=runtimeStuff.Sessions->GetLocals();
			RWTValHashTableIterator<ResReference> iter(*pLocals);
			CURRCONTEXT.Out() << "ActiveSessions: List {";
			int prev = 0;

			while (iter())
			{
				ResReference resref = iter.key();
				if (!resref.isValid())
					continue;

				if (prev)
					CURRCONTEXT.Out() << ", ";

				CURRCONTEXT.Out() << "\"" << resref->Name() << "\"";
				prev = 1;
			}
			CURRCONTEXT.Out() << " };\n" << flush;
		}
	| CMD_DUMP NumOrIntID
		{
			ResContext *rc = runtimeStuff.FindSession($2);

			if (rc)
				rc->print(CURRCONTEXT.Out());
			else
				CURRCONTEXT.Out() << "Session `" << ($2) << "' not found.\n";

			CURRCONTEXT.Out() << flush;
		}
	| CMD_QUIT
		{
			CURRCONTEXT.done = lexer_context::Done;
			YYACCEPT;
		}
	| CMD_NEW
		{
			char buf[20];
			ostrstream stm(buf, 20);
			do {
				NextSessionID += 1;
				bzero(buf, (size_t) 20);
				stm << "D" << NextSessionID;			
			} while (NULL != runtimeStuff.FindSession(buf));
			
			CURRCONTEXT.Out() << "D" << NextSessionID << endl << flush;
//			CURRCONTEXT.done = ;
			YYACCEPT;
		}
	| CMD_STATS
		{
			runtimeStuff.eciStats(CURRCONTEXT.Out());
			CURRCONTEXT.Out() << endl << flush;
		}
	| _GLOBAL decl_flags idl_data
		{
			decl *dat = $3;
			dat->setFlag((decl::declflag_t) $2);
			runtimeStuff.AddGlobalEvent(new LocalDecl((data_decl *) dat));
		}
	| _GLOBAL sblock
		{
			runtimeStuff.AddGlobalEvent($2);
		}
	| _SESSION sblock
		{
			runtimeStuff.AddSessionEvent($2);
		}
	| DOC_COMMENT
		{
			// add to lexer-state "last comment"
			CURRCONTEXT.last_global_comment = $1;
		}
	;

NumOrIntID: NUMID | INT | ID
	;

ntclass: decl_flags _CLASS ID { CURRCONTEXT.SetKeyState(lexer_context::InClass); } idl_classcontents
		{
			CURRCONTEXT.SetKeyState(lexer_context::Everywhere);
			// Finalize res_class_decl object
#ifdef RSL_DEBUG_PARSE
			cout << "Recognized class \"" << $3 << ENDQNL << flush;
#endif
			res_class_decl *rcd = $5;
			if (rcd)
			{
				// add the class description as provided by a previous
				// documentation comment. then clear the last comment
				// so it is only used once.
				rcd->description = CURRCONTEXT.last_global_comment;
				CURRCONTEXT.last_global_comment = "";
				CURRCONTEXT.last_comment = "";
				rcd->flags = (decl::declflag_t) ($1);
				
				rcd->CloseDeclarations();
			}

			// LinkToResClass: see comments below
			res_class *rc = res_class::LinkToResClass(rcd, $3);
		}
	| decl_flags _CLASS ID _EXTENDS ID { CURRCONTEXT.SetKeyState(lexer_context::InClass); } idl_classcontents
		{		
			CURRCONTEXT.SetKeyState(lexer_context::Everywhere);

#ifdef RSL_DEBUG_PARSE
			cout << "Recognized class \"" << $3 << "\", subclass of \""
				<< $5 << ENDQNL << flush;
#endif
			// Finalize res_class object
			res_class_decl *rcd = $7;
			if (rcd)
			{
				// add the class description as provided by a previous
				// documentation comment. then clear the last comment
				// so it is only used once.
				rcd->description = CURRCONTEXT.last_global_comment;
				CURRCONTEXT.last_global_comment = "";
				CURRCONTEXT.last_comment = "";
				rcd->flags = (decl::declflag_t) ($1);
				
				rcd->CloseDeclarations();
			}

			// Attach the method declarations (res_class_decl) to an
			// existing res_class, if one exists. If there isn't one,
			// then create a new one and add it to the static res_class
			// collection.
			res_class *rc = res_class::LinkToResClass(rcd, $3);

			if (rc)
				rc->SetParent($5);
		}	
	;

idl_classcontents: '{' { CURRCONTEXT.SetKeyState(lexer_context::InClass); } idl_decl_list '}'
		{
			CURRCONTEXT.SetKeyState(lexer_context::Everywhere);
#ifdef RSL_DEBUG_PARSE
			cout << " -*- recognize idl_class contents -*-\n";
#endif
			$$ = $3;
		}
	;

idl_decl_list: qualified_decl
		{
#ifdef RSL_DEBUG_PARSE
			cout << "CREATE res_class (idl_decl_list)\n" << flush;
#endif
			res_class_decl *rc = new res_class_decl;
			rc->AddDecl($1);
			$$ = rc;
		}
	| idl_decl_list qualified_decl
		{
#ifdef RSL_DEBUG_PARSE
			cout << "ADD declaration to res_class\n" << flush;
#endif
			res_class_decl *rc = $1;
			if (rc)
				rc->AddDecl($2);
			$$ = rc;
		}
	;
	
qualified_decl: decl_flags idl_decl
		{
			if ($2)
			{
				decl *dec = (decl *) $2;
				dec->setFlag((decl::declflag_t) $1);
				$$ = dec;
			}
			else
				$$ = NULL;
		}
	| decl_flags uproc
		{
			if ($2)
			{
			// an 'inline' method implementation.
				decl *md = $2;
				md->setFlag((decl::declflag_t) $1);

				if (CURRCONTEXT.last_comment.length() > 0)
				{
					md->description = CURRCONTEXT.last_comment;
					CURRCONTEXT.last_comment = "";
				}

				$$ = md;
			}
			else
				$$ = NULL;
		}
	| DOC_COMMENT
		{
			// add to lexer-state "last comment"
			CURRCONTEXT.last_comment = $1;
			$$ = NULL;
		}
	;

idl_decl: idl_data
		{	// decl_flags_list 
			decl *dec = (decl *) $1;

			if (CURRCONTEXT.last_comment.length() > 0)
			{
				dec->description = CURRCONTEXT.last_comment;
				CURRCONTEXT.last_comment = "";
			}

			$$ = dec;
		}
	| idl_method
		{
			// notice that methods can have decl_flags at both ends
			// (the right side is specified in idl_method -- before the SEMI)
			method_decl *md = $1;

			if (CURRCONTEXT.last_comment.length() > 0)
			{
				md->description = CURRCONTEXT.last_comment;

				// test.. don't clear the last comment so that
				// we can use one comment for a block of similar
				// methods, eg all the comparison operators don't
				// need to be explained individually.
				// CURRCONTEXT.last_comment = "";
			}

			$$ = (decl *) md;
		}
	;

decl_flags: /* null */
		{
			$$ = (int) decl::undefined;
		}
	| decl_flags decl_flag
		{
			$$ = (int) ($1 | $2);
		}
	;
	
decl_flag: _CONST
		{	$$ = (int) decl::vconst; }
	| _PUBLIC
		{	$$ = (int) decl::vpublic; }
	| _PRIVATE
		{	$$ = (int) decl::vprivate; }
	| _PROTECTED
		{	$$ = (int) decl::vprotected; }
	| _NATIVE
		{	$$ = (int) decl::vnative; }
	| _DEPRECATED
		{	$$ = (int) decl::vdeprecated; }
	| _ABSTRACT
		{	$$ = (int) decl::vdeprecated; }
	;


idl_method: method_name '(' idl_pdecl ')'  decl_flags SEMI
		{	// decl_flags_list
			// method declaration with no return value

#ifdef RSL_DEBUG_PARSE
			cout << "    idl_method \"" << $1 << ENDQNL << flush;
#endif
			method_decl *md = $3;
			if (md)
			{
				md->name = $1;
				md->returnType = "void";
				md->memberOf = "unassigned";
				md->setFlag((decl::declflag_t) $5);

			}
			$$ = md;
		}
	| ID method_name '(' idl_pdecl ')' decl_flags SEMI
		{
			// method declaration with a return type

#ifdef RSL_DEBUG_PARSE
			cout <<  "    idl_method \"" << $2 << ENDQNL << flush;
#endif

			method_decl *md = $4;	// created in idl_pdecl:idl_param
			if (md)
			{
				md->name = $2;
				//	md->returnType = res_class::theIDHash($2);
				md->returnType = $1;
				md->memberOf = "unassigned";
				md->setFlag((decl::declflag_t) $6);
			}
			$$ = md;
		}
	;

method_name: ID | BINCOMPARE_OP | BINLTGT_OP | ADD_OP | MUL_OP | ASSIGN_OP | BRACKETS
	;

idl_pdecl:
		{	/* null parameter list */

#ifdef RSL_DEBUG_PARSE
			cout << " idl_pdecl (NULL)\n" << flush;
#endif

			// Still need to create the method for consistency.
			$$ = new method_decl;
		}
	| idl_param
		{
#ifdef RSL_DEBUG_PARSE
			cout << " idl_pdecl:idl_param-- new method_decl\n\n" << flush;
#endif
			method_decl *md = new method_decl; // Create method
			if ($1 == &InfinityParam)
				md->infinity=1;

			md->AddParam($1);
			$$ = md;
		}
	| idl_pdecl ',' idl_param
		{
#ifdef RSL_DEBUG_PARSE
			cout << " idl_pdecl:idl_pdecl,idl_param\n\n" << flush;
#endif
			method_decl *md = $1;
			if ($3 == &InfinityParam)
				md->infinity=1;

			md->AddParam($3);
			$$ = md;
		}
	;

idl_param: INFINITY
		{	// this could set a flag for the method which says
			// it can take an unlimited number of arguments
			$$ = &InfinityParam;
		}
	| sparam
		{
			// A parameter with unspecified type (matches all types)
			$$ = $1;
			$$->SetType(PARAM_ALL_TYPES);
		}
	| ID sparam
		{
			// Add the type specifier to the param object
			param *p = $2;
			p->SetType($1);
			$$ = p;
		}
	;

sparam: ID
		{	// a named formal parameter
			// By default, accepts all types (as does the type "Resource")
			$$ = new param($1);
		}
	 | ID ':' literal
		{
			// a	named formal parameter which matches a value (event	handler)
			// was		ID	':'	ix
			$$ = new param_preset($1, $3);
		}
	| ID ':' '{' alist '}'
		{
			// named formal which matches an inline resource.
			// Not this specification is different than the normal inline
			// resource spec, in that the type is omitted before the `{'.
			// This is because the type is already specified as part of
			// the parameter, so for example we can have Status s: { Severity: 3};
			// instead of Status s: Status { Severity: 3};
			ListArg *toEval = new ListArg((char *) "", $4);	// untyped!
			toEval->kind |= event::programCodeKind;	// shouldn't be necessary here, though

			param_presetIR *pir = new param_presetIR($1, toEval);

			runtimeStuff.AddPreEvalParam(pir);
			$$ = pir;
		}
	| ID ':' ID '{' alist '}'
		{
			// Alternate form of the above. Best use of this one is something like
			//		s: Status { Severity: 3}
			// using the un-typed idl_param production.

			ListArg *toEval = new ListArg($3, $5);	// type, contents.
			toEval->kind |= event::programCodeKind;	// shouldn't be necessary here, though

			param_presetIR *pir = new param_presetIR($1, toEval);
			pir->SetType(toEval->argType);

			runtimeStuff.AddPreEvalParam(pir);
			$$ = pir;
			
		}
	/* 
	 * | ID	EQ literal
	 *	   {
	 *		   // a	named formal parameter with	a default value
	 *		   printf("		 param name	\"%s\",	default	of \"%s\"\n", $1, $3);
	 *		   $$ =	new	param($1); //  , $3);
	 *	   }
	 */
	;

idl_data: ID idl_data_list SEMI
		{
			data_decl *dd = $2;
			if (dd)
				dd->SetType($1);
			$$ = dd;
		}
	;

idl_data_list: ID
		{
			$$ = new data_decl($1);
		}
	| idl_data_list ',' ID 
		{	
			data_decl *dd = $1;
			if ($1)
				dd->AddVar($3);

			$$ = dd;
		}
	;

uproc: method_name '(' idl_pdecl ')' decl_flags sblock
		{
#ifdef RSL_DEBUG_PARSE
			cout << "    method `" << $1 << "' of class \"" << $1 << ENDQNL;
#endif
			method_decl *md = $3;
			if (md)
			{
//				md->memberOf = $1;
				md->name = $1;
				md->setFlag((decl::declflag_t) $5);
			}
			rslMethod *rm = new rslMethod(md, $6);
			md->implementation = rm;
			$$ = md;
		}
	| ID method_name '(' idl_pdecl ')' decl_flags sblock
		{
			method_decl *md = $4;
			if (md)
			{
//				md->memberOf = $2;
				md->returnType = $1;
				md->name = $2;
				md->setFlag((decl::declflag_t) $6);
			}
			rslMethod *rm = new rslMethod(md, $7);
			md->implementation = rm;
			$$ = md;
		}
	;

proc: ID SCOPE method_name '(' idl_pdecl ')' decl_flags docComment sblock
		{
#ifdef RSL_DEBUG_PARSE
			cout << "    method `" << $3 << "' of class \"" << $1 << ENDQNL;
#endif
			method_decl *md = $5;
			if (md)
			{
				md->memberOf = $1;
				md->name = $3;
				md->setFlag((decl::declflag_t) $8);
			}

			rslMethod *rm = new rslMethod(md, $9, $8);

			// if there was a doc comment BEFORE the method, use
			// that instead of the method doc comment.
			if (CURRCONTEXT.last_comment.length() > 0)
				rm->description = CURRCONTEXT.last_comment;

			CURRCONTEXT.last_comment = "";

			$$ = rm;
		}
	| ID ID SCOPE method_name '(' idl_pdecl ')' decl_flags docComment sblock
		{
			method_decl *md = $6;
			if (md)
			{
				md->memberOf = $2;
				md->returnType = $1;
				md->name = $4;
				md->setFlag((decl::declflag_t) $8);
			}

			rslMethod *rm = new rslMethod(md, $10, $9);

			// if there was a doc comment BEFORE the method, use
			// that instead of the method doc comment.
			if (CURRCONTEXT.last_comment.length() > 0)
				rm->description = CURRCONTEXT.last_comment;

			CURRCONTEXT.last_comment = "";

			$$ = rm;
		}
	;

docComment: /* null */
		{ $$ = (char *) NULL; }
	| DOC_COMMENT
	;
	
sblock: '{' slist '}'
	{ $$ = $2; }
	;
	
slist: stmt
		{
			elist *el = new elist;
			el->add($1);
			$$ = el;
			$$->kind |= event::programCodeKind;

			if (Running == TRUE)
				Remember(el);
		}
	|	slist stmt
		{
			$$ = $1;
			if ($$)
				$$->add($2);
		}
	;

stmt: expr SEMI
		{
			$$ = $1;
		}
	| idl_data
		{
			$$ = new LocalDecl($1);
			$$->kind |= event::programCodeKind;
		}
	| _IF '(' argvalue ')' sblock _ELSE stmt
		{	$$ = new IfRequest($3, $5, $7);
			$$->kind |= event::programCodeKind;
		}
	| _IF '(' argvalue ')' stmt %prec LOWER_THAN_ELSE
		{	$$ = new IfRequest($3, $5);
			$$->kind |= event::programCodeKind;
		}
	| sblock
		{ $$ = $1; }
	| _RETURN alist SEMI
		{
			$$ = new controlRequest(controlRequest::crReturn, $2);
			$$->kind |= event::programCodeKind;
		}
	| _BREAK SEMI
		{
			$$ = new controlEvent(controlEvent::ctBreak);
			$$->kind |= event::programCodeKind;
		}
	| _SHARE ID SEMI
		{
			$$ = new hijackContext($2);
			$$->kind |= event::programCodeKind;
		}
	| _FOREACH ID _IN '(' argvalue ')' stmt
		{
#ifdef RSL_DEBUG_PARSE
			cout << "\nrecognized foreach <id> in <argvalue> <stmt>\n"
				<< flush;
#endif

			$$ = new foreach($2, $5, $7);
			$$->kind |= event::programCodeKind;
		}
	| ID ID REFERENCE_OP argvalue SEMI
		{
			$$ = new valdecl($1, $2, $4);
			$$->kind |= event::programCodeKind;
		}
	| ID ID ASSIGN_OP argvalue SEMI
		{
			$$ = new valdecl($1, $2, $4);
			$$->kind |= event::programCodeKind;
		}
	;


expr: argvalue BINCOMPARE_OP argvalue
		{	$$ = new BinaryRequest($1, $2, $3, BinaryRequest::biCompare, BGROUPOPEXPR);
			$$->kind |= event::programCodeKind;
		}
	| argvalue BINLTGT_OP argvalue
		{	$$ = new BinaryRequest($1, $2, $3, BinaryRequest::biCompare, BGROUPOPEXPR);
			$$->kind |= event::programCodeKind;
		}
	| argvalue ADD_OP argvalue
		{
			$$ = new BinaryRequest($1, $2, $3, BinaryRequest::biMethod, BGROUPOPEXPR);
			$$->kind |= event::programCodeKind;
		}
	| argvalue MUL_OP argvalue
		{	$$ = new BinaryRequest($1, $2, $3, BinaryRequest::biMethod, BGROUPOPEXPR);
			$$->kind |= event::programCodeKind;
		}
	| argvalue ASSIGN_OP argvalue
		{
			$$ = new BinaryRequest($1, $2, $3, BinaryRequest::biMethod, BGROUPOPEXPR);
			$$->kind |= event::programCodeKind;
		}
	| argvalue REFERENCE_OP argvalue
		{
			$$ = new BiRefRequest($1, $3, BGROUPOPEXPR);
			$$->kind |= event::programCodeKind;

			// check some left side semantic validities..
			if ( !( $1->isA(event::objReqArgKind))
				&& !($1->isA(event::requestArgKind)) )
			{
				CURRCONTEXTP->berror("operator `<-' requires a named object on the left.");
			}
		}
	| term
		{ $$ = $1; }
	| NOT_OP argvalue
		{
			$$ = new BinaryRequest($2, "not", NULL, BinaryRequest::biMethod, BGROUPOPEXPR);
			$$->kind |= event::programCodeKind;
		}

	| argvalue AT_OP method_name '(' alist ')'
		{
			$$ = new BinaryRequest($1, $3, $5, BinaryRequest::biMethod, BGROUPOPEXPR);
			$$->kind |= event::programCodeKind;
		}
	;


term: objreq
		{
			$$ = $1;
			if ($$)
				$$->kind |= event::dataMemberReqKind;	// mark as a data member request
		}
	| methodreq '(' alist ')'
		{
			$1->arguments = $3;
			$$ = $1;
		}
	| ID '[' alist ']'
		{
			$$ = new Request($1, (char *) "[]", $3);
			$$->kind |= event::programCodeKind;
		}
	| '(' expr ')'
		{
			$$ = new eventGroup($2);
			$$->kind |= event::programCodeKind;
			if (Running)
				Remember($$);
		}
	;

methodreq: ID
		{
			// Implicit request to object `self'.
			$$ = new Request("self", $1, (elist *) NULL);
			$$->kind |= event::programCodeKind;
		}
	| objreq
	;
	
objreq: ID '.' { CURRCONTEXT.SetKeyState(lexer_context::InExpr); } method_name
		{
			// turn keyword recognition back on!
			CURRCONTEXT.SetKeyState(lexer_context::Everywhere);

			// object . datamember
			$$ = new Request($1, $4, (elist *) NULL);
			$$->kind |= event::programCodeKind;
		}
	;


alist:	{
			$$ = (elistArg *) NULL;	// no arguments !
		}
	| ar
		{
#ifdef RSL_DEBUG_PARSE
			cout << "..CREATE an argument list (elist)..\n";
#endif
			$$ = new elistArg;
			$$->add($1);
			if (Running)
				Remember($$);
			else
				$$->kind |= event::programCodeKind;
		}
	| alist ',' ar
		{
			if ($1)
				$1->add($3);
			$$ = $1;
		}
	;

ar: argvalue
		{
			$$ = $1;
			$$->argName = UNNAMED_ARG;	// is the default, but safer to set.
		}
	| ID ':' argvalue
		{
			$$ = $3;
			$$->argName = $1;	// name the argument
		}
	;

argvalue: literal
		{
			$$ = new ResArg($1);
			if (Running)
				Remember($$);
			else
				$$->kind |= event::programCodeKind;
		}
	| ID
		{
			$$ = new ObjRequestArg($1);
			if (Running)
				Remember($$);
			else
				$$->kind |= event::programCodeKind;
		}
	| ID '{' alist '}'
		{
			$$ = new ListArg($1, $3);	// type, contents.
			if (Running)
				Remember($$);
			else
				$$->kind |= event::programCodeKind;
		}
	| expr
		{
			$$ = new RequestArg($1);
			if (Running)
				Remember($$);
			else
				$$->kind |= event::programCodeKind;
		}
	;
	
literal: QSTRING
		{
//			$$ = new R_String($1);
			$$ = R_String::New("<parse>", $1);
			((R_String *) $$)->ConvertEscapes();
			nProgramResources++;
		}
	| QCHAR
		{
//			$$ = new R_String($1);
			$$ = R_String::New("<parse>", $1);
			nProgramResources++;
		}
	| INT
		{
//			$$ = new R_Integer(atoi($1));
			$$ = R_Integer::New("<parse>", atoi($1));
			nProgramResources++;
		}
	| FLOAT
		{ $$ = new R_String("<parse>", $1);	//	R_Float($1);
			nProgramResources++;
		}
	| HEXVAL
		{ $$ = new R_String("<parse>", $1);	//	R_Integer($1, hexify);
			nProgramResources++;
		}
	| _BOOLTRUE
		{
//			$$ = new R_Boolean(1);
			$$ = R_Boolean::New("<parse>", 1);
			nProgramResources++;
		}
	| _BOOLFALSE
		{
//			$$ = new R_Boolean(0);
			$$ = R_Boolean::New("<parse>", 0);
			nProgramResources++;
		}
	;

	
%%

#include <fstream.h>
#include "lexer_context.h"

int parse_it(lexer_context& lexc)
{
	lexc.nerrors = 0;
	lexc.mylineno = 0;

//	while(lexc.lexable())
//		yyparse();

	while(lexc.lexable())
		yyparse( (void *) &lexc);

	return lexc.nerrors;
}


