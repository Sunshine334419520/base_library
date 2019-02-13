#include "command_line.h"

#include <cassert>

#include "base/string_util.h"

namespace base {

namespace {

const CommandLine::CharType kSwitchTerminator[] = "--";
const CommandLine::CharType kSwitchValueSeparator[] = "=";

const CommandLine::CharType* const kSwitchPrefixs[] = { "--", "-" };
const std::size_t switch_prefixs_count = 2;

std::size_t GetSwitchPrefixLength(const std::string& switch_string) {
    for (int i = 0; i < switch_prefixs_count; i++) {
        const std::string prefix(kSwitchPrefixs[i]);
        if (switch_string.compare(0, prefix.length(), prefix) == 0)
            return prefix.length();
    }
    return 0;
}

bool IsSwitch(const std::string& str,
              std::string* switch_key,
              std::string* switch_value) {
    switch_key->clear();
    switch_value->clear();
    // 获取str的前缀字符, 没有就是选项格式错误，有且只有前缀也是错误
    std::size_t prefix_length = GetSwitchPrefixLength(str);
    if (prefix_length == 0 || str.length() == prefix_length)
        return false;
    const std::size_t pos = str.find(kSwitchValueSeparator);
    *switch_key = str.substr(0, pos);
    if (pos != std::string::npos)
        *switch_value = str.substr(pos + 1);
    return true;
}

void AppendSwitchesAndArguments(CommandLine* comm_line,
                                const CommandLine::StringVector & argv) {
    
    const std::size_t length = argv.size();
    for (std::size_t i = 1; i < length; i++) {
        std::string arg = argv[i];
        // 清理前后空格
        TrimWhitespace(arg, TrimPositions::kTrimAll, &arg);
        std::string switch_key;
        std::string switch_value;
        if (IsSwitch(arg, &switch_key, &switch_value)) {
            comm_line->AppendSwitch(switch_key, switch_value);
        }
        else {
            comm_line->AppendArg(arg);
        }
    }
}

}

CommandLine* CommandLine::current_process_commandline_ = nullptr;

CommandLine::CommandLine(const NoProgram no_program) {
}

CommandLine::CommandLine(int argc, const char * const * argv)
    : argv_(1),
      begin_args_(1){
    InitFromArgv(argc, argv);
}

CommandLine::CommandLine(const StringVector & argv) {
}

CommandLine::CommandLine(const CommandLine & other) {
}

CommandLine & CommandLine::operator=(const CommandLine & other) {
    // TODO: 在此处插入 return 语句
    return *this;
}

CommandLine::~CommandLine() {
}

CommandLine * CommandLine::ForCurrentProcess() {
    return current_process_commandline_;
}

bool CommandLine::Init(int argc, const char * const * argv) {
    if (current_process_commandline_) {
        return false;
    }

    current_process_commandline_ = new CommandLine(argc, argv);
    return true;
}

void CommandLine::Reset() {
    assert(current_process_commandline_);
    delete current_process_commandline_;
    current_process_commandline_ = NULL;
}

bool CommandLine::InitializedForCurrentProcess() {
    return !!current_process_commandline_;
}

void CommandLine::SetProgram(const std::string & path) {
    TrimWhitespace(path, kTrimAll, &argv_[0]);
}

void CommandLine::SetProgram(const char * path) {
    std::string new_path(path);
    SetProgram(new_path);
}

const std::string & CommandLine::GetProgram() const {
    return argv_[0];
}

bool CommandLine::HasSwitch(const std::string & switch_string) const {
    return switches_.find(switch_string) != switches_.end();
}

bool CommandLine::HasSwitch(const char switch_constant[]) const {
    std::string switch_string(switch_constant);
    return HasSwitch(switch_string);
}

std::string CommandLine::GetSwitchValue(
    const std::string & switch_string) const {
    auto result = switches_.find(switch_string);
    return result == switches_.end() ? std::string() : result->second;
}

std::string CommandLine::GetSwitchValue(
    const char * switch_constant) const {
    std::string switch_string(switch_constant);
    return GetSwitchValue(switch_string);
}

void CommandLine::AppendSwitch(const std::string & switch_string) {
    return AppendSwitch(switch_string, std::string());
}

void CommandLine::AppendSwitch(const std::string & switch_string,
                               const std::string & value) {
    std::string switch_key(switch_string); 
    std::string switch_value(value);
    TrimWhitespace(switch_string, kTrimAll, &switch_key);
    TrimWhitespace(value, kTrimAll, &switch_value);

    const std::size_t prefix_length = GetSwitchPrefixLength(switch_key);
    auto insertion = switches_.insert(
        std::make_pair(switch_key.substr(prefix_length), switch_value));
    if (!insertion.second)
        insertion.first->second = switch_value;

    // 合并switch_key and switch_value，可能出现switch_key有前缀或者没有，
    // 没有前缀就将key的前缀补上，最后将合并的string插入到argv_.
    std::string combined_switch_string = switch_key;
    if (prefix_length == 0)
        combined_switch_string = kSwitchPrefixs[0] + switch_key;
    if (!value.empty())
        combined_switch_string += kSwitchValueSeparator + switch_value;
    argv_.insert(argv_.begin() + begin_args_++, combined_switch_string);
}

void CommandLine::AppendArg(const std::string & arg) {
    argv_.push_back(arg);
}

void CommandLine::InitFromArgv(int argc, const CharType * const * argv) {
    StringVector new_argv;
    for (int i = 0; i < argc; i++) {
        new_argv.push_back(argv[i]);
    }
    InitFromArgv(new_argv);
}

void CommandLine::InitFromArgv(const StringVector & argv) {
    argv_ = StringVector(1);
    switches_.clear();
    SetProgram(argv.empty() ? std::string() : argv.at(0));
    AppendSwitchesAndArguments(this, argv);
}

}
