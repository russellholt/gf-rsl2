#include <rw/cstring.h>
#include <rw/ctoken.h>
#include <rw/tvhdict.h>
#include <rw/tvslist.h>
#include <stdlib.h>

#include "RegistrationList.h"
#include "destiny.h"

/**
 * Implementation of the list of registered brokers.  Assigns an
 * identifier to each registered broker.
 *
 * $Id: RegistrationList.cc,v 1.1 1998/11/17 23:42:59 toddm Exp $
 */

/**
 * Constructs a list based on a string of broker records.
 *
 * Params:
 *    list - list of broker's, e.g., "B1=h1:100,B2=h2:38"
 * Throws:
 *    IllegalArgumentException
 */
RegistrationList::RegistrationList (const RWCString& list):
    _nextId (1), _listById (RWCString::hash), _listByAddr (NetAddress::hash)
{
    RegistrationList ();
    // Fill with real brokers.
    RWCTokenizer st (list);
    for (RWCString id = st ("=,"); !id.isNull (); id = st ("=,"))
    {
        // Get id and host:port and add to lists.
        RWCString strNetAddress = st ("=,");
        if (strNetAddress.isNull ())
	{
	    throw RWCString (
	      "IllegalArgumentException: Missing host and port");
	}
	_listById.insertKeyAndValue (id, NetAddress (strNetAddress));
	_listByAddr.insertKeyAndValue (NetAddress (strNetAddress), id);
    }
}

/**
 * Adds a broker to the list if it is not yet
 * registered.
 *
 * Params:
 *    address - the broker's network address
 * Returns:
 *    the newly assigned broker ID
 */
RWCString RegistrationList::add (const NetAddress& address)
{
    if (_listByAddr.contains (address))
    {
        return _listByAddr [address];
    }
    RWCString id = newId ();
    _listById.insertKeyAndValue (id, address);
        _listByAddr.insertKeyAndValue (address, id);
    return id;
}

/**
 * Adds a named broker to the list if it is not yet
 * registered.
 *
 * Params:
 *    bId - the broker's name
 *    address - the broker's network address
 */
void RegistrationList::add (const RWCString& bId, const NetAddress& address)
{
    if (_listByAddr.contains (address))
    {
        return;
    }
    _listById.insertKeyAndValue (bId, address);
    _listByAddr.insertKeyAndValue (address, bId);
}

/**
 * Removes the specified broker from the registry.
 *
 * Params: bId the broker's id
 * Returns: true if success, false on failure.
 */
RWBoolean RegistrationList::remove (const RWCString& bId)
{
    if (!_listById.contains (bId))
    {
        return FALSE;
    }
    NetAddress address = _listById [bId];
    _listById.remove (bId);
    _listByAddr.remove (address);
    return TRUE;
}

/**
 * Gets a randomized Broker list in string format,
 * e.g., "B1=vision.destinyusa.com:2345,B2=..."
 *
 * Returns: the string
 */
RWCString RegistrationList::toString ()
{
    RWTValSlist<RWCString> ids;
    RWTValHashDictionaryIterator<RWCString,NetAddress> iter (_listById);
    for (; iter (); )
    {
        ids.insert (iter.key ());
    }
    RWCString s (""), id;
    while (ids.entries () != 0)
    {
        int j = (int) (rand () % ids.entries ());
	if (s.compareTo ("") != 0)
	    s = s + ",";
	id = ids [j];
	s = s + id + "=" + _listById[id].toString ();
	ids.removeAt (j);
    }
    return s;
}

/**
 * Used for iterating through the address list.
 * Very inefficient (n^2)!
 *
 * Params:
 *    the index
 * Returns:
 *    the net address.
 */
NetAddress RegistrationList::operator [] (const int i)
{ 
    RWTValHashDictionaryIterator<NetAddress,RWCString> iter (_listByAddr);
    for (int count = 0; iter(); count++)
        if (i == count)
	    return iter.key ();
    return NetAddress ("unknown:0");
}

//--------------------------- TESTING ------------------------------

#ifdef _RL_MAIN

/**
 * For unit testing only!
 *
 * Usage:  java RegistrationList
 */
void case1 ()
{
    RegistrationList list;
    while (TRUE)
    {
        RWCString line;
        try
        {
            cout << "Enter host:port" << endl;
	    cin >> line;
	    RWCString id = list.add (NetAddress (line));
	    cout << id << endl << list.toString () << endl;
        }
        catch (RWCString s)
        {
            cerr << s << endl;
        }
    }
}

void case2 ()
{
    while (TRUE)
    {
        RWCString line;
        try
        {
            cout << "Enter list";
	    cin >> line;
	    RegistrationList list (line);
	    cout << list.toString () << endl;
        }
        catch (RWCString s)
        {
            cerr << s << endl;
        }
    }
}

int main (int argc, char *argv[])
{
    switch (argc) {
    case 1:
        case1 ();
	break;
    case 2:
        case2 ();
	break;
    default:
        cerr << "usage: " << argv[0] << " [reglist]";
	break;
    }
}

#endif
