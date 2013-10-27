#include "R_D.h"
#include "R_List.h"
#include "D.h"
#include "DList.h"
#include "DComposite.h"

extern Resource *_DtoR(DRef dr);

void listcallback(DR_List dlist, R_List *rl)
{
	DR_Enumerator dre = dlist->elements();
	
	while(dre->hasMoreElements())
		rl->append(_DtoR(dre->nextElement()));
}

DR_List rlist2dlist(R_List *rl)
{
	if (!rl)
		return DR_null;

	DR_List dl;

	ResIterator *re = rl->elements();
	if (!re)
	{
		cerr << "rlist2dlist(): ?? Can't create an R_List iterator??\n";
		return DR_null;
	}

	while (re->hasMoreElements())
	{
		dl->add(R_D::RtoD(re->nextElement()));
	}

	delete re;

	return dl;

}

DR_Composite makeDComposite(ResReference ref)
{
	if (!ref.isValid() || !ref->isRSLStruct())
		return DR_null;

	DR_Composite dobj;
	ResStructure *rx = (ResStructure *) (ref());

/*
	// ResIterator is a conceptually better way than using the rogue wave
	// iterators directly, but at the momemnt for some reason it seems
	// to be buggy!
	ResIterator *RI = rx->elements();

	while(RI->hasMoreElements())
*/

	ResReference elem;

	RWTValHashSet<ResReference> *hs = rx->GetLocalContext().GetLocals();
	if (!hs)
		return DR_null;

	RWTValHashTableIterator<ResReference> iter(*hs);
	while(iter())
	{
		//elem = RI->nextElement();
		elem = iter.key();

		if (elem.isValid())
		{
#ifdef DMEMORY
			cerr << "\tmakeDComposite with element ";
			elem.print(cerr);
			cerr << endl;
#endif
		}

		dobj->add(elem.Name().data(), R_D::RtoD(elem));
	}

//	delete RI;

	return dobj;
}

