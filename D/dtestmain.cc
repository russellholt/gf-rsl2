#include "D.h"
#include "DString.h"
#include "DList.h"
#include "DDictionary.h"
#include "DClass.h"

#include "Dcomparator.h"

#include "DZ.h"

#include "DSortedBinaryTree.h"
#include "DECI_OStream.h"

#include <iostream.h>

#include <stdio.h>


#ifdef PURIFY
#include "purify.h"
#endif

main()
{
	DR_String s = "assign in constructor";
	DR_String t = "another string";
	
	s->prepend("hey: ");
	
	cerr << s << endl;
	
	DR_List l;
	
	l->append(s);
	l->append(t);

	cerr << "The list is of length [" << l->size() << "]\n";

	if (l.isValid())
	{
			DR_Enumerator dre = l->elements();

			while (dre->hasMoreElements())
			{
				cerr << DR_String(dre->nextElement()) << endl;
			}
	}

	cerr << "Dictionary testing: adding keys 'abc', 'def'\n";
	
	DR_Dictionary dict;
	
	dict->cc_add("abc", s);
	dict->cc_add("def", t);

	cerr << "the dictionary is of size [" << dict->size() << "]\n";
	
	cerr << "'def' is: " << dict->cc_get("def") << endl;
	cerr << "'abc' is: " << dict->cc_get("abc") << endl;

	cerr << "Creating a message & adding dictionary.\n";
	
	DR_Message m;
	m->setData(dict);
	
	cerr << "Enumerating the message's dictionary:\n";
	DR_KeyedEnumerator di(m->elements());
	while (di->hasMoreElements())
	{
		cerr << "key: `" << di->nextKey() << "' = `" << di->nextElement() << "'\n";
	}
	
	
	cerr << endl << "hash of `String' is " << theIDHash("String") << endl;
	cerr << "t's dtypeid() is " << t->dtypeid() << endl;
	
	cerr << "Analyzing the class object of t (`" << t << "')\n";
	DR_Class cl = t->Class();
	if (cl.isValid())
		cerr << "t's class object says it is a: `"<< cl->className() << "'\n";
	else
		cerr << "t has no valid class object ???\n";
	
	
	cerr << "Analyzing the class object of the dictionary\n";
	DR_Class cld = dict->Class();
	if (cld.isValid())
		cerr << "dict's class object says it is a: `"<< cld->className() << "'\n";
	else
		cerr << "dict has no valid class object ???\n";
	
	
	cerr << "Analyzing the class object of the List\n";
	DR_Class cll = l->Class();
	if (cll.isValid())
		cerr << "l's class object says it is a: `"<< cll->className() << "'\n";
	else
		cerr << "l has no valid class object ???\n";
	

	DR_List list1, list2;
	DR_String str1 = "a string";
	list2->append(str1);
	list1->append(list2);
	list2->append(list1, secondary_ref);

	DR_Z z1, z2;
	z1.New();
	z2.New();
	z1->s1 = "h";
	z2->s1 = "w";

	DR_comparator drc;
	drc << "s2";

	dcompare_t type = drc->compare(z1, z2);
	cerr << "Comparison on 's2' = " << (int)type << ".\n";

	drc->clearNames();
	drc << "s1" << "s2";
	type = drc->compare(z1, z2);
	cerr << "Comparison on 's1' = " << (int)type << ".\n";

	DR_Z z3, z4, z5, z6, z7, z8, z9, z10;
	z3->s1 = "a";
	z4->s1 = "b";
	z5->s1 = "Z";
	z6->s1 = "Y";
	z6->s2 = "a";
	z7->s1 = "X";
	z8->s1 = "c";
	z9->s1 = "e";
	z10->s1 = "f";

	DR_ECI_OStream dout;
	DR_List alist;

	/**********
	DR_SortedBinaryTree sbt;
	sbt->add(z7, drc);
	sbt->add(z1);
	sbt->add(z2);
	sbt->add(z8);
	sbt->add(z6);
	sbt->add(z10);
	sbt->add(z3);
	sbt->add(z5);
	sbt->add(z9);
	sbt->add(z4);
	cerr << "in order: " << dout->collection(sbt->root->inOrder()) << endl;
	***********/

	alist->add(z7);
	alist->add(z1);
	alist->add(z2);
	alist->add(z8);
	alist->add(z6);
	alist->add(z10);
	alist->add(z3);
	alist->add(z5);
	alist->add(z9);
	alist->add(z4);

	cerr << "unsorted: " << dout->stream(alist) << endl;
	dout->destroy();
	dout->init();
	alist->sort(drc);
	cerr << "  sorted: " << dout->stream(alist) << endl;

	DR_Composite athing;

	athing->add(DR_String("ack"), DR_String("what"));

	dout->destroy();
	dout->init();
	cerr << "\na thing: " << dout->stream(athing) << endl;


#ifdef PURIFY
	purify_printf("-- End of program --\n");
#endif


}

