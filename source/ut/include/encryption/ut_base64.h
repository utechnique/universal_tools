//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "text/ut_string.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Encodes provided data to the base64 string.
//    @param data - pointer to the data to be encoded.
//    @param size - size of the data in bytes.
//    @return - base64 string.
String EncodeBase64(const byte* data, size_t size);

// Decodes provided base64 string to the array of bytes.
//    @param base64 - reference to the base64 string to be decoded.
//    @return - decoded byte array.
Array<ut::byte> DecodeBase64(const String& base64);

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//