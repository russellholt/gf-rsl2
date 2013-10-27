// *******************************************************************************
// *
// * Module Name: CryptoBsafeUtil.cc
// * 
// * Description: 
// *
// * History: Create - CKING 10/30/98
// * 
// * 
// * Copyright 1998 by Destiny Software Corporation.
// * 
// *******************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aglobal.h"
#include "bsafe.h"

/* If the standard C library comes with a memmove() that correctly
 * handles overlapping buffers, MEMMOVE_PRESENT should be  defined
 * as 1, else 0.  The following defines MEMMOVE_PRESENT as 1 if it
 * has not already been defined as 0 with C compiler flags.
 */
#ifndef MEMMOVE_PRESENT
#define MEMMOVE_PRESENT 1
#endif

void CALL_CONV T_memset (POINTER p, int c, unsigned int count)
{
  if (count != 0)
    memset (p, c, count);
}

void CALL_CONV T_memcpy (POINTER d, POINTER s, unsigned int count)
{
  if (count != 0)
    memcpy (d, s, count);
}

void CALL_CONV T_memmove (POINTER d, POINTER s, unsigned int count)
{
#if MEMMOVE_PRESENT
  if (count != 0)
    memmove (d, s, count);
#else
  unsigned int i;

  if ((char *)d == (char *)s)
    return;
  else if ((char *)d > (char *)s) {
    for (i = count; i > 0; i--)
      ((char *)d)[i-1] = ((char *)s)[i-1];
  }
  else {
    for (i = 0; i < count; i++)
      ((char *)d)[i] = ((char *)s)[i];
  }
#endif
}

int CALL_CONV T_memcmp (POINTER s1, POINTER s2, unsigned int count)
{
  if (count == 0)
    return (0);
  else
    return (memcmp (s1, s2, count));
}

POINTER CALL_CONV T_malloc (unsigned int size)
{
  return ((POINTER)malloc (size == 0 ? 1 : size));
}

POINTER CALL_CONV T_realloc (POINTER p, unsigned int size)
{
  POINTER result;
  
  if (p == NULL_PTR)
    return (T_malloc (size));

  if ((result = (POINTER)realloc (p, size == 0 ? 1 : size)) == NULL_PTR)
    free (p);
  return (result);
}

void CALL_CONV T_free (POINTER p)
{
  if (p != NULL_PTR)
  {
    free (p);
    p = NULL_PTR;
  }
}
