#include <iostream>
#include <string>
#include <unicode/unistr.h>
#include <unicode/ustring.h> // Необходим для функции u_strFromWCS

std::wstring Utf8ToWstringICU(const std::string& utf8Str);
std::string WstringToUtf8ICU(const std::wstring& wStr);
