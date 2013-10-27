
//this is only the most relevant pieces of what will be
//the final object templatizer resource class.
//The structure still needs to be generated with rsl2c++.

#include "Resource.h"

class R_objectTemplatizer : public ResObj {
    ResContext permanentNames;
    ResContext templates;

    int currentSizeKB;
    int upperBoundKB;

  public:
    
    void loadTemplate(RWCString templatePathname, int permanent);
    void loadTemplates(R_List& templatePathnames, int permanent);
    
    void clearCache();
    
    
};

	

/* ---- */


R_objectTemplatizer::clearCache()
{
    Clear();	// ResStructure::Clear()
    permanentNames.clear(); 	
}


/**
 * Give a list of template path names.
 */
R_objectTemplatizer::loadTemplates(R_List& templatePathnames, int permanent)
{
    
}

void R_objectTemplatizer::loadTemplate(RWCString templatePathname, int permanent)
{
    
}

