// *******************************************************************************
// *
// * Module Name: CryptoRC4.cc
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
#include "aglobal.h"
#include "bsafe.h"

#include "CryptoRC4.h"
#include "CryptoLog.h"

extern void PrintBuf PROTO_LIST ((unsigned char *, unsigned int));

#define min(a, b)               ((a) > (b) ? (b) : (a))

B_ALGORITHM_METHOD *RANDOM_CHOOSER[] = {
  &AM_SHA_RANDOM,
  (B_ALGORITHM_METHOD *)NULL_PTR
};

/* This chooser selects the standard C implementations of the algorithm
methods.
*/
B_ALGORITHM_METHOD *DEMO_ALGORITHM_CHOOSER[] = {
  &AM_DES_ENCRYPT,
  &AM_DES_DECRYPT,
  &AM_DESX_ENCRYPT,
  &AM_DESX_DECRYPT,
  &AM_DES_EDE_ENCRYPT,
  &AM_DES_EDE_DECRYPT,
  &AM_CBC_ENCRYPT,
  &AM_CBC_DECRYPT,
  &AM_CBC_INTER_LEAVED_ENCRYPT,
  &AM_CBC_INTER_LEAVED_DECRYPT,
  &AM_CFB_ENCRYPT,
  &AM_CFB_DECRYPT,
  &AM_CFB_PIPELINED_ENCRYPT,
  &AM_CFB_PIPELINED_DECRYPT,
  &AM_ECB_ENCRYPT,
  &AM_ECB_DECRYPT,
  &AM_OFB_ENCRYPT,
  &AM_OFB_DECRYPT,
  &AM_OFB_PIPELINED_ENCRYPT,
  &AM_OFB_PIPELINED_DECRYPT,
  &AM_TOKEN_DES_CBC_ENCRYPT,
  &AM_TOKEN_DES_CBC_DECRYPT,
  &AM_TOKEN_DES_EDE3_CBC_ENCRYPT,
  &AM_TOKEN_DES_EDE3_CBC_DECRYPT,
  &AM_HW_RANDOM,
  &AM_FORMAT_X931,
  &AM_EXTRACT_X931,
  &AM_TOKEN_RSA_PRV_ENCRYPT,
  &AM_TOKEN_RSA_PUB_DECRYPT,
  &AM_TOKEN_RSA_ENCRYPT,
  &AM_TOKEN_RSA_DECRYPT,
  &AM_TOKEN_RSA_CRT_ENCRYPT,
  &AM_TOKEN_RSA_CRT_DECRYPT,
  &AM_DES_CBC_DECRYPT,
  &AM_DES_CBC_ENCRYPT,
  &AM_DES_EDE3_CBC_DECRYPT,
  &AM_DES_EDE3_CBC_ENCRYPT,
  &AM_DESX_CBC_DECRYPT,
  &AM_DESX_CBC_ENCRYPT,
  &AM_DH_KEY_AGREE,
  &AM_DH_PARAM_GEN,
  &AM_DSA_KEY_GEN,
  &AM_DSA_PARAM_GEN,
  &AM_DSA_SIGN,
  &AM_DSA_VERIFY,
  &AM_DSA_KEY_TOKEN_GEN,
  &AM_TOKEN_DSA_SIGN,
  &AM_TOKEN_DSA_VERIFY,
  &AM_ECFP_PARAM_GEN,
  &AM_ECF2POLY_PARAM_GEN,
  &AM_ECFP_BLD_ACCEL_TABLE,
  &AM_ECFP_BLD_PUB_KEY_ACC_TAB,
  &AM_ECF2POLY_BLD_ACCEL_TABLE,
  &AM_ECF2POLY_BLD_PUB_KEY_ACC_TAB,
  &AM_ECFP_DECRYPT,
  &AM_ECF2POLY_DECRYPT,
  &AM_ECFP_ENCRYPT,
  &AM_ECF2POLY_ENCRYPT,
  &AM_ECFP_KEY_GEN,
  &AM_ECF2POLY_KEY_GEN,
  &AM_ECFP_DH_KEY_AGREE,
  &AM_ECF2POLY_DH_KEY_AGREE,
  &AM_ECFP_DSA_SIGN,
  &AM_ECF2POLY_DSA_SIGN,
  &AM_ECFP_DSA_VERIFY,
  &AM_ECF2POLY_DSA_VERIFY,
  &AM_MAC,
  &AM_MD2,
  &AM_MD2_RANDOM,
  &AM_MD2_RANDOM_2X,
  &AM_MD5,
  &AM_MD5_RANDOM,
  &AM_MD5_RANDOM_2X,
  &AM_MD,
  &AM_RC2_DECRYPT,
  &AM_RC2_ENCRYPT,
  &AM_RC2_CBC_DECRYPT,
  &AM_RC2_CBC_ENCRYPT,
  &AM_RC4_DECRYPT,
  &AM_RC4_ENCRYPT,
  &AM_RC5_ENCRYPT,
  &AM_RC5_64DECRYPT,
  &AM_RC5_64ENCRYPT,
  &AM_RC5_DECRYPT,
  &AM_RC5_CBC_ENCRYPT,
  &AM_RC5_CBC_DECRYPT,
  &AM_RSA_CRT_DECRYPT,
  &AM_RSA_CRT_ENCRYPT,
  &AM_RSA_CRT_DECRYPT_BLIND,
  &AM_RSA_CRT_ENCRYPT_BLIND,
  &AM_RSA_CRT_X931_ENCRYPT,
  &AM_RSA_X931_DECRYPT,
  &AM_RSA_DECRYPT,
  &AM_RSA_ENCRYPT,
  &AM_RSA_KEY_GEN,
  &AM_RSA_STRONG_KEY_GEN,
  &AM_RSA_KEY_TOKEN_GEN,
  &AM_SHA,
  &AM_SHA_RANDOM,
  &AM_SYMMETRIC_KEY_TOKEN_GEN,
  (B_ALGORITHM_METHOD *)NULL_PTR
};




CryptoError CryptoRsaRC4::generateSymmetricKey (unsigned char *randomSeedValue, int randomSeedValueLength,
                                                unsigned char *keyDataBuffer, int keyDataBufferLength)
{
    unsigned int status;
    CryptoError rc = CRYPTO_ERR_OK;  


    if ( (keyDataBuffer == NULL) || (keyDataBufferLength <= 0) )
        return (CRYPTO_ERR_RC4);
        
    do
    {
        cryptosubsystem->logMessage (DBUG, "Generating random bytes ");
        cryptosubsystem->logMessage (DBUG, "======================= ");

        /*  Step 1:  Create a random algorithm object */
        if ((status = B_CreateAlgorithmObject (&randomAlgorithm)) != 0)
          break;

        /*  Step 2:  Set the random algorithm object to SHA1 */
        if ((status = B_SetAlgorithmInfo
            (randomAlgorithm, AI_SHA1Random, NULL_PTR)) != 0)
           break;

        /*  Step 3:  Initialize the random algorithm. */
        if ((status = B_RandomInit
            (randomAlgorithm, RANDOM_CHOOSER,
            (A_SURRENDER_CTX *)NULL_PTR)) != 0)
            break;

        /*  Step 4:  Update
         */
        /*  Step 4a:  The Random Seed.  First, set aside memory to hold
                      this seed.  Then get the random seed.  The method
                      used here is almost certainly not a method to use
                      when writing a real application.  It is simply for
                      illustrative purposes.
         */
        randomSeedLen = 256;
        randomSeed = T_malloc (randomSeedLen);
        if ((status = (randomSeed == NULL_PTR)) != 0)
            break;

        T_memset (randomSeed, 0, randomSeedLen);

        cryptosubsystem->logMessage (DBUG, "Contents of randomSeed before seeding: ");
        PrintBuf (randomSeed, randomSeedLen);

        // Get current time as the random seed value, in order to generate a new
        // RC4 symmetric-key.
        memcpy (randomSeed, randomSeedValue, min (randomSeedLen, randomSeedValueLength));
          
#ifdef DEBUG_CODE

        puts ("Enter a random seed");
        if ((status =
	     (NULL_PTR == (unsigned char *)gets ((char *)randomSeed))) != 0)
          break;
#endif
                
        cryptosubsystem->logMessage (DBUG, "Contents of randomSeed after seeding: ");
        PrintBuf (randomSeed, randomSeedLen);

        /*  Step 4b:  Now we have a random seed and its length.  Pass both
                     into B_RandomUpdate. */
        if ((status = B_RandomUpdate
             (randomAlgorithm, randomSeed, randomSeedLen,
              (A_SURRENDER_CTX *)NULL_PTR)) != 0)
          break;

        /*  Step 5:  Generate.  First, prepare a buffer for receiving the
                    random bytes before calling B_GenerateRandomBytes.
        */
        randomByteBuffer = T_malloc (keyDataBufferLength);
        if ((status = (randomByteBuffer == NULL_PTR)) != 0)
            break;

        T_memset (randomByteBuffer, 0, keyDataBufferLength);

        if ((status = B_GenerateRandomBytes
            (randomAlgorithm, randomByteBuffer, keyDataBufferLength,
            (A_SURRENDER_CTX *)NULL_PTR)) != 0)
           break;

        sprintf (logbuf, "%i bytes of random-generated values: ", keyDataBufferLength);
        cryptosubsystem->logMessage (DBUG, logbuf);
        PrintBuf (randomByteBuffer, keyDataBufferLength);

    } while (0);

    if (status == 0)
    {
        memset (keyDataBuffer, 0, keyDataBufferLength);
        memcpy (keyDataBuffer, randomByteBuffer, keyDataBufferLength);
    }
    else
    {
        sprintf (logbuf, "Status = %i ", status);      
        cryptosubsystem->logMessage (ERR, logbuf);
        cryptosubsystem->logMessage (ERR, "Generate random bytes failed");
        
        rc = CRYPTO_ERR_RC4;
    }

    /*  Step 6:  Remember to destroy all objects, and free all memory
     */
    B_DestroyAlgorithmObject (&randomAlgorithm);
    T_memset (randomSeed, 0, randomSeedLen);
    T_free (randomSeed);
    T_free (randomByteBuffer);
    
    return (rc);

} /*  end generateSymmetricKey  */


CryptoError CryptoRsaRC4::encrypt (CryptoSubSystem *subsystem, RSAkey *key,
                                   unsigned char *randomSeedValue, int randomSeedValueLength,
                                   char *plainTextData, int plainTextDataLength,
                                   char **cipherTextData, int *cipherTextDataLength)
{
    unsigned int status;
    CryptoError rc = CRYPTO_ERR_OK;  
    unsigned int encryptedDataLen;
    unsigned int outputLenUpdate, outputLenFinal;
    unsigned int rc4KeyDataLen = MAX_RC4_SYMMETRIC_KEY_BYTES - 11;
    unsigned char rc4KeyData[MAX_RC4_SYMMETRIC_KEY_BYTES];


    if ( (subsystem == NULL) ||
         (key == NULL) ||
         (randomSeedValue == NULL) ||
         (plainTextData == NULL) ||
         (cipherTextDataLength == NULL) )
    {
        if (subsystem != NULL)
            subsystem->logMessage (ERR, "Error: Invalid parameter in RC4 encryption request.");
        return (CRYPTO_ERR_RC4);
    }
        
         // Initialize cryptography subsystem associated with this object.        
    cryptosubsystem = subsystem;

        // Generate the symmetric-key for the RC4 algorithm, via the RSA random
        // number generator.
    if ( (rc = generateSymmetricKey (randomSeedValue, randomSeedValueLength,
                                     rc4KeyData, MAX_RC4_SYMMETRIC_KEY_BYTES - 11)) != CRYPTO_ERR_OK)
        return (rc);
        
    do
    {
        cryptosubsystem->logMessage (DBUG, "RC4 algorithm:  Encryption phase ");
        cryptosubsystem->logMessage (DBUG, "================================ ");

//      sprintf (logbuf, "Input data: %s ", plainTextData);
//      cryptosubsystem->logMessage (DBUG, logbuf);
        plainTextDataLen = strlen (plainTextData) + 1;
        PrintBuf ((unsigned char *)plainTextData, plainTextDataLen);
  
        /*  ========================================================  */
    
        /*  Step 1:  Create an algorithm object. */
        if ((status = B_CreateAlgorithmObject (&rc4Encrypter)) != 0)
          break;

        /*  Step 2:  Set the algorithm to a type that does rc4 encryption.
                     AI_RC4 will do. */
        if ((status = B_SetAlgorithmInfo
            (rc4Encrypter, AI_RC4, NULL_PTR)) != 0)
           break;
    
        /*  Step 3a:  Create a key object.
         */
        if ((status = B_CreateKeyObject (&rc4Key)) != 0)
           break;
        
        /*  Step 3b:  Set the key object with the new key value. */
        rc4KeyItem.data = rc4KeyData;
        rc4KeyItem.len = rc4KeyDataLen;

        if ((status = B_SetKeyInfo
            (rc4Key, KI_Item, (POINTER)&rc4KeyItem)) != 0)
        break;

        /*  Step 3:  Init */
        if ((status = B_EncryptInit
            (rc4Encrypter, rc4Key, DEMO_ALGORITHM_CHOOSER,
            (A_SURRENDER_CTX *)NULL_PTR)) != 0)
        break;

        /*  Step 4:  Update */
        encryptedData = T_malloc (plainTextDataLen);
        if ((status = (encryptedData == NULL_PTR)) != 0)
           break;

        if ((status = B_EncryptUpdate
            (rc4Encrypter, encryptedData, &outputLenUpdate,
             plainTextDataLen, (unsigned char *)plainTextData,
             plainTextDataLen, (B_ALGORITHM_OBJ)NULL_PTR,
             (A_SURRENDER_CTX *)NULL_PTR)) != 0)
        break;

        /*  Step 5:  Final */
        if ((status = B_EncryptFinal
            (rc4Encrypter, encryptedData + outputLenUpdate,
            &outputLenFinal, plainTextDataLen - outputLenUpdate,
            (B_ALGORITHM_OBJ)NULL_PTR,
            (A_SURRENDER_CTX *)NULL_PTR)) != 0)
        break;

        encryptedDataLen = outputLenUpdate + outputLenFinal;
        sprintf (logbuf, "Encrypted data (%u bytes):", encryptedDataLen);    
        cryptosubsystem->logMessage (DBUG, logbuf);
        PrintBuf (encryptedData, encryptedDataLen);
    
    } while (0);

    if (status == 0)
    { 

        *cipherTextDataLength = encryptedDataLen;
        *cipherTextData = (char *) malloc (encryptedDataLen);
        if (*cipherTextData == NULL)
           rc = CRYPTO_ERR_MEMORY;
        else
        {
            memcpy (*cipherTextData, encryptedData, encryptedDataLen);

            key->keyDataLength = rc4KeyDataLen;
            key->keyData = (char *) malloc (rc4KeyDataLen);
            if (key->keyData == NULL)
            {
                free (*cipherTextData);
                rc = CRYPTO_ERR_MEMORY;
            }
            else
                memcpy (key->keyData, rc4KeyData, rc4KeyDataLen);
        }
    }
    else
    {
        sprintf (logbuf, "Status = %i ", status);
        cryptosubsystem->logMessage (ERR, logbuf);
        cryptosubsystem->logMessage (ERR, "Encrypting/Decrypting with AI_RC4 failed");
        
        rc = CRYPTO_ERR_RC4;
    }
    
    /*  Done with the key and algorithm objects, so destroy them.
     */
    B_DestroyKeyObject (&rc4Key);
    B_DestroyAlgorithmObject (&rc4Encrypter);

    /*  Free up any memory allocated, save it to a file or print it out first
        if you need to save it.
     */
    if (rc4KeyItem.data != NULL_PTR)
    {
       T_memset (rc4KeyItem.data, 0, rc4KeyItem.len);
       rc4KeyItem.data = NULL_PTR;
       rc4KeyItem.len = 0;
    }
  
    if (encryptedData != NULL_PTR)
    {
       T_memset (encryptedData, 0, plainTextDataLen);
       T_free (encryptedData);
       encryptedData = NULL_PTR;
    }

    return (rc);

} /*  end encrypt */


CryptoError CryptoRsaRC4::decrypt (CryptoSubSystem *subsystem, RSAkey *key,  
                     		   char *cipherTextData, int cipherTextDataLength,
                    		   char **plainTextData, int *plainTextDataLength)
{
    unsigned int status;
    unsigned int decryptedLenTotal;
    CryptoError rc = CRYPTO_ERR_OK;
    unsigned int decryptedLenUpdate, decryptedLenFinal;


    if ( (subsystem == NULL) ||
         (key == NULL) ||
         (cipherTextData == NULL) ||
         (plainTextDataLength == NULL) )
    {
        if (subsystem != NULL)
            subsystem->logMessage (ERR, "Error: Invalid parameter in RC4 decryption request.");
        return (CRYPTO_ERR_RC4);
    }
        
         // Initialize cryptography subsystem associated with this object.        
    cryptosubsystem = subsystem;

    do
    {
        sprintf (logbuf, "Encrypted data (%u bytes):", cipherTextDataLength);    
        cryptosubsystem->logMessage (DBUG, logbuf);
        PrintBuf ((unsigned char *)cipherTextData, cipherTextDataLength);
    
        cryptosubsystem->logMessage (DBUG, "RC4 algorithm:  Decryption phase ");
        cryptosubsystem->logMessage (DBUG, "================================ ");

        /*  Step 1:  Create an algorithm object. */
        if ((status = B_CreateAlgorithmObject (&rc4Decrypter)) != 0)
            break;

        /*  Step 2:  Set the algorithm to a type that does rc4 decryption.
                 AI_RC4 will do. */
        if ((status = B_SetAlgorithmInfo (rc4Decrypter, AI_RC4, NULL_PTR)) != 0)
           break;

        /*  Step 3a:  Create a key object.
         */
        if ((status = B_CreateKeyObject (&rc4Key)) != 0)
            break;
        
        /*  Step 3b:  Set the key object with the 10-byte key. */
        rc4KeyItem.data = (unsigned char *)key->keyData;
        rc4KeyItem.len = key->keyDataLength;

        if ((status = B_SetKeyInfo (rc4Key, KI_Item, (POINTER)&rc4KeyItem)) != 0)
           break;
  
        /*  Step 3:  Init */
        if ((status = B_DecryptInit (rc4Decrypter, rc4Key, DEMO_ALGORITHM_CHOOSER,
                                     (A_SURRENDER_CTX *)NULL_PTR)) != 0)
            break;

        /*  Set the buffer that will take the decrypted data to be the same size
            as the encrypted data's buffer.
         */
        decryptedDataLen = cipherTextDataLength;
        decryptedData = T_malloc (decryptedDataLen);
        if ((status = (decryptedData == NULL_PTR)) != 0)
           break;

        if ((status = B_DecryptUpdate (rc4Decrypter, decryptedData, &decryptedLenUpdate,
                                       cipherTextDataLength, (unsigned char *)cipherTextData,
                                       cipherTextDataLength, (B_ALGORITHM_OBJ)NULL_PTR,
                                       (A_SURRENDER_CTX *)NULL_PTR)) != 0)
           break;
    
        if ((status = B_DecryptFinal (rc4Decrypter, decryptedData + decryptedLenUpdate,
                                      &decryptedLenFinal, cipherTextDataLength - decryptedLenUpdate,
                                      (B_ALGORITHM_OBJ)NULL_PTR,
                                      (A_SURRENDER_CTX *)NULL_PTR)) != 0)
           break;

        decryptedLenTotal = decryptedLenUpdate + decryptedLenFinal;
        sprintf (logbuf, "Decrypted data in hex (%u bytes):", decryptedLenTotal);
        cryptosubsystem->logMessage (DBUG, logbuf);
        PrintBuf (decryptedData, decryptedLenTotal);
    
//      sprintf (logbuf, "Decrypted data: %s ", decryptedData);
//      cryptosubsystem->logMessage (DBUG, logbuf);
  
    } while (0);

    if (status == 0)
    {
        *plainTextDataLength = decryptedLenTotal;
        *plainTextData = (char *) malloc (decryptedLenTotal);
        if (*plainTextData == NULL)
            rc = CRYPTO_ERR_MEMORY;
        else
            memcpy (*plainTextData,  decryptedData, decryptedLenTotal);
    }
    else
    {
        sprintf (logbuf, "Status = %i ", status);
        cryptosubsystem->logMessage (ERR, logbuf);
        cryptosubsystem->logMessage (ERR, "Encrypting/Decrypting with AI_RC4 failed");
        
        rc = CRYPTO_ERR_RC4;
    }  

    /*  Done with the key and algorithm objects, so destroy them.
     */
    B_DestroyKeyObject (&rc4Key);
    B_DestroyAlgorithmObject (&rc4Decrypter);

    if (decryptedData != NULL_PTR)
    {
        T_memset (decryptedData, 0, decryptedDataLen);
        T_free (decryptedData);
        decryptedData = NULL_PTR;
    }

    return (rc);
    
}   /* end decrypt */


CryptoRsaRC4::CryptoRsaRC4 (void)
{
    /*  Initialize the random key generation related resources
     */
    randomAlgorithm = (B_ALGORITHM_OBJ)NULL_PTR;
    randomSeed = NULL_PTR;
    randomSeedLen = 0;
    randomByteBuffer = NULL_PTR;

    /*  Initialize the RC4 encryption related resources
     */
    plainTextDataLen = 0;
    rc4Key = (B_KEY_OBJ)NULL_PTR;
    encryptedData = NULL_PTR;
    rc4Encrypter = (B_ALGORITHM_OBJ)NULL_PTR;
    rc4Decrypter = (B_ALGORITHM_OBJ)NULL_PTR;
    
    memset (&rc4KeyItem, 0, sizeof(ITEM));
    
    /*  Initialize the RC4 decryption related resources
     */
    rc4Decrypter = (B_ALGORITHM_OBJ)NULL_PTR;
    decryptedData = NULL_PTR;
    decryptedDataLen = 0;

}   /* end constructor */ 

   
CryptoRsaRC4::~CryptoRsaRC4 (void)
{
    /*  Destroy random key generation related resources
     */
    randomAlgorithm = (B_ALGORITHM_OBJ)NULL_PTR;
    randomSeed = NULL_PTR;
    randomSeedLen = 0;
    randomByteBuffer = NULL_PTR;

    /*  Destroy RC4 encryption related resources
     */
    B_DestroyKeyObject (&rc4Key);
    B_DestroyAlgorithmObject (&rc4Encrypter);

    if (rc4KeyItem.data != NULL_PTR)
    {
       T_memset (rc4KeyItem.data, 0, rc4KeyItem.len);
       rc4KeyItem.data = NULL_PTR;
       rc4KeyItem.len = 0;
    }
  
    if (encryptedData != NULL_PTR)
    {
       T_memset (encryptedData, 0, plainTextDataLen);
       T_free (encryptedData);
       encryptedData = NULL_PTR;
    }

    /*  Destroy RC4 decryption related resources
     */
    /*  Done with the key and algorithm objects, so destroy them.
     */
    B_DestroyKeyObject (&rc4Key);
    B_DestroyAlgorithmObject (&rc4Decrypter);

    if (decryptedData != NULL_PTR)
    {
        T_memset (decryptedData, 0, decryptedDataLen);
        T_free (decryptedData);
        decryptedData = NULL_PTR;
    }
  
}   /* end destructor */ 
