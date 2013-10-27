// *******************************************************************************
// *
// * Module Name: CryptoSubSystem.cc
// * 
// * Description: 
// *
// * History: Create - CKING 10/28/98
// * 
// * 
// * Copyright 1998 by Destiny Software Corporation.
// * 
// *******************************************************************************
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <stream.h>
#include <iostream.h>
#include <strstream.h>

#include "aglobal.h"
#include "bsafe.h"
#include "CryptoSubSystem.h"
#include "CryptoSession.h"

#include "CryptoDH.h"
#include "CryptoDHDE.h"
#include "CryptoRSApkcs.h"


extern int dhInitialized;
extern int pkInitialized;
extern B_ALGORITHM_METHOD *DH_SAMPLE_CHOOSER[];
extern B_ALGORITHM_METHOD *RSA_SAMPLE_CHOOSER[];
extern DHcentralAuthorityNodeParameters diffieHellmanParameters; 
extern RSAkey applicationPublicKey;
extern RSAkey applicationPrivateKey;
extern int GeneralSurrenderFunction PROTO_LIST ((POINTER handle));
extern void PrintBuf PROTO_LIST ((unsigned char *, unsigned int));
extern CryptoError transmitDataLength (iosockinet *io, int length);
extern CryptoError receiveDataLength (iosockinet *io, int *length);

#define MAX_INPUT_STREAM_BUFFER_LENGTH		50000
#define min(a, b)               ((a) > (b) ? (b) : (a))


#ifdef DEBUG_ENCRYPTION
int totalInboundMessages = 0;
#endif


void CryptoLog (char *pstrMessage)
{
    int iLen;

    iLen = strlen(pstrMessage);

    /* The atomic version uses locking to prevent interference */
//  write(fdDebugLog, pstrMessage, iLen);
    printf (pstrMessage);

    return;
}


CryptoError CryptoSubSystem::initialize (void)
{
    printf ("Granite core encryption support enabled.\n");

    if (!initialized)
    {
        initialized = TRUE;
    }

    return (CRYPTO_ERR_OK);

}   /* end initialize */
    

CryptoError CryptoSubSystem::resetSession (CryptoSession *session)
{
    sessionCrtlBlk findSn (session);
    sessionCrtlBlk *sn = NULL;
    CryptoTechnique *technique = NULL;
    
    
    if (session == NULL)
       return (CRYPTO_ERR_INVALID_SESSION);
       
        // remove technique associated with the session.       
    if ( (technique = session->getTechnique ()) != NULL)
    {
        delete technique;
        session->setTechnique (NULL);        
    }

        // remove session from sub-system session hash list.    
    if ( (sn = SessionCrtlBlkList.find (&findSn)) != NULL) 
    {
        SessionCrtlBlkList.remove (sn);
        delete (sn);  
    }
    session->setState (closed);
    
}   /* end resetSession */


CryptoError CryptoSubSystem::openSession (CryptoSession *session)
{
    CryptoError rc;
    sessionCrtlBlk * newSn = NULL;
    CryptoTechnique * technique = NULL;
 
    

    if (!initialized)
       return (CRYPTO_ERR_NOT_INITIALIZED);

    if (session == NULL) 
       return (CRYPTO_ERR_INVALID_SESSION);

    if (session->getState() == openned)
        return (CRYPTO_ERR_OK);
            
    session->setState (openning);
        
        // initialize the session with the appropriate technique object.
    switch (session->getTechniqueType ())
    {
        case CRYPTO_TECH_DH_DE:
                // Create technique object to be associated with the session.
            technique = new CryptoDHDETechnique ();
            if (technique == NULL)
            {
                resetSession (session);
                return (CRYPTO_ERR_MEMORY);
            }
            break;
            
        default:
            resetSession (session);
            return (CRYPTO_ERR_INVALID_SESSION);
            break;
    }

    session->setTechnique (technique);
               
        // Create new session entry, add entry to the hash table.
    newSn = new sessionCrtlBlk (session);
    if (newSn == NULL)
    {
        CryptoLog ("Fatal error: unable to allocate session.");
        resetSession (session);
        return (CRYPTO_ERR_MEMORY);
    }
        
    newSn->lock();
    SessionCrtlBlkList.insert (newSn);
       
        // Open the session via the technique associated with the session.
    rc = technique->openSessionTech (this, session);
    if (rc == CRYPTO_ERR_OK)
    {
        session->setState (openned);
        newSn->unlock();
    }
    else
    {
        resetSession (session);
    }

   
    return (rc);
    
}   /* end openSession */ 


CryptoError CryptoSubSystem::closeSession (CryptoSession *session)
{
    CryptoError rc;
    CryptoTechnique * technique = NULL;
    
    
    if (!initialized)
       return (CRYPTO_ERR_NOT_INITIALIZED);

    if ( (session == NULL) || (session->isValid () != CRYPTO_ERR_OK) )
       return (CRYPTO_ERR_INVALID_SESSION);
    
    technique = session->getTechnique ();
    technique->closeSessionTech (this, session);

    resetSession (session);
    session->setState (closed);
    
    return (CRYPTO_ERR_OK);

}   /* end closeSession */


CryptoError CryptoSubSystem::abortSession (CryptoSession *session)
{
    CryptoError rc;
    
    

    if (!initialized)
       return (CRYPTO_ERR_NOT_INITIALIZED);

    if ( (session == NULL) || (session->isValid () != CRYPTO_ERR_OK) )
       return (CRYPTO_ERR_INVALID_SESSION);
    
    closeSession (session); 
       
    return (CRYPTO_ERR_OK);
    
}   /* end abortSession */


CryptoError CryptoSubSystem::encryptData (CryptoSession *session, char *plaintext, int plength, char **ciphertext, int *clength)
{
    CryptoError rc;
    CryptoTechnique * technique = NULL;
    
    

    if (!initialized)
       return (CRYPTO_ERR_NOT_INITIALIZED);

    if ( (session == NULL) || (session->isValid () != CRYPTO_ERR_OK) )
       return (CRYPTO_ERR_INVALID_SESSION);

//    newSn->lock();
    
        // Perform encryption processing for session, via the technique associated with the session.
        //
    technique = session->getTechnique ();
    if ((rc = technique->encryptDataTech (this, session, plaintext, plength, ciphertext, clength)) == CRYPTO_ERR_OK)
    {
//        newSn->unlock();
    }
    
    return (rc);
    
}   /* end encryptData */


CryptoError CryptoSubSystem::encryptData (CryptoSession *session, iosockinet * io, char *plaintext, int plength)
{
    CryptoError rc;
    int iobytes;
    char *eciCiphertext;
    int eciCipherTextLength;
    CryptoSocketHandle peerNodeSocketHandle;
    char logbuf [100];
    
    

    if (!initialized)
       return (CRYPTO_ERR_NOT_INITIALIZED);

    if ( (session == NULL) || (session->isValid () != CRYPTO_ERR_OK) )
       return (CRYPTO_ERR_INVALID_SESSION);

#ifdef DEBUG_ENCRYPTION
    printf ("Before encrypting data (len = %d)\n", plength);
#endif    
    sprintf (logbuf, "Before encrypting data (len = %d)", plength);
    logMessage (DBUG, logbuf);

    rc = encryptData (session, plaintext, plength, &eciCiphertext, &eciCipherTextLength);
    if (rc == CRYPTO_ERR_OK)
    {
#ifdef DEBUG_ENCRYPTION
        printf ("Before sending encrypted data (len = %d)\n", eciCipherTextLength);
#endif
        sprintf (logbuf, "Before sending encrypted data (len = %d)", eciCipherTextLength);
        logMessage (DBUG, logbuf);
            // Write length value which indicates length of data to be transmitted
            // to node.        
        if ( (rc = transmitDataLength (io, eciCipherTextLength)) == CRYPTO_ERR_OK )
        {
                // Transmit data to the remote node.
            iobytes = (io->rdbuf())->write (eciCiphertext, eciCipherTextLength);
            if ( (iobytes < 0) || (iobytes != eciCipherTextLength) )
            {
                rc = CRYPTO_ERR_IO;
                sprintf (logbuf, "Error: socket write failed; error = %d: %s", errno, strerror(errno));
                logMessage (ERR, logbuf);
            }

#ifdef DEBUG_ENCRYPTION
            printf ("After  sending encrypted data (len = %d, io = %d)\n", eciCipherTextLength, iobytes);
#endif
            sprintf (logbuf, "After  sending encrypted data (len = %d, io = %d)", eciCipherTextLength, iobytes);
            logMessage (DBUG, logbuf);
    
        }
        else
        {
            sprintf (logbuf, "Error: socket write length failed; error = %d: %s", errno, strerror(errno));
            logMessage (ERR, logbuf);
        }

        free (eciCiphertext);	// free buffer allocated by encrption method
    }
    else
    {
#ifdef DEBUG_ENCRYPTION
        printf ("Error: unable to encrypt output data stream; error = %d.\n", rc);
#endif    
        sprintf (logbuf, "Error: unable to encrypt output data stream; error = %d.", rc);
        logMessage (ERR, logbuf);
    }

#ifdef DEBUG_ENCRYPTION
    printf ("After encryting data");
#endif    
    logMessage (DBUG, "After encryting data");
    
    return (rc);
    
}   /* end encryptData */


CryptoError CryptoSubSystem::decryptData (CryptoSession * session, char *ciphertext, int clength, char **plaintext, int *plength)
{
    CryptoError rc;
    CryptoTechnique * technique = NULL;
    
    

    if (!initialized)
       return (CRYPTO_ERR_NOT_INITIALIZED);

    if ( (session == NULL) || (session->isValid () != CRYPTO_ERR_OK) )
       return (CRYPTO_ERR_INVALID_SESSION);

//    newSn->lock();
    
        // Perform decryption processing for session, via the technique associated with the session.
        //
    technique = session->getTechnique ();
    if ((rc = technique->decryptDataTech (this, session, ciphertext, clength, plaintext, plength)) == CRYPTO_ERR_OK)
    {
//        newSn->unlock();
    }
    
    return (rc);
    
}   /* end decryptData */

    		     
CryptoError CryptoSubSystem::decryptData (CryptoSession * session, iosockinet * io, char **plaintext, int *plength)
{
    CryptoError crc;
    ostrstream _ostrstream;		// no buffer specification to the ostrstream, will cause the ostrstream
                                        // to manage any memory required to support the data stream directed to it.
    int ioRead;
    int eciPlainTextLength;
    int iolength = 0; 
    char *cryptographyBuffer = NULL;
    char *eciPlaintext = NULL;
    char logbuf [100];


#ifdef DEBUG_ENCRYPTION
    printf ("Before reading data\n");
#endif
    logMessage (DBUG, "Before reading data");

    eciPlaintext = NULL;
    cryptographyBuffer = (char *) malloc (MAX_INPUT_STREAM_BUFFER_LENGTH);
    if (cryptographyBuffer == NULL)
    {
       logMessage (ERR, "Error: Insufficient memory to allocate input stream buffer.");
       return (CRYPTO_ERR_MEMORY);
    }

        // Read length value which indicates length of data to be received
        // from remote node.        
    if (receiveDataLength (io, &iolength) != CRYPTO_ERR_OK)
    {
        sprintf (logbuf, "Error: unable to read data length; error = %d: %s", errno, strerror(errno));
        logMessage (ERR, logbuf);
        free (cryptographyBuffer);	// deallocate input stream buffer
        return (CRYPTO_ERR_IO);
    }
            
    memset (cryptographyBuffer, 0, MAX_INPUT_STREAM_BUFFER_LENGTH);

#ifdef DEBUG_ENCRYPTION
    printf ("Data bytes read length = %d\n", iolength);
#endif
    sprintf (logbuf, "Data bytes read length = %d", iolength);
    logMessage (DBUG, logbuf);

        // Read specified bytes of data from specified socket into the read buffer
        // for cryptography decryption processing.            
    while (_ostrstream.pcount () < iolength)
    {
        ioRead = (io->rdbuf())->read (cryptographyBuffer, 
                                     min (iolength - _ostrstream.pcount (), MAX_INPUT_STREAM_BUFFER_LENGTH - 1));
        if (ioRead == 0)
        {
            free (cryptographyBuffer);	// deallocate input stream buffer
            return (CRYPTO_ERR_IO);	// no data received to decrypt
        }
        else if (ioRead < 0)
        {
#ifdef DEBUG_ENCRYPTION
            printf ("Error: socket read failed; error = %d: %s\n", errno, strerror(errno));
#endif                
            sprintf (logbuf, "Error: socket read failed; error = %d: %s", errno, strerror(errno));
            logMessage (ERR, logbuf); 

            free (cryptographyBuffer);	// deallocate input stream buffer
            return (CRYPTO_ERR_IO);
        }
            
        _ostrstream.write (cryptographyBuffer, ioRead);	// write data to memory output string stream object. 
        
#ifdef DEBUG_ENCRYPTION
        printf ("    data bytes read (len = %d)\n", _ostrstream.pcount());
#endif        
        sprintf (logbuf, "    data bytes read (len = %d)", _ostrstream.pcount());
        logMessage (DBUG, logbuf);

    }    /* end while */

#ifdef DEBUG_ENCRYPTION
    printf ("    data bytes read (len = %d)\n", _ostrstream.pcount ());
#endif
    sprintf (logbuf, "    data bytes read (len = %d)", _ostrstream.pcount ());
    logMessage (DBUG, logbuf);

    if (_ostrstream.pcount () != iolength)
    {
#ifdef DEBUG_ENCRYPTION
        printf ("Error: too many bytes read\n", errno, strerror(errno));
#endif                
        free (cryptographyBuffer);	// deallocate input stream buffer
        logMessage (ERR, "Too many bytes read"); 
        return (CRYPTO_ERR_IO);
    }
    
        // Decrypt ECI request via specified cryptography session.
#ifdef DEBUG_ENCRYPTION
    printf ("Before decrypting data (len = %d)\n", _ostrstream.pcount ());
#endif    
    sprintf (logbuf, "Before decrypting data (len = %d)", _ostrstream.pcount ());
    logMessage (DBUG, logbuf);

    crc = decryptData (session, _ostrstream.str (), _ostrstream.pcount (), &eciPlaintext, &eciPlainTextLength);
    if (crc != CRYPTO_ERR_OK)
    {
        free (cryptographyBuffer);	// deallocate input stream buffer

#ifdef DEBUG_ENCRYPTION
        printf ("Error: unable to decrypt input data stream; error = %d.\n", crc);
#endif
        sprintf (logbuf, "Error: unable to decrypt input data stream; error = %d.", crc);
        logMessage (ERR, logbuf);
        return (crc);
    }
           
#ifdef DEBUG_ENCRYPTION
    printf ("Total Messages Received = %d\n", ++totalInboundMessages);
    sprintf (logbuf, "Total Messages Received = %d", ++totalInboundMessages);
    logMessage (DBUG, logbuf);
#endif

    free (cryptographyBuffer);		// deallocate input stream buffer
    
    *plaintext = eciPlaintext;
    *plength = eciPlainTextLength - 1;

#ifdef DEBUG_ENCRYPTION
    printf ("After decrypting data (len = %d, io = %d)\n", eciPlainTextLength - 1, _ostrstream.pcount());
#endif
    sprintf (logbuf, "After decrypting data (len = %d, io = %d)", eciPlainTextLength - 1, _ostrstream.pcount());
    logMessage (DBUG, logbuf);
    
    return (CRYPTO_ERR_OK);

}   /* end decryptData */


CryptoError CryptoSubSystem::mgntKey (CryptoSession *session)
{
    CryptoError rc;
    
    
    if ( (session == NULL) || (session->isValid () != CRYPTO_ERR_OK) )
       return (CRYPTO_ERR_INVALID_SESSION);
    
    return (CRYPTO_ERR_OK);
    
}   /* end mgntKey */


CryptoError CryptoSubSystem::mgntKeys (void)
{
    return (CRYPTO_ERR_OK);
    
}   /* end mgntKeys */
    

CryptoError CryptoSubSystem::resetKeyPointer (void)
{
    return (CRYPTO_ERR_NOT_IMPLEMENTED);
    
}   /* end resetKeyPointer */


CryptoError CryptoSubSystem::getKey (CryptoKey *key)
{
    if (key == NULL)
    {
        return (CRYPTO_ERR_INVALID_KEY);
    }
    
    return (CRYPTO_ERR_NOT_IMPLEMENTED);
    
}   /* end getKey */


CryptoError CryptoSubSystem::getNextKey (CryptoKey *key)
{
    if (key == NULL)
    {
        return (CRYPTO_ERR_INVALID_KEY);
    }
    
    return (CRYPTO_ERR_NOT_IMPLEMENTED);
    
}   /* end getNextKey */


CryptoError CryptoSubSystem::getKey (CryptoSession *session, CryptoKey *key)
{
    if ( (key == NULL) || (session->isValid () != CRYPTO_ERR_OK) )
    {
        return (CRYPTO_ERR_INVALID_KEY);
    }
    
    return (CRYPTO_ERR_NOT_IMPLEMENTED);
    
}   /* end getKey */


CryptoError CryptoSubSystem::getNextKey (CryptoTechniqueType techType, CryptoKey *key)
{
    if (key == NULL)
    {
        return (CRYPTO_ERR_INVALID_KEY);
    }
    
    return (CRYPTO_ERR_NOT_IMPLEMENTED);
    
}   /* end getNextKey */

   
CryptoError CryptoSubSystem::initializeDiffieHellmanParameters (void)
{
    CryptoError rc = CRYPTO_ERR_OK;
    time_t currentTime;
    B_ALGORITHM_OBJ randomAlgorithm = (B_ALGORITHM_OBJ)NULL_PTR;
    B_ALGORITHM_OBJ dhParamGenerator = (B_ALGORITHM_OBJ)NULL_PTR;
    B_ALGORITHM_OBJ dhParametersObj = (B_ALGORITHM_OBJ)NULL_PTR;

    A_DH_PARAM_GEN_PARAMS dhParams;

    POINTER randomSeed = NULL_PTR;
    unsigned int randomSeedLen;
  
    A_SURRENDER_CTX generalSurrenderContext;

    ITEM *bsafeDHParametersBER;
    ITEM myDHParametersBER;

    unsigned int status, generalFlag;
    char logbuf[256];



    if (!initialized)
       return (CRYPTO_ERR_NOT_INITIALIZED);

    printf ("Please wait, while initializating Diffie-Hellman Algorithm parameters.\n");

    // Generate Diffie-Hellman parameters for peer nodes requesting the parameters.
    // This process can be time consuming, therefore, the generation of the is only 
    // done once, and stored for future requests.
    //

    generalSurrenderContext.Surrender = GeneralSurrenderFunction;
    generalSurrenderContext.handle = (POINTER)&generalFlag;
    generalSurrenderContext.reserved = NULL_PTR;
    
    myDHParametersBER.data = NULL_PTR;

    do
    {
        logMessage (DBUG, "Diffie-Hellman Algorithm ");
        logMessage (DBUG, "======================== ");

        /*  ========================================================  */
        /*  The following code was pulled from genbytes.c, steps 1-4.  
         */
        if ((status = B_CreateAlgorithmObject (&randomAlgorithm)) != 0)
            break;

        if ((status = B_SetAlgorithmInfo
           (randomAlgorithm, AI_SHA1Random, NULL_PTR)) != 0)
        break;

        if ((status = B_RandomInit
             (randomAlgorithm, DH_SAMPLE_CHOOSER,
             (A_SURRENDER_CTX *)NULL_PTR)) != 0)
        break;

        randomSeedLen = 256;
        randomSeed = T_malloc (randomSeedLen);
        if ((status = (randomSeed == NULL_PTR)) != 0)
           break;

        T_memset (randomSeed, 0, randomSeedLen);

        // Get current time as the random seed value, in order to generate a new
        // Diffie-Hellman parameters.
        time (&currentTime);
        strcpy ((char *) randomSeed, ctime(&currentTime));
        //printf ("Random seed value = %s", randomSeed);
          
#ifdef DEBUG_CODE
        puts ("Enter a random seed");
        if ((status =
            (NULL_PTR == (unsigned char *)gets ((char *)randomSeed))) != 0)
           break;
#endif
                
        if ((status = B_RandomUpdate
            (randomAlgorithm, randomSeed, randomSeedLen,
            (A_SURRENDER_CTX *)NULL_PTR)) != 0)
           break;
        /*  ========================================================  */
    
        logMessage (DBUG, "   Generating DH parameters -- will take awhile");
        logMessage (DBUG, "   ============================================ ");

        /*  Step 1:  Create algorithm object */
        if ((status = B_CreateAlgorithmObject (&dhParamGenerator)) != 0)
           break;

        /*  Step 2:  Set algorithm object to AI_DHParamGen
         */
        dhParams.primeBits = DH_MAX_PRIME_BIT_LENGTH;
        dhParams.exponentBits = 504;
        if ((status = B_SetAlgorithmInfo
            (dhParamGenerator, AI_DHParamGen,
            (POINTER)&dhParams)) != 0)
          break;

        /*  Step 3:  Init */
        if ((status = B_GenerateInit
            (dhParamGenerator, DH_SAMPLE_CHOOSER,
            (A_SURRENDER_CTX *)NULL_PTR)) != 0)
          break;

        /*  Step 4:  no step 4 (Update) in generating DH parameters */

        /*  Step 5:  Generate DH parameters
         */
        if ((status = B_CreateAlgorithmObject (&dhParametersObj)) != 0)
           break;

        /*  generalFlag is for the surrender function. */
        generalFlag = 0;
#ifdef DEBUG_SURRENDER        
        if ((status = B_GenerateParameters
            (dhParamGenerator, dhParametersObj, randomAlgorithm,
             &generalSurrenderContext)) != 0)
           break;
#endif           
        if ((status = B_GenerateParameters
            (dhParamGenerator, dhParametersObj, randomAlgorithm,
             (A_SURRENDER_CTX *)NULL_PTR)) != 0)
           break;

        logMessage (DBUG, "   Distributing DH parameters ");
        logMessage (DBUG, "   ========================== ");

        if ((status = B_GetAlgorithmInfo
            ((POINTER *)&bsafeDHParametersBER, dhParametersObj,
             AI_DHKeyAgreeBER)) != 0)
          break;

        myDHParametersBER.len = bsafeDHParametersBER->len;
        myDHParametersBER.data = T_malloc (myDHParametersBER.len);
        if ((status = (myDHParametersBER.data == NULL_PTR)) != 0)
            break;
        T_memcpy (myDHParametersBER.data, bsafeDHParametersBER->data,
                  myDHParametersBER.len);

        sprintf (logbuf, "The DH parameters (%u bytes):", myDHParametersBER.len);
        logMessage (DBUG, logbuf);
        PrintBuf (myDHParametersBER.data, myDHParametersBER.len);

    } while (0);
    
    if (status != 0)
    {
        sprintf (logbuf, "Status = %i ", status);
        logMessage (ERR, logbuf);
        logMessage (ERR, "DH Parameter Generation failed.");

        printf ("DH Parameter Generation failed.\n");

        rc = CRYPTO_ERR_DH;
    }  
    else
    {
        memset (&diffieHellmanParameters, 0, sizeof(diffieHellmanParameters));
        diffieHellmanParameters.dataLength = myDHParametersBER.len;
        diffieHellmanParameters.data = (char *) malloc (myDHParametersBER.len);
        if (diffieHellmanParameters.data == NULL)
            rc = CRYPTO_ERR_MEMORY;
        else
        {
            memcpy (diffieHellmanParameters.data, myDHParametersBER.data, myDHParametersBER.len);
            dhInitialized = TRUE;
        }
    }

    /*  Step 6:  Destroy resource associated with the parameters
     */
    B_DestroyAlgorithmObject (&randomAlgorithm);
    B_DestroyAlgorithmObject (&dhParametersObj);
    B_DestroyAlgorithmObject (&dhParamGenerator);

    /*  Free up any memory allocated
     */
    T_free (randomSeed);
    T_free (myDHParametersBER.data);

    printf ("Diffie-Hellman Algorithm parameters initialization completed successfully.\n");

    return (rc);

}   /*  end initializeDiffieHellmanParameters */


CryptoError CryptoSubSystem::initializePublicPrivateKeys (void)
{
    CryptoError rc = CRYPTO_ERR_OK;
    time_t currentTime;
    int generalFlag;
    unsigned int status;
    unsigned int randomSeedLen;
    A_RSA_KEY_GEN_PARAMS keygenParams;
    A_SURRENDER_CTX generalSurrenderContext;
    POINTER randomSeed = NULL_PTR;
    ITEM *getPrivateKey = (ITEM *)NULL_PTR;
    B_ALGORITHM_OBJ randomAlgorithm = (B_ALGORITHM_OBJ)NULL_PTR;
    B_ALGORITHM_OBJ keypairGenerator = (B_ALGORITHM_OBJ)NULL_PTR;
    B_KEY_OBJ publicKeyObject = (B_KEY_OBJ)NULL_PTR;
    B_KEY_OBJ privateKeyObject = (B_KEY_OBJ)NULL_PTR;
    ITEM myPublicKeyBER;
    ITEM privateKeyBER;
    ITEM *bsafePublicKeyBER;
    static unsigned char f4Data[3] = {0x01, 0x00, 0x01};
    char logbuf[256];
    
    
    myPublicKeyBER.data = NULL_PTR;
    privateKeyBER.data = NULL_PTR;

    generalSurrenderContext.Surrender = GeneralSurrenderFunction;
    generalSurrenderContext.handle = (POINTER)&generalFlag;
    generalSurrenderContext.reserved = NULL_PTR;

    if (!initialized)
       return (CRYPTO_ERR_NOT_INITIALIZED);

    printf ("Please wait, while generating RSA Pkcs Keys.\n");

    do
    {
        logMessage (DBUG, "Generating random bytes ");
        logMessage (DBUG, "======================= ");

        if ((status = B_CreateAlgorithmObject (&randomAlgorithm)) != 0)
            break;

        if ((status = B_SetAlgorithmInfo (randomAlgorithm, AI_SHA1Random, NULL_PTR)) != 0)
            break;

        if ((status = B_RandomInit (randomAlgorithm, RSA_SAMPLE_CHOOSER,
                                    (A_SURRENDER_CTX *)NULL_PTR)) != 0)
            break;

        randomSeedLen = 256;
        randomSeed = T_malloc (randomSeedLen);
        if ((status = (randomSeed == NULL_PTR)) != 0)
            break;

        T_memset (randomSeed, 0, randomSeedLen);

        // Get current time as the random seed value, in order to generate a new
        // RSA private-key and public-key.
        time (&currentTime);
        strcpy ((char *) randomSeed, ctime(&currentTime));
        //printf ("Random seed value = %s", randomSeed);
          
#ifdef DEBUG_CODE
        puts ("Enter a random seed");
        if ((status = (NULL_PTR == (unsigned char *)gets ((char *)randomSeed))) != 0)
            break;
#endif
        
        if ((status = B_RandomUpdate (randomAlgorithm, randomSeed, randomSeedLen,
                                      (A_SURRENDER_CTX *)NULL_PTR)) != 0)
            break;

        logMessage (DBUG, "   Generating a Keypair ");
        logMessage (DBUG, "   ==================== ");

        /*  Step 1:  Create algorithm object */
        if ((status = B_CreateAlgorithmObject (&keypairGenerator)) != 0)
            break;

        /*  Step 2:  Set algorithm object to AI_RSAKeyGen
         */
        keygenParams.modulusBits = RSA_MODULUS_BITS;
        keygenParams.publicExponent.data = f4Data;
        keygenParams.publicExponent.len = 3;

        if ((status = B_SetAlgorithmInfo (keypairGenerator, AI_RSAKeyGen, (POINTER)&keygenParams)) != 0)
            break;

        /*  Step 3:  Init */
        if ((status = B_GenerateInit (keypairGenerator, RSA_SAMPLE_CHOOSER,
                                      (A_SURRENDER_CTX *)NULL_PTR)) != 0)
            break;

        /*  Step 4:  no step 4 (Update) in generating a keypair */

        /*  Step 5:  Generate
         */
        if ((status = B_CreateKeyObject (&publicKeyObject)) != 0)
          break;
          
        if ((status = B_CreateKeyObject (&privateKeyObject)) != 0)
            break;

        /*  generalFlag is for the surrender function */
        generalFlag = 0;
#ifdef DEBUG_SURRENDER        
        if ((status = B_GenerateKeypair (keypairGenerator, publicKeyObject, privateKeyObject,
                                         randomAlgorithm, &generalSurrenderContext)) != 0)
            break;
#endif
        if ((status = B_GenerateKeypair (keypairGenerator, publicKeyObject, privateKeyObject,
                                         randomAlgorithm, (A_SURRENDER_CTX *)NULL_PTR)) != 0)
            break;
            
    
        logMessage (DBUG, "   Distributing Your Public Key ");
        logMessage (DBUG, "   ============================ ");

        myPublicKeyBER.data = NULL_PTR;
        if ((status = B_GetKeyInfo ((POINTER *)&bsafePublicKeyBER, publicKeyObject, KI_RSAPublicBER)) != 0)
            break;

        myPublicKeyBER.len = bsafePublicKeyBER->len;
        myPublicKeyBER.data = T_malloc (myPublicKeyBER.len);

        if ((status = (myPublicKeyBER.data == NULL_PTR)) != 0)
            break;
        T_memcpy (myPublicKeyBER.data, bsafePublicKeyBER->data, myPublicKeyBER.len);

        logMessage (DBUG, "The public key DER-encoded as X.509 SubjectPublicKeyInfo type");
        sprintf (logbuf, " (%u bytes):", myPublicKeyBER.len);
        logMessage (DBUG, logbuf);
        PrintBuf (myPublicKeyBER.data, myPublicKeyBER.len);


        privateKeyBER.data = NULL_PTR;
        if ((status = B_GetKeyInfo ((POINTER *)&getPrivateKey, privateKeyObject, KI_PKCS_RSAPrivateBER)) != 0)
            break;

        privateKeyBER.len = getPrivateKey->len;
        privateKeyBER.data = T_malloc (privateKeyBER.len);

        if ((status = (privateKeyBER.data == NULL_PTR)) != 0)
            break;
        T_memcpy (privateKeyBER.data, getPrivateKey->data, privateKeyBER.len);

        logMessage (DBUG, "The private key encoded as a PKCS #8 PrivateKeyInfo type");
        sprintf (logbuf, " (%u bytes):", privateKeyBER.len);
        logMessage (DBUG, logbuf);
        PrintBuf (privateKeyBER.data, privateKeyBER.len);

    } while (0);

    if (status == 0)
    {
        memset (&applicationPublicKey, 0, sizeof(applicationPublicKey));
        applicationPublicKey.keyDataLength = myPublicKeyBER.len;
        applicationPublicKey.keyData = (char *) malloc (myPublicKeyBER.len);
        if (applicationPublicKey.keyData == NULL)
            rc = CRYPTO_ERR_MEMORY;
        else
        {
            memcpy (applicationPublicKey.keyData, myPublicKeyBER.data, myPublicKeyBER.len);

            memset (&applicationPrivateKey, 0, sizeof(applicationPrivateKey));
            applicationPrivateKey.keyDataLength = privateKeyBER.len;
            applicationPrivateKey.keyData = (char *) malloc (privateKeyBER.len);
            if (applicationPrivateKey.keyData == NULL)
            {
                rc = CRYPTO_ERR_MEMORY;
                free (applicationPublicKey.keyData);
            }
            else
            {
                memcpy (applicationPrivateKey.keyData, privateKeyBER.data, privateKeyBER.len);
                pkInitialized = TRUE;
            }
        }
    }
    else
    {
        sprintf (logbuf, "Status = %i ", status);
        logMessage (ERR, logbuf);
        logMessage (ERR, "RSA Public Key generation failed.");

        printf ("RSA Public Key generation failed.\n");
        
        rc = CRYPTO_ERR_RSA_PUBLIC_KEY;
    }  

    /*  Step 6:  Destroy
     */
    B_DestroyAlgorithmObject (&randomAlgorithm);
    B_DestroyKeyObject (&publicKeyObject);
    publicKeyObject = (B_KEY_OBJ)NULL_PTR;

    B_DestroyKeyObject (&privateKeyObject);
    privateKeyObject = (B_KEY_OBJ)NULL_PTR;

    B_DestroyAlgorithmObject (&keypairGenerator);
    keypairGenerator = (B_ALGORITHM_OBJ)NULL_PTR;

    /*  Free up any memory allocated
     */
    T_free (randomSeed);
    T_free (myPublicKeyBER.data);
    T_free (privateKeyBER.data);

    printf ("Generation of RSA Pkcs Keys completed successfully.\n");

    return (rc);
    
}   /* end initializePublicPrivateKeys */

CryptoSubSystem::CryptoSubSystem (void)
{
}   /* end constructor */


CryptoSubSystem::~CryptoSubSystem (void)
{
}   /* end destructor */
