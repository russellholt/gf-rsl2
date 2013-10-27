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

#include <stdio.h>   
#include <string.h>  
#include <time.h>

#include "CryptoRSApkcs.h"
#include "CryptoLog.h"


B_ALGORITHM_METHOD *RSA_SAMPLE_CHOOSER[] = {
  &AM_SHA_RANDOM,
  &AM_RSA_KEY_GEN,
  &AM_RSA_ENCRYPT,
  &AM_RSA_DECRYPT,
  &AM_RSA_CRT_ENCRYPT,
  &AM_RSA_CRT_DECRYPT,
  (B_ALGORITHM_METHOD *)NULL_PTR
};

extern int GeneralSurrenderFunction PROTO_LIST ((POINTER handle));
extern void PrintBuf PROTO_LIST ((unsigned char *, unsigned int));

static unsigned char f4Data[3] = {0x01, 0x00, 0x01};

int pkInitialized = 0;
RSAkey applicationPublicKey;
RSAkey applicationPrivateKey;


CryptoError CryptoRSApublicKey::generatePublicPrivateKeys (CryptoSubSystem *subsystem, RSAkey *privateKey, RSAkey *publicKey)
{
    CryptoError rc = CRYPTO_ERR_OK;
    time_t currentTime;
    int generalFlag;
    unsigned int status;
    unsigned int randomSeedLen;
    ITEM *getPrivateKey = (ITEM *)NULL_PTR;
    A_RSA_KEY_GEN_PARAMS keygenParams;
    A_SURRENDER_CTX generalSurrenderContext;
    
    
    myPublicKeyBER.data = NULL_PTR;
    privateKeyBER.data = NULL_PTR;

    generalSurrenderContext.Surrender = GeneralSurrenderFunction;
    generalSurrenderContext.handle = (POINTER)&generalFlag;
    generalSurrenderContext.reserved = NULL_PTR;

    if ( (subsystem == NULL) ||
         (privateKey == NULL) ||
         (publicKey == NULL) )
    {
        if (subsystem != NULL)
            subsystem->logMessage (ERR, "Error: Invalid parameter in generate RSA pkcs request.");
        return (CRYPTO_ERR_RSA_PUBLIC_KEY);
    }

        // Check if a previous session openned by the application generated the RSA
        // Public Pkcs keys.  If so, then it is not necessary to do this process again.     
    if (pkInitialized)
    {
        subsystem->logMessage (DBUG, "RSA Public pkcs global keys are being used for this session.");
        
        publicKey->keyDataLength = applicationPublicKey.keyDataLength;
        publicKey->keyData = (char *) malloc (applicationPublicKey.keyDataLength);
        if (publicKey->keyData == NULL)
            rc = CRYPTO_ERR_MEMORY;
        else
        {
            memcpy (publicKey->keyData, applicationPublicKey.keyData, applicationPublicKey.keyDataLength);

            privateKey->keyDataLength = applicationPrivateKey.keyDataLength;
            privateKey->keyData = (char *) malloc (applicationPrivateKey.keyDataLength);
            if (privateKey->keyData == NULL)
            {
                free (publicKey->keyData);
                rc = CRYPTO_ERR_MEMORY;
            }
            else
                memcpy (privateKey->keyData, applicationPrivateKey.keyData, applicationPrivateKey.keyDataLength);
        }

        return (rc);
    }
    
    do
    {
        subsystem->logMessage (DBUG, "Generating random bytes ");
        subsystem->logMessage (DBUG, "======================= ");

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
//      sprintf (logbuf, "Random seed value = %s", randomSeed);
//      subsystem->logMessage (DBUG, logbuf);
                  
#ifdef DEBUG_CODE
        puts ("Enter a random seed");
        if ((status = (NULL_PTR == (unsigned char *)gets ((char *)randomSeed))) != 0)
            break;
#endif
        
        if ((status = B_RandomUpdate (randomAlgorithm, randomSeed, randomSeedLen,
                                      (A_SURRENDER_CTX *)NULL_PTR)) != 0)
            break;

        subsystem->logMessage (DBUG, "   Generating a Keypair ");
        subsystem->logMessage (DBUG, "   ==================== ");

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
            
    
        subsystem->logMessage (DBUG, "   Distributing Your Public Key ");
        subsystem->logMessage (DBUG, "   ============================ ");

        myPublicKeyBER.data = NULL_PTR;
        if ((status = B_GetKeyInfo ((POINTER *)&bsafePublicKeyBER, publicKeyObject, KI_RSAPublicBER)) != 0)
            break;

        myPublicKeyBER.len = bsafePublicKeyBER->len;
        myPublicKeyBER.data = T_malloc (myPublicKeyBER.len);

        if ((status = (myPublicKeyBER.data == NULL_PTR)) != 0)
            break;
        T_memcpy (myPublicKeyBER.data, bsafePublicKeyBER->data, myPublicKeyBER.len);

        subsystem->logMessage (DBUG, "The public key DER-encoded as X.509 SubjectPublicKeyInfo type");
        sprintf (logbuf, " (%u bytes):", myPublicKeyBER.len);
        subsystem->logMessage (DBUG, logbuf);
        PrintBuf (myPublicKeyBER.data, myPublicKeyBER.len);

        privateKeyBER.data = NULL_PTR;
        if ((status = B_GetKeyInfo ((POINTER *)&getPrivateKey, privateKeyObject, KI_PKCS_RSAPrivateBER)) != 0)
            break;

        privateKeyBER.len = getPrivateKey->len;
        privateKeyBER.data = T_malloc (privateKeyBER.len);

        if ((status = (privateKeyBER.data == NULL_PTR)) != 0)
            break;
        T_memcpy (privateKeyBER.data, getPrivateKey->data, privateKeyBER.len);

        subsystem->logMessage (DBUG, "The private key encoded as a PKCS #8 PrivateKeyInfo type");
        sprintf (logbuf, " (%u bytes):", privateKeyBER.len);
        subsystem->logMessage (DBUG, logbuf);
        PrintBuf (privateKeyBER.data, privateKeyBER.len);

    } while (0);

    if (status == 0)
    {
        publicKey->keyDataLength = myPublicKeyBER.len;
        publicKey->keyData = (char *) malloc (myPublicKeyBER.len);
        if (publicKey->keyData == NULL)
           rc = CRYPTO_ERR_MEMORY;
        else
        {
            memcpy (publicKey->keyData, myPublicKeyBER.data, myPublicKeyBER.len);

            privateKey->keyDataLength = privateKeyBER.len;
            privateKey->keyData = (char *) malloc (privateKeyBER.len);
            if (privateKey->keyData == NULL)
            {
               rc = CRYPTO_ERR_MEMORY;
               free (publicKey->keyData);
            }
            else
            {
                memcpy (privateKey->keyData, privateKeyBER.data, privateKeyBER.len);
        
                    // Initialize the RSA public / priviate keys of the sub-system.
                    // This will improve performance for sessions, by generating the
                    // keys only once, instead of every time that a new session is
                    // openned by the application.         
                memset (&applicationPublicKey, 0, sizeof(applicationPublicKey));
                applicationPublicKey.keyDataLength = myPublicKeyBER.len;
                applicationPublicKey.keyData = (char *) malloc (myPublicKeyBER.len);
                if (applicationPublicKey.keyData == NULL)
                {
                    rc = CRYPTO_ERR_MEMORY;
                    free (publicKey->keyData);
                    free (privateKey->keyData);
                }
                else
                {
                    memcpy (applicationPublicKey.keyData, myPublicKeyBER.data, myPublicKeyBER.len);

                    memset (&applicationPrivateKey, 0, sizeof(applicationPrivateKey));
                    applicationPrivateKey.keyDataLength = privateKeyBER.len;
                    applicationPrivateKey.keyData = (char *) malloc (privateKeyBER.len);
                    if (applicationPrivateKey.keyData == NULL)
                    {
                        rc = CRYPTO_ERR_MEMORY;
                        free (publicKey->keyData);
                        free (privateKey->keyData);
                        free (applicationPublicKey.keyData);
                    }
                    else
                    {
                        memcpy (applicationPrivateKey.keyData, privateKeyBER.data, privateKeyBER.len);
                            // Set the flag which indicates that the RSA Public Pkcs keys have
                            // been generated by a session openned by the application.  Future
                            // sessions can use this key information, instead of generating them.
                        pkInitialized = TRUE;
                    }
                }
            }
        }
    }
    else
    {
        sprintf (logbuf, "Status = %i ", status);
        subsystem->logMessage (ERR, logbuf);
        subsystem->logMessage (ERR, "RSA Public Key generation failed");
        
        rc = CRYPTO_ERR_RSA_PUBLIC_KEY;
    }  

    /*  Step 6:  Destroy
     */
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

    return (rc);
    
}   /* end generatepPublicPrivateKeys */

CryptoError CryptoRSApublicKey::encrypt (CryptoSubSystem *subsystem, RSAkey *publicKey, int blockSize,
                                         char *plainTextData, int plainTextDataLength,
                                         char **cipherTextData, int *cipherTextDataLength)
{
    int generalFlag;
    unsigned int status;
    time_t currentTime;
    CryptoError rc = CRYPTO_ERR_OK;
    unsigned int randomSeedLen;
    unsigned int encryptedDataLength;
    A_SURRENDER_CTX generalSurrenderContext;
    unsigned int outputLenUpdate, outputLenFinal, outputLenTotal;
    ITEM publicKeyItem;
    


    if ( (subsystem == NULL) ||
         (publicKey == NULL) ||
         (plainTextData == NULL) ||
         (cipherTextDataLength == NULL) )
    {
        if (subsystem != NULL)
            subsystem->logMessage (ERR, "Error: Invalid parameter in RSA pkcs encrypt request.");
        return (CRYPTO_ERR_RSA_PUBLIC_KEY);
    }

    generalSurrenderContext.Surrender = GeneralSurrenderFunction;
    generalSurrenderContext.handle = (POINTER)&generalFlag;
    generalSurrenderContext.reserved = NULL_PTR;

    do
    {
        sprintf (logbuf, "The data to encrypt (%u bytes):", plainTextDataLength);
        subsystem->logMessage (DBUG, logbuf);
        PrintBuf ((unsigned char*) plainTextData, plainTextDataLength);

        subsystem->logMessage (DBUG, "Generating random bytes ");
        subsystem->logMessage (DBUG, "======================= ");

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
//      sprintf (logbuf, "Random seed value = %s", randomSeed);
//      subsystem->logMessage (DBUG, logbuf);
          
#ifdef DEBUG_CODE
        puts ("Enter a random seed");
        if ((status = (NULL_PTR == (unsigned char *)gets ((char *)randomSeed))) != 0)
            break;
#endif
        
        if ((status = B_RandomUpdate (randomAlgorithm, randomSeed, randomSeedLen,
                                      (A_SURRENDER_CTX *)NULL_PTR)) != 0)
            break;

        subsystem->logMessage (DBUG, "   Encrypting with the RSA Public Key ");
        subsystem->logMessage (DBUG, "   ================================== ");

        /*  Step 1a:  Creating an Algorithm Object */
        if ((status = B_CreateAlgorithmObject (&rsaEncryptor)) != 0)
            break;

        /*  Step 1b:  Creating an Key Object */
        if ((status = B_CreateKeyObject (&publicKeyObject)) != 0)
            break;

        /*  Step 2a:  Set the algorithm object to AI_PKCS_RSAPublic */
        if ((status = B_SetAlgorithmInfo (rsaEncryptor, AI_PKCS_RSAPublic, NULL_PTR)) != 0)
            break;
  
        /*  Step 2b:  Set the key object to the specified public key data, which is in BER format */
        publicKeyItem.data = (unsigned char *) publicKey->keyData;
        publicKeyItem.len = publicKey->keyDataLength;
        if ((status = B_SetKeyInfo (publicKeyObject, KI_RSAPublicBER, (POINTER)&publicKeyItem)) != 0)
            break;
          
        /*  Step 3:  Init -- encrypt with the recipient's public key */
        if ((status = B_EncryptInit (rsaEncryptor, publicKeyObject, RSA_SAMPLE_CHOOSER,
                                     (A_SURRENDER_CTX *)NULL_PTR)) != 0)
            break;

        /*  Step 4:  Update */
        generalFlag = 0;
        encryptedDataLength = blockSize;		// buffer should be divisible by 8 bytes
        encryptedData = T_malloc (encryptedDataLength);
        if (encryptedData == NULL_PTR)
           break;

#ifdef DEBUG_SURRENDER           
        if ((status = B_EncryptUpdate (rsaEncryptor, encryptedData, &outputLenUpdate, encryptedDataLength,
                                       (unsigned char *)plainTextData, plainTextDataLength,
                                       randomAlgorithm, &generalSurrenderContext)) != 0)
            break;
#endif
        if ((status = B_EncryptUpdate (rsaEncryptor, encryptedData, &outputLenUpdate, encryptedDataLength,
                                       (unsigned char *)plainTextData, plainTextDataLength,
                                       randomAlgorithm, (A_SURRENDER_CTX *)NULL_PTR)) != 0)
            break;
            

        /*  Step 5:  Final
         */
        generalFlag = 0;

#ifdef DEBUG_SURRENDER        
        if ((status = B_EncryptFinal (rsaEncryptor, encryptedData + outputLenUpdate,
                                      &outputLenFinal, encryptedDataLength - outputLenUpdate,
                                      randomAlgorithm, &generalSurrenderContext)) != 0)
            break;
#endif
        if ((status = B_EncryptFinal (rsaEncryptor, encryptedData + outputLenUpdate,
                                      &outputLenFinal, encryptedDataLength - outputLenUpdate,
                                      randomAlgorithm, (A_SURRENDER_CTX *)NULL_PTR)) != 0)
            break;
            
    
        outputLenTotal = outputLenUpdate + outputLenFinal;
        sprintf (logbuf, "The encrypted data (%u bytes):", outputLenTotal);
        subsystem->logMessage (DBUG, logbuf);
        PrintBuf (encryptedData, outputLenTotal);
        
    } while (0);

    if (status == 0)
    {
        *cipherTextDataLength = outputLenTotal;
        *cipherTextData = (char *) malloc (outputLenTotal);
        if (*cipherTextData == NULL)
            rc = CRYPTO_ERR_MEMORY;
        else
            memcpy (*cipherTextData, encryptedData, outputLenTotal);
    }
    else
    {
      sprintf (logbuf, "Status = %i ", status);
      subsystem->logMessage (ERR, logbuf);
      subsystem->logMessage (ERR, "RSA Public Key encryption failed");

      rc = CRYPTO_ERR_RSA_PUBLIC_KEY;
    }  

    /*  Step 6:  Destroy
     */
    B_DestroyAlgorithmObject (&randomAlgorithm);
    B_DestroyAlgorithmObject (&rsaEncryptor);
    rsaEncryptor = (B_ALGORITHM_OBJ)NULL_PTR;

    B_DestroyKeyObject (&publicKeyObject);
    publicKeyObject = (B_KEY_OBJ)NULL_PTR;

    /*  Free up any memory allocated
     */
    T_free (encryptedData);
    T_free (randomSeed);

    return (rc);
    
}   /* end encrypt */


CryptoError CryptoRSApublicKey::decrypt (CryptoSubSystem *subsystem, RSAkey *privateKey, int blockSize,
                                         char *cipherTextData, int cipherTextDataLength,
                                         char **plainTextData, int *plainTextDataLength)
{
    int generalFlag;
    unsigned int status;
    CryptoError rc = CRYPTO_ERR_OK;
    unsigned int decryptedDataLength;
    A_SURRENDER_CTX generalSurrenderContext;
    unsigned int outputLenUpdate, outputLenFinal, outputLenTotal;


    if ( (subsystem == NULL) ||
         (privateKey == NULL) ||
         (cipherTextData == NULL) ||
         (plainTextDataLength == NULL) )
    {
        if (subsystem != NULL)
            subsystem->logMessage (ERR, "Error: Invalid parameter in RSA pkcs decrypt request.");
        return (CRYPTO_ERR_RSA_PUBLIC_KEY);
    }

    memset ((void *) &generalSurrenderContext, 0, sizeof (generalSurrenderContext));
    generalSurrenderContext.Surrender = GeneralSurrenderFunction;
    generalSurrenderContext.handle = (POINTER)&generalFlag;
    generalSurrenderContext.reserved = NULL_PTR;

    do
    {
        sprintf (logbuf, "The encrypted data (%u bytes):", cipherTextDataLength);
        subsystem->logMessage (DBUG, logbuf);
        PrintBuf ((unsigned char*) cipherTextData, cipherTextDataLength);

        subsystem->logMessage (DBUG, "   Decrypting with the RSA Private Key ");
        subsystem->logMessage (DBUG, "   =================================== ");
  
        /*  Step 1a:  Creating an Algorithm Object */
        if ((status = B_CreateAlgorithmObject (&rsaDecryptor)) != 0)
            break;

        /*  Step 1b:  Creating a Key Object */
        if ((status = B_CreateKeyObject (&privateKeyObject)) != 0)
            break;

        /*  Step 2a:  Set the algorithm object to AI_PKCS_RSAPrivate */
        if ((status = B_SetAlgorithmInfo (rsaDecryptor, AI_PKCS_RSAPrivate, NULL_PTR)) != 0)
            break;

        /*  Step 2b:  Set the key object to the specified private key data, which is in BER format */
        privateKeyItem.data = (unsigned char *) privateKey->keyData;
        privateKeyItem.len = privateKey->keyDataLength;
        if ((status = B_SetKeyInfo (privateKeyObject, KI_PKCS_RSAPrivateBER, (POINTER)&privateKeyItem)) != 0)
            break;
          
        /*  Step 3:  Init -- Use the private key associated with the public
                 key used to encrypt */
        if ((status = B_DecryptInit (rsaDecryptor, privateKeyObject, RSA_SAMPLE_CHOOSER,
                                     (A_SURRENDER_CTX *)NULL_PTR)) != 0)
            break;

        /*  Step 4:  Update */
        generalFlag = 0;
        decryptedDataLength = blockSize;		// buffer should be divisible by 8 bytes
        decryptedData = T_malloc (decryptedDataLength);
        if (decryptedData == NULL_PTR)
           break;

#ifdef DEBUG_SURRENDER           
        if ((status = B_DecryptUpdate (rsaDecryptor, decryptedData, &outputLenUpdate, decryptedDataLength,
                                       (unsigned char *)cipherTextData, cipherTextDataLength, NULL_PTR,
                                       &generalSurrenderContext)) != 0)
            break;
#endif
        if ((status = B_DecryptUpdate (rsaDecryptor, decryptedData, &outputLenUpdate, decryptedDataLength,
                                       (unsigned char *)cipherTextData, cipherTextDataLength, NULL_PTR,
                                       (A_SURRENDER_CTX *)NULL_PTR)) != 0)
            break;
            

        /*  Step 5:  Final */
        generalFlag = 0;
#ifdef DEBUG_SURRENDER
        if ((status = B_DecryptFinal (rsaDecryptor, decryptedData + outputLenUpdate,
                                      &outputLenFinal, decryptedDataLength - outputLenUpdate, NULL_PTR,
                                      &generalSurrenderContext)) != 0)
            break;
#endif
        if ((status = B_DecryptFinal (rsaDecryptor, decryptedData + outputLenUpdate,
                                      &outputLenFinal, decryptedDataLength - outputLenUpdate, NULL_PTR,
                                      (A_SURRENDER_CTX *)NULL_PTR)) != 0)
            break;
            

        outputLenTotal = outputLenUpdate + outputLenFinal;

        sprintf (logbuf, "The decrypted data (%u bytes): ", outputLenTotal);
        subsystem->logMessage (DBUG, logbuf);
        PrintBuf (decryptedData, outputLenTotal);

        subsystem->logMessage (DBUG, "The decrypted data matches the original data to encrypt.");
    
    } while (0);

    if (status == 0)
    {
        *plainTextDataLength = outputLenTotal;
        *plainTextData = (char *) malloc (outputLenTotal);
        if (*plainTextData == NULL)
            rc = CRYPTO_ERR_MEMORY;
        else
            memcpy (*plainTextData,  decryptedData, outputLenTotal);
    }
    else
    {
      sprintf (logbuf, "Status = %i ", status);
      subsystem->logMessage (ERR, logbuf);
      subsystem->logMessage (ERR, "RSA Public Key decryption failed");
      
      rc = CRYPTO_ERR_RSA_PUBLIC_KEY;
    }  

    /*  Step 6:  Destroy
     */
    B_DestroyAlgorithmObject (&rsaDecryptor);
    rsaDecryptor = (B_ALGORITHM_OBJ)NULL_PTR;

    B_DestroyKeyObject (&privateKeyObject);
    privateKeyObject = (B_KEY_OBJ)NULL_PTR;

    /*  Free up any memory allocated
     */
    T_free (decryptedData);
    
    return (rc);

} /*  end decrypt  */


CryptoRSApublicKey::CryptoRSApublicKey (void)
{
        // Initialize encrption related resources.
    encryptedData = NULL_PTR; 
    publicKeyObject = (B_KEY_OBJ)NULL_PTR;
    rsaEncryptor = (B_ALGORITHM_OBJ)NULL_PTR;

        // Initialize decrption related resources. 
    decryptedData = NULL_PTR;
    rsaDecryptor = (B_ALGORITHM_OBJ)NULL_PTR;
    privateKeyObject = (B_KEY_OBJ)NULL_PTR;
    memset ((void *) &privateKeyItem, 0, sizeof (privateKeyItem));
    
        // Initialize key generation related resources.
    randomAlgorithm = (B_ALGORITHM_OBJ)NULL_PTR;
    keypairGenerator = (B_ALGORITHM_OBJ)NULL_PTR;
    randomSeed = NULL_PTR;
    bsafePublicKeyBER = (ITEM *)NULL_PTR;
    memset ((void *) &myPublicKeyBER, 0, sizeof(myPublicKeyBER));

    memset ((void *) &privateKeyBER, 0, sizeof(privateKeyBER));
  
}    /* end constructor */


CryptoRSApublicKey::~CryptoRSApublicKey (void)
{
return;		// DEBUG_CODE --- REMOVE
        /*  Step 6:  Destroy
         */

        // Destroy public/private key gernation resources.
    if (randomAlgorithm != NULL_PTR)     
        B_DestroyAlgorithmObject (&randomAlgorithm);
    if (keypairGenerator != NULL_PTR)
        B_DestroyAlgorithmObject (&keypairGenerator);

    T_free (randomSeed);
    T_free (myPublicKeyBER.data);
    T_free (privateKeyBER.data);
    
        // Destroy encrption related resources.
    if (rsaEncryptor != NULL_PTR) 
        B_DestroyAlgorithmObject (&rsaEncryptor);
    if (publicKeyObject != NULL_PTR)
        B_DestroyKeyObject (&publicKeyObject);
    T_free (encryptedData);

        // Destroy decrption related resources.
    if (rsaDecryptor != NULL_PTR) 
        B_DestroyAlgorithmObject (&rsaDecryptor);
    if (privateKeyObject != NULL_PTR)
        B_DestroyKeyObject (&privateKeyObject);
    T_free (decryptedData);

}    /* end destructor */