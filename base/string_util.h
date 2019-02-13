/**
* @Author: YangGuang
* @Date:   2019-01-21
* @Email:  guang334419520@126.com
* @Filename: string_util.h
* @Last modified by:  YangGuang
*/
#ifndef BASE_STRING_STRING_UTIL_H
#define BASE_STRING_STRING_UTIL_H

#include <string>

#include "base/base_export.h"

namespace base {

using string16 = std::wstring;

enum TrimPositions {
    kTrimNone = 0,
    kTrimLeading = 1 << 0,
    kTrimTrailing = 1 << 1,
    kTrimAll = kTrimLeading | kTrimTrailing
};

BASE_EXPORT TrimPositions TrimString(const std::string& input,
                                     TrimPositions positions,
                                     const std::string& piece,
                                     std::string* output);

BASE_EXPORT TrimPositions TrimWhitespace(const std::string& input,
                                         TrimPositions positions, 
                                         std::string* output);

BASE_EXPORT TrimPositions TrimWhitespace(const string16& input,
                                         TrimPositions positions,
                                         string16* output);

}   // namespace base

#endif // !BASE_STRING_STRING_UTIL_H