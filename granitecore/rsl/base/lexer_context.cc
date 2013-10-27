#include <stdlib.h>
#include "lexer_context.h"

int lexer_context::total_lexers=0;

lexer_context::lexer_context(void)
{
	Init();
}

lexer_context::~lexer_context()
{
	destroyLexerAllocs();
}

void lexer_context::Init(void)
{
	done = Ready;
	type = interactive;
	keystate = Everywhere;
	dynamic_request = NULL;
	SetSource(None);
	bGroupOpExpr = 0;
	
	lexer_number = total_lexers++;
}

void lexer_context::SetKeyState(keyword_t newval)
{
	keystate = newval;
}

void lexer_context::SetLexerIO(istream& in, ostream& out)
{
		switch_streams(&in, &out);
}

int lexer_context::lexable(void)
{
#ifdef RSL_DEBUG_PARSE
	cerr << "lexer_context #" << lexer_number << " lexable: "
		<< (done == Ready && yyin && !yyin->eof()) << endl << flush;
#endif

	return (done == Ready && yyin && !yyin->eof());
}

// _blex()
// wrapper function for bison to call with an argument
//int lexer_context::_blex(RSL_YYSTYPE *blval_v)
//{
//	yystype_v = blval_v;
//	return yylex();
//}


int lexer_context::berror(char *msg)
{
	nerrors++;
	
	if (!yyin->eof())
	{
		cerr << filename << ", line " << mylineno << ": "
			 << msg << " at or before symbol `"
			 << (YYText()) << "'\n";
	}

	if (nerrors > 5)
	{
		cerr << Filename() << ": Too many errors, unable to recover.\n\n";
		done = Done;
	}

	return 1;
}

// destroyLexerAllocs()
// frees string memory allocated in the lexer
void lexer_context::destroyLexerAllocs(void)
{
	while(!lexer_allocs_to_free.isEmpty())
	{
		char *c = lexer_allocs_to_free.get();
		if (c)
		{
//			cerr << "\"" << c << "\"  ";
			free(c);
		}
	}
	cerr << endl;

	while(!lexer_allocs_to_delete.isEmpty())
		delete lexer_allocs_to_delete.get();

	while(!lexer_allocs_to_arraydelete.isEmpty())
		delete[] lexer_allocs_to_arraydelete.get();
}

int lexer_context::is_request_pending (void)
{
    int nchars = 0;
    
    
    if (!yy_init)
    {
        nchars = yy_n_chars;
    }
    return (nchars);
}


// *************************************************************************
// NOTE about method KeywordState() in the .h
//
// SMARTKEYWORDS - the lexer will recognize certain keywords only when the 
// parser is in a state where that keyword is valid.  KeywordState(tok,val) 
// specifes that token 'tok' should be returned only if lexer_keystate does 
// NOT contain 'val', where 'val' is the logical OR of those parser states 
// (defined in lexer_context.h) we want to ignore keywords, which means, 
// consider it an identifier if the parser is in the state given (and return 
// ID), otherwise consider it a keyword (and return 'tok').  Eg, it's ok to 
// name a method "class" or "while" if SMARTKEYWORDS is defined, (since it is 
// illegal to use the keyword "class" inside a class definition or inside a 
// method body), otherwise, a syntax error will result.
// *************************************************************************
