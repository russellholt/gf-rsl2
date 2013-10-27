// *****************************************************************************
// *
// * RSA BSAFE Diffie-Hellman Cryptography Technique Classes
// *
// * History: Create - CKING 10/26/98
// * 
// * 
// * Copyright 1998 by Destiny Software Corporation.
// * 
// *******************************************************************************
#ifndef _CRYPTO_DH_H_
#define _CRYPTO_DH_H_

#include "aglobal.h"
#include "bsafe.h"
#include "CryptoSubSystem.h"


#define MAX_CA_RESPONSE_LENGTH		256		// DEBUG_CODE --- ???
#define DH_MAX_PRIME_BIT_LENGTH		512

//
// RSA BSAFE Diffie-Hellman Cryptography Technique Class 
//
class CryptoRsaDiffieHellman {
private:
    CryptoSubSystem *cryptosubsystem;
     
    CryptoPeerNodeType peerNodeType;
    CryptoSocketHandle caNodeSocketHandle;
    CryptoSocketHandle peerNodeSocketHandle;
    int caResponseLength; 
    char caResponseBuffer[MAX_CA_RESPONSE_LENGTH];
        //
        // central authority node related attributes
        //
    B_ALGORITHM_OBJ randomAlgorithm;
    B_ALGORITHM_OBJ dhParamGenerator;
    B_ALGORITHM_OBJ dhParametersObj;

    A_DH_PARAM_GEN_PARAMS dhParams;

    POINTER randomSeed;
    unsigned int randomSeedLen;
  
    A_SURRENDER_CTX generalSurrenderContext;

    ITEM *bsafeDHParametersBER;
    ITEM myDHParametersBER;

    unsigned int status, generalFlag;
    
        //
        // User node related attributes
        //
    ITEM dhParametersBER;
  
    B_ALGORITHM_OBJ urandomAlgorithm; 
    B_ALGORITHM_OBJ dhKeyAgreeAlg;
#ifdef DEBUG_CODE
    B_ALGORITHM_OBJ otherPartyAlg;
#endif    
  
    POINTER urandomSeed;
    unsigned int urandomSeedLen;
  
    A_SURRENDER_CTX ugeneralSurrenderContext;
    int ugeneralFlag;

    A_DH_KEY_AGREE_PARAMS *getParams;
    
    unsigned char *myPublicValue;
    unsigned int myPublicValueLen;

    unsigned char *otherPublicValue;
    unsigned int otherPublicValueLen;

    unsigned char *agreedUponSecretValue;
    unsigned int agreedUponSecretValueLen;

    unsigned char *otherAgreedUponValue;
    unsigned int otherAgreedUponValueLen;
        
    unsigned int ustatus;

    char logbuf [CRYPTO_LOG_BUFFER_LENGTH];
    
    
         // methods
//  void setCentralAuthorityNodeConnection (int socketHandle) { caNodeSocketHandle = socketHandle; }
//  void setPeerNodeConnection (int socketHandle) { peerNodeSocketHandle = socketHandle; }
    
    CryptoError exchangePublicKeysWithPeerNode (void);
    CryptoError exchangeCApublicKeysWithPeerNode (void);
    CryptoError generateDiffieHellmanParameters (DHcentralAuthorityNodeParameters *parameters);
    CryptoError userNodeKeyAgreement (DHuserNodeKeys *keys);
    CryptoError centralAuthorityNodeKeyAgreement (DHuserNodeKeys *keys);
    CryptoError receiveCentralAuthorityNodeParameters (void);
    CryptoError receivePublicKeyFromPeerNode (void);
    CryptoError transmitPublicKeyToPeerNode (void);


public:
         // methods
    CryptoError manageCentralAuthorityNode (CryptoSubSystem *subsystem, CryptoSession *session);
    CryptoError manageUserNodeKeyAgreement (CryptoSubSystem *subsystem, CryptoSession *session);

    CryptoRsaDiffieHellman (void);
    ~CryptoRsaDiffieHellman (void);
};

#endif
