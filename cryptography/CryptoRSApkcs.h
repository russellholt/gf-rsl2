// *****************************************************************************
// *
// * RSA BSAFE RSA Public-Key Cryptography Technique Classes
// *
// * History: Create - CKING 11/03/98
// * 
// * 
// * Copyright 1998 by Destiny Software Corporation.
// * 
// *******************************************************************************
#ifndef _CRYPTO_RSAPK_H_
#define _CRYPTO_RSAPK_H_

#include "aglobal.h"
#include "bsafe.h"
#include "CryptoSubSystem.h"



#define RSA_MODULUS_BITS 512

//
// RSA BSAFE RSA Public-Key Cryptography Technique Class 
//
class CryptoRSApublicKey {
private:
         // key generation related attributes
    ITEM *bsafePublicKeyBER;
    ITEM myPublicKeyBER;
    ITEM privateKeyBER;
    B_ALGORITHM_OBJ randomAlgorithm;
    B_ALGORITHM_OBJ keypairGenerator;
    
         // encryption related attributes
    B_KEY_OBJ publicKeyObject;
    B_ALGORITHM_OBJ rsaEncryptor;
    unsigned char *encryptedData;
    POINTER randomSeed;

         // decryption related attributes
    ITEM privateKeyItem;
    B_KEY_OBJ privateKeyObject;
    B_ALGORITHM_OBJ rsaDecryptor;
    unsigned char *decryptedData;

    char logbuf [CRYPTO_LOG_BUFFER_LENGTH];
    
public:
         // methods
    CryptoError generatePublicPrivateKeys (CryptoSubSystem *subsystem, RSAkey *privateKey, RSAkey *publicKey);
    CryptoError encrypt (CryptoSubSystem *subsystem, RSAkey *publicKey, int blockSize, 
                         char *plainTextData, int plainTextDataLength,
                         char **cipherTextData, int *cipherTextDataLength);
    CryptoError decrypt (CryptoSubSystem *subsystem, RSAkey *privateKey, int blockSize,
                         char *cipherTextData, int cipherTextDataLength,
                         char **plainTextData, int *plainTextDataLength);

    CryptoRSApublicKey (void);
    ~CryptoRSApublicKey (void);
};

#endif
