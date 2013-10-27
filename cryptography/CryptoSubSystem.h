// *****************************************************************************
// *
// * Cryptography Management SubSystem Classes
// *
// * History: Create - CKING 10/26/98
// * 
// * 
// * Copyright 1998 by Destiny Software Corporation.
// * 
// *****************************************************************************
#ifndef _CRYPTO_SUBSYSTEM_H_
#define _CRYPTO_SUBSYSTEM_H_

#include <stdlib.h>
#include <sockinet.h>


#define	TRUE	1
#define FALSE	0
#define CRYPTO_LOG_BUFFER_LENGTH	512


struct RSAkey {
    char *keyData;
    int keyDataLength;
};

enum CryptoError
{
    CRYPTO_ERR_OK = 0,
    CRYPTO_ERR_NOT_IMPLEMENTED,
    CRYPTO_ERR_INVALID_SESSION,
    CRYPTO_ERR_INVALID_KEY,
    CRYPTO_ERR_MEMORY,
    CRYPTO_ERR_DH,
    CRYPTO_ERR_DHDE,
    CRYPTO_ERR_DE,
    CRYPTO_ERR_RC4,
    CRYPTO_ERR_RC2,
    CRYPTO_ERR_RSA_PUBLIC_KEY,
    CRYPTO_ERR_NOT_INITIALIZED,
    CRYPTO_ERR_IO,  
    CRYPTO_ERR_EOF 
};

enum CryptoLogMsgType
{
    INFO = 0,
    DBUG,
    ERR
};

enum CryptoTechniqueType
{
    CRYPTO_TECH_NONE = 0,
    CRYPTO_TECH_DH_DE   
};

enum CryptoKeyMgntType
{
    CRYPTO_KEYM_NONE = 0
};

enum CryptoSnStateType
{
    closed = 0,
    openning,
    openned,
    closing
};

struct CryptoKey {
    void *pSymmetricKey;
    void *pPrivateKey;
    void *pPublicKey;
};

class CryptoSession;
class CryptoTechnique;
class CryptoSubSystem;


//
// Cryptography Technique Base Class 
//
class CryptoTechnique {
private:
      
public:
         // methods
    virtual CryptoError openSessionTech (CryptoSubSystem *subsystem, CryptoSession *session) { return (CRYPTO_ERR_NOT_IMPLEMENTED); }
    virtual CryptoError closeSessionTech (CryptoSubSystem *subsystem, CryptoSession *session) { return (CRYPTO_ERR_NOT_IMPLEMENTED); }
    virtual CryptoError abortSessionTech (CryptoSubSystem *subsystem, CryptoSession *session) { return (CRYPTO_ERR_NOT_IMPLEMENTED); }
    virtual CryptoError encryptDataTech (CryptoSubSystem *subsystem, CryptoSession *session,
    					 char *plaintext,
    			         	 int plength,
    			         	 char **ciphertext,
    			         	 int *clength) { return (CRYPTO_ERR_NOT_IMPLEMENTED); }
    virtual CryptoError decryptDataTech (CryptoSubSystem *subsystem, CryptoSession *session, 
    					 char *ciphertext,
    		         	 	 int clength,
    		         	 	 char **plaintext,
    		         	 	 int *plength) { return (CRYPTO_ERR_NOT_IMPLEMENTED); }
    virtual CryptoError getSymmetricKey (void **key, int *length) { return (CRYPTO_ERR_NOT_IMPLEMENTED); }
    virtual CryptoError getPrivateKey (void **key, int *length) { return (CRYPTO_ERR_NOT_IMPLEMENTED); }
    virtual CryptoError getPublicKey (void **key, int *length) { return (CRYPTO_ERR_NOT_IMPLEMENTED); }
    
    CryptoTechnique (void) { return; }
    ~CryptoTechnique (void) { return; }
};


//
// Cryptography Base Session Descriptor Class 
//
class CryptoSession {
private:
         // attributes
    CryptoTechniqueType techniqueType;
    CryptoTechnique *technique;
    CryptoKeyMgntType keyMgntType;
    CryptoSnStateType state;
    
public:
         // methods
    virtual CryptoError isValid (void) { return (CRYPTO_ERR_INVALID_SESSION); }
    
    inline void setTechniqueType (CryptoTechniqueType type) { techniqueType = type; }
    inline CryptoTechniqueType getTechniqueType (void) { return (techniqueType); }    

    inline void setState (CryptoSnStateType st) { state = st; }
    inline CryptoSnStateType getState (void) { return (state); }    

    inline void setTechnique (CryptoTechnique *tech) { technique = tech; }
    inline CryptoTechnique * getTechnique (void) { return (technique); }

    inline void setKeyMgntType (CryptoKeyMgntType type) { keyMgntType = type; }
    inline CryptoKeyMgntType getKeyMgntType (void) { return (keyMgntType); }
       
    CryptoSession (void)
    {
        setTechniqueType (CRYPTO_TECH_NONE);
        setTechnique (NULL);
        setKeyMgntType (CRYPTO_KEYM_NONE);
        setState (closed);
    }
    ~CryptoSession (void) { return; }
};

//
// Cryptography Management Sub-System Class 
//
class CryptoSubSystem {
private:
         // variables
    int initialized;
         // methods
    CryptoError resetSession (CryptoSession *session);
    
public:
         // methods
    CryptoError initialize (void);
    
         // general session operation related methods
    CryptoError openSession (CryptoSession *session);
    CryptoError closeSession (CryptoSession *session);
    CryptoError abortSession (CryptoSession *session);
    CryptoError encryptData (CryptoSession *session,
    		     	    char *plaintext,
    		    	    int plength,
    		    	    char **ciphertext,
    		     	    int *clength);
    CryptoError encryptData (CryptoSession *session, iosockinet * io, char *plaintext, int plength);
    CryptoError decryptData (CryptoSession * session,
    		     	   char *ciphertext,
    		     	   int clength,
    		     	   char **plaintext,
    		     	   int *plength);
    CryptoError decryptData (CryptoSession * session, iosockinet * io, char **plaintext, int *plength);
    CryptoError initializeDiffieHellmanParameters (void);
    CryptoError initializePublicPrivateKeys (void);
    		     
         // general session key management related methods
    CryptoError mgntKey (CryptoSession *session);
    CryptoError mgntKeys (void);
    
         // advanced session key management related methods
    CryptoError resetKeyPointer (void);
    CryptoError getKey (CryptoKey *key);
    CryptoError getNextKey (CryptoKey *key);
    CryptoError getKey (CryptoSession *session, CryptoKey *key);
    CryptoError getNextKey (CryptoTechniqueType techType, CryptoKey *key);

    virtual void logMessage (CryptoLogMsgType msgType, char *pstrMessage);
   
    CryptoSubSystem (void);
    ~CryptoSubSystem (void);
};

class _CryptoSubSystem : public CryptoSubSystem {
    void logMessage (CryptoLogMsgType msgType, char *pstrMessage);
};

#include "CryptoDHDE.h"
#endif
