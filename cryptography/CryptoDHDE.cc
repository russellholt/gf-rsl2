// *******************************************************************************
// *
// * Module Name: CryptoDHDE.cc
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
#include <string.h>
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

#include "CryptoSubSystem.h"
#include "CryptoDH.h"
#include "CryptoRC4.h"
#include "CryptoRSApkcs.h"
#include "CryptoLog.h"

extern CryptoError transmitDataLength (iosockinet *io, int length);
extern CryptoError receiveDataLength (iosockinet *io, int *length);

#define MAX_DIGITAL_ENVELOPE_RC4_KEY_BYTES	64
#define min(a, b)               ((a) > (b) ? (b) : (a))



CryptoError CryptoDHDEsession::isValid (void)
{
    CryptoSocketHandle peerSocket;
    CryptoSocketHandle caSocket;
    

    getPeerNodeConnection (&peerSocket); 
    getCentralAuthorityNodeConnection (&caSocket);
      
    if ( (getTechniqueType() == CRYPTO_TECH_NONE) ||
         (getTechnique() == NULL) ||
         (getNodeType() == CRYPTO_NODE_NONE) ||
         (getPeerNodeType() == CRYPTO_PEER_NODE_NONE) ||
         (peerSocket.type == CRYPTO_SOCKET_NONE) ||
         ( (getPeerNodeType() == CRYPTO_PEER_NODE_CENTRAL_AUTHORITY) &&
           (caSocket.type == CRYPTO_SOCKET_NONE) ) )
    {
        return (CRYPTO_ERR_INVALID_SESSION);
    }
   
   return (CRYPTO_ERR_OK);
   
}   /* end isValid */


void CryptoDHDEsession::setPeerNodeConnection (int * socketHandle)
{
    peerSocketHandle.type = CRYPTO_SOCKET_INT;
    peerSocketHandle.handle = socketHandle;
}   /* end setPeerNodeConnection */
    
void CryptoDHDEsession::setPeerNodeConnection (iosockinet * socketHandle)
{
    peerSocketHandle.type = CRYPTO_SOCKET_IOSTREAM;
    peerSocketHandle.handle = socketHandle;
}   /* end setPeerNodeConnection */
    
void CryptoDHDEsession::getPeerNodeConnection (CryptoSocketHandle * socketHandle)
{
    socketHandle->handle = peerSocketHandle.handle;
    socketHandle->type = peerSocketHandle.type;
}   /* end getPeerNodeConnection */

void CryptoDHDEsession::setCentralAuthorityNodeConnection (int * socketHandle)
{
    centralAuthoritySocketHandle.type = CRYPTO_SOCKET_INT;
    centralAuthoritySocketHandle.handle = socketHandle;
}   /* end setCentralAuthorityNodeConnection */
    
void CryptoDHDEsession::setCentralAuthorityNodeConnection (iosockinet * socketHandle)
{
    centralAuthoritySocketHandle.type = CRYPTO_SOCKET_IOSTREAM;
    centralAuthoritySocketHandle.handle = socketHandle;
}   /* end setCentralAuthorityNodeConnection */
    
void CryptoDHDEsession::getCentralAuthorityNodeConnection (CryptoSocketHandle * socketHandle)
{
    socketHandle->handle = centralAuthoritySocketHandle.handle;
    socketHandle->type = centralAuthoritySocketHandle.type;
}   /* end getCentralAuthorityNodeConnection */

CryptoDHDEsession::CryptoDHDEsession (void)
{
    setTechniqueType (CRYPTO_TECH_DH_DE);
    
    setNodeType (CRYPTO_NODE_NONE);
    setPeerNodeType (CRYPTO_PEER_NODE_NONE);
    setDHprimeBitSize (CRYPTO_DEFAULT_DH_PRIME_LENGTH);
    setDHprivateByteSize (CRYPTO_DEFAULT_DH_PRIVATE_LENGTH);

    memset (&peerSocketHandle, 0, sizeof (peerSocketHandle.type));
    memset (&centralAuthoritySocketHandle, 0, sizeof (centralAuthoritySocketHandle.type));    
    memset (&dhUserKeys, 0, sizeof(dhUserKeys));
    memset (&dhCentralAuthorityParameters, 0, sizeof(dhCentralAuthorityParameters));
    
}   /* end constructor */


CryptoDHDEsession::~CryptoDHDEsession (void)
{
}   /* end destructor */


void CryptoDHDETechnique::resetSessionTech (CryptoSession *session)
{
    CryptoDHDEsession *sn = (CryptoDHDEsession *) session;
 
 
    if (digitalEnvelope != NULL)
    {
       delete (digitalEnvelope);
       digitalEnvelope = NULL;
    }
       
    if (sessionPublicKey.keyData != NULL)
    {
        free (sessionPublicKey.keyData);
        sessionPublicKey.keyData = NULL;
    }
        
    if (sessionPrivateKey.keyData != NULL)
    {
        free (sessionPrivateKey.keyData);
        sessionPrivateKey.keyData = NULL;
    }
        
    if (sn->dhUserKeys.agreedUponSecretValue != NULL)
    {
        free (sn->dhUserKeys.agreedUponSecretValue);
        sn->dhUserKeys.agreedUponSecretValue = NULL;
    }
        
    if (sn->dhUserKeys.publicValue != NULL)
    {
        free (sn->dhUserKeys.publicValue);
        sn->dhUserKeys.publicValue = NULL;
    }
        
    if (sn->dhUserKeys.peerPublicValue != NULL)
    {
        free (sn->dhUserKeys.peerPublicValue);
        sn->dhUserKeys.peerPublicValue = NULL;
    }
        
#ifdef DEBUG_CODE
    if (sn->dhCentralAuthorityParameters.data != NULL)
        free (sn->dhCentralAuthorityParameters.data);
#endif        

}   /* end resetSessionTech */


CryptoError CryptoDHDETechnique::transferRSApublicKeyToPeerNode (CryptoSession *session)
{
    int ioBytes;
    CryptoSocketHandle socketHandle;
    CryptoDHDEsession *sn = (CryptoDHDEsession *) session;
    
        // Transmit RSA public-key of session to peer node, in order for the
        // peer node to encrypt the RC4 symmetric-key during Digital Envelope
        // processing.
    sprintf (logbuf, "Before transmitting RSA public-key to peer node (io len = %d)", sessionPublicKey.keyDataLength);
    cryptosubsystem->logMessage (DBUG, logbuf);
    
    sn->getPeerNodeConnection (&socketHandle); 
       
    switch (socketHandle.type)
    {
        case CRYPTO_SOCKET_INT:
            int *s;
            s = (int *) socketHandle.handle;
            if ((ioBytes = write (*s, sessionPublicKey.keyData, sessionPublicKey.keyDataLength)) < 0)
            {
                sprintf (logbuf, "Error: socket write failed; error = %d: %s", errno, strerror(errno));
                cryptosubsystem->logMessage (ERR, logbuf);
                return (CRYPTO_ERR_DHDE);
            }
            break;
           
        case CRYPTO_SOCKET_IOSTREAM:
            iosockinet *io;
            io = (iosockinet *) socketHandle.handle;

                // Write length value which indicates length of data to be transmitted
                // to node.        
            if (transmitDataLength (io, sessionPublicKey.keyDataLength) != CRYPTO_ERR_OK)
            {
                sprintf (logbuf, "Error: socket write failed; error = %d: %s", errno, strerror(errno));
                cryptosubsystem->logMessage (ERR, logbuf);
                return (CRYPTO_ERR_IO);
            }
            
            if ((ioBytes = (io->rdbuf())->write (sessionPublicKey.keyData, sessionPublicKey.keyDataLength)) < 0)
            {
                sprintf (logbuf, "Error: socket write failed; error = %d: %s", errno, strerror(errno));
                cryptosubsystem->logMessage (ERR, logbuf);
                return (CRYPTO_ERR_DHDE);
            }
            break;
            
        default:
            cryptosubsystem->logMessage (ERR, "Error: invalid socket type.");
            return (CRYPTO_ERR_DHDE);
            break;
    }

    sprintf (logbuf, "After transmitting RSA public-key to peer node (io len = %d)", ioBytes);
    cryptosubsystem->logMessage (DBUG, logbuf);

    return (CRYPTO_ERR_OK);
    
}   /* end transferRSApublicKeyToPeerNode */


CryptoError CryptoDHDETechnique::receiveRSApublicKeyFromPeerNode (CryptoSession *session)
{
    CryptoSocketHandle socketHandle;
    CryptoDHDEsession *sn = (CryptoDHDEsession *) session;


        // Receive peer node's RSA public-key, in order to encrypt the RC4 symmetric-key
        // during Digital Envelope processing.
    sprintf (logbuf, "Before receiving RSA public-key from peer node (rd len = %d)", MAX_RSA_PUBLIC_KEY_BYTES);
    cryptosubsystem->logMessage (DBUG, logbuf);

    sn->getPeerNodeConnection (&socketHandle);    

    switch (socketHandle.type)
    {
        case CRYPTO_SOCKET_INT:
            int *s;
            s = (int *) socketHandle.handle;
            if ((peerPublicKeyLength = read (*s, peerPublicKey, MAX_RSA_PUBLIC_KEY_BYTES)) < 0)
            {
                sprintf (logbuf, "Error: socket read failed; error = %d: %s", errno, strerror(errno));
                cryptosubsystem->logMessage (ERR, logbuf);
                return (CRYPTO_ERR_DHDE);
            }
            break;
           
        case CRYPTO_SOCKET_IOSTREAM:
            iosockinet *io;
            int iolength;
            io = (iosockinet *) socketHandle.handle;
                // Read length value which indicates length of data to be received
                // from node.        
            if (receiveDataLength (io, &iolength) != CRYPTO_ERR_OK)
            {
                sprintf (logbuf, "Error: socket read failed; error = %d: %s", errno, strerror(errno));
                cryptosubsystem->logMessage (ERR, logbuf);
                return (CRYPTO_ERR_IO);
            }
            
            if ((peerPublicKeyLength = (io->rdbuf())->read (peerPublicKey, min (iolength, MAX_RSA_PUBLIC_KEY_BYTES))) < 0)
            {
                sprintf (logbuf, "Error: socket read failed; error = %d: %s", errno, strerror(errno));
                cryptosubsystem->logMessage (ERR, logbuf);
                return (CRYPTO_ERR_DHDE);
            }
            break;
            
        default:
            cryptosubsystem->logMessage (ERR, "Error: invalid socket type.");
            return (CRYPTO_ERR_DHDE);
            break;
    }
    
    sprintf (logbuf, "After receiving RSA public-key from peer node (io len = %d)", peerPublicKeyLength);
    cryptosubsystem->logMessage (DBUG, logbuf);
    
    return (CRYPTO_ERR_OK);
    
}   /* end receiveRSApublicKeyFromPeerNode */

            
CryptoError CryptoDHDETechnique::exchangeRSApublicKeyWithPeerNode (CryptoSession *session)
{
    CryptoError rc;
    CryptoDHDEsession *sn = (CryptoDHDEsession *) session;
   
   
        // Exchange RSA public-keys with peer node.
    switch (sn->getPeerNodeType ())
    {
        case CRYPTO_PEER_NODE_USER_PASSIVE:
        case CRYPTO_PEER_NODE_CENTRAL_AUTHORITY:
            if ((rc = transferRSApublicKeyToPeerNode (sn)) == CRYPTO_ERR_OK)
            {
                rc = receiveRSApublicKeyFromPeerNode (sn); 
            }
            break;

        case CRYPTO_PEER_NODE_USER_ACTIVE:
            if ((rc = receiveRSApublicKeyFromPeerNode (sn)) == CRYPTO_ERR_OK)
            {
                rc = transferRSApublicKeyToPeerNode (sn); 
            }
            break;

        default:
            rc = CRYPTO_ERR_DHDE;
            break;
    }
    
    return (rc);
    
}   /* end exchangeRSApublicKeyWithPeerNode */

            
CryptoError CryptoDHDETechnique::openSessionTech (CryptoSubSystem *subsystem, CryptoSession *session)
{
    CryptoError rc;
    CryptoRsaDiffieHellman *dh = NULL;
    CryptoDHDEsession *sn = (CryptoDHDEsession *) session;

    
    if ( (subsystem == NULL) ||
         (session == NULL) ||
         (session->isValid () != CRYPTO_ERR_OK) )
    {
        return (CRYPTO_ERR_INVALID_SESSION);
    }

        // Initialize cryptography subsystem associated with this object.
    cryptosubsystem = subsystem;

        // Create Diffie-Hellman object to be associated with this session. 
    dh = new CryptoRsaDiffieHellman ();
    if (dh == NULL)
        return (CRYPTO_ERR_MEMORY);
        
        // Create Digital Envelope object to be associated with this session. 
        // And generate the public and private keys of the RSA Public Key
        // algorithm.  
    digitalEnvelope = new CryptoRSApublicKey ();
    if (digitalEnvelope == NULL)
    {
        delete (dh);
        return (CRYPTO_ERR_MEMORY);
    }

    rc = digitalEnvelope->generatePublicPrivateKeys (subsystem, &sessionPrivateKey, &sessionPublicKey); 
    if (rc != CRYPTO_ERR_OK)
    {
        delete (dh);
        resetSessionTech (session);
        return(rc);
    }

        // Perform Diffie-Hellman Key Agreemement processing for this session.       
    switch (sn->getNodeType ())
    {
        case CRYPTO_NODE_USER_PARTY:
                // perform Diffie-Hellman Key Agreement with peer node, in order to
                // dynamically generate the secret value for this node.
            rc = dh->manageUserNodeKeyAgreement (subsystem, sn);
            break;

        case CRYPTO_NODE_CENTRAL_AUTHORITY:
                // generate diffie-hellman parameters, and send parameters to peer
                // node requesting the parameters.
            if (sn->getPeerNodeType () == CRYPTO_PEER_NODE_USER_PASSIVE)
            {
                rc = CRYPTO_ERR_DHDE;
                break;
            }
            rc = dh->manageCentralAuthorityNode (subsystem, sn);
            break;
            
        default:
            cryptosubsystem->logMessage (ERR, "Invalid peer node type specified.");
            rc = CRYPTO_ERR_DH;
            break;
    }

        // Exchange RSA public-keys with peer node, for Digital Envelope processing
        // during this session. 
    if (rc == CRYPTO_ERR_OK)
    {
        rc = exchangeRSApublicKeyWithPeerNode (sn);
    }

    delete (dh);
            
    return (rc);
    
}   /* end openSessionTech */


CryptoError CryptoDHDETechnique::closeSessionTech (CryptoSubSystem *subsystem, CryptoSession *session)
{
    CryptoError rc;
    
    
    if ( (subsystem == NULL) ||
         (session == NULL) ||
         (session->isValid () != CRYPTO_ERR_OK) )
    {
        return (CRYPTO_ERR_INVALID_SESSION);
    }

        // Initialize cryptography subsystem associated with this object.    
    cryptosubsystem = subsystem;

    resetSessionTech (session);
    
    return (CRYPTO_ERR_OK);
    
}   /* end closeSessionTech */


CryptoError CryptoDHDETechnique::abortSessionTech (CryptoSubSystem *subsystem, CryptoSession *session)
{
    CryptoError rc;
    
    
    if ( (subsystem == NULL) ||
         (session == NULL) ||
         (session->isValid () != CRYPTO_ERR_OK) )
    {
        return (CRYPTO_ERR_INVALID_SESSION);
    }

        // Initialize cryptography subsystem associated with this object.    
    cryptosubsystem = subsystem;

    closeSessionTech (subsystem, session);
    
    return (CRYPTO_ERR_OK);

}   /* end abortSessionTech */


CryptoError CryptoDHDETechnique::encryptDataTech (CryptoSubSystem *subsystem, CryptoSession *session,
                                                  char *plaintext, int plength,
                                                  char **ciphertext, int *clength)
{
    CryptoError rc;
    RSAkey rc4Key;
    RSAkey dePublicKey;
    char *ptr;
    CryptoRsaRC4 *rc4 = NULL;
    char *rc4EncryptedKey = NULL;
    int rc4EncryptedKeyLength;
    char *encryptedData = NULL;
    int encryptedDataLength;
    
    CryptoDHDEsession *sn = (CryptoDHDEsession *) session;

    
    if ( (subsystem == NULL) ||
         (session == NULL) ||
         (session->isValid () != CRYPTO_ERR_OK) )
    {
        return (CRYPTO_ERR_INVALID_SESSION);
    }

        // Initialize cryptography subsystem associated with this object.    
    cryptosubsystem = subsystem;

        // Create RC4 symmetric-key object associated with this session.
    rc4 = new CryptoRsaRC4 ();
    if (rc4 == NULL)
       return (CRYPTO_ERR_MEMORY);
    
    memset ((void *) &rc4Key, 0, sizeof(rc4Key));
    rc = rc4->encrypt (subsystem, &rc4Key,
                      (unsigned char *)sn->dhUserKeys.agreedUponSecretValue, sn->dhUserKeys.agreedUponSecretValueLength,
                      plaintext, plength,
                      &encryptedData, &encryptedDataLength);
    if (rc == CRYPTO_ERR_OK)
    {
        switch (sn->getNodeType ())
        {
            case CRYPTO_NODE_USER_PARTY:
            case CRYPTO_NODE_CENTRAL_AUTHORITY:
            
                    // Create Digital Envelope, by encrypting the RC4 symmetric-key with
                    // the RSA public-key of peer node application.  The peer node public-key
                    // was acquired via the RSA Public Key Algorithm, during the openning of 
                    // the session.
                dePublicKey.keyData = peerPublicKey;
                dePublicKey.keyDataLength = peerPublicKeyLength;
        
                rc = digitalEnvelope->encrypt (subsystem, &dePublicKey, MAX_DIGITAL_ENVELOPE_RC4_KEY_BYTES,
                                              rc4Key.keyData, rc4Key.keyDataLength,
                                              &rc4EncryptedKey, &rc4EncryptedKeyLength);
                if (rc == CRYPTO_ERR_OK)
                {
                        // Package all encrypted data to be sent to the peer in the following
                        // format: <RC4 symmetric-key encrypted data><ciphertext data>
                        // The encrypted RC4 symmetric-key data is always stored in the first
                        // <n> bytes defined by the MAX_DIGITAL_ENVELOPE_RC4_KEY_BYTES define.
                    *clength = encryptedDataLength + rc4EncryptedKeyLength;
                    *ciphertext = (char *) malloc (*clength);
                    if (*ciphertext == NULL)
                       rc = CRYPTO_ERR_MEMORY;
                    else
                    {
                        ptr = *ciphertext;
                        memset (ptr, 0, *clength);
                        memcpy (ptr, rc4EncryptedKey, rc4EncryptedKeyLength);
                        ptr = ptr + rc4EncryptedKeyLength;
                        memcpy (ptr, encryptedData, encryptedDataLength);
                    }
                }
                break;
                
            default:
               break;
        }
    }

    delete (rc4);
        
    if (encryptedData != NULL)
       free (encryptedData);
       
    if (rc4EncryptedKey != NULL)
       free (rc4EncryptedKey);
       
    if (rc4Key.keyData != NULL)
       free (rc4Key.keyData);

    return (rc);

}   /* end encryptDataTech */


CryptoError CryptoDHDETechnique::decryptDataTech (CryptoSubSystem *subsystem, CryptoSession *session,
                                                  char *ciphertext, int clength,
                                                  char **plaintext, int *plength)
{
    CryptoError rc;
    CryptoRsaRC4 *rc4 = NULL;
    RSAkey rc4Key;
    char *cipherTextPtr;
    int cipherTextLen;
    RSAkey dePublicKey;
    char *decryptedData = NULL;
    int decryptedDataLength;
    
    CryptoDHDEsession *sn = (CryptoDHDEsession *) session;


    if ( (subsystem == NULL) ||
         (session == NULL) ||
         (session->isValid () != CRYPTO_ERR_OK) )
    {
        return (CRYPTO_ERR_INVALID_SESSION);
    }

        // Initialize cryptography subsystem associated with this object.    
    cryptosubsystem = subsystem;

       // Decrypt the RC4 symmetric-key of the Digital Envelope, located
       // at the beginning of the ciphertext data.  The length of the RC4
       // symmetric-key is defined by the MAX_DIGITAL_ENVELOPE_RC4_KEY_BYTES
       // define.
    cipherTextPtr = ciphertext;
    memset ((void *) &rc4Key, 0, sizeof(rc4Key));
    rc = digitalEnvelope->decrypt (subsystem, &sessionPrivateKey, MAX_DIGITAL_ENVELOPE_RC4_KEY_BYTES,
                                   ciphertext, MAX_DIGITAL_ENVELOPE_RC4_KEY_BYTES,
                                   &rc4Key.keyData, &rc4Key.keyDataLength);

    if (rc == CRYPTO_ERR_OK)
    {
            // Create RC4 symmetric-key object associated with this session.
        rc4 = new CryptoRsaRC4 ();
        if (rc4 == NULL)
           return (CRYPTO_ERR_MEMORY);

        cryptosubsystem->logMessage (DBUG, "Decryption of RC4 Key successful.");
        cipherTextPtr = cipherTextPtr + MAX_DIGITAL_ENVELOPE_RC4_KEY_BYTES;
        cipherTextLen = clength - MAX_DIGITAL_ENVELOPE_RC4_KEY_BYTES;
        rc = rc4->decrypt (subsystem, &rc4Key, cipherTextPtr, cipherTextLen, &decryptedData, &decryptedDataLength);
        if (rc == CRYPTO_ERR_OK)
        {
            cryptosubsystem->logMessage (DBUG, "Decryption of RC4 encrypted data successful.");
            *plaintext = decryptedData;
            *plength = decryptedDataLength;
        }

        delete (rc4);
    }

   return (rc);

}   /* end decryptDataTech */


CryptoDHDETechnique::CryptoDHDETechnique (void)
{
    digitalEnvelope = NULL;
    memset (&sessionPublicKey, 0, sizeof (sessionPublicKey));
    memset (&sessionPrivateKey, 0, sizeof (sessionPrivateKey));
    
}   /* end constructor */


CryptoDHDETechnique::~CryptoDHDETechnique (void)
{
    if (digitalEnvelope)
       delete (digitalEnvelope);
       
}   /* end destructor */

