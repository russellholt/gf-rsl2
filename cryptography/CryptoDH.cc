// *******************************************************************************
// *
// * Module Name: CryptoDH.cc
// * 
// * Description: 
// *
// * History: Create - CKING 10/28/98
// * 
// * 
// * Copyright 1998 by Destiny Software Corporation.
// * 
// *******************************************************************************
#include <stdio.h>
#include <stdlib.h>   
#include <string.h>  
#include <time.h>
#include <errno.h>
#include <sockinet.h>
extern "C" 
{
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/filio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
}

#include "CryptoDH.h"
#include "CryptoLog.h"


extern int GeneralSurrenderFunction PROTO_LIST ((POINTER handle));
extern void PrintBuf PROTO_LIST ((unsigned char *, unsigned int));

B_ALGORITHM_METHOD *DH_SAMPLE_CHOOSER[] = {
  &AM_SHA_RANDOM,
  &AM_DH_PARAM_GEN,
  (B_ALGORITHM_METHOD *)NULL_PTR
};

B_ALGORITHM_METHOD *DH_AGREE_SAMPLE_CHOOSER[] = {
  &AM_SHA_RANDOM,
  &AM_DH_KEY_AGREE,
  (B_ALGORITHM_METHOD *)NULL_PTR
};

int dhInitialized = 0;
static DHuserNodeKeys centralAuthorityNodeKeys;
DHcentralAuthorityNodeParameters diffieHellmanParameters = {0, NULL}; 


#define min(a, b)               ((a) > (b) ? (b) : (a))


CryptoError transmitDataLength (iosockinet *io, int length)
{
    int data = length;
    int iobytes;


    iobytes = (io->rdbuf())->write (&data, sizeof(int)); 
    if ( (iobytes < 0) || (iobytes != sizeof(int)) )
    {
        return (CRYPTO_ERR_IO);
    }
    return (CRYPTO_ERR_OK);

}   /* end transmitDataLength */


CryptoError receiveDataLength (iosockinet *io, int *length)
{
    int data = 0;
    int bytesRead;


    *length = 0;
    while (1)
    {
        if ((bytesRead = (io->rdbuf())->read (&data, sizeof(int))) <= 0)
        {
            return (CRYPTO_ERR_IO);	// receive error or receive timeout has occurred
        }
            // Sometimes garbage characters are in the socket stream, try again. 
        if (bytesRead == sizeof(int))
            break;
    }
       
    *length = data;
    
    return (CRYPTO_ERR_OK);

}   /* end receiveDataLength */

            
CryptoError CryptoRsaDiffieHellman::receiveCentralAuthorityNodeParameters (void)
{
    int bytesRead;
    

        // Receive Diffie-Hellman parameters from central authority node, in order to
        // perform Diffie-Hellman Key Agreement algorithm. 
    memset (caResponseBuffer, 0, MAX_CA_RESPONSE_LENGTH);
       
    switch (caNodeSocketHandle.type)
    {
        case CRYPTO_SOCKET_INT:
            int *s;
            s = (int *) caNodeSocketHandle.handle;

            sprintf (logbuf, "Before receiving Diffie-Hellman parameters from central authority (rd len = %d)", MAX_CA_RESPONSE_LENGTH);
            cryptosubsystem->logMessage (DBUG, logbuf);
     
            if ((bytesRead = read (*s, caResponseBuffer, MAX_CA_RESPONSE_LENGTH)) < 0)
            {
                sprintf (logbuf, "Error: socket read failed; error = %d: %s", errno, strerror(errno));
                cryptosubsystem->logMessage (ERR, logbuf);
                return (CRYPTO_ERR_DH);
            }
            break;
            
        case CRYPTO_SOCKET_IOSTREAM:
            iosockinet *io;
            int iolength;
            io = (iosockinet *) caNodeSocketHandle.handle; 

                // Read length value which indicates length of data to be received
                // from node.        
            if (receiveDataLength (io, &iolength) != CRYPTO_ERR_OK)
            {
                sprintf (logbuf, "Error: socket read failed; error = %d: %s", errno, strerror(errno));
                cryptosubsystem->logMessage (ERR, logbuf);
                return (CRYPTO_ERR_IO);
            }
            
            sprintf (logbuf, "Before receiving Diffie-Hellman parameters from central authority (rd len = %d)", 
                     min (iolength, MAX_CA_RESPONSE_LENGTH));
            cryptosubsystem->logMessage (DBUG, logbuf);
     
            if ((bytesRead = (io->rdbuf())->read (caResponseBuffer, min (iolength, MAX_CA_RESPONSE_LENGTH))) < 0)
            {
                sprintf (logbuf, "Error: socket read failed; error = %d: %s", errno, strerror(errno));
                cryptosubsystem->logMessage (ERR, logbuf);
                return (CRYPTO_ERR_DH);
            }
            break;
            
        default:
                cryptosubsystem->logMessage (ERR, "Error: invalid socket type");
                return (CRYPTO_ERR_DH);
           break;
    }
        
    sprintf (logbuf, "After receiving Diffie-Hellman parameters from central authority (io len = %d)", bytesRead);
    cryptosubsystem->logMessage (DBUG, logbuf);
    
        // Initialize diffie-hellman global structure for future session to access,
        // instead of querying the central authority node.

        // diffie-hellman global structure may have previously been initialized by
        // another session, if so, free the memory associated with the parameters. 
    if ( dhInitialized && (diffieHellmanParameters.data != NULL) )
        free (diffieHellmanParameters.data);        

    memset (&diffieHellmanParameters, 0, sizeof(diffieHellmanParameters));
    diffieHellmanParameters.dataLength = bytesRead;
    diffieHellmanParameters.data = (char *) malloc (bytesRead);
    if (diffieHellmanParameters.data == NULL)
        return (CRYPTO_ERR_MEMORY);
        
    memcpy (diffieHellmanParameters.data, caResponseBuffer, bytesRead);

    dhInitialized = TRUE;
    
    dhParametersBER.len = diffieHellmanParameters.dataLength;
    dhParametersBER.data = (unsigned char *) diffieHellmanParameters.data;
    
    return (CRYPTO_ERR_OK);

}   /* end receiveCentralAuthorityNodeParameters */ 

    
CryptoError CryptoRsaDiffieHellman::receivePublicKeyFromPeerNode (void)
{
    int bytesRead = 0;
    

         // Receive peer node's public-key value.
         //
    switch (peerNodeSocketHandle.type)
    {
        case CRYPTO_SOCKET_INT:
            int *s;
            s = (int *) peerNodeSocketHandle.handle;

            sprintf (logbuf, "Before receiving public-key value of peer node (rd len = %d)", getParams->prime.len);
            cryptosubsystem->logMessage (DBUG, logbuf);

            if ((bytesRead = read (*s, otherPublicValue, getParams->prime.len)) < 0)
            {
                sprintf (logbuf, "Error: socket read failed; error = %d: %s", errno, strerror(errno));
                cryptosubsystem->logMessage (ERR, logbuf);
                return (CRYPTO_ERR_DH);
            }
            break;
            
        case CRYPTO_SOCKET_IOSTREAM:
            iosockinet *io;
            int iolength;
            io = (iosockinet *) peerNodeSocketHandle.handle;       

                // Read length value which indicates length of data to be received
                // from node.        
            if (receiveDataLength (io, &iolength) != CRYPTO_ERR_OK)
            {
                sprintf (logbuf, "Error: socket read failed; error = %d: %s", errno, strerror(errno));
                cryptosubsystem->logMessage (ERR, logbuf);
                return (CRYPTO_ERR_IO);
            }
            
            sprintf (logbuf, "Before receiving public-key value of peer node (rd len = %d)",
                     min (iolength, getParams->prime.len));
            cryptosubsystem->logMessage (DBUG, logbuf);

            if ((bytesRead = (io->rdbuf())->read (otherPublicValue, min (iolength, getParams->prime.len))) < 0)
            {
                sprintf (logbuf, "Error: socket read failed; error = %d: %s", errno, strerror(errno));
                cryptosubsystem->logMessage (ERR, logbuf);
                return (CRYPTO_ERR_DH);
            }
            break;
            
        default:
                cryptosubsystem->logMessage (ERR, "Error: invalid socket type");
                return (CRYPTO_ERR_DH);
           break;
    }

    otherPublicValueLen = bytesRead;
    
    sprintf (logbuf, "After receiving public-key value of peer node (io len = %d)", otherPublicValueLen);
    cryptosubsystem->logMessage (DBUG, logbuf);

    return (CRYPTO_ERR_OK);
    
}   /* end receivePublicKeysFromPeerNode */


CryptoError CryptoRsaDiffieHellman::transmitPublicKeyToPeerNode (void)
{
         // Transmit node's public-key value to peer node.
         //
    sprintf (logbuf, "Before transmitting public-key value to peer node (io len = %d)", myPublicValueLen);
    cryptosubsystem->logMessage (DBUG, logbuf);

    switch (peerNodeSocketHandle.type)
    {
        case CRYPTO_SOCKET_INT:
            int *s;
            s = (int *) peerNodeSocketHandle.handle;

            if (write (*s, myPublicValue, myPublicValueLen) < 0)
            {
                sprintf (logbuf, "Error: socket write failed; error = %d: %s", errno, strerror(errno));
                cryptosubsystem->logMessage (ERR, logbuf);
                return (CRYPTO_ERR_DH);
            }
            break;
            
        case CRYPTO_SOCKET_IOSTREAM:
            iosockinet *io;
            io = (iosockinet *) peerNodeSocketHandle.handle;

                // Write length value which indicates length of data to be transmitted
                // to node.        
            if (transmitDataLength (io, myPublicValueLen) != CRYPTO_ERR_OK)
            {
                sprintf (logbuf, "Error: socket write failed; error = %d: %s", errno, strerror(errno));
                cryptosubsystem->logMessage (ERR, logbuf);
                return (CRYPTO_ERR_IO);
            }
            
            if ((io->rdbuf())->write (myPublicValue, myPublicValueLen) < 0)
            {
                sprintf (logbuf, "Error: socket write failed; error = %d: %s", errno, strerror(errno));
                cryptosubsystem->logMessage (ERR, logbuf);
                return (CRYPTO_ERR_DH);
            }
            break;
            
        default:
                cryptosubsystem->logMessage (ERR, "Error: invalid socket type");
                return (CRYPTO_ERR_DH);
           break;
    }

    cryptosubsystem->logMessage (DBUG, "After transmitting public-key value to peer node");

    return (CRYPTO_ERR_OK);
    
}   /* end transmitPublicKeysToPeerNode */


CryptoError CryptoRsaDiffieHellman::exchangePublicKeysWithPeerNode (void)
{
   CryptoError rc;
   
   
        // Exchange RSA public-keys with peer node.
    switch (peerNodeType)
    {
        case CRYPTO_PEER_NODE_USER_PASSIVE:
        case CRYPTO_PEER_NODE_CENTRAL_AUTHORITY:
            if ((rc = transmitPublicKeyToPeerNode ()) == CRYPTO_ERR_OK)
            {
                rc = receivePublicKeyFromPeerNode (); 
            }
            break;

        case CRYPTO_PEER_NODE_USER_ACTIVE:
            if ((rc = receivePublicKeyFromPeerNode ()) == CRYPTO_ERR_OK)
            {
                rc = transmitPublicKeyToPeerNode (); 
            }
            break;

        default:
            rc = CRYPTO_ERR_DH;
            cryptosubsystem->logMessage (ERR, "Invalid peer node type specified.");
            break;
    }
    
    return (rc);
    
}   /* end exchangePublicKeysWithPeerNode */


CryptoError CryptoRsaDiffieHellman::exchangeCApublicKeysWithPeerNode (void)
{
    int ioBytes;
    

         // Get public-key value received from peer node.
         //
    memcpy (otherPublicValue, caResponseBuffer, caResponseLength);
    otherPublicValueLen = caResponseLength;

         // Transmit Central Authority node's public-key value to peer node.
         //
    sprintf (logbuf, "Before transmitting public-key value to peer node (io len = %d)", myPublicValueLen);
    cryptosubsystem->logMessage (DBUG, logbuf);

    switch (peerNodeSocketHandle.type)
    {
        case CRYPTO_SOCKET_INT:
            int *s;
            s = (int *) peerNodeSocketHandle.handle;
            if ((ioBytes = write (*s, myPublicValue, myPublicValueLen)) < 0)
            {
                sprintf (logbuf, "Error: socket write failed; error = %d: %s", errno, strerror(errno));
                cryptosubsystem->logMessage (ERR, logbuf);
                return (CRYPTO_ERR_DH);
            }
            break;
            
        case CRYPTO_SOCKET_IOSTREAM:
            iosockinet *io;
            io = (iosockinet *) peerNodeSocketHandle.handle;

                // Write length value which indicates length of data to be transmitted
                // to node.        
            if (transmitDataLength (io, myPublicValueLen) != CRYPTO_ERR_OK)
            {
                sprintf (logbuf, "Error: socket write failed; error = %d: %s", errno, strerror(errno));
                cryptosubsystem->logMessage (ERR, logbuf);
                return (CRYPTO_ERR_IO);
            }
            
            if ((ioBytes = (io->rdbuf())->write (myPublicValue, myPublicValueLen)) < 0)
            {
                sprintf (logbuf, "Error: socket write failed; error = %d: %s", errno, strerror(errno));
                cryptosubsystem->logMessage (ERR, logbuf);
                return (CRYPTO_ERR_DH);
            }
            break;
            
        default:
                cryptosubsystem->logMessage (ERR, "Error: invalid socket type");
                return (CRYPTO_ERR_DH);
           break;
    }

    sprintf (logbuf, "After transmitting public-key value to peer node (io len = %d)", ioBytes);
    cryptosubsystem->logMessage (DBUG, logbuf);


    return (CRYPTO_ERR_OK);
    
}   /* end exchangeCApublicKeysWithPeerNode */


CryptoError CryptoRsaDiffieHellman::generateDiffieHellmanParameters (DHcentralAuthorityNodeParameters *parameters)
{
    CryptoError rc = CRYPTO_ERR_OK;
    time_t currentTime;
    
    
    do
    {
        cryptosubsystem->logMessage (DBUG, "Diffie-Hellman Algorithm ");
        cryptosubsystem->logMessage (DBUG, "======================== ");

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

        // Get current time as the random seed value, in order to generate
        // new Diffie-Hellman parameters.
        time (&currentTime);
        strcpy ((char *) randomSeed, ctime(&currentTime));
//      sprintf (logbuf, "Random seed value = %s", randomSeed);
//      cryptosubsystem->logMessage (DBUG, logbuf);
          
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
    
        cryptosubsystem->logMessage (DBUG, "   Generating DH parameters -- will take awhile");
        cryptosubsystem->logMessage (DBUG, "   ============================================ ");

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
           

        cryptosubsystem->logMessage (DBUG, "   Distributing DH parameters ");
        cryptosubsystem->logMessage (DBUG, "   ========================== ");

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
        cryptosubsystem->logMessage (DBUG, logbuf);
        PrintBuf (myDHParametersBER.data, myDHParametersBER.len);

    } while (0);
    
    if (status != 0)
    {
        sprintf (logbuf, "Status = %i ", status);
        cryptosubsystem->logMessage (ERR, logbuf);
        cryptosubsystem->logMessage (ERR, "DH Parameter Generation failed");

        rc = CRYPTO_ERR_DH;
    }  
    else
    {
        parameters->dataLength = myDHParametersBER.len;
        parameters->data = (char *) malloc (myDHParametersBER.len);
        if (parameters->data == NULL)
            rc = CRYPTO_ERR_MEMORY;
        else
            memcpy (parameters->data, myDHParametersBER.data, myDHParametersBER.len);
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

    return (rc);

}   /*  end generateDiffieHellmanParameters  */


CryptoError CryptoRsaDiffieHellman::userNodeKeyAgreement (DHuserNodeKeys *keys)
{
    CryptoError rc = CRYPTO_ERR_OK;
    time_t currentTime;
    
    
    do 
    {
        cryptosubsystem->logMessage (DBUG, "Diffie-Hellman Key Agreement Algorithm ");
        cryptosubsystem->logMessage (DBUG, "====================================== ");

        /*  ========================================================  */
        /*  The following code was pulled from genbytes.c, steps 1-4.
         */
        if ((ustatus = B_CreateAlgorithmObject (&urandomAlgorithm)) != 0)
          break;

        if ((ustatus = B_SetAlgorithmInfo
            (urandomAlgorithm, AI_SHA1Random, NULL_PTR)) != 0)
          break;

        if ((ustatus = B_RandomInit
            (urandomAlgorithm, DH_AGREE_SAMPLE_CHOOSER,
            (A_SURRENDER_CTX *)NULL_PTR)) != 0)
          break;

        urandomSeedLen = 256;
        urandomSeed = T_malloc (urandomSeedLen);
        if ((ustatus = (urandomSeed == NULL_PTR)) != 0)
             break;

        T_memset (urandomSeed, 0, urandomSeedLen);

        // Get current time as the random seed value, in order to generate a new
        // Diffie-Hellman private-key.
        time (&currentTime);
        strcpy ((char *) urandomSeed, ctime(&currentTime));
        //sprintf (logbuf, "Random seed value = %s", urandomSeed);
        //cryptosubsystem->logMessage (ERR, logbuf);
          
#ifdef DEBUG_CODE
        puts ("Enter a random seed for private value");
        if ((ustatus =
    	    (NULL_PTR == (unsigned char *)gets ((char *)urandomSeed))) != 0)
          break;
#endif          

        if ((ustatus = B_RandomUpdate
            (urandomAlgorithm, urandomSeed, urandomSeedLen,
            (A_SURRENDER_CTX *)NULL_PTR)) != 0)
          break;
      
        /*  ========================================================  */

        cryptosubsystem->logMessage (DBUG, "   Use previously-generated DH parameters");
        cryptosubsystem->logMessage (DBUG, "   ====================================== ");
    
        //
        // Get Diffie-Hellman parameters from central authority node,
        // in Basic Encoding Rules Format (BEF).
        //
        switch (peerNodeType)
        {
            case CRYPTO_PEER_NODE_USER_PASSIVE:
            case CRYPTO_PEER_NODE_USER_ACTIVE:
                if (!dhInitialized)
                    rc = CRYPTO_ERR_DH;
                else   
                {
                    rc = CRYPTO_ERR_OK;
                    dhParametersBER.len = diffieHellmanParameters.dataLength;
                    dhParametersBER.data = (unsigned char *) diffieHellmanParameters.data;
                }
                break;
                
            case CRYPTO_PEER_NODE_CENTRAL_AUTHORITY:
                rc = receiveCentralAuthorityNodeParameters ();
                break;

            default:
                cryptosubsystem->logMessage (ERR, "Invalid peer node type specified.");
                rc = CRYPTO_ERR_DH;
                break;
        }
        if (rc != CRYPTO_ERR_OK)
            break;
    
        cryptosubsystem->logMessage (DBUG, "   Key Agreement -- Phase 1 ");
        cryptosubsystem->logMessage (DBUG, "   ======================== ");

        /*  Step 1:  Create algorithm object
         */
        if ((ustatus = B_CreateAlgorithmObject (&dhKeyAgreeAlg)) != 0)
          break;

#ifdef DEBUG_CODE_REMOVE
        if ((ustatus = B_CreateAlgorithmObject (&otherPartyAlg)) != 0)
           break;
#endif

        /*  Step 2:  Set the algorithm objects to AI_DHKeyAgreeBER, using the parameters 
         *           from the central authority.
         */
        if ((ustatus = B_SetAlgorithmInfo
            (dhKeyAgreeAlg, AI_DHKeyAgreeBER,
            (POINTER)&dhParametersBER)) != 0)
          break;

#ifdef DEBUG_CODE_REMOVE
        if ((ustatus = B_SetAlgorithmInfo
            (otherPartyAlg, AI_DHKeyAgreeBER,
            (POINTER)&dhParametersBER)) != 0)
          break;
#endif

        /*  Step 3:  Initialize the algorithm objects
         */
       if ((ustatus = B_KeyAgreeInit
           (dhKeyAgreeAlg, (B_KEY_OBJ)NULL_PTR, DH_AGREE_SAMPLE_CHOOSER,
           (A_SURRENDER_CTX *)NULL_PTR)) != 0)
        break;

#ifdef DEBUG_CODE_REMOVE
       if ((ustatus = B_KeyAgreeInit
           (otherPartyAlg, (B_KEY_OBJ)NULL_PTR, DH_AGREE_SAMPLE_CHOOSER,
           (A_SURRENDER_CTX *)NULL_PTR)) != 0)
          break;
#endif

        /*  Step 4:  Phase 1
         */
     
        /* Determine size of prime parameter received from central authority, in order
           know how many bytes to allocate for the public value buffer.
         */
        if ((ustatus = B_GetAlgorithmInfo
             ((POINTER *)&getParams, dhKeyAgreeAlg, AI_DHKeyAgree)) != 0)
          break;
    
        myPublicValue = T_malloc (getParams->prime.len);
        if ((ustatus = (myPublicValue == NULL_PTR)) != 0)
           break;

        otherPublicValue = T_malloc (getParams->prime.len);
        if ((ustatus = (otherPublicValue == NULL_PTR)) != 0)
           break;

        /*  ugeneralFlag is for the surrender function */
        ugeneralFlag = 0;
#ifdef DEBUG_SURRENDER        
        if ((ustatus = B_KeyAgreePhase1
            (dhKeyAgreeAlg, myPublicValue, &myPublicValueLen,
            getParams->prime.len, urandomAlgorithm,
            &ugeneralSurrenderContext)) != 0)
           break;
#endif
        if ((ustatus = B_KeyAgreePhase1
            (dhKeyAgreeAlg, myPublicValue, &myPublicValueLen,
            getParams->prime.len, urandomAlgorithm,
            (A_SURRENDER_CTX *)NULL_PTR)) != 0)
           break;
           
      
        cryptosubsystem->logMessage (DBUG, "  The public value to give to the other party");
        sprintf (logbuf, " (%u bytes):", myPublicValueLen);
        cryptosubsystem->logMessage (DBUG, logbuf);
        PrintBuf (myPublicValue, myPublicValueLen);

        //
        // Exchange public-key values with peer node.
        //
        if ((rc = exchangePublicKeysWithPeerNode ()) != CRYPTO_ERR_OK)
            break;

#ifdef DEBUG_CODE_REMOVE
    ugeneralFlag = 0;
    if ((ustatus = B_KeyAgreePhase1
         (otherPartyAlg, otherPublicValue, &otherPublicValueLen,
          getParams->prime.len, urandomAlgorithm,
          &ugeneralSurrenderContext)) != 0)
      break;
#endif

        cryptosubsystem->logMessage (DBUG, "  The public value received from the other party");
        sprintf (logbuf, " (%u bytes):", otherPublicValueLen);
        cryptosubsystem->logMessage (DBUG, logbuf);
        PrintBuf (otherPublicValue, otherPublicValueLen);

        cryptosubsystem->logMessage (DBUG, "   Key Agreement -- Phase 2 ");
        cryptosubsystem->logMessage (DBUG, "   ======================== ");

        /*  Step 5:  Phase 2
                     The other party should send their public value and its
                     length.    
         */
        agreedUponSecretValue = T_malloc (getParams->prime.len);
        if ((ustatus = (agreedUponSecretValue == NULL_PTR)) != 0)
           break;

        otherAgreedUponValue = T_malloc (getParams->prime.len);
        if ((ustatus = (otherAgreedUponValue == NULL_PTR)) != 0)
           break;

        ugeneralFlag = 0;
#ifdef DEBUG_SURRENDER        
        if ((ustatus = B_KeyAgreePhase2
            (dhKeyAgreeAlg, agreedUponSecretValue,
            &agreedUponSecretValueLen, getParams->prime.len,
            otherPublicValue, otherPublicValueLen,
            &ugeneralSurrenderContext)) != 0)
           break;
#endif
        if ((ustatus = B_KeyAgreePhase2
            (dhKeyAgreeAlg, agreedUponSecretValue,
            &agreedUponSecretValueLen, getParams->prime.len,
            otherPublicValue, otherPublicValueLen,
            (A_SURRENDER_CTX *)NULL_PTR)) != 0)
           break;
           
        cryptosubsystem->logMessage (DBUG, "  The value derived by me");
        sprintf (logbuf, " (%u bytes):", agreedUponSecretValueLen);
        cryptosubsystem->logMessage (DBUG, logbuf);
        PrintBuf (agreedUponSecretValue, agreedUponSecretValueLen);

#ifdef DEBUG_CODE
    ugeneralFlag = 0;
    if ((ustatus = B_KeyAgreePhase2
         (otherPartyAlg, otherAgreedUponValue,
          &otherAgreedUponValueLen, getParams->prime.len,
          myPublicValue, myPublicValueLen,
          &ugeneralSurrenderContext)) != 0)
      break;
    cryptosubsystem->logMessage (DBUG, "  The value derived by the other party");
    sprintf (logbuf, " (%u bytes):", otherAgreedUponValueLen);
    cryptosubsystem->logMessage (DBUG, logbuf);
    PrintBuf (otherAgreedUponValue, otherAgreedUponValueLen);

    if ((ustatus = (agreedUponSecretValueLen != otherAgreedUponValueLen)) != 0)
      break;

    if ((ustatus = T_memcmp (agreedUponSecretValue, otherAgreedUponValue,
                            agreedUponSecretValueLen)) != 0)
      break;

    cryptosubsystem->logMessage (DBUG, "Keys agree");
#endif

    } while (0);
    
    if (ustatus != 0)
    {
        sprintf (logbuf, "Status = %i ", ustatus);
        cryptosubsystem->logMessage (ERR, logbuf);
        cryptosubsystem->logMessage (ERR, "Key Agreement failed");

        rc = CRYPTO_ERR_DH;
    }
    else
    {
        keys->agreedUponSecretValueLength = agreedUponSecretValueLen;
        keys->agreedUponSecretValue = (char *) malloc (agreedUponSecretValueLen);
        if (keys->agreedUponSecretValue == NULL)
            rc = CRYPTO_ERR_MEMORY;
        else
        {
            memcpy (keys->agreedUponSecretValue, agreedUponSecretValue, agreedUponSecretValueLen);
            
            keys->publicValueLength = myPublicValueLen;
            keys->publicValue = (char *) malloc (myPublicValueLen);
            if (keys->publicValue == NULL)
            {
                rc = CRYPTO_ERR_MEMORY;
                free (keys->agreedUponSecretValue);
            }
            else
            {
                memcpy (keys->publicValue, myPublicValue, myPublicValueLen);

                keys->peerPublicValueLength = otherPublicValueLen;
                keys->peerPublicValue = (char *) malloc (otherPublicValueLen);
                if (keys->peerPublicValue == NULL)
                {
                    rc = CRYPTO_ERR_MEMORY;
                    free (keys->publicValue);
                    free (keys->agreedUponSecretValue);
                }
                else
                    memcpy (keys->peerPublicValue, otherPublicValue, otherPublicValueLen);
            }
        }
    }  
  
    /*  Step 6:  Destroy user node related parameter resources
     */
    B_DestroyAlgorithmObject (&dhKeyAgreeAlg);
#ifdef DEBUG_CODE_REMOVE
    B_DestroyAlgorithmObject (&otherPartyAlg);
#endif  
    B_DestroyAlgorithmObject (&urandomAlgorithm);

    /*  Free up any memory allocated
     */
    T_free (urandomSeed);
    T_free (myPublicValue);
    T_free (agreedUponSecretValue);
    T_free (otherPublicValue);
    T_free (otherAgreedUponValue);

    return (rc);

} /*  end userNodeKeyAgreement  */

CryptoError CryptoRsaDiffieHellman::centralAuthorityNodeKeyAgreement (DHuserNodeKeys *keys)
{
  time_t currentTime;
  CryptoError rc = CRYPTO_ERR_OK;

  
  
  do {
    cryptosubsystem->logMessage (DBUG, "Diffie-Hellman Key Agreement Algorithm ");
    cryptosubsystem->logMessage (DBUG, "====================================== ");

    /*  ========================================================  */
    /*  The following code was pulled from genbytes.c, steps 1-4.
     */
    if ((ustatus = B_CreateAlgorithmObject (&urandomAlgorithm)) != 0)
      break;

    if ((ustatus = B_SetAlgorithmInfo
         (urandomAlgorithm, AI_SHA1Random, NULL_PTR)) != 0)
      break;

    if ((ustatus = B_RandomInit
         (urandomAlgorithm, DH_AGREE_SAMPLE_CHOOSER,
          (A_SURRENDER_CTX *)NULL_PTR)) != 0)
      break;

    urandomSeedLen = 256;
    urandomSeed = T_malloc (urandomSeedLen);
    if ((ustatus = (urandomSeed == NULL_PTR)) != 0)
         break;

    T_memset (urandomSeed, 0, urandomSeedLen);

    // Get current time as the random seed value, in order to generate a new
    // Diffie-Hellman private-key.
    time (&currentTime);
    strcpy ((char *) urandomSeed, ctime(&currentTime));
//  sprintf (logbuf, "Random seed value = %s", urandomSeed);
//  cryptosubsystem->logMessage (DBUG, logbuf);
          
#ifdef DEBUG_CODE
    puts ("Enter a random seed for private value");
    if ((ustatus =
	 (NULL_PTR == (unsigned char *)gets ((char *)urandomSeed))) != 0)
      break;
#endif      

    if ((ustatus = B_RandomUpdate
         (urandomAlgorithm, urandomSeed, urandomSeedLen,
          (A_SURRENDER_CTX *)NULL_PTR)) != 0)
      break;
      
    /*  ========================================================  */

    cryptosubsystem->logMessage (DBUG, "   Use previously-generated DH parameters");
    cryptosubsystem->logMessage (DBUG, "   ====================================== ");
    
    //
    // Get Diffie-Hellman parameters from central authority node,
    // in Basic Encoding Rules Format (BEF).
    //
    if (!dhInitialized)
    {
        // Generate Diffie-Hellman parameters for peer nodes requesting the parameters.
        // This process can be time consuming, therefore, the generation of the is only 
        // done once, and stored for future requests.
        //
        memset (&diffieHellmanParameters, 0, sizeof(DHcentralAuthorityNodeParameters));
        if (generateDiffieHellmanParameters (&diffieHellmanParameters) != CRYPTO_ERR_OK)
            break;

        dhInitialized = TRUE;
    }
    dhParametersBER.len = diffieHellmanParameters.dataLength;
    dhParametersBER.data = (unsigned char *) diffieHellmanParameters.data;

    
    cryptosubsystem->logMessage (DBUG, "   Key Agreement -- Phase 1 ");
    cryptosubsystem->logMessage (DBUG, "   ======================== ");

    /*  Step 1:  Create algorithm object
     */
    if ((ustatus = B_CreateAlgorithmObject (&dhKeyAgreeAlg)) != 0)
      break;

#ifdef DEBUG_CODE_REMOVE
    if ((ustatus = B_CreateAlgorithmObject (&otherPartyAlg)) != 0)
      break;
#endif

    /*  Step 2:  Set the algorithm objects to AI_DHKeyAgreeBER, using the parameters 
     *           from the central authority.
     */
    if ((ustatus = B_SetAlgorithmInfo
         (dhKeyAgreeAlg, AI_DHKeyAgreeBER,
          (POINTER)&dhParametersBER)) != 0)
      break;

#ifdef DEBUG_CODE_REMOVE
    if ((ustatus = B_SetAlgorithmInfo
         (otherPartyAlg, AI_DHKeyAgreeBER,
          (POINTER)&dhParametersBER)) != 0)
      break;
#endif

    /*  Step 3:  Initialize the algorithm objects
     */
    if ((ustatus = B_KeyAgreeInit
         (dhKeyAgreeAlg, (B_KEY_OBJ)NULL_PTR, DH_AGREE_SAMPLE_CHOOSER,
          (A_SURRENDER_CTX *)NULL_PTR)) != 0)
      break;

#ifdef DEBUG_CODE_REMOVE
    if ((ustatus = B_KeyAgreeInit
         (otherPartyAlg, (B_KEY_OBJ)NULL_PTR, DH_AGREE_SAMPLE_CHOOSER,
          (A_SURRENDER_CTX *)NULL_PTR)) != 0)
      break;
#endif

    /*  Step 4:  Phase 1
     */
     
    /* Determine size of prime parameter received from central authority, in order
       know how many bytes to allocate for the public value buffer.
     */
    if ((ustatus = B_GetAlgorithmInfo
         ((POINTER *)&getParams, dhKeyAgreeAlg, AI_DHKeyAgree)) != 0)
      break;
    
    myPublicValue = T_malloc (getParams->prime.len);
    if ((ustatus = (myPublicValue == NULL_PTR)) != 0)
      break;

    otherPublicValue = T_malloc (getParams->prime.len);
    if ((ustatus = (otherPublicValue == NULL_PTR)) != 0)
      break;

    /*  ugeneralFlag is for the surrender function */
    ugeneralFlag = 0;
#ifdef DEBUG_SURRENDER    
    if ((ustatus = B_KeyAgreePhase1
         (dhKeyAgreeAlg, myPublicValue, &myPublicValueLen,
          getParams->prime.len, urandomAlgorithm,
          &ugeneralSurrenderContext)) != 0)
      break;
#endif
    if ((ustatus = B_KeyAgreePhase1
         (dhKeyAgreeAlg, myPublicValue, &myPublicValueLen,
          getParams->prime.len, urandomAlgorithm,
          (A_SURRENDER_CTX *)NULL_PTR)) != 0)
      break;
      
      
    cryptosubsystem->logMessage (DBUG, "  The public value to give to the other party");
    sprintf (logbuf, " (%u bytes):", myPublicValueLen);
    cryptosubsystem->logMessage (DBUG, logbuf);
    PrintBuf (myPublicValue, myPublicValueLen);

    //
    // Exchange Central Authority public-key values with peer node.
    //
    if ((exchangeCApublicKeysWithPeerNode ()) != CRYPTO_ERR_OK)
       break;

#ifdef DEBUG_CODE_REMOVE
    ugeneralFlag = 0;
    if ((ustatus = B_KeyAgreePhase1
         (otherPartyAlg, otherPublicValue, &otherPublicValueLen,
          getParams->prime.len, urandomAlgorithm,
          &ugeneralSurrenderContext)) != 0)
      break;
#endif

    cryptosubsystem->logMessage (DBUG, "  The public value received from the other party");
    sprintf (logbuf, " (%u bytes):", otherPublicValueLen);
    cryptosubsystem->logMessage (DBUG, logbuf);
    PrintBuf (otherPublicValue, otherPublicValueLen);

    cryptosubsystem->logMessage (DBUG, "   Key Agreement -- Phase 2 ");
    cryptosubsystem->logMessage (DBUG, "   ======================== ");

    /*  Step 5:  Phase 2
                 The other party should send their public value and its
                 length.    
     */
    agreedUponSecretValue = T_malloc (getParams->prime.len);
    if ((ustatus = (agreedUponSecretValue == NULL_PTR)) != 0)
      break;

    otherAgreedUponValue = T_malloc (getParams->prime.len);
    if ((ustatus = (otherAgreedUponValue == NULL_PTR)) != 0)
      break;

    ugeneralFlag = 0;
#ifdef DEBUG_SURRENDER    
    if ((ustatus = B_KeyAgreePhase2
         (dhKeyAgreeAlg, agreedUponSecretValue,
          &agreedUponSecretValueLen, getParams->prime.len,
          otherPublicValue, otherPublicValueLen,
          &ugeneralSurrenderContext)) != 0)
      break;
#endif
    if ((ustatus = B_KeyAgreePhase2
         (dhKeyAgreeAlg, agreedUponSecretValue,
          &agreedUponSecretValueLen, getParams->prime.len,
          otherPublicValue, otherPublicValueLen,
          (A_SURRENDER_CTX *)NULL_PTR)) != 0)
      break;
      
    cryptosubsystem->logMessage (DBUG, "  The value derived by me");
    sprintf (logbuf, " (%u bytes):", agreedUponSecretValueLen);
    cryptosubsystem->logMessage (DBUG, logbuf);
    PrintBuf (agreedUponSecretValue, agreedUponSecretValueLen);

#ifdef DEBUG_CODE
    ugeneralFlag = 0;
    if ((ustatus = B_KeyAgreePhase2
         (otherPartyAlg, otherAgreedUponValue,
          &otherAgreedUponValueLen, getParams->prime.len,
          myPublicValue, myPublicValueLen,
          &ugeneralSurrenderContext)) != 0)
      break;
    cryptosubsystem->logMessage (DBUG, "  The value derived by the other party");
    sprintf (logbuf, " (%u bytes):", otherAgreedUponValueLen);
    cryptosubsystem->logMessage (DBUG, logbuf);
    PrintBuf (otherAgreedUponValue, otherAgreedUponValueLen);

    if ((ustatus = (agreedUponSecretValueLen != otherAgreedUponValueLen)) != 0)
      break;

    if ((ustatus = T_memcmp (agreedUponSecretValue, otherAgreedUponValue,
                            agreedUponSecretValueLen)) != 0)
      break;

    cryptosubsystem->logMessage (DBUG, "Keys agree");
#endif

  } while (0);
    
  if (ustatus == 0)
  {
     keys->agreedUponSecretValueLength = agreedUponSecretValueLen;
     keys->agreedUponSecretValue = (char *) malloc (agreedUponSecretValueLen);
     if (keys->agreedUponSecretValue == NULL)
         rc = CRYPTO_ERR_MEMORY;
     else
     {
         memcpy (keys->agreedUponSecretValue, agreedUponSecretValue, agreedUponSecretValueLen);
   
         keys->publicValueLength = myPublicValueLen;
         keys->publicValue = (char *) malloc (myPublicValueLen);
         if (keys->publicValue == NULL)
         {
             rc = CRYPTO_ERR_MEMORY;
             free (keys->agreedUponSecretValue);
         }
         else
         {
             memcpy (keys->publicValue, myPublicValue, myPublicValueLen);

             keys->peerPublicValueLength = otherPublicValueLen;
             keys->peerPublicValue = (char *) malloc (otherPublicValueLen);
             if (keys->peerPublicValue == NULL)
             {
                 rc = CRYPTO_ERR_MEMORY;
                 free (keys->publicValue);
                 free (keys->agreedUponSecretValue);
             }
             else
                 memcpy (keys->peerPublicValue, otherPublicValue, otherPublicValueLen);
         }
     }
  }
  else
  {
    sprintf (logbuf, "Status = %i ", ustatus);
    cryptosubsystem->logMessage (ERR, logbuf);
    cryptosubsystem->logMessage (ERR, "Key Agreement failed");

    rc = CRYPTO_ERR_DH;
  }  
  
  /*  Step 6:  Destroy user node related parameter resources
   */
  B_DestroyAlgorithmObject (&dhKeyAgreeAlg);
#ifdef DEBUG_CODE_REMOVE
  B_DestroyAlgorithmObject (&otherPartyAlg);
#endif  
  B_DestroyAlgorithmObject (&urandomAlgorithm);

  /*  Free up any memory allocated
   */
  T_free (urandomSeed);
  T_free (myPublicValue);
  T_free (agreedUponSecretValue);
  T_free (otherPublicValue);
  T_free (otherAgreedUponValue);

  return (rc);

} /*  end centralAuthorityNodeKeyAgreement  */


CryptoError CryptoRsaDiffieHellman::manageCentralAuthorityNode (CryptoSubSystem *subsystem, CryptoSession *session)
{
    CryptoError rc;
    int ioBytes;
    CryptoDHDEsession *sn = (CryptoDHDEsession *) session;
    
    
    if ( (subsystem == NULL) || (session == NULL) )
        return (CRYPTO_ERR_DH);

         // Initialize cryptography subsystem associated with this object.        
    cryptosubsystem = subsystem;

        // Initialize central authority and peer node socket handles of object.
    sn->getPeerNodeConnection (&peerNodeSocketHandle);

         // Generate Diffie-Hellman parameters and Central Authority private-key
         // and public-key for peer nodes requesting the parameters.  This process
         // can be time consuming, therefore, the generation of the 
         // is only done once, and stored for future requests.
         //
    if (!dhInitialized)
    {
        memset (&diffieHellmanParameters, 0, sizeof(DHcentralAuthorityNodeParameters));
        if ((rc  = generateDiffieHellmanParameters (&diffieHellmanParameters)) != CRYPTO_ERR_OK)
            return (rc);

        dhInitialized = TRUE;
    }

    switch (peerNodeSocketHandle.type)
    {
        case CRYPTO_SOCKET_INT:
            int *s;
            s = (int *) peerNodeSocketHandle.handle;

                // Transmit Diffie-Hellman parameters to peer node requesting the parameters.
                //
            sprintf (logbuf, "Before transmitting Diffie-Hellman parameters value to peer node (io len = %d)", diffieHellmanParameters.dataLength);
            cryptosubsystem->logMessage (DBUG, logbuf);

            if ((ioBytes = write (*s, diffieHellmanParameters.data, diffieHellmanParameters.dataLength)) < 0)
            {
                sprintf (logbuf, "Error: socket write failed; error = %d: %s", errno, strerror(errno));
                cryptosubsystem->logMessage (ERR, logbuf);
                return (CRYPTO_ERR_DH);
            }
            sprintf (logbuf, "After transmitting Diffie-Hellman parameters value to peer node (io len = %d)", ioBytes);
            cryptosubsystem->logMessage (DBUG, logbuf);

                // Receive peer node's public-key value.
                //
            sprintf (logbuf, "Before receiving public-key value of ca's peer node (rd len = %d)", DH_MAX_PRIME_BIT_LENGTH);
            cryptosubsystem->logMessage (DBUG, logbuf);

            if ((caResponseLength = read (*s, caResponseBuffer, DH_MAX_PRIME_BIT_LENGTH)) < 0)
            {
                sprintf (logbuf, "Error: socket read failed; error = %d: %s", errno, strerror(errno));
                cryptosubsystem->logMessage (ERR, logbuf);
                return (CRYPTO_ERR_DH);
            }
    
            sprintf (logbuf, "After receiving public-key value of ca's peer node (io len = %d)", caResponseLength);
            cryptosubsystem->logMessage (DBUG, logbuf);
            break;

        case CRYPTO_SOCKET_IOSTREAM:
            iosockinet *io;
            int iolength;
            io = (iosockinet *) peerNodeSocketHandle.handle;
                    
                // Write length value which indicates length of data to be transmitted
                // to node.        
            if (transmitDataLength (io, diffieHellmanParameters.dataLength) != CRYPTO_ERR_OK)
            {
                sprintf (logbuf, "Error: socket write failed; error = %d: %s", errno, strerror(errno));
                cryptosubsystem->logMessage (ERR, logbuf);
                return (CRYPTO_ERR_IO);
            }
            
                // Transmit Diffie-Hellman parameters to peer node requesting the parameters.
                //
            sprintf (logbuf, "Before transmitting Diffie-Hellman parameters value to peer node (io len = %d)", diffieHellmanParameters.dataLength);
            cryptosubsystem->logMessage (DBUG, logbuf);

            if ((ioBytes = (io->rdbuf())->write (diffieHellmanParameters.data, diffieHellmanParameters.dataLength)) < 0)
            {
                sprintf (logbuf, "Error: socket write failed; error = %d: %s", errno, strerror(errno));
                cryptosubsystem->logMessage (ERR, logbuf);
                return (CRYPTO_ERR_DH);
            }
            sprintf (logbuf, "After transmitting Diffie-Hellman parameters value to peer node (io len = %d)", ioBytes);
            cryptosubsystem->logMessage (DBUG, logbuf);

                // Receive peer node's public-key value.
                //

                // Read length value which indicates length of data to be received
                // from node.        
            if (receiveDataLength (io, &iolength) != CRYPTO_ERR_OK)
            {
                sprintf (logbuf, "Error: socket read failed; error = %d: %s", errno, strerror(errno));
                cryptosubsystem->logMessage (ERR, logbuf);
                return (CRYPTO_ERR_IO);
            }
            
            sprintf (logbuf, "Before receiving public-key value of ca's peer node (rd len = %d)",
                     min (iolength, DH_MAX_PRIME_BIT_LENGTH));
            cryptosubsystem->logMessage (DBUG, logbuf);

            if ((caResponseLength = (io->rdbuf())->read (caResponseBuffer, min (iolength, DH_MAX_PRIME_BIT_LENGTH))) < 0)
            {
                sprintf (logbuf, "Error: socket read failed; error = %d: %s", errno, strerror(errno));
                cryptosubsystem->logMessage (ERR, logbuf);
                return (CRYPTO_ERR_DH);
            }
    
            sprintf (logbuf, "After receiving public-key value of ca's peer node (io len = %d)", caResponseLength);
            cryptosubsystem->logMessage (DBUG, logbuf);
            break;

        default:
            cryptosubsystem->logMessage (ERR, "Error: Invalid socket type");
            return (CRYPTO_ERR_DH);
            break;
    }
    
    cryptosubsystem->logMessage (DBUG, "Before central authority node key agreement");
         // Perform Diffie-Hellman Key Agreement with peer nodes.
    if ((rc = centralAuthorityNodeKeyAgreement (&sn->dhUserKeys)) != CRYPTO_ERR_OK)
    {
       cryptosubsystem->logMessage (DBUG, "After central authority node key agreement");
       return (rc);
    }
    cryptosubsystem->logMessage (DBUG, "After central authority node key agreement");

    return (CRYPTO_ERR_OK);
    
}   /* end manageCentralAuthorityNode */


CryptoError CryptoRsaDiffieHellman::manageUserNodeKeyAgreement (CryptoSubSystem *subsystem, CryptoSession *session)
{
    CryptoError rc;
    CryptoDHDEsession *sn = (CryptoDHDEsession *) session;
    
    
    if ( (subsystem == NULL) || (session == NULL) )
        return (CRYPTO_ERR_DH);

         // Initialize cryptography subsystem associated with this object.        
    cryptosubsystem = subsystem;

        // Initialize central authority and peer node socket handles of object.
    peerNodeType = sn->getPeerNodeType ();
    sn->getCentralAuthorityNodeConnection (&caNodeSocketHandle);
    sn->getPeerNodeConnection (&peerNodeSocketHandle);

        // Perform Diffie-Hellman Key Agreement with peer nodes.
    rc = userNodeKeyAgreement (&sn->dhUserKeys);
    
    return (rc);

}   /* end manageUserNodeKeyAgreement */


CryptoRsaDiffieHellman::CryptoRsaDiffieHellman (void)
{
    /*  Initialize central authority node related parameters
     */
    randomAlgorithm = (B_ALGORITHM_OBJ)NULL_PTR;
    dhParamGenerator = (B_ALGORITHM_OBJ)NULL_PTR;
    dhParametersObj = (B_ALGORITHM_OBJ)NULL_PTR;

    memset (&dhParams, 0, sizeof (A_DH_PARAM_GEN_PARAMS));
    randomSeed = NULL_PTR;
    randomSeedLen = 0;
  

    bsafeDHParametersBER = (ITEM *)NULL_PTR;
    memset (&myDHParametersBER, 0, sizeof(ITEM));
    myDHParametersBER.data = NULL_PTR;
    
    status = 0;
    generalFlag = 0;

    memset (&generalSurrenderContext, 0, sizeof (A_SURRENDER_CTX));
    generalSurrenderContext.Surrender = GeneralSurrenderFunction;
    generalSurrenderContext.handle = (POINTER)&generalFlag;
    generalSurrenderContext.reserved = NULL_PTR;
    
        //
        // User node related attributes
        //
    memset (&dhParametersBER, 0, sizeof (ITEM));

    urandomAlgorithm = (B_ALGORITHM_OBJ)NULL_PTR;
    dhKeyAgreeAlg = (B_ALGORITHM_OBJ)NULL_PTR;
#ifdef DEBUG_CODE_REMOVE
    otherPartyAlg = (B_ALGORITHM_OBJ)NULL_PTR;
#endif
  
    urandomSeed = NULL_PTR;
    urandomSeedLen = 0;
  
    ugeneralFlag = 0;
    ustatus = 0;
    getParams = (A_DH_KEY_AGREE_PARAMS *) NULL_PTR;
    
    myPublicValue = NULL_PTR;
    myPublicValueLen = 0;

    otherPublicValue = NULL_PTR;
    otherPublicValueLen = 0;

    agreedUponSecretValue = NULL_PTR;
    agreedUponSecretValueLen = 0;

    otherAgreedUponValue = NULL_PTR;
    otherAgreedUponValueLen = 0;
        

    memset (&ugeneralSurrenderContext, 0, sizeof (A_SURRENDER_CTX)); 
    ugeneralSurrenderContext.Surrender = GeneralSurrenderFunction;
    ugeneralSurrenderContext.handle = (POINTER)&ugeneralFlag;
    ugeneralSurrenderContext.reserved = NULL_PTR;

}   /* end constructor */


CryptoRsaDiffieHellman::~CryptoRsaDiffieHellman (void)
{
    /*  Step 6:  Destroy central authority node related parameter resources
     */
    B_DestroyAlgorithmObject (&randomAlgorithm);
    B_DestroyAlgorithmObject (&dhParametersObj);
    B_DestroyAlgorithmObject (&dhParamGenerator);

    /*  Free up any memory allocated
     */
    T_free (randomSeed);
    T_free (myDHParametersBER.data);

  
    /*  Step 6:  Destroy user node related parameter resources
     */
    B_DestroyAlgorithmObject (&dhKeyAgreeAlg);
#ifdef DEBUG_CODE_REMOVE
    B_DestroyAlgorithmObject (&otherPartyAlg);
#endif

    B_DestroyAlgorithmObject (&urandomAlgorithm);

    /*  Free up any memory allocated
     */
    T_free (urandomSeed);
    T_free (myPublicValue);
    T_free (agreedUponSecretValue);
    T_free (otherPublicValue);
    T_free (otherAgreedUponValue);

}   /* end destructor */


int GeneralSurrenderFunction (POINTER handle)
{
  char s[100];
  static time_t currentTime;
  time_t getTime;

  if ((int)*handle == 0) {
    getTime = time(NULL);
    strftime (s, 100, "%H:%M:%S on %A, %d %B %Y", localtime(&getTime));
    //printf (logbuf, "%s", s);
    logMsg (DBUG, s);
    logMsg (DBUG, "Surrender function ...");
    *handle = 1;
    time (&currentTime);
  }
  else {
    time (&getTime);
    if (currentTime != getTime) {
      logMsg (DBUG, " .");
      currentTime = getTime;
    }
  }
  return (0);
  
} /*  end GeneralSurrenderFunction  */


void PrintBuf (unsigned char *buffer, unsigned int bufferLen)
{
  unsigned int i;
  char logbuf[50];

  return;
  
  for (i = 0; i < bufferLen; ++i) {
    if ( ((i & 7) == 7) || (i == (bufferLen - 1)) )
    {
        sprintf (logbuf, "  %02x", buffer[i]);
        logMsg (DBUG, logbuf);
    }
    else
    {
        sprintf (logbuf, "  %02x", buffer[i]);
        logMsg (DBUG, logbuf);
    }
  }

  logMsg (DBUG, "");
  
} /*  end PrintBuf  */

