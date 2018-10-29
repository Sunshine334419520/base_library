/**
* @Author: YangGuang
* @Date:   2018-10-29
* @Email:  guang334419520@126.com
* @Filename: default_destroy.h
* @Last modified by:  YangGuang
*/
#ifndef BASE_DEFAULT_DESTROY_H
#define BASE_DEFAULT_DESTROY_H

namespace base {

// 删除函数，为了std::shared_ptr.
struct DefaultDestroyTraits {
	template <typename Object>
	static void Destroy(Object* object) {
		delete object;
		object = nullptr;
	}
};

}	// namespace base.

#endif // !BASE_DEFAULT_DESTROY_H
