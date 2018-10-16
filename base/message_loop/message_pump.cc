/**
* @Author: YangGuang
* @Date:   2018-10-13
* @Email:  guang334419520@126.com
* @Filename: message_pump.h
* @Last modified by:  YangGuang
*/
#include "base/message_loop/message_pump.h"

namespace base {
MessagePump::MessagePump() = default;

MessagePump::~MessagePump() = default;

void MessagePump::SetTimerSlack()
{
}

}