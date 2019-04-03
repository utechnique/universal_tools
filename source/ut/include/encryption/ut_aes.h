//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// #define the macros below to 1/0 to enable/disable the mode of operation.
//
// CBC enables AES128 encryption in CBC-mode of operation and handles 0-padding.
// ECB enables the basic ECB 16-byte block algorithm. Both can be enabled simultaneously.

// The #ifndef-guard allows it to be configured before #include'ing or at compile time.
#ifndef CBC
#define CBC 1
#endif

#ifndef ECB
#define ECB 1
#endif

// jcallan@github points out that declaring Multiply as a function 
// reduces code size considerably with the Keil ARM compiler.
// See this link for more information: https://github.com/kokke/tiny-AES128-C/pull/3
#ifndef AES_MULTIPLY_AS_A_FUNCTION
#define AES_MULTIPLY_AS_A_FUNCTION 0
#endif

class AES128Encryptor : public NonCopyable
{
public:
#if defined(ECB) && ECB
	void EncryptECB(const uint8* input, const uint8* key, uint8 *output);
	void DecryptECB(const uint8* input, const uint8* key, uint8 *output);
#endif
#if defined(CBC) && CBC
	void EncryptCBC(uint8* output, const uint8* input, uint32 length, const uint8* key, const uint8* iv);
	void DecryptCBC(uint8* output, uint8* input, uint32 length, const uint8* key, const uint8* iv);
#endif

private:
	uint8 TimeX(uint8 x);
	void KeyExpansion(void);
	void AddRoundKey(uint8 round);
	void SubBytes(void);
	void ShiftRows(void);
	void MixColumns(void);
#if AES_MULTIPLY_AS_A_FUNCTION
	uint8 AESMultiply(uint8 x, uint8 y);
#endif
	void InvMixColumns(void);
	void InvSubBytes(void);
	void InvShiftRows(void);
	void Cipher(void);
	void InvCipher(void);
	void BlockCopy(uint8* output, const uint8* input);
#if defined(CBC) && CBC
	void XorWithIv(uint8* buf);
#endif

private:
	// state - array holding the intermediate results during decryption.
	typedef uint8 State[4][4];
	State* state;

	// The array that stores the round keys.
	uint8 RoundKey[176];

	// The Key input to the AES Program
	const uint8* Key;

#if defined(CBC) && CBC
	// Initial Vector used only for CBC mode
	uint8* Iv;
#endif
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//