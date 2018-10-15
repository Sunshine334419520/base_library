/**
* @Author: YangGuang
* @Date:   2018-10-13
* @Email:  guang334419520@126.com
* @Filename: message_loop.h
* @Last modified by:  YangGuang
*/
#ifndef BASE_MESSAGE_LOOP_MESSAGE_LOOP_H
#define BASE_MESSAGE_LOOP_MESSAGE_LOOP_H

#include <memory>
#include <queue>
#include <string>

#include "base/base_export.h"
#include "base/callback.h"
#include "base/macor.h"



namespace base {

class BASE_EXPORT MessageLoop : public MessagePump::Delegate {

};

}

#endif // !BASE_MESSAGE_LOOP_MESSAGE_LOOP_H
