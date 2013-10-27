// R_Table.cc
// $Id: R_Table.cc,v 1.1 1998/11/17 23:47:06 toddm Exp $
#include <stdlib.h>

#include "R_Table.h"
#include "R_Integer.h"
#include "R_String.h"
#include "slog.h"
#include "destiny.h"
#include "rsldefaults.h"
#include "rw_utils.h"

static char rcsid[] = "$Id: R_Table.cc,v 1.1 1998/11/17 23:47:06 toddm Exp $";

#define _hADD 6382692   // add
#define _hSETDELIMITER 2138929243   // setDelimiter
#define _hFIND 1718185572   // find
#define _hLOADFILE 705039617    // loadFile


// R_Table static member
rc_Table R_Table::rslType("Table");

extern "C" res_class *Create_Table_RC()
{
	return &(R_Table::rslType);
}

// Spawn - create a new resource of this type (R_Table)
// Called by res_class::New() if there is no object
// to pull off the free list.
Resource* rc_Table::spawn(RWCString nm)
{
	return new R_Table(nm);	// or other constructor
}

// New
// static convenience function
// See R_String::New(), R_Integer::New(), etc.
// If no functionality beyond res_class::New() is required
// (ie, no special values to be set conveniently), then this
// function simply eliminates the need to cast the result
// of res_class::New().
R_Table *R_Table::New(RWCString nm)
{
	Resource *r= R_Table::rslType.New(nm);

	return (R_Table *) r;
}

// R_Table constructor
R_Table::R_Table(RWCString nm)
//	: Resource(nm), ResContext(nm.data(), BUCKETS_IN_NAMESPACE)
	: ResStructure(nm.data(), nm.data(), BUCKETS_IN_NAMESPACE)
{
	delimiter = "\t";
}

R_Table::~R_Table()
{
	
}

// SetFromInline()
// Overrides ResStructure::SetFromInline()
// Difference being that if a named resource (member of the
// rvalue, that is, a key/value) is  not found in the
// lvalue (this), it is added (otherwise, assigned).
void R_Table::SetFromInline(RWTPtrSlist<Resource>& inliner)
{
	RWTPtrSlistIterator<Resource> iter(inliner);
	Resource *r=NULL;

	while(iter())
	{
		r = iter.key();
		ResStatus stat = locals.Find(r->Name());
		if (stat.status == ResStatus::rslOk)
			stat.r->Assign(r);
		else
			locals.AddResource(r);
	}
}


// LoadFile()
// C++ interface: load table from a file.
void R_Table::LoadFile(RWCString fname, RWCString rowclass)
{
	// Open & read the table's file. filename is first argument.
	ifstream f(fname);	
	RWTValSlist<RWCString> lines;
	f >> lines;
	
#ifdef DEBUG
	cout << "first line: `" << lines[0] << "'\n";
#endif

	RWCTokenizer firstline(lines[0]);

	// *****************************************************************
	// * Find the RSL class which represents each row.
	// * the first field of the first line is the name of the RSL class
	// * which models each row, ie, contains a data member named by
	// * column headings.
	// *****************************************************************

	// the name of the class to find can come from one of two places;
	// either it is given as an argument, in which case this name is used,
	// otherwise, it is taken from the first column of the first row.
	RWCString rcName;

	if (rowclass.length() > 0)
		rcName = rowclass;		// use given name for rowclass
	else
		rcName = firstline(delimiter.data());	// parse for identifier (till first tab)

#ifdef DEBUG
	cout << "ResClass name: `" << rcName << "'\n";
#endif

	res_class rcLookup(rcName);
	res_class *rc = ResClasses.find(&rcLookup);

	if (!rc)
	{
		logf->error(LOGRSL)
			<< "Table: Unable to find row class `"
			<< rcName << "'" << endline;
		return;
	}
	
	// ********************************************************************
	// * Create a list of the column headings so that we can
	// * match a cell to a class datamember named by its column heading.
	// ********************************************************************
	RWTValSlist<RWCString> columns;
	RWCString col;
	
	// using the tokenizer, advance each tab-delimited field
	// and insert it into the list.
	while( !(col = firstline(delimiter.data())).isNull())
	{
#ifdef DEBUG
		cout << "column: `" << col << "'\n";
#endif
		columns.insert(col);
	}

	// remove the first line
	lines.remove(lines[0]);

	RWCString s, cell;
	RWTValSlistIterator<RWCString> iter(lines);
	ResStructure *rowObject = NULL;

	while (iter())
	{
		if (iter.key().length() <= 0)
			continue;

		rowObject = (ResStructure *) rc->New("a_row");

		RWCTokenizer next(iter.key());	// tokenize the line
		RWTValSlistIterator<RWCString> colNameIter(columns);

		RWCString rowkey = next(delimiter.data());

#ifdef DEBUG
		cout << "KEY:`" << rowkey << "'-- ";
#endif
		
		while(! (cell = next(delimiter.data())).isNull())
		{
			if (colNameIter())
			{
#ifdef DEBUG
				cout << colNameIter.key() << "<- `" << cell << "'; ";
#endif

				ResReference ref = rowObject->GetDataMember(colNameIter.key());
				if (ref.isValid())
				{
					switch(ref.TypeID())
					{
						case R_String_ID:
							((R_String *) ref())->Set(cell);
							break;

						case R_Integer_ID:
							((R_Integer *) ref())->Set(atoi(cell.data()));
							break;

						default:
							ref->Assign(R_String::New("", cell));	// ResReference::operator->()
					}
				}
			}
		}
		
#ifdef DEBUG
		cout << "\n\tROW OBJECT: ";
		rowObject->print(cout);
		cout << endl << flush;
#endif		

		locals.AddReferenceTo(rowkey, rowObject);

	}

#ifdef DEBUG
	cout << endl << endl << flush;
#endif		
	
}



ResStatus R_Table::execute(int method, ResList& arglist)
{
	switch(method)
	{
		case _hLOADFILE:	// "loadFile"
			return rsl_loadFile(arglist);

		case _hADD:	// "add"
			return rsl_add(arglist);
			
		case _hSETDELIMITER:	// "setDelimiter"
			return rsl_setDelimiter(arglist);

		case _hFIND:	// "find"
			return rsl_find(arglist);

		default: ;
	}

	return ResStatus(ResStatus::rslFail);
}

//	rsl_setDelimiter()
//
//	/** Set the delimiter character for use in subsequent
//		loadFile() messages. */
//	setDelimiter(String delim);
ResStatus R_Table::rsl_setDelimiter(const ResList& arglist)
{
	delimiter = arglist[0].StrValue();

	return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "LoadFile"
//	LoadFile(String filename);
//	LoadFile(String filename, String rowClass);
ResStatus R_Table::rsl_loadFile(const ResList& arglist)
{
	logf->debug(LOGRSL) << "Table: loading from file `"
		<< (arglist[0].StrValue()) << "'" << endline;
	
	LoadFile(arglist[0].StrValue(), arglist[1].StrValue());
	
	return ResStatus(ResStatus::rslOk, NULL);
}


// RSL method "addRow"
//	addRow(String key, row);
ResStatus R_Table::rsl_add(const ResList& arglist)
{
	if (arglist[1].isValid())
		locals.AddReferenceTo(arglist[0].StrValue(), arglist[1].RealObject());
	else
		Logf.error(LOGAPPENV) << "Table::addRow(\"" << (arglist[0].StrValue())
			<< "\", row) FAILED; row object is invalid." << endline;

	return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "find"
//	find(String key);
ResStatus R_Table::rsl_find(const ResList& arglist)
{
	return locals.Find(arglist[0].StrValue());
}
