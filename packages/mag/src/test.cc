#include <iostream.h>
#include "DDecimal.h"
#include "DDate.h"
#include "DTime.h"
#include <DInt.h>
int main(int argc, char *argv[])
	{
	cerr << "**************Decimal Test****************"  << endl;

	DR_Decimal dec1(4.3);
	DR_Decimal dec2 (3.2);

	dec1 = DR_null;
	dec1 = 3.32;
	dec2 = dec1;
	dec1 = DR_String("$5,456.67");
	dec2 = 4;
	dec2 = DR_String("2.2");
	cerr << "dec1: " << dec1->toString() << ", dec2: " << dec2->toString() <<
		", sum: " << (dec1 + dec2)->toString() << endl;

	if (dec1.isValid())
		cerr << "dec1 is null"  << endl;
	else
		cerr << "dec1 is NOT null"  << endl;

	dec1 = DR_null;
	if (dec1.isValid())
		cerr << "dec1 is null"  << endl;
	else
		cerr << "dec1 is NOT null"  << endl;
	cerr << "**************Date Test****************"  << endl;
	DR_Date d1;
	d1 = "10/23/98";
	DR_Date d2 = "10/25/2000";
	cerr << d1.toString() << ", " << d2.toString() << endl;
	if (d1 < d2)
		cerr << "less than" << endl;
	DR_Date d3;
	d3 = DR_Date("11/12/1999");
	cerr << "d3:" << d3.toString() << endl;
	DR_Date d4 = DR_Date("12/12/1997");
	cerr << "d4:" << d4.toString() << endl;
	d4 = "12/28/97";
	cerr << "d4: " << d4.toString() << endl;

	cerr << "**************Time Test****************"  << endl;
	DR_Time t1;

	DR_Time t2;
	cerr << t1->toString() << ", " << t2->toString() << endl;
	if (t1 < t2)
		cerr << "less than" << endl;
	}
