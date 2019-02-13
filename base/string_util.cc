#include "string_util.h"
/**
* @Author: YangGuang
* @Date:   2019-01-21
* @Email:  guang334419520@126.com
* @Filename: string_util.h
* @Last modified by:  YangGuang
*/

namespace base {

namespace {
const char kWhiteSpaceChat[] = " ";
}   // namespace.

TrimPositions TrimString(const std::string & input,
                         TrimPositions positions,
                         const std::string & trim_char, 
                         std::string * output) {
    
    // 寻找最开始的有效的位置，和最后的有效位置，如果需要不需要清理前部分，则将
    // first_good_char赋值为0，如果不需要后部分trim，则将last_good_char赋值
    // 为最后一个字符长度.
    const std::size_t last_char = input.length() - 1;
    const std::size_t first_good_char = positions & kTrimLeading ? 
        input.find_first_not_of(trim_char) : 0;
    const std::size_t last_good_char = positions & kTrimTrailing ?
        input.find_last_not_of(trim_char) : last_char;

    if (input.empty() ||
        first_good_char == std::string::npos ||
        last_good_char == std::string::npos) {
        bool input_was_empty = input.empty();  // in case output == &input
        output->clear();
        return input_was_empty ? kTrimNone : positions;
    }

    // Trim.
    *output = 
        input.substr(first_good_char, last_good_char - first_good_char + 1);

    // Return where we trimmed from.
    return static_cast<TrimPositions>(
        ((first_good_char == 0) ? kTrimNone : kTrimLeading) |
        ((last_good_char == last_char) ? kTrimNone : kTrimTrailing));
}

TrimPositions TrimWhitespace(const std::string & input,
                             TrimPositions positions, 
                             std::string * output) {
    return TrimString(input, positions, kWhiteSpaceChat, output);
}

TrimPositions TrimWhitespace(const string16 & input,
                             TrimPositions positions,
                             string16 * output) {
    return TrimPositions();
}

}   // namespace base.