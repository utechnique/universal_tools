//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "encryption/ut_base64.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Set of base64 symbols.
static const char* skBase64Characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                        "abcdefghijklmnopqrstuvwxyz"
                                        "0123456789+/";

//----------------------------------------------------------------------------//
// Returns true if provided character is one of the base64 characters.
inline bool IsBase64(unsigned char c)
{
	return ((c >= 'A' && c <= 'Z') ||
	        (c >= 'a' && c <= 'z') ||
	        (c >= '0' && c <= '9') ||
	        (c == '+') || (c == '/'));
}

// Encodes provided data to the base64 string.
//    @param data - pointer to the data to encoded.
//    @param size - size of the data in bytes.
//    @return - base64 string.
String EncodeBase64(const byte* data, size_t size)
{
	String ret;
	size_t i = 0;
	size_t j = 0;
	byte char_array_3[3];
	byte char_array_4[4];

	while (size--)
	{
		char_array_3[i++] = *(data++);
		if (i == 3)
		{
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i < 4); i++)
			{
				ret.Add(skBase64Characters[char_array_4[i]]);
			}
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 3; j++)
		{
			char_array_3[j] = '\0';
		}

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
		{
			ret.Add(skBase64Characters[char_array_4[j]]);
		}

		while ((i++ < 3))
		{
			ret.Add('=');
		}
	}

	ret.Validate();
	return ret;
}

// Decodes provided base64 string to the array of bytes.
//    @param base64 - reference to the base64 string to be decoded.
//    @return - decoded byte array.
Array<ut::byte> DecodeBase64(const String& base64)
{
	size_t size = base64.Length();
	size_t i = 0;
	size_t j = 0;
	size_t in = 0;
	byte char_array_4[4], char_array_3[3];
	Array<ut::byte> ret;

	while (size-- && (base64[in] != '=') && IsBase64(base64[in]))
	{
		char_array_4[i++] = base64[in];
		in++;
		if (i == 4)
		{
			for (i = 0; i < 4; i++)
			{
				const char* c = StrChr(skBase64Characters, char_array_4[i]);
				char_array_4[i] = static_cast<ut::byte>(c - skBase64Characters);
			}

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
			{
				ret.Add(char_array_3[i]);
			}
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 4; j++)
		{
			char_array_4[j] = 0;
		}


		for (j = 0; j < 4; j++)
		{
			const char* c = StrChr(skBase64Characters, char_array_4[j]);
			char_array_4[j] = static_cast<ut::byte>(c - skBase64Characters);
		}

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++)
		{
			ret.Add(char_array_3[j]);
		}
	}

	return ret;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
