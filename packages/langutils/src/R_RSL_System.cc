// *******************************************************************
// R_RSL_System.cc
//
// automatically generated from RSL_System.rsl and the template:
// $Id: R_RSL_System.cc,v 1.3 1999/01/12 15:32:36 toddm Exp $
// *******************************************************************
#include <strstream.h>
#include <stdlib.h>
#include <ctype.h>

#include "R_RSL_System.h"
#include "R_String.h"
#include "R_Integer.h"
#include "R_Boolean.h"
#include "RslObjectParser.h"
#include "runtime.h"
#include "destiny.h"
#include "rsldefaults.h"
#include "slog.h"

#define _hREADOBJECT 1584597761     // readObject
#define _hWRITEOBJECT 2002681630    // writeObject
#define _hSTREAM 303657573          // stream
#define _hDESTREAM 369111577        // deStream
#define _hINSERT 454718309          // insert
#define _hCLASSNAME 1965162526      // className
#define _hREMOVE 67136879           // remove
#define _hREMOVECLASS 1702059267    // removeClass
#define _hHASHCODE 722343693        // hashCode
#define _hASSIGN 102593385          // assign
#define _hGET 6776180               // get
#define _hQUITSESSION 1803514887    // QuitSession
#define _hINSTANTIATE 1769234973    // instantiate
#define _hDOCUMENTCLASS 963013746   // documentClass

// R_RSL_System static member
rc_RSL_System R_RSL_System::rslType("RSL_System");

extern "C" res_class *Create_RSL_System_RC()
{
    return &(R_RSL_System::rslType);
}


// Spawn - create a new resource of this type (R_RSL_System)
// Called by res_class::New() if there is no object
// to pull off the free list.
Resource* rc_RSL_System::spawn(RWCString nm)
{
    return new R_RSL_System(nm);    // or other constructor
}

// New
// static convenience function
// See R_String::New(), R_Integer::New(), etc.
// If no functionality beyond res_class::New() is required
// (ie, no special values to be set conveniently), then this
// function simply eliminates the need to cast the result
// of res_class::New().
R_RSL_System *R_RSL_System::New(RWCString n)
{
    Resource *r= R_RSL_System::rslType.New(n);
//  ((R_RSL_System *) r)->Set( /* values (add this function if needed) */ );
    return (R_RSL_System *) r;
}

// R_RSL_System constructor
R_RSL_System::R_RSL_System(RWCString nm)
    : Resource(nm)
{

}

// StrValue()
// Return a String version of this Resource, only if applicable.
RWCString R_RSL_System::StrValue(void)
{
    return ClassName();
}

// LogicalValue()
// Evaluate the "trueness" of this Resource (1 for true, 0 for false)
// Used in logical comparisons.
int R_RSL_System::LogicalValue()
{
    return 1;
}

// IsEqual()
// Test for equality with another Resource.
// (ResStructure provides a default version)
int R_RSL_System::IsEqual(Resource *r)
{
    return (r == this);
}

// SetFromInline
// Given a list of Resources, match with a data member of the same
// name and assign. eg, in RSL, "myclass { a:1, b:2, /* etc */ }"
// an object of type `myclass' is created and SetFromInline() is called
// for the list of resources enclosed in { }.
// (ResStructure provides a default version)
void R_RSL_System::SetFromInline(RWTPtrSlist<Resource>& inliner)
{
    /* modify this */
}

// Assign
// set this resource equal to r.
// (ResStructure provides a default version)
void R_RSL_System::Assign(Resource *r)
{
    /* modify this */
}

// Clear()
// Memory management - called to restore an object
// to a "just created" state, for free-list management.
// (ResStructure provides a default version)
void R_RSL_System::Clear()
{
    /* modify this */
}

// print()
// ECI syntax
// (ResStructure provides a default version)
void R_RSL_System::print(ostream &out)
{
    out << "RSL_System { }";
}

// rslprint()
// Printing from within RSL
// (ResStructure provides a default version)
void R_RSL_System::rslprint(ostream &out)
{

}

// ************************************************************************
// *
// * NAME:  GetAllMembers           Private function
// *
// * DESCRIPTION:
// *
// * INPUT:
// *        None 
// *
// * RETURNS:
// *        None
// *
// ************************************************************************
void  R_RSL_System::GetAllMembers(Resource *Destination, Resource *Source)
{
    // ********************************************************
    // * Is the transfer resource a resStructure or resObject *
    // ********************************************************

    // russell asks: why not
    //   if (Source->isRSLStruct() && Destination->isRSLStruct())

    if (((Source->InternalType() == Resource::resStructType) ||
         (Source->InternalType() == Resource::resObjType)) &&
        ((Destination->InternalType() == Resource::resStructType) ||
         (Destination->InternalType() == Resource::resObjType)))
    {

        cout << "Souce and Destination are resStructType\n";

        // ******************************************************
        // * Get the local context of the Destination resource **
        // ******************************************************
        ResContext& DestContext = ((ResStructure *) Destination)->GetLocalContext();

        // *************************************************
        // * Get the local context of the Source resource **
        // *************************************************
        ResContext& SrcContext = ((ResStructure *) Source)->GetLocalContext();
        RWTValHashSet<ResReference>* locallist = SrcContext.GetLocals();
        
        if (locallist)
        {
            // ****************************************************************
            // * For each resource in the From's context, check to see if it **
            // * is a shared object if so, call GetAllMember recursively.    **
            // * If this is not a shared object then add the resource to the **
            // * destination object.                                         **
            // ****************************************************************
            RWTValHashTableIterator<ResReference> iter(*locallist);
            ResReference refDataMember;
            while(iter())
            {
                refDataMember = iter.key();

//                cout << "DataMember Name: " << refDataMember.Name() << "\n";

                if (refDataMember.Name() == REFNAME_TO_SHARED_OBJ)
                {
                    // *******************************************************
                    // * Call GetAllMembers recursively following the       **
                    // * pointer to the shared resource                     **
                    // *******************************************************
                    GetAllMembers(Destination, refDataMember());
                }
                else
                {
                    // **********************************************
                    // * Add the DataMember to the Context of the   *
                    // * destination resource                       *
                    // **********************************************
                    DestContext.AddResource(refDataMember());
                }
            }
        }
    }
}    


// ***********************************************************************
// *
// * NAME:    execute         Public Function
// *
// * DESCRIPTION:
// *        This is the interface between RSL and C++ Resources.
// *
// * INPUT: 
// *      method -  hash value for a method within the       
// *                resource                                  
// *      arglist - argument list.                           
// *                                                                  
// *  RETURNS: 
// *      ResStatus
// *                                                                  
// * Automatically generated from "RSL_Utils.rsl"
// * DO NOT MODIFY !
// ************************************************************************
ResStatus R_RSL_System::execute(int method, ResList& arglist)
{
    switch(method)
    {
        case _hREADOBJECT:                  // "readObject"
            return rsl_readObject(arglist);

        case _hWRITEOBJECT:                 // "writeObject"
            return rsl_writeObject(arglist);

        case _hSTREAM:                      // "stream"
            return rsl_stream(arglist);

        case _hDESTREAM:                    // "deStream"
            return rsl_deStream(arglist);

        case _hCLASSNAME:                   // "className"
            return rsl_className(arglist);
 
        case _hINSERT:                      // "insert"
            return rsl_insert(arglist);
 
        case _hREMOVE:                      // "remove"
            return rsl_remove(arglist);

        case _hREMOVECLASS:                 // "removeClass"
            return rsl_removeClass(arglist);
 
        case _hASSIGN:                      // "assign"
            return rsl_assign(arglist);

		case _hGET:	                        // "get"
			return rsl_get(arglist);

		case _hQUITSESSION:	                // "QuitSession"
			return rsl_QuitSession(arglist);

        case _hHASHCODE:                    // "hashCode"
            return rsl_hashCode(arglist);

        case _hINSTANTIATE:                 // "instantiate"
            return rsl_instantiate(arglist);
 
        case _hDOCUMENTCLASS:               // "documentClass"
            return rsl_documentClass(arglist);

        default: ;
    }

    return ResStatus(ResStatus::rslFail);
}

// RSL method "readObject"
//  readObject(String filename);
ResStatus R_RSL_System::rsl_readObject(const ResList& arglist)
{
    ifstream inf(arglist[0].StrValue());
    RWCString theobj;

    // **********************************
    // * Should check if inf is valid!! *
    // **********************************
    if (!inf)
    {
        logf->error(LOGRSL) << "RSL_System::readObject() can not open file." << endline;
        return ResStatus(ResStatus::rslOk, NULL);
    }

    theobj.readFile(inf);
    inf.close();

    RslObjectParser p(theobj);
    p.go();

    ResReference refReturn = p.ResourceResult();

    return ResStatus(ResStatus::rslOk, refReturn.RealObject());
}

// RSL method "writeObject"
//  writeObject(String filename, x);
ResStatus R_RSL_System::rsl_writeObject(const ResList& arglist)
{
    ResReference ref_name = arglist[0], ref_obj = arglist[1];
    ofstream outf(ref_name.StrValue().data());

    ref_obj->print(outf);
    outf.close();

    return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "stream"
//  String stream(x);
ResStatus R_RSL_System::rsl_stream(const ResList& arglist)
{
    ResReference ref = arglist[0];
    if (ref.isValid())
    {
        cout << "Streamed object: `";
        ref()->print(cout);
        cout << "'\n";
    }
    else
        cout << "Streamed object: invalid.\n";

    return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "deStream"
//  deStream(String streamed);
ResStatus R_RSL_System::rsl_deStream(const ResList& arglist)
{
    ResReference ref = arglist[0];
    RWCString s = ref.StrValue();

    RslObjectParser p(s);
    p.go();

    ResReference refReturn = p.ResourceResult();
    p.cleanUp();

    return ResStatus(ResStatus::rslOk, refReturn.RealObject());
}
 
// RSL method "className"
ResStatus R_RSL_System::rsl_className(const ResList& arglist)
{
    return ResStatus(ResStatus::rslOk,
        R_String::New("", arglist[0].ClassName()) );
}

// RSL method "insert"
//  insert(container, ...);
ResStatus R_RSL_System::rsl_insert(const ResList& arglist)
{
    ResReference container = arglist[0];

    if (container.isValid() && container->isRSLStruct())
    {
        // Get the context of the container object, and
        // insert each argument.
        int i, len = arglist.entries();
        ResContext& localRC = ((ResStructure *) container())->GetLocalContext();
        
        // skipping the first arg.. don't want to insert the container
        // into itself!
        for(i=1; i<len; i++)
            localRC.AddResource(arglist.get(i));
    }
    return ResStatus(ResStatus::rslOk, NULL);
}
 

// RSL method "remove"
//  remove(container, ...);
ResStatus R_RSL_System::rsl_remove(const ResList& arglist)
{
    logf->error(LOGRSL) << "RSL_System::Remove() is obsolete." << endline;
    return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "removeClass"
ResStatus R_RSL_System::rsl_removeClass(const ResList& arglist) 
{
    logf->error(LOGRSL) << "removeClass for `" << (arglist[0].StrValue())
        << "': not implemented." << endline;

    return ResStatus(ResStatus::rslOk, NULL);
}
 
// RSL method "hashCode"
ResStatus R_RSL_System::rsl_hashCode(const ResList& arglist)
{
//  unsigned int Resource::theIDHash(const char *s0)
    unsigned int hcode = Resource::theIDHash(arglist[0].StrValue().data());

    char s[20]; // bigger than the max # of digits in an unsigned int.
    ostrstream(s, 20) << hcode;

    return ResStatus(ResStatus::rslOk, R_String::New("", s));
}

// RSL method "assign"
//  assign(left, right);
ResStatus R_RSL_System::rsl_assign(const ResList& arglist)
{
    // calling Resource::Assign()
    arglist.get(0)->Assign(arglist.get(1));

    return ResStatus(ResStatus::rslOk, NULL);
}

// RSL method "instantiate"
//  instantiate(String classname);
//  ResStatus R_RSL_System::rsl_instantiate(const ResList& arglist)      
//  {                                                                    
//      res_class lookup(arglist[0].StrValue());                         
//      res_class *rcp = ResClasses.find(&lookup);                       
//                                                                       
//      if (rcp)                                                         
//          return ResStatus(ResStatus::rslOk, rcp->New(""));            
//      else                                                             
//          logf->notice(LOGRSL) << "RSL_System::instantiate(): Class `" 
//              << arglist[0].StrValue() << "' not found." << endline;   
//                                                                       
//      return ResStatus(ResStatus::rslOk, NULL);                        
//  }                                                                    
 
// RSL method "instantiate"
//  instantiate(String classname);
ResStatus R_RSL_System::rsl_instantiate(const ResList& arglist) 
{
    // ***********************
    // * Get the object name *
    // ***********************
    RWCString strObject = arglist[0].StrValue();

    // ***********************
    // * Check for a context *
    // ***********************
    ResContext *rcSession = NULL;
    Resource *rArg = arglist.get(1);
    if (rArg)
    {
        RWCString strSession = rArg->StrValue();
        rcSession = runtimeStuff.FindSession(strSession);
    }

    ResStructure *prsNewObject = runtimeStuff.CreateResource(strObject, strObject, NULL, rcSession);
    return(ResStatus(ResStatus::rslOk, (Resource *) prsNewObject));
}
 
// RSL method "documentClass"
//  documentClass(String classname, String directory);
ResStatus R_RSL_System::rsl_documentClass(const ResList& arglist)
{
    ResReference classname_ref = arglist[0], dir_ref = arglist[1];

    res_class lookup(classname_ref.StrValue());
    res_class *rcp = ResClasses.find(&lookup);

    if (rcp)
    {
        RWCString outfname = dir_ref.StrValue()
            + RWCString("/")
            + classname_ref.StrValue() + ".html";

        ofstream ofs(outfname);

        // htmlify the class to the stream.
        rcp->html(ofs);
    }

    return ResStatus(ResStatus::rslOk, NULL);
}

// ************************************************************************
// *                                                                       
// * NAME:    rsl_get             Public Function                           
// *                                                                       
// * DESCRIPTION:                                                          
// *                                                                       
// * INPUT:                                                                
// *                                                                       
// * RETURNS:                                                              
// *                                                                       
// ************************************************************************
ResStatus R_RSL_System::rsl_get(const ResList& arglist)
{
    logf->debug(LOGRSL) << "(R_RSL_System) In rsl_get" << endline;

    GetAllMembers(arglist.get(0), arglist.get(1));

    return ResStatus(ResStatus::rslOk, NULL);
}


// ************************************************************************
// *                                                                       
// * NAME:    rsl_QuitSession             Public Function                           
// *                                                                       
// * DESCRIPTION: Calls the KillSession to remove the specified
// *              session.
// *                                                                       
// * INPUT: RWCString - Containing the session name to delete.
// *                                                                       
// * RETURNS:                                                              
// *                                                                       
// ************************************************************************
ResStatus R_RSL_System::rsl_QuitSession(const ResList& arglist)
{
    logf->debug(LOGRSL) << "(R_RSL_Utils) In rsl_QuitSession" << endline;

    runtimeStuff.KillSession(arglist.get(0)->StrValue());

    return ResStatus(ResStatus::rslOk, NULL);
}
