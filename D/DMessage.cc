#include "DMessage.h"

#include "DDictionary.h"

#include "DClass.h"

// ************************
// * DR_Message
// ************************



DR_Message::DR_Message(D *d) : DR_Object(d)
{

}

DR_Message::DR_Message(const DRef& ref) : DR_Object(ref)
{

}

// If you redefine dump() or Recycle(), you must
// call dump() from this destructor because DRef::dump(), invoked
// from DRef::~DRef(), may call Recycle() - and destructors do not
// call functions in derived classes. otherwise, don't call it.
DR_Message::~DR_Message()
{

}

// const_get()
// get the object as the correct type, but without the potential
// to modify it if it is null.
DO_Message *DR_Message::const_get()
{
	return dynamic_cast<DO_Message *> (unsafe_get());
}

// safe_get()
// this method is how operator->() is implemented; 
// the purpose is to get the object, correctly typed,
// in a way that can be directly dereferenced witout checking.
// Because it is possible for the object to be null, safe_get()
// will create it by calling New(). This is the correct behavior
// 99% of the time. If it is not, use either const_get() or
// look in D_macros.h to find the actual code. It's possible that
// throwing an exception is more appropriate in some cases instead
// of creating a new object.
//
// Note that this is not a virtual function.
DO_Message* DR_Message::safe_get()
{
	SAFE_GET(DO_Message);	// D_macros.h
}

// rarely used.
// _set() when safe_get() is used avoids double checking.
DO_Message* DR_Message::safe_set(D* d)
{
	SAFE_SET(DO_Message,d);	// D_macros.h
}

DO_Message *DR_Message::New()
{
	DO_NEW(DO_Message);
}

DRef DR_Message::operator()(const char* argName)
{
	return safe_get()->get(argName);
}

// ************************
// * DO_Message
// ************************

// C++ constructor
// use init() for individual object initialization
DO_Message::DO_Message()
{

}

// C++ destructor
// use destroy() for individual object de-construction
DO_Message::~DO_Message()
{
}

// init(): the "constructor"
void DO_Message::init()
{
	// initialize superclass first
	DO_Object::init();
	
	// when we become a DO_Composite subclass...
	//	add("messageName", messageName.New());
	//	add("data", data.New());

}

// destroy(): the "destructor"
void DO_Message::destroy()
{
	message.dump();
	dataDict.dump();

	// destroy superclass last
	DO_Object::destroy();
}

DR_String DO_Message::toString()
{
	return message;
}


DRef DO_Message::get(const char* argName)
{
// used to be that what is now 'dataDict' was declared as
// DR_Dictionary data;
// and get() was:
//	return data->cc_get(argName);

	// If the dictionary exists, that is, if dataDict points
	// to something...
	if (dataDict.isValid())
		return DR_Dictionary(dataDict)->cc_get(argName);
	else
		dataDict = DO_Dictionary::New();

	// otherwise, make dataDict point to a DO_Dictionary
	// and we know it doesn't contain anything so just return!

	return DR_null;
}

void DO_Message::add(const char* argName, const DRef& obj)
{
// used to be that what is now 'dataDict' was declared as
// DR_Dictionary data;
// and add() was:
//	data->cc_add(argName, obj);

	if (!dataDict.isValid())
		dataDict = DO_Dictionary::New();

	DR_Dictionary(dataDict)->cc_add(argName, obj);
}


DO_Object *DO_Message::elements()
{
	if (!dataDict.isValid())
		dataDict = DO_Dictionary::New();

	return DR_Dictionary(dataDict)->elements();
}

// ******************************************
// route()
//
// the message-oriented interface.
// Transforms messages into function calls.
// ******************************************
/*
DRef DO_Message::route(DR_Message m)
{
	switch(m.messageID())
	{


		default: ;
	}
	return DO_Object::route(m);
}
*/

class DOC_Message : public DO_Class {
	D *spawn();

  public:
	DOC_Message() : DO_Class("Message") { }

};

DRef DO_Message::Messageclass = new DOC_Message();


D *DOC_Message::spawn() {
	return new DO_Message();
}

DRef DO_Message::Class()
{
	return Messageclass;
}

// New()
// Create a new DO_Message by asking for one from
// the static class object, DO_Message::Messageclass.
// That object will either create one, by calling the virtual
// function spawn(), that is, DOC_Message::spawn(),
// or getting one from its list of free objects.
//
// A static function
DR_Message DO_Message::New()
{
	return DR_Class(DO_Message::Messageclass)->New();
}

/*
// Create_DOC_Message()
// not currently in use as of Oct 1, 1998
// It will be used as the "bootstrap" function
// to create the class object, which will in turn
// create and recycle instances of DO_Message
extern "C" DRef& Create_DOC_Message()
{
	DO_Message::Messageclass = new DOC_Message();
	return DO_Message::Messageclass;
}

*/
