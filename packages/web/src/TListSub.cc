// ***************************************************************************
// *
// *  NAME:  R_TListSub.cc
// *
// *  RESOURCE NAME:    WebServer                                        
// *                                                                    
// *  RELATED RESOURCES: 
// *                                                                    
// *  DESCRIPTION: This file contains the method to process the R_LIST and
// *                R_CLIST HTML tags.                                                 
// *                                                                    
// * $Id: TListSub.cc,v 1.1 1998/11/17 23:30:11 toddm Exp $
// *
// * $Log: TListSub.cc,v $
// * Revision 1.1  1998/11/17 23:30:11  toddm
// * Initial revision
// *
// * Revision 2.6  1998/11/12 21:30:21  toddm
// * Add new subsystems
// *
// * Revision 2.4  1998/11/09 20:58:26  toddm
// * Fix R_LIST processing
// *
// * Revision 2.3  1998/04/17 15:46:13  toddm
// * Comment out CallMethod
// *
// * Revision 2.2  1998/04/03 21:35:35  toddm
// * Fix R_LIST processing
// *
// * Revision 2.1  1998/02/13 21:34:04  toddm
// * Start work on splitting the Web Channel from the Granite Core
// *
// * Revision 1.9  1997/11/05 16:53:53  toddm
// * HTM extension support / Fix multiple cookie problem
// *
// * Revision 1.8  1997/09/24 23:59:46  toddm
// * Fix Memory Leaks
// *
// * Revision 1.7  1997/09/19 18:33:58  toddm
// * Fix session counting
// *
// *
// * Copyright (c) 1995, 1996, 1997 by Destiny Software Corporation
// *
// ***************************************************************************

// *******************
// * System Includes *
// *******************
#include <fstream.h>
#include <stream.h>

// ******************
// * Local Includes *
// ******************
#include "rw_utils.h"
#include "R_WebServer.h"
#include "R_String.h"
#include "R_List.h"
#include "PerfLog.h"


// *********************************************************************
// *                                                                    
// * Function: ReportMessage
// *                                                                    
// * Description:   This function is used to report a message to the user.
// *                It reads the specified file and replaces the
// *                MESSAGETEXT with the specified text.
// *
// * Inputs:    strHtmlMsgFile - RWCString containg the file name of the html
// *                            page to display.
// *            strMessage - RWCString containing the text to display
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: Nothing
// *                                                                    
// *********************************************************************
void R_WebServer::ReportMessage(RWCString strHtmlMsgFile, RWCString strMessage)
{
    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In ReportMessage" << endline;
    
    OutHtmlFile.clear();
    iFileLen = 0;

    // ****************************************
    // * Open and read the specifed HTML file *
    // ****************************************
    RWCString strFile;
    strFile = strDocumentRoot + strHtmlMsgFile + "." + strDocumentExt;
    
    RWTValSlist<RWCString> InHtmlFile;
    DRWCString strHtmlLine;
    
    logf->info(LOGWEBCHANNEL) << "(R_WebServer) Reading HTML file '"
                          << strFile << "'" << endline;

    ifstream fHtml(strFile);
    if (fHtml)
    {
        fHtml >> InHtmlFile;
        fHtml.close();

        RWTValSlistIterator<RWCString> iterFile(InHtmlFile);

        while(iterFile())
        {
            // *************************
            // * Special substitutions *
            // *************************
            strHtmlLine = iterFile.key();

            // *********************************************
            // * Remove the special comments from the HTML *
            // *********************************************
            if (strHtmlLine.contains("<!--."))
                continue;

            // ***************************************************
            // * Replace the Text string tag with text from the  *
            // * specified variable                              *
            // *                                                 *
            // * Must be on a line by itself                     *
            // ***************************************************
            if (strHtmlLine.contains("MESSAGETEXT"))
            {
                strHtmlLine.replace("MESSAGETEXT", strMessage.data());
            }
            
            iFileLen += strHtmlLine.length() + 1;
            OutHtmlFile.append(strHtmlLine);
        }
    }
    else
    {
        logf->alert(LOGWEBCHANNEL) << "(R_WebServer) Unable to open '"
                << strFile 
                << "'!" << endline;
        

        strHtmlLine = "<HTML>"; 
        iFileLen += strHtmlLine.length()+1; 
        OutHtmlFile.append(strHtmlLine);
        
        strHtmlLine = "<HEAD>";
        iFileLen += strHtmlLine.length()+1; 
        OutHtmlFile.append(strHtmlLine);

        strHtmlLine = "<TITLE>Application Message</TITLE></HEAD>";
        iFileLen += strHtmlLine.length()+1; 
        OutHtmlFile.append(strHtmlLine);

        strHtmlLine = "<BODY bgcolor=e0e0e0>";
        iFileLen += strHtmlLine.length()+1; 
        OutHtmlFile.append(strHtmlLine);

        strHtmlLine = "<CENTER>";
        iFileLen += strHtmlLine.length()+1; 
        OutHtmlFile.append(strHtmlLine);

        strHtmlLine = "<TABLE BORDER=2 WIDTH=60% ALIGN=CENTER CELLPADDING=15 bgcolor=ffffff>";
        iFileLen += strHtmlLine.length()+1; 
        OutHtmlFile.append(strHtmlLine);

        strHtmlLine = "<TR><TD ALIGN=CENTER>";
        iFileLen += strHtmlLine.length()+1; 
        OutHtmlFile.append(strHtmlLine);

        strHtmlLine = "<BR><B>";
        iFileLen += strHtmlLine.length()+1; 
        OutHtmlFile.append(strHtmlLine);

        strHtmlLine = strMessage;
        iFileLen += strHtmlLine.length()+1; 
        OutHtmlFile.append(strHtmlLine);

        strHtmlLine = "</B><BR><BR>";
        iFileLen += strHtmlLine.length()+1; 
        OutHtmlFile.append(strHtmlLine);

        strHtmlLine = "</TD></TR>";
        iFileLen += strHtmlLine.length()+1; 
        OutHtmlFile.append(strHtmlLine);

        strHtmlLine = "</TABLE>";
        iFileLen += strHtmlLine.length()+1; 
        OutHtmlFile.append(strHtmlLine);

        strHtmlLine = "</CENTER>";
        iFileLen += strHtmlLine.length()+1; 
        OutHtmlFile.append(strHtmlLine);

        strHtmlLine = "</BODY></HTML>";
        iFileLen += strHtmlLine.length()+1; 
        OutHtmlFile.append(strHtmlLine);
    }
}


// *********************************************************************
// *                                                                    
// * Function: MergeTemplate
// *                                                                    
// * Description:   This function does the HTML Template substitution
// *
// *         resDisplayObject - ResReference to the form object
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: 1 if substitutions were made
// *          0 if not.
// *                                                                    
// *********************************************************************
void R_WebServer::MergeTemplate(ResReference resDisplayObject)
{
    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In MergeTemplate" << endline;
    
    RWCString strObjectName;
    RWCString strTemplateName;
    RWCString strFile;

    // *****************************************
    // * Merge the resource with the template **
    // *****************************************
    if (resDisplayObject.isValid())
    {
        // ******************************************
        // * Get the objectName data member of the **
        // * object.                               **
        // ******************************************
        ResReference resObjectName =
            ((ResStructure *) resDisplayObject())->GetDataMember("objectName");
        if (resObjectName.isValid() && resObjectName.StrValue().length() > 0)
        {
            strObjectName = resObjectName.StrValue();
        }
        else
            logf->alert(LOGWEBCHANNEL) << "(R_WebServer) Object name was not specified" << endline;

        // ****************************************************
        // * Derive the HTML file name from the document root *
        // * and the TemplateName data member of the object   *
        // * If no TemplateName is specified then use the     *
        // * object name.                                     *
        // ****************************************************
        ResReference resTemplateName =
            ((ResStructure *) resDisplayObject())->GetDataMember("TemplateName");
        if (resTemplateName.isValid() && resTemplateName.StrValue().length() > 0)
        {
            strTemplateName = resTemplateName.StrValue();
            strFile = strDocumentRoot + strTemplateName + "." + strDocumentExt;
        }
        else
            strFile = strDocumentRoot + strObjectName + "." + strDocumentExt;


        logf->info(LOGWEBCHANNEL) << "(R_WebServer) Reading HTML file '"
                              << strFile << "'" << endline;

        // ****************************************
        // * Open and read the specifed HTML file *
        // ****************************************
        RWTValSlist<RWCString> InHtmlFile;

        ifstream fHtml(strFile);
        if (fHtml)
        {
            fHtml >> InHtmlFile;
            fHtml.close();

            RWTValSlistIterator<RWCString> iterFile(InHtmlFile);
            DRWCString strHtmlLine;
            
            while(iterFile())
            {
                // *************************
                // * Special substitutions *
                // *************************
                strHtmlLine = iterFile.key();

                // *********************************************
                // * Remove the special comments from the HTML *
                // *********************************************
                if (strHtmlLine.contains("<!--."))
                    continue;

                // ****************************************************
                // * Replace the <R_TEXT sVarname> tag with text      *
                // * from the specified variable                      *
                // ****************************************************
                if (strHtmlLine.contains("<R_TEXT"))
                {
                    RTextSub(strHtmlLine, resDisplayObject);
                }
                else
                
                // ************************************
                // *  R_LIST and R_CLIST Substitution *
                // ************************************
                if (strHtmlLine.contains("<R_LIST") || 
                    strHtmlLine.contains("<R_CLIST"))
                {
                    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) Processing a List...\n" <<
                               strHtmlLine << endline;

                    // ***********************
                    // * Performance logging *
                    // ***********************
                    PerfLogger tmRList;
                    tmRList.StartTime();

                    DRWCString strTagLine = strHtmlLine;

                    // ***************************************************
                    // * Setup a list containing the whole R_LIST block **
                    // ***************************************************
                    RWTValSlist<DRWCString> RListBlock;
                    RListBlock.append(strHtmlLine);

                    // ********************************************
                    // * Keep reading the Html file until we get  *
                    // * the </R_LIST> or the </R_CLIST>          *
                    // *                                          *
                    // * R_LISTs and R_CLISTs can be nested but   *
                    // * R_LIST or R_CLIST must be on a sererate  *
                    // * line for now.                            *
                    // ********************************************
                    int bOk=FALSE;
                    int iNestedCount=0;
                    while(iterFile())
                    {
                        strHtmlLine = iterFile.key();

                        // **********************************************
                        // * Insert the session data into the HTML page *
                        // **********************************************
                        strHtmlLine.replace("APPID", strCurrentSession.data());
                        strHtmlLine.replace("OBJID", strObjectName.data());

                        RListBlock.append(strHtmlLine);

                        logf->debug(LOGWEBCHANNEL) << "(R_WebServer)" << strHtmlLine << endline;

                        // *****************************
                        // * Check for nested R_LISTs **
                        // *****************************
                        if (strHtmlLine.contains("<R_LIST") || 
                            strHtmlLine.contains("<R_CLIST"))
                        {
                            iNestedCount++;
                        }

                        if (strHtmlLine.contains("</R_LIST>") || 
                            strHtmlLine.contains("</R_CLIST>"))
                        {
                            if (iNestedCount-- <= 0)
                            {
                                bOk = TRUE;
                                break;  // done with this substitution
                            }
                        }
                    }

                    if (bOk)
                    {
                        // ************************
                        // * Is this a count list *
                        // ************************
                        int bCList = FALSE;
                        DRWCString strListName;
                        if (strTagLine.contains("<R_CLIST"))
                        {
                            bCList = TRUE;                        
                            strListName = strTagLine.after("<R_CLIST");
                        }
                        else
                            strListName = strTagLine.after("<R_LIST");

                        // *****************************************
                        // * Get the name of the list from the tag *
                        // *****************************************
                        strListName = strListName.strip(RWCString::both);
                        strListName = strListName.before(">");

                        // ***************************************
                        // * Find the list in the current object *
                        // ***************************************
                        logf->debug(LOGWEBCHANNEL) << "(R_WebServer) Looking for list - '" 
                                                << strListName << "'" << endline;

                        ResReference resList =
                            ((ResStructure *) resDisplayObject())->GetDataMember(strListName);

                        if (bCList)
                            RCListSub(RListBlock, resDisplayObject, resList);
                        else
                            RListSub(RListBlock, resDisplayObject, resList);
                    }
                    else
                        logf->alert(LOGWEBCHANNEL) << "(R_WebServer) Unable to find end of R_LIST block!"
                                               << endline;
                    // ***********************
                    // * Performance logging *
                    // ***********************
                    tmRList.EndTime();
                    tmRList.ReportPerf("R_List Processing", logf);

                    continue;
                }
                else

                // ***************************************************
                // * Replace the <R_MESSAGETEXT> string tag with the *
                // * variable of name MessageText in the given       *
                // * object.                                         *
                // ***************************************************
                if (strHtmlLine.contains("<R_MESSAGETEXT>"))
                {
                    ResReference resMessage =
                        ((ResStructure *) resDisplayObject())->GetDataMember("MessageText");
                    if (resMessage.isValid())
                    {
                        strHtmlLine.replace("<R_MESSAGETEXT>", resMessage.StrValue().data());
                    }
                }
                else

                // ***************************************************
                // * Replace the <R_FORMTITLE> string tag with the   *
                // * variable of name MessageText in the given       *
                // * object.                                         *
                // ***************************************************
                if (strHtmlLine.contains("<R_FORMTITLE>"))
                {
                    ResReference resTitle =
                        ((ResStructure *) resDisplayObject())->GetDataMember("Title");
                    if (resTitle.isValid())
                    {
                        strHtmlLine.replace("<R_FORMTITLE>", resTitle.StrValue().data());
                    }
                }

                // **********************************************
                // * Insert the session data into the HTML page *
                // **********************************************
                strHtmlLine.replace("APPID", strCurrentSession.data());
                strHtmlLine.replace("OBJID", strObjectName.data());

                iFileLen += strHtmlLine.length() + 1;
                OutHtmlFile.append(strHtmlLine);
            }
        }
        else
        {
            logf->alert(LOGWEBCHANNEL) << "(R_WebServer) Unable to open '"
                    << strFile 
                    << "'!" << endline;
        }
    }
}    


// *********************************************************************
// *                                                                    
// * Function: RTextSub                                             
// *                                                                    
// * Description:   This function does the HTML Template substitution
// *                for the R_TEXT tags.
// *
// *                Assumption: The <R_TEXT varname> is on a single line.
// *                                                  
// * Inputs: strHtmlLine - DRWCString containing the current html line.
// *         resCurrentObj - ResReference to the form object
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: 1 if substitutions were made
// *          0 if not.
// *                                                                    
// *********************************************************************
void R_WebServer::RTextSub(DRWCString& strHtmlLine, ResReference resCurrentObj)
{
    static RWCRegexp RXmatchGetTextVar("<R_TEXT *[a-zA-Z_][a-zA-Z_0-9]*>");
    static RWCRegexp RXmatchText("<R_TEXT *");

    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In RTextSub" << endline;

    if (strHtmlLine(RXmatchGetTextVar).isNull())
    {
        return;
    }

    DRWCString strLine = strHtmlLine;
    DRWCString strVarName;

	ResReference resText;

    int iFreq = strLine.freq("<R_TEXT");
    for(int i=0; i<iFreq; i++)
    {
        strLine = strLine.after(RXmatchText);
        strVarName = strLine.before(">");

        logf->info(LOGWEBCHANNEL) << "(R_WebServer) Get Data Member'"
                              << strVarName << "'" << endline;

        if (strVarName.length() > 0)
        {
            resText = ((ResStructure *) resCurrentObj())->GetDataMember(strVarName);

            // ************************************
            // * Have we gained access to he data *
            // ************************************
            if (resText.isValid())
            {
                logf->info(LOGWEBCHANNEL) << "(R_WebServer) R_TEXT Value - '" 
                                      << resText.StrValue() << "'" << endline;

                strHtmlLine(RXmatchGetTextVar) = resText.StrValue(); 
            }
            else
            {
                strHtmlLine(RXmatchGetTextVar) = "";
            }
        }
        else
        {
            strHtmlLine(RXmatchGetTextVar) = "";
        }
    }

    return;
}

// *********************************************************************
// *                                                                    
// * Function: RListSub
// *                                                                    
// * Description:   This function does the HTML Template substitution
// *                for the R_LIST tags.
// *
// *                Assumption: The open <R_LIST varname> and the close
// *                            </R_LIST> is each on a single line.
// *                                                  
// * Inputs: RListBlock - RWTValSlist containing the R_LIST block.
// *         resCurrentObj - ResReference to the form object
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: 1 if substitutions were made
// *          0 if not.
// *                                                                    
// *********************************************************************
void R_WebServer::RListSub(RWTValSlist<DRWCString> &RListBlock, ResReference resCurrentObj,
                            ResReference resList)
{
    static RWCRegexp RXwhite("[ \n\t\r\v\f]+");

    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In RListSub" << endline;

    // ********************************************
    // * Set up a iterator on the RListBlock and  *
    // * Grab the R_ tag line from the RListBlock *
    // ********************************************
    RWTValSlistIterator<DRWCString> iter(RListBlock);
    iter();                         // First Element in list

    // ************************************
    // * Are we dealing with a valid list *
    // ************************************
    if (!resList.isValid() || !resList.HierarchyContains(R_List_ID))
    {
        logf->error(LOGWEBCHANNEL) << "(R_WebServer) Could not find variable or it is not a List" 
                               << endline;
        return;
    }

    // ****************************************
    // * Get the specified list and setup an **
    // * interator on the list               **
    // ****************************************
    RWTValSlist<ResReference> &theList = ((R_List *) resList())->GetList();
    RWTValSlistIterator<ResReference> iterList(theList);

    ResReference resListEntry;
    
    // ****************************************
    // * Loop through each object in the list *
    // ****************************************
    while (iterList())
    {
        resListEntry = iterList.key();

        // ********************************************
        // * Keep reading the Html file until we get  *
        // * the </R_LIST>                            *
        // ********************************************
        while(iter())
        {
            DRWCString strHtmlLine = iter.key();

            // **************************************
            // * Is this the end of the LIST block **
            // **************************************
            if (strHtmlLine.contains("</R_LIST>")) 
            {
                break;  // done with this substitution
            }

            // *******************************
            // * Have we found a nested List *
            // *******************************
            if (strHtmlLine.contains("<R_LIST") || 
                strHtmlLine.contains("<R_CLIST"))
            {
                logf->debug(LOGWEBCHANNEL) << "(R_WebServer) Processing a nested List...\n" <<
                           strHtmlLine << endline;

                DRWCString strTagLine = strHtmlLine;

                // ***************************************************
                // * Setup a list containing the whole R_LIST block **
                // ***************************************************
                RWTValSlist<DRWCString> nestedListBlock;
                nestedListBlock.append(strHtmlLine);

                // ********************************************
                // * Keep reading the Html file until we get  *
                // * the </R_LIST> or the </R_CLIST>          *
                // *                                          *
                // * R_LISTs and R_CLISTs can be nested but   *
                // * R_LIST or R_CLIST must be on a sererate  *
                // * line for now.                            *
                // ********************************************
                int bOk=FALSE;
                int iNestedCount=0;
                while(iter())
                {
                    strHtmlLine = iter.key();

                    nestedListBlock.append(strHtmlLine);

                    logf->debug(LOGWEBCHANNEL) << "(R_WebServer)" << strHtmlLine << endline;

                    // *****************************
                    // * Check for nested R_LISTs **
                    // *****************************
                    if (strHtmlLine.contains("<R_LIST") || 
                        strHtmlLine.contains("<R_CLIST"))
                    {
                        iNestedCount++;
                    }

                    if (strHtmlLine.contains("</R_LIST>") || 
                        strHtmlLine.contains("</R_CLIST>"))
                    {
                        if (iNestedCount-- <= 0)
                        {
                            bOk = TRUE;
                            break;  // done with this substitution
                        }
                    }
                }

                // ******************************************
                // * A count list must specify a list name **
                // * since it doesn't make sense to put    **
                // * a index on nested lists               **
                // ******************************************
                if (bOk)
                {
                    DRWCString strNestedListName;

                    if (strTagLine.contains("<R_CLIST"))
                    {
                        strNestedListName = strTagLine.after("<R_CLIST");

                        // *****************************************
                        // * Get the name of the list from the tag *
                        // *****************************************
                        strNestedListName = strNestedListName.after(RXwhite);
                        strNestedListName = strNestedListName.before(">");

                        // ***************************************
                        // * Find the list in the current object *
                        // ***************************************
                        logf->debug(LOGWEBCHANNEL) << "(R_WebServer) Looking for list - '" 
                                                << strNestedListName << "'" << endline;

                        ResReference resNestedList =
                            ((ResStructure *) resCurrentObj())->GetDataMember(strNestedListName);

                        RCListSub(nestedListBlock, resCurrentObj, resNestedList);
                    }
                    else
                    {
                        strNestedListName = strTagLine.after("<R_LIST");

                        // *****************************************
                        // * Get the name of the list from the tag *
                        // *****************************************
                        strNestedListName = strNestedListName.strip(RWCString::both);
                        if (strNestedListName.length() > 1)
                            strNestedListName = strNestedListName.before(">");
                        else
                            strNestedListName="";

                        // ************************************************
                        // * If there was no list name specified          *
                        // * then we should use the entry from the        *
                        // * list currently being process, assuming       *
                        // * this list is a list of lists                 *
                        // ************************************************
                        if (strNestedListName.isNull())
                        {
                            logf->debug(LOGWEBCHANNEL) << "(R_WebServer) No list name found, using list entry from previous list "  << endline;

                            RListSub(nestedListBlock, resCurrentObj, resListEntry);
                        }
                        else
                        {
                            // ***************************************
                            // * Find the list in the current object *
                            // ***************************************
                            logf->debug(LOGWEBCHANNEL) << "(R_WebServer) Looking for list - '" 
                                                    << strNestedListName << "'" << endline;

                            ResReference resNestedList =
                                ((ResStructure *) resCurrentObj())->GetDataMember(strNestedListName);

                            RListSub(nestedListBlock, resCurrentObj, resListEntry);
                        }
                    }
                }
                else
                    logf->alert(LOGWEBCHANNEL) << "(R_WebServer) Unable to find end of nested R_LIST block!"
                                           << endline;

                continue;
            }

            // ***************
            // * Substitute  *
            // ***************
            if (VariableSub(strHtmlLine, resListEntry, -1))
            {
                iFileLen += strHtmlLine.length() + 1;   
                OutHtmlFile.append(strHtmlLine);
            }
        }

        iter.reset();
        iter();             // Skip the first line <R_LIST varname>
    }
}


// *********************************************************************
// *                                                                    
// * Function: RCListSub                                             
// *                                                                    
// * Description:   This function does the HTML Template substitution
// *                for the R_CLIST tags.
// *
// *                Assumption: The open <R_CLIST varname> and the close
// *                            </R_CLIST> is each on a single line.
// *                                                  
// * Inputs: RListBlock - RWTValSlist containing the R_LIST block.
// *         resCurrentObj - ResReference to the form object
// *                                                                    
// * Outputs: None                                                          
// *                                                                    
// * Returns: 1 if substitutions were made
// *          0 if not.
// *                                                                    
// *********************************************************************
void R_WebServer::RCListSub(RWTValSlist<DRWCString> &RListBlock, ResReference resCurrentObj,
                            ResReference resList)
{
    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In RCListSub" << endline;

    // ********************************************
    // * Set up a iterator on the RListBlock and  *
    // * Grab the R_ tag line from the RListBlock *
    // ********************************************
    RWTValSlistIterator<DRWCString> iter(RListBlock);
    iter();                         // First Element in list

    // ************************************
    // * Are we dealing with a valid list *
    // ************************************
    if (!resList.isValid() || !resList.HierarchyContains(R_List_ID))
    {
        logf->error(LOGWEBCHANNEL) << "(R_WebServer) Could not find variable or it is not a List" 
                               << endline;

        return;
    }

    // ****************************************
    // * Get the specified list and setup an **
    // * interator on the list               **
    // ****************************************
    RWTValSlist<ResReference> &theList = ((R_List *) resList())->GetList();
    RWTValSlistIterator<ResReference> iterList(theList);

    ResReference resListEntry;
    
    // ******************************************
    // * Loop through each resource in the list *
    // ******************************************
    int doCount=0;
    while (iterList())
    {
        resListEntry = iterList.key();

        // ********************************************
        // * Keep reading the Html file until we get  *
        // * the </R_CLIST>                           *
        // ********************************************
        while(iter())
        {
            DRWCString strHtmlLine = iter.key();

            if (strHtmlLine.contains("</R_CLIST>"))
            {
                break;  // done with this substitution
            }

            // ***************
            // * Substitute  *
            // ***************
            if (VariableSub(strHtmlLine, resListEntry, doCount))
            {
                iFileLen += strHtmlLine.length() + 1;
                OutHtmlFile.append(strHtmlLine);
            }
        }

        // **********************************************
        // * Increment counter once for every list item *
        // **********************************************
        doCount++;

        iter.reset();
        iter();             // Skip the first line <R_LIST varname>
    }
}

// *********************************************************************
// *                                                                    
// * Function: VariableSub                                             
// *                                                                    
// * Description:   This function does the HTML Template substitution
// *                for the R_NAME and R_VALUE tags.
// *
// * Inputs: strHtmlLine - DRWCString containing the HTML line we are 
// *                        currently processing.
// *         resListEntry - ResReference containing the reference to the
// *                        resource from the R_List.
// *         doCount     - Integer that specifies whether this is a R_CLIST-1
// *                        or an R_LIST-0
// *                                                                    
// * Outputs: strHtmlLine - DRWCString containing all the necessary substitutions.
// *                                                                    
// * Returns: 1 if substitutions were made
// *          0 if not.
// *                                                                    
// *********************************************************************
int R_WebServer::VariableSub(DRWCString& strHtmlLine, ResReference resListEntry, 
                                int doCount)
{
    static char szCount[10];

    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In VariableSub" << endline;

    if (!resListEntry.isValid()) 
        return 0;

    if (doCount >= 0)   // negative number means no count
    {
        sprintf(szCount, "%d", doCount);
        strHtmlLine.replace("<R_NAME>", szCount);
        strHtmlLine.replace("R_NAME", szCount);
    }
    else
    {
        strHtmlLine.replace("<R_NAME>", resListEntry.Name().data());
        strHtmlLine.replace("R_NAME", resListEntry.Name().data());
    }
   
    strHtmlLine.replace("<R_VALUE>", resListEntry.StrValue().data());
    strHtmlLine.replace("R_VALUE", resListEntry.StrValue().data());

    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) Line after substitution - " 
                           << strHtmlLine << endline;
    
    // ****************************************
    // * Substitute the <R_CALL varname> tags *
    // ****************************************
    LineCallSub(strHtmlLine, resListEntry);

    return 1;
}

// *********************************************************************
// *                                                                    
// * Function: LineCallSub                                             
// *                                                                    
// * Description:   This function does the HTML Template substitution
// *                for the <R_CALL varname> tags, where varname is 
// *                an RSL method of r.  If the invocation returns an 
// *                error, eg if varname is an error, then the tag is 
// *                simply cut. This may not actually be an error, depending 
// *                on how the resource handles incoming messages.
// *
// * Inputs: strHtmlLine - DRWCString containing the HTML line we are 
// *                        currently processing.
// *         resListEntry - ResReference containing the reference to the
// *                        resource from the R_List.
// *                                                                    
// * Outputs: strHtmlLine - DRWCString containing all the necessary substitutions.
// *                                                                    
// * Returns: 1 if substitutions were made
// *          0 if not.
// *                                                                    
// *********************************************************************
int R_WebServer::LineCallSub(DRWCString& strHtmlLine, ResReference resListEntry)
{
    static RWCRegexp RXmatchGetVar("<R_CALL *[a-zA-Z_][a-zA-Z_0-9]*>");
    static RWCRegexp RXmatchCall("<R_CALL *");

    logf->debug(LOGWEBCHANNEL) << "(R_WebServer) In LineCallSub" << endline;

    if (strHtmlLine(RXmatchGetVar).isNull())
    {
        return 0;
    }

    DRWCString strLine = strHtmlLine;
    DRWCString strVarName;

	ResReference resResult;

    int iFreq = strLine.freq("<R_CALL");
    for(int i=0; i<iFreq; i++)
    {
        strLine = strLine.after(RXmatchCall);
        strVarName = strLine.before(">");

        // *******************************************
        // * Is this a resource of the resStructType *
        // *******************************************
        if ((resListEntry()->InternalType() == Resource::resStructType) ||
            (resListEntry()->InternalType() == Resource::resObjType))
        {
            logf->info(LOGWEBCHANNEL) << "(R_WebServer) Get Data Member'"
                                  << strVarName << "'" << endline;

            resResult = ((ResStructure *) resListEntry())->GetDataMember(strVarName);

        }

        // ************************************
        // * Have we gained access to he data *
        // ************************************
        if (resResult.isValid())
        {

            logf->info(LOGWEBCHANNEL) << "(R_WebServer) R_CALL Value - '" 
                                  << resResult.StrValue() << "'" << endline;

            strHtmlLine(RXmatchGetVar) = resResult.StrValue(); 
        }
        else
        {
            strHtmlLine(RXmatchGetVar) = "";
        }
    }

    return 1;
}
