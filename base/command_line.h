/**
* @Author: YangGuang
* @Date:   2018-10-10
* @Email:  guang334419520@126.com
* @Filename: command_line.h
* @Last modified by:  YangGuang
*/

#ifndef BASE_COMMAND_LINE_H
#define BASE_COMMAND_LINE_H

#include <string>
#include <vector>
#include <map>

#include "base/base_export.h"

namespace base {

class BASE_EXPORT CommandLine {
 public:
     using StringType = std::string;
     using CharType = StringType::value_type;
     using StringVector = std::vector<StringType>;
     using SwitchMap = std::map<std::string, StringType>;

     enum NoProgram { kNoProgram };
     explicit CommandLine(const NoProgram no_program);

     CommandLine(int argc, const char* const* argv);
     explicit CommandLine(const StringVector& argv);

     // Override copy and assign to ensure |switches_by_stringpiece_| is valid.
     CommandLine(const CommandLine& other);
     CommandLine& operator=(const CommandLine& other);

     ~CommandLine();
     // 获取当前进程的commandline，这个是一个signaled对象，一个进程只可能存在一个
     // CommandLine对象，再使用时用这个函数来获取CommandLine对象指针.
     static CommandLine* ForCurrentProcess();

     static bool Init(int argc, const char* const* argv);

     static void Reset();

     // 如果当前进程已经给CommandLine初始化了, 放回true，否则false.
     static bool InitializedForCurrentProcess();

     // 设置当前进程的名称，默认为argv[0].
     void SetProgram(const std::string& path);
     void SetProgram(const char* path);

     // 获得当前进程的名称，默认为argv[0].
     const std::string& GetProgram() const;

     // 判读一个switch是否存在，一般情况下都需要使用这个接口之后，再使用
     // GetSwitchValue接口. 如果存在放回true，否则false.
     bool HasSwitch(const std::string& switch_string) const;
     bool HasSwitch(const char switch_constant[]) const;

     // 获取指定switch的值，再调用这个函数之前，尽量先调用HasSwitch判断这个
     // switch是否存在，存在之后再调用当前方法来获取value, 成功放回switch对应
     // 的值.
     std::string GetSwitchValue(const std::string& switch_string) const;
     std::string GetSwitchValue(const char* switch_constant) const;

     const SwitchMap& GetSwitchs() const { return switches_; }
     StringVector GetArgs() const {
         return  StringVector(argv_.begin() + begin_args_, argv_.end());
     }

     // 添加一个Swithch，for example:
     // AppendSwitch(std::string("--foo"), std::string("100"));
     void AppendSwitch(const std::string& switch_string);
     void AppendSwitch(const std::string& switch_string, 
                       const std::string& value);

     void AppendArg(const std::string& arg);

    
 private:
     
     void InitFromArgv(int argc, const CharType* const* argv);
     void InitFromArgv(const StringVector& argv);


     static CommandLine* current_process_commandline_;

     // 一个argv的例子 key的前缀必须是[-,--]，key和value的分节符是[=], 之间可以又空格
     // 选项之间用空格或者,号隔开
     // for example : --value = 2 --switch = false --off-debug
     StringVector argv_;

     SwitchMap switches_;

     // 记录着argv_开始的地方.
     std::size_t begin_args_;
};

}   // namespace base

#endif // !BASE_COMMAND_LINE_H
