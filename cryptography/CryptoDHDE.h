// *****************************************************************************
// *
// * Diffie-Hellman & Digital Envelope Cryptography Technique Classes
// *
// * History: Create - CKING 10/26/98
// * 
// * 
// * Copyright 1998 by Destiny Software Corporation.
// * 
// *******************************************************************************
#ifndef _CRYPTO_DHDE_H_
#define _CRYPTO_DHDE_H_

#include <sockinet.h>

#define CRYPTO_DEFAULT_DH_PRIME_LENGTH		512		
#define CRYPTO_DEFAULT_DH_PRIVATE_LENGTH	64
#define MAX_RSA_PUBLIC_KEY_BYTES		512
enum CryptoNodeType
{
    CRYPTO_NODE_NONE = 0,
    CRYPTO_NODE_USER_PARTY,
    CRYPTO_NODE_CENTRAL_AUTHORITY
};

enum CryptoSocketType
{
    CRYPTO_SOCKET_NONE = 0,
    CRYPTO_SOCKET_INT,
    CRYPTO_SOCKET_IOSTREAM
};

enum CryptoPeerNodeType
{
    CRYPTO_PEER_NODE_NONE = 0,
    CRYPTO_PEER_NODE_USER_PASSIVE,
    CRYPTO_PEER_NODE_USER_ACTIVE,
    CRYPTO_PEER_NODE_CENTRAL_AUTHORITY
};

struct CryptoSocketHandle {
    CryptoSocketType type;
    void * handle;
    
};

struct DHcentralAuthorityNodeParameters {
    char *data;
    int dataLength;
};    

struct DHuserNodeKeys {
    char *agreedUponSecretValue;
    int agreedUponSecretValueLength;
    char *publicValue;
    int publicValueLength;
    char *peerPublicValue;
    int peerPublicValueLength;
};    




//
// Cryptography Diffie-Hellman & Digital Envelope Session Descriptor Class 
//
class CryptoDHDEsession : public CryptoSession {
private:
         // attributes
    CryptoNodeType nodeType;
    CryptoPeerNodeType peerNodeType;
    CryptoSocketHandle peerSocketHandle;
    CryptoSocketHandle centralAuthoritySocketHandle;
    CryptoSocketType centralAuthoritySocketType;
    CryptoSocketType peerSocketType;
    int dhPrimeBitSize;
    int dhPrivateKeyByteSize;
    DHcentralAuthorityNodeParameters dhCentralAuthorityParameters; 
       
public:
         // attributes
    DHuserNodeKeys dhUserKeys;
        
         // virtual methods
    CryptoError isValid (void);
    
    	 // technique specific methods
    inline void setNodeType (CryptoNodeType type) { nodeType = type;}
    inline CryptoNodeType getNodeType (void) { return (nodeType); }

    inline void setPeerNodeType (CryptoPeerNodeType type) { peerNodeType = type; }
    inline CryptoPeerNodeType getPeerNodeType (void) { return (peerNodeType); }

    void setPeerNodeConnection (int * socketHandle);
    void setPeerNodeConnection (iosockinet * socketHandle);
    void getPeerNodeConnection (CryptoSocketHandle * socketHandle);
    void setCentralAuthorityNodeConnection (int * socketHandle);
    void setCentralAuthorityNodeConnection (iosockinet * socketHandle);
    void getCentralAuthorityNodeConnection (CryptoSocketHandle * socketHandle);
    
    inline void setDHprimeBitSize (int size) { dhPrimeBitSize = size; }
    inline int getDHprimeBitSize (void) { return (dhPrimeBitSize); }

    inline void setDHprivateByteSize (int size) { dhPrivateKeyByteSize = size; }
    inline int getDHprivateByteSize (void) { return (dhPrivateKeyByteSize); }

    CryptoDHDEsession (void);
    ~CryptoDHDEsession (void);
};

//
// Cryptography Technique Base Class 
//
class CryptoRSApublicKey;
class CryptoDHDETechnique : public CryptoTechnique {
private:
    CryptoSubSystem *cryptosubsystem;
    
    RSAkey sessionPublicKey;
    RSAkey sessionPrivateKey;
    char peerPublicKey[MAX_RSA_PUBLIC_KEY_BYTES];
    int peerPublicKeyLength;
    CryptoRSApublicKey * digitalEnvelope;
    char logbuf [CRYPTO_LOG_BUFFER_LENGTH];
    
    void resetSessionTech (CryptoSession *session);
    CryptoError transferRSApublicKeyToPeerNode (CryptoSession *session);
    CryptoError receiveRSApublicKeyFromPeerNode (CryptoSession *session);
    CryptoError exchangeRSApublicKeyWithPeerNode (CryptoSession *session);
    
public:
         // methods
    CryptoError openSessionTech (CryptoSubSystem *subsystem, CryptoSession *session);
    CryptoError closeSessionTech (CryptoSubSystem *subsystem, CryptoSession *session);
    CryptoError abortSessionTech (CryptoSubSystem *subsystem, CryptoSession *session);
    CryptoError encryptDataTech (CryptoSubSystem *subsystem, CryptoSession *session,
                                 char *plaintext,
    		         	 int plength,
    		         	 char **ciphertext,
    		         	 int *clength);
    CryptoError decryptDataTech (CryptoSubSystem *subsystem, CryptoSession *session,
                                 char *ciphertext,
    		         	 int clength,
    		         	 char **plaintext,
    		         	 int *plength);

         // methods
    CryptoDHDETechnique (void);
    ~CryptoDHDETechnique (void);
};

#endif
