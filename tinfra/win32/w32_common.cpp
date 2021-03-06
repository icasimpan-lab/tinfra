//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/platform.h"

#ifdef TINFRA_W32

#include "tinfra/win32.h"

#include "tinfra/fmt.h"
#include "tinfra/string.h"

#include <stdexcept>
#include <cassert>

#ifndef NOMINMAX
#define WIN32_LEAN_AND_MEAN
#endif 

#ifndef NOMINMAX
#define NOMINMAX
#endif

#define _WIN32_WINNT 0x0500 // Windows 2000
#include <windows.h>

namespace tinfra { namespace win32 {
    
std::string get_error_string(unsigned int error_code)
{
    LPVOID lpMsgBuf;
    if( ::FormatMessageW(
	FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
	NULL,
	error_code,
	MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
	(LPWSTR) &lpMsgBuf,
	0,
	NULL
	) < 0 || lpMsgBuf == NULL) {

	return tsprintf("unknown error: %i", error_code);
    }
    std::wstring w_result((WCHAR*)lpMsgBuf);
    ::LocalFree(lpMsgBuf);
    std::string result = make_utf8(w_result.c_str());
    strip_inplace(result);
    return result;
}

template <typename E>
static void throw_system_error2(unsigned int error, const std::string& message)
{
    std::string error_str = get_error_string(error);
    std::string full_message = tinfra::tsprintf("%s: %s", message, error_str);
    throw E(full_message);
}

void throw_system_error(unsigned int error, std::string const& message)
{
    switch( error ) {
    case WSAEFAULT:
    case ERROR_INVALID_HANDLE:
        throw_system_error2<std::invalid_argument>(error, message);
        
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
    
    case WSAHOST_NOT_FOUND:
    case WSANO_DATA:
    case WSAECONNREFUSED:
    default:
        throw_system_error2<std::runtime_error>(error, message);
    }
}
void throw_system_error(std::string const& message)
{
    unsigned int error = ::GetLastError();
    throw_system_error(error, message);
}

void get_available_drives(std::vector<std::string>& result)
{
    TCHAR drives[1024];
    DWORD len;
    len = ::GetLogicalDriveStrings(sizeof(drives), drives);
    
    if( len == 0 || len > sizeof(drives)) {
        throw_system_error("GetLogicalDriveStrings failed"); 
    }
    size_t remaining_bytes = len;

    TCHAR* p = drives;
    while( *p && remaining_bytes > 0 ) {
        const size_t current_fragment_length = strlen(p);    
        result.push_back(fmt("%s:/") % p[0]);
        p += current_fragment_length + 1;
        remaining_bytes -= current_fragment_length;
    }
}

std::wstring make_wstring_from_utf8(tstring const& str)
{
    // TODO: write test for this one
    // http://msdn.microsoft.com/en-us/library/dd319072(VS.85).aspx
    const UINT codePage = CP_UTF8;
    const DWORD flags   = MB_ERR_INVALID_CHARS;

    const int buffer_size = MultiByteToWideChar(
        codePage, flags, 
        str.data(), int(str.size()),
        0, 0 );

    wchar_t* buffer = new wchar_t[buffer_size+1];
    
    int written = MultiByteToWideChar(
        codePage, flags, 
        str.data(), int(str.size()),
        buffer, buffer_size );
    
    if( written == 0 ) {
        delete [] buffer;
        throw_system_error("unable to convert utf8 -> unicode");
    }
    buffer[written] = 0;
    
    std::wstring result(buffer);
    delete[] buffer;
    return result;
}

// TODO: implement it
//std::wstring make_wstring_from(tstring const& str, int encoding)
//{
//}

std::string make_utf8(wchar_t const* str)
{
    // TODO: write test for this one
    // http://msdn.microsoft.com/en-us/library/dd374130(VS.85).aspx
    const UINT codePage = CP_UTF8;
    const DWORD flags   = 0;
    const int result_size = WideCharToMultiByte(
        codePage, flags,
        str, -1,
        0, 0,
        NULL,
        NULL);
    char* buffer = new char[result_size+1];
    const int written = WideCharToMultiByte(
        codePage, flags,
        str, -1,
        buffer, result_size,
        NULL,
        NULL);
    if( written == 0 ) {
        delete [] buffer;
        throw_system_error("unable to convert unicode -> utf8");
    }
    buffer[written] = 0;
    std::string result(buffer, written-1);
    delete [] buffer;
    return result;
}
    

} } // end namespace tinfra::win32

#endif // TINFRA_W32

