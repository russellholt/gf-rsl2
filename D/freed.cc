#include "freed.h"

freeManager::freeManager() { }

freeManager::~freeManager() { }

free_list::free_list()
{

}

free_list::~free_list()
{
	inUse.clear();
}


void free_list::add(D *d)
{
	inUse.append(d);
}

D* free_list::get()
{
	if (!inUse.isEmpty())
		return inUse.get();

	return DNULL;
}

size_t free_list::entries()
{
	if (inUse.isEmpty())
		return 0;

	return inUse.entries();
}


// *********************************************************************


free_hashset::free_hashset() : inUse(free_hashset::D_PtrHash)
{
}

free_hashset::~free_hashset()
{
	clear();
}

void free_hashset::add(D *d)
{
	inUse.insert(d);
}

D* free_hashset::remove(D* x)
{
	return inUse.remove(x);
}

D* free_hashset::get()
{
	if (!inUse.isEmpty())
	{
		RWTPtrHashSetIterator<D> iter(inUse);
		if (iter())
			return iter.key();
	}

	return DNULL;
}

size_t free_hashset::entries()
{
	if (inUse.isEmpty())
		return 0;

	return inUse.entries();
}

// a static
unsigned free_hashset::D_PtrHash(const D& d)
{
	return (unsigned) (&d);
}
