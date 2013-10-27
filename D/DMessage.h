// DMessage.h contains the classes
// DR_Message and DO_Message
//

/*******************************************
 the message 
 *******************************************/

#include "D.h"

#ifndef _D_Message_
#define _D_Message_

#define _D_Message_ID 738334323

class DO_Message;


// a DR_Object:DRef class - a strongly-typed smart pointer
class DR_Message : public DR_Object {
public:
	DR_Message (D *d=0);
	DR_Message (const DRef& ref);
	virtual ~DR_Message();

	DO_Message *const_get();
	DO_Message *safe_get();
	DO_Message *safe_set(D* d);
	DO_Message *New();

	inline DO_Message *operator->() { return safe_get(); }
	DRef operator()(const char* argName);
	
/* 	DRef operator[](int i) { return safe_get()->data[i]; } */
	
};

// a DO_Object class - the guts
class DO_Message : public DO_Object {
  public:
	//DR_Dictionary data;
	DRef dataDict;	// points to a DO_Dictionary in implementation

	unsigned int messageCode;
	DR_String message;

	DO_Message();
	DO_Message(DRef r); // casting constructor
	virtual ~DO_Message();

	static DRef Messageclass; // points to a DO_Message
	static DR_Message New();
	DRef Class();

	//DRef route(DR_Message m);
	void init();
	void destroy();
	DR_String toString();
	
	DRef get(const char* argName);
	void add(const char* argName, const DRef& obj);
	inline void setData(const DRef& d) { dataDict.replace(d.unsafe_get()); }

	DO_Object *elements();
	

/* 	friend DO_Message *DR_Message::New(); */
};
	
#endif

