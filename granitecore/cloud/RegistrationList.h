#include <rw/cstring.h>
#include <rw/tvhdict.h>
#include "Log.h"
#include "NetAddress.h"

#ifndef _RegistrationList_H_
#define _RegistrationList_H_

/**
 * The list of registered brokers.  Assigns an identifier
 * to each registered broker.
 *
 * $Id: RegistrationList.h,v 1.1 1998/11/17 23:44:21 toddm Exp $
 */
class RegistrationList {

  //--------------------------- PUBLIC -------------------------------
  public:

    /**
     * Constructs a blank list.
     */
    RegistrationList (): _nextId (1), _listById (RWCString::hash),
			 _listByAddr (NetAddress::hash) {}

    /**
     * Constructs a list based on a string of broker records.
     *
     * Params:
     *    list - list of broker's, e.g., "B1=h1:100,B2=h2:38"
     * Throws:
     *    IllegalArgumentException
     */
    RegistrationList (const RWCString& list);

    /**
     * Adds a broker to the list if it is not yet
     * registered.
     *
     * Params:
     *    address - the broker's network address
     * Returns: 
     *    the newly assigned broker ID
     */
    RWCString add (const NetAddress& address);

    /**
     * Adds a named broker to the list if it is not yet
     * registered.
     *
     * Params:
     *    bId - the broker's name
     *    address - the broker's network address
     */
    void add (const RWCString& bId, const NetAddress& address);

    /**
     * Removes the specified broker from the registry.
     *
     * Params:
     *    bId - the broker's id
     * Returns:
     *    true if success, false on failure
     */
    RWBoolean remove (const RWCString& bId);

    /**
     * Gets a randomized Broker list in string format,
     * e.g., "B1=vision.destinyusa.com:2345,B2=..."
     *
     * Returns:
     *    the string
     */
    RWCString toString ();

    /**
     * Finds the network address of the specified broker.
     * 
     * Params: 
     *    id - the broker's id
     * Returns:
     *    the host and port
     */
    NetAddress lookup (const RWCString& id)
        { return _listById [id]; }

    /**
     * Determines if the list is empty.
     *
     * Returns:
     *    true if list empty, false otherwise
     */
    RWBoolean isEmpty () const { return _listById.isEmpty (); }

    /**
     * Gets the list size.
     *
     * Returns:
     *    the size
     */
    int size () const { return _listById.entries (); }

    /**
     * Used for iterating through the list of net addresses.
     * Very inefficient (n^2)!
     *
     * Params:
     *    i - the index
     * Returns:
     *    the net address
     */
    NetAddress operator [] (const int i);

  //--------------------------- PRIVATE ------------------------------
  private:

    /**
     * Creates a new broker identifier.
     *
     * Returns: the identifier
     */
    RWCString newId () { char strId [10];
                         sprintf (strId, "B%d", _nextId++); 
			 return RWCString (strId); }

    /**
     * A map from ID's to network addresses.
     */
    RWTValHashDictionary<RWCString,NetAddress> _listById;
    
    /**
     * A map from network addresses to ID's.
     */
    RWTValHashDictionary<NetAddress,RWCString> _listByAddr;
    
    /**
     * Id counter.
     */
    int _nextId;
};

#endif
