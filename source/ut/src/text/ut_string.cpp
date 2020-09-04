//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "text/ut_string.h"
//----------------------------------------------------------------------------//
#define UT_UTF8_MAX 6
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// StrConvert spec for utf8: wchar -> char
template<> String StrConvert<wchar, char, cp_utf8>(const wchar* src)
{
#if UT_WINDOWS
	// check length
	size_t length = StrLen<wchar>(src);
	if (length == 0)
	{
		return String();
	}

	// get size of the char string in elements
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, src, (int)length, NULL, 0, NULL, NULL);
	String out_str(size_needed);

	// convert string and add null terminator
	WideCharToMultiByte(CP_UTF8, 0, src, (int)length, out_str.GetAddress(), size_needed, NULL, NULL);
	return out_str;
#elif UT_UNIX
	size_t src_length = StrLen<wchar>(src) + 1;
	size_t dst_max_l = src_length * UT_UTF8_MAX;
	String out_str(dst_max_l);
	iconv_t ic = iconv_open("UTF-8", "WCHAR_T");
	size_t iconv_in_bytes = src_length * sizeof(wchar_t);
	size_t iconv_out_bytes = dst_max_l;
	char* iconv_in = (char*)src;
	char* iconv_out = out_str.GetAddress();
	iconv(ic, &iconv_in, &iconv_in_bytes, &iconv_out, &iconv_out_bytes);
	iconv_close(ic);
	return out_str;
#endif
}

// StrConvert spec for utf8: char -> wchar
template<> WString StrConvert<char, wchar, cp_utf8>(const char* src)
{
#if UT_WINDOWS
	// check length
	size_t length = StrLen<char>(src);
	if (length == 0)
	{
		return WString();
	}

	// get size of the wchar string in bytes
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, src, (int)length, NULL, 0);
	WString out_str(size_needed);

	// convert string and add null terminator
	MultiByteToWideChar(CP_UTF8, 0, src, (int)length, out_str.GetAddress(), size_needed);
	return out_str;
#elif UT_UNIX
	size_t src_length = StrLen<char>(src) + 1;
	size_t dst_max_l = src_length * sizeof(wchar_t);
	WString out_str(dst_max_l);
	iconv_t ic = iconv_open("WCHAR_T", "UTF-8");
	size_t iconv_in_bytes = src_length * UT_UTF8_MAX;
	size_t iconv_out_bytes = dst_max_l * sizeof(wchar_t);
	Array<char> src_cpy(src_length * 5 * UT_UTF8_MAX + 1);
	memory::Set(src_cpy.GetAddress(), 0, src_cpy.GetSize());
	memory::Copy(src_cpy.GetAddress(), src, src_length);
	char* iconv_in = (char*)src_cpy.GetAddress();
	char* iconv_out = (char*)out_str.GetAddress();
	iconv(ic, &iconv_in, &iconv_in_bytes, &iconv_out, &iconv_out_bytes);
	iconv_close(ic);
	return out_str;
#endif
}

//----------------------------------------------------------------------------//
// 'bool' specialization of the template ut::Print<>() function
template<> String Print<bool>(const bool& val)
{
	return val ? String("true") : String("false");
}

// 'int8' specialization of the template ut::Print<>() function
template<> String Print<int8>(const int8& val)
{
	String str;
	str.Print("%hhi", val);
	return str;
}

// 'byte' specialization of the template ut::Print<>() function
template<> String Print<byte>(const byte& val)
{
	String str;
	str.Print("%hhu", val);
	return str;
}

// 'int16' specialization of the template ut::Print<>() function
template<> String Print<int16>(const int16& val)
{
	String str;
	str.Print("%hi", val);
	return str;
}

// 'uint16' specialization of the template ut::Print<>() function
template<> String Print<uint16>(const uint16& val)
{
	String str;
	str.Print("%hu", val);
	return str;
}

// 'int32' specialization of the template ut::Print<>() function
template<> String Print<int32>(const int32& val)
{
	String str;
	str.Print("%i", val);
	return str;
}

// 'uint32' specialization of the template ut::Print<>() function
template<> String Print<uint32>(const uint32& val)
{
	String str;
	str.Print("%u", val);
	return str;
}

// 'int64' specialization of the template ut::Print<>() function
template<> String Print<int64>(const int64& val)
{
	String str;
	str.Print("%lli", val);
	return str;
}

// 'uint64' specialization of the template ut::Print<>() function
template<> String Print<uint64>(const uint64& val)
{
	String str;
	str.Print("%llu", val);
	return str;
}

// 'float' specialization of the template ut::Print<>() function
template<> String Print<float>(const float& val)
{
	String str;
	str.Print("%f", val);
	return str;
}

// 'double' specialization of the template ut::Print<>() function
template<> String Print<double>(const double& val)
{
	String str;
	str.Print("%lf", val);
	return str;
}

// 'long double' specialization of the template ut::Print<>() function
template<> String Print<long double>(const long double& val)
{
	String str;
	str.Print("%Lf", val);
	return str;
}

// 'void*' specialization of the template ut::Print<>() function
template<> String Print<void*>(void* const& val)
{
	String str;
	str.Print("0x%0x", val);
	return str;
}

// 'const char*' specialization of the template ut::Print<>() function
template<> String Print<const char*>(const char* const& str)
{
	return String(str);
}

// 'ut::String' specialization of the template ut::Print<>() function
template<> String Print<String>(const String& str)
{
	return str;
}

//----------------------------------------------------------------------------//
// 'bool' specialization of the template ut::Scan<>() function
template<> bool Scan<bool>(const String& str)
{
	if (StrCmp<char>(str.GetAddress(), "true") ||
		StrCmp<char>(str.GetAddress(), "True") ||
		StrCmp<char>(str.GetAddress(), "TRUE"))
	{
		return true;
	}
	return false;
}

// 'int8' specialization of the template ut::Scan<>() function
template<> int8 Scan<int8>(const String& str)
{
	// note that we need to use 16 bit integer variable
	// because visual studio 2010 and older don't support %hhu modifier
	int16 val;
	str.Scan("%hi", &val);
	return (int8)val;
}

// 'byte' specialization of the template ut::Scan<>() function
template<> byte Scan<byte>(const String& str)
{
	// note that we need to use 16 bit integer variable
	// because visual studio 2010 and older don't support %hhu modifier
	uint16 val;
	str.Scan("%hu", &val);
	return (byte)val;
}

// 'int16' specialization of the template ut::Scan<>() function
template<> int16 Scan<int16>(const String& str)
{
	int16 val;
	str.Scan("%hi", &val);
	return val;
}

// 'uint16' specialization of the template ut::Scan<>() function
template<> uint16 Scan<uint16>(const String& str)
{
	uint16 val;
	str.Scan("%hu", &val);
	return val;
}

// 'int32' specialization of the template ut::Scan<>() function
template<> int32 Scan<int32>(const String& str)
{
	int32 val;
	str.Scan("%i", &val);
	return val;
}

// 'uint32' specialization of the template ut::Scan<>() function
template<> uint32 Scan<uint32>(const String& str)
{
	uint32 val;
	str.Scan("%u", &val);
	return val;
}

// 'int64' specialization of the template ut::Scan<>() function
template<> int64 Scan<int64>(const String& str)
{
	int64 val;
	str.Scan("%lli", &val);
	return val;
}

// 'int64' specialization of the template ut::Scan<>() function
template<> uint64 Scan<uint64>(const String& str)
{
	uint64 val;
	str.Scan("%llu", &val);
	return val;
}

// 'float' specialization of the template ut::Scan<>() function
template<> float Scan<float>(const String& str)
{
	float val;
	str.Scan("%f", &val);
	return val;
}

// 'double' specialization of the template ut::Scan<>() function
template<> double Scan<double>(const String& str)
{
	double val;
	str.Scan("%lf", &val);
	return val;
}

// 'long double' specialization of the template ut::Scan<>() function
template<> long double Scan<long double>(const String& str)
{
	long double val;
	str.Scan("%lf", &val);
	return val;
}

// 'ut::String' specialization of the template ut::Scan<>() function
template<> String Scan<String>(const String& str)
{
	return str;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//