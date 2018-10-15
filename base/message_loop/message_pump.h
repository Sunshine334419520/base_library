/**
* @Author: YangGuang
* @Date:   2018-10-13
* @Email:  guang334419520@126.com
* @Filename: message_pump.h
* @Last modified by:  YangGuang
*/
#ifndef BASE_MESSAGE_LOOP_MESSAGE_PUMP_H
#define BASE_MESSAGE_LOOP_MESSAGE_PUMP_H

#include "base/base_export.h"

namespace base {

class BASE_EXPORT MessagePump {
 public:
	 class BASE_EXPORT Delegate {
	  public:
		  //virtual ~Delegate() {}
	 };
};
}

#endif // !BASE_MESSAGE_LOOP_MESSAGE_PUMP
