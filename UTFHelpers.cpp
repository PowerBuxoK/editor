#include "UTFHelpers.h"

std::wstring Utf8ToWstringICU(const std::string &utf8Str) {
  if (utf8Str.empty())
    return L"";

  icu::UnicodeString uStr = icu::UnicodeString::fromUTF8(utf8Str);

  UErrorCode status = U_ZERO_ERROR;
  int32_t requiredSize = 0;

  u_strToWCS(nullptr, 0, &requiredSize, uStr.getBuffer(), uStr.length(),
             &status);

  if (status == U_BUFFER_OVERFLOW_ERROR) {
    status = U_ZERO_ERROR;
  }

  std::wstring wStr;
  wStr.resize(requiredSize);

  u_strToWCS(&wStr[0], requiredSize + 1, nullptr, uStr.getBuffer(),
             uStr.length(), &status);

  if (U_FAILURE(status)) {

    return L"";
  }

  return wStr;
}

std::string WstringToUtf8ICU(const std::wstring &wStr) {
  if (wStr.empty())
    return "";

  UErrorCode status = U_ZERO_ERROR;
  int32_t requiredSize = 0;

  u_strFromWCS(nullptr, 0, &requiredSize, wStr.c_str(), wStr.length(), &status);

  if (status == U_BUFFER_OVERFLOW_ERROR) {
    status = U_ZERO_ERROR;
  }

  icu::UnicodeString uStr;
  UChar *buffer = uStr.getBuffer(requiredSize + 1);

  u_strFromWCS(buffer, requiredSize + 1, nullptr, wStr.c_str(), wStr.length(),
               &status);

  uStr.releaseBuffer(U_SUCCESS(status) ? requiredSize : 0);

  if (U_FAILURE(status)) {
    return "";
  }

  std::string utf8Str;
  uStr.toUTF8String(utf8Str);

  return utf8Str;
}
