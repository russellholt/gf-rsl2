// *****************************************************************************
// *
// * RSA BSAFE RC4 Symmetric-Key Stream Cipher Cryptography Technique Classes
// *
// * History: Create - CKING 11/01/98
// * 
// * 
// * Copyright 1998 by Destiny Software Corporation.
// * 
// *******************************************************************************
#ifndef _CRYPTO_RC4_H_
#define _CRYPTO_RC4_H_

#include "aglobal.h"
#include "bsafe.h"
#include "CryptoSubSystem.h"


#define MAX_RC4_SYMMETRIC_KEY_BYTES	64

//
// RSA BSAFE RC4 Symmetric-Key Stream Cipher Cryptography Technique Class 
//
class CryptoRsaRC4 {
private:
    CryptoSubSystem *cryptosubsystem;
    
         // random symmetric-key generator related attributes
    B_ALGORITHM_OBJ randomAlgorithm;
    POINTER randomSeed;
    unsigned int randomSeedLen;
    unsigned char *randomByteBuffer;

         // encryption related attributes
    B_KEY_OBJ rc4Key;
    ITEM rc4KeyItem;
    B_ALGORITHM_OBJ rc4Encrypter;
    unsigned char *encryptedData;
    unsigned int plainTextDataLen;


         // random decryption related attributes
    B_ALGORITHM_OBJ rc4Decrypter;
    unsigned char *decryptedData;
    unsigned int decryptedDataLen;

    char logbuf [CRYPTO_LOG_BUFFER_LENGTH];

         // methods
    CryptoError generateSymmetricKey (unsigned char *randomSeedValue, int randomSeedValueLength,
                                      unsigned char *keyDataBuffer, int keyDataBufferLength);

public:
         // methods
    CryptoError decrypt (CryptoSubSystem *subsystem, RSAkey *key,
                         char *cipherTextData, int cipherTextDataLength,
                         char **plainTextData, int *plainTextDataLength);
    CryptoError encrypt (CryptoSubSystem *subsystem, RSAkey *key,
                         unsigned char *randomSeedValue, int randomSeedValueLength,
                         char *plainTextData, int plainTextDataLength,
                         char **cipherTextData, int *cipherTextDataLength);

    CryptoRsaRC4 (void);
    ~CryptoRsaRC4 (void);
};

#endif
