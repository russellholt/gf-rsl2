#include "drwcstring.h"

#ifndef _STRING_PAIR_H_
#define _STRING_PAIR_H_ 1

class StringPair {
protected:
    DRWCString l, r;

public:
    // ****************
    // * Constructors *
    // ****************
    StringPair(void) { }

    StringPair(char *left, char *right)
    { l = left; r = right; }

    StringPair(RWCString left, RWCString right)
    { l = left; r = right; }

    StringPair(DRWCString left, DRWCString right)
    { l = left; r = right; }

    StringPair( const StringPair& strpr)
    { l = strpr.l; r = strpr.r; }

    // *************
    // * Operators *
    // *************
    StringPair& operator=(const StringPair& strpr)
    { l = strpr.l; r = strpr.r; return *this; }

    int operator==(const StringPair& strpr)
    { return (( l == strpr.l) && (r == strpr.r)); }

    // double set
    void operator() (char *left, char *right)
    { l = left; r = right; }

    void operator() (RWCString left, RWCString right)
    { l = left; r = right; }

    void operator() (DRWCString left, DRWCString right)
    { l = left; r = right; }

    // **************************
    // * Public Member Function *
    // **************************
    void SetLeft(char *str)
        { l = str; }
    void SetLeft(RWCString left)
        { l = left; }
    void SetLeft(DRWCString left)
        { l = left; }

    void SetRight(char *str)
        { r = str; }
    void SetRight(RWCString right)
        { r = right; }
    void SetRight(DRWCString right)
        { r = right; }

    DRWCString& left(void)
        { return l; }
    DRWCString& right(void)
        { return r; }
    
};

#endif
