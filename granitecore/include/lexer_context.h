// lexer_context.h
// A lexer/parser context
//
// Encapsulates various states of the lexer/parser, such as:
//     input/output streams
//     type of source (stream, file)
//        filename (if applicable)
//     type of lexer (batch/interactive)
// etc.
//
// Meant for use with bison 1.24 and flex++ 2.5.2
//
// $Id: lexer_context.h,v 1.1 1998/11/18 00:01:17 toddm Exp $

// *******************
// * System Includes *
// *******************
#include <iostream.h>
#include <fstream.h>
#include <rw/cstring.h>
#include <FlexLexer.h>
#include <rw/tpslist.h>

// ******************
// * Local Includes *
// ******************

#ifndef _LEXER_CONTEXT_H_
#define _LEXER_CONTEXT_H_

class event;
class rslMethod;

// RSL_YYSTYPE is void because I can't have a pointer to a YYSTYPE here.
// Casting access macro RSL_BLVAL defined in b.l
#define RSL_YYSTYPE void

class lexer_context : public yyFlexLexer {
	RWCString filename;
	RWTPtrSlist<char> lexer_allocs_to_free,
		lexer_allocs_to_delete,
		lexer_allocs_to_arraydelete;
		
public:
	enum lex_t { batch=0, interactive=1, prompt=2 };
	enum keyword_t { Nowhere=0, Everywhere=1, InClass=2, InExpr=4,
		InComm=8, Numeric=16 };
	enum source_t { None=0, Stream, File };
	enum done_t { Ready=0, Done=1, GotECIReq=2, GotECIMethod=3 };

	//-----------------------------------------------------------
	int nerrors, mylineno;
	done_t done;
	keyword_t keystate;
	lex_t type;
	int bGroupOpExpr;
	
	int lexer_number;
	static int total_lexers;
	
	event *dynamic_request;
	rslMethod *dynamic_method;
	RWCString last_comment, last_global_comment;
	//-----------------------------------------------------------

	lexer_context(void);
	~lexer_context();

	void Init(void);
	void SetKeyState(keyword_t newval);
	int lexable(void);

	inline RWCString Filename() const { return filename; }
	inline source_t Source() const { return source; }
	void SetLexerIO(istream& in=cin, ostream& out=cout);

	inline void SetSource(source_t src, RWCString fname ="")
		{ source = src; filename = fname; }

	inline void ClearDynamic()
		{ dynamic_request = NULL; dynamic_method = NULL; }

	inline void insertAlloc_free(char *data)
		{ lexer_allocs_to_free.insert(data); }
	inline void insertAlloc_delete(char *data)
		{ lexer_allocs_to_delete.insert(data); }
	inline void insertAlloc_arraydelete(char *data)
		{ lexer_allocs_to_arraydelete.insert(data); }
	void destroyLexerAllocs(void);

        int is_request_pending (void);
	int berror(char *msg);
	int blex(RSL_YYSTYPE *blval_p);	// implemented by flex (lex.yy.cc)
	
// 	inline ostream& Out() const { return yyout? (*yyout) : cout; }
 	inline ostream& Out() const 
        { 
            if (yyout)
                return (*yyout);
            else
                return (cout); 
        }

//	inline istream& In() const { return yyin? (*yyin) : cin; }
	inline istream& In() const 
        { 
            if (yyin)
                return (*yyin);
            else
                return (cin); 
        }

private:
	source_t source;

};

// reentrant parameter access for yyparse() and yylex() for
// bison and flex.
#define CURRCONTEXTP ((lexer_context *) currentContext)
#define CURRCONTEXT (*CURRCONTEXTP)

#endif



