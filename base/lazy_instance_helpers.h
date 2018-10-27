/**
* @Author: YangGuang
* @Date:   2018-10-15
* @Email:  guang334419520@126.com
* @Filename: lazy_instacne_helpers.h
* @Last modified by:  YangGuang
*/
#ifndef BASE_LAZY_INSTANCE_HELPERS_H
#define BASE_LAZY_INSTANCE_HELPERS_H

#include <atomic>

#include "base/base_export.h"


namespace base {

namespace internal {

extern void* kLazyInstanceStateCreating;
extern void* kLazyDefaultInstanceState;

BASE_EXPORT bool NeedsLazyInstance(std::atomic<void*>* state);
BASE_EXPORT void CompleteLazyInstance(void(*destructor)(void*),
									  void* destructor_arg);


} // internal


namespace subtle {

template <typename Type>
Type* GetOrCreateLazyPointer(std::atomic<Type*>* state,
							 Type* (*creator_func)(void*),
							 void* creator_arg,
							 void(*destrucotr)(void*),
							 void* destructor_arg) {
	std::atomic<Type*> instance(state->load(std::memory_order_acquire));

	if (!instance.load()) {

		//state->compare_exchange_strong(nullptr, const_cast<void*>(internal::kLazyInstanceStateCreating));
		if (internal::NeedsLazyInstance(reinterpret_cast<std::atomic<void*>*>(state))) {
			instance = (*creator_func)(creator_arg);

			state->store(instance.load(), std::memory_order_release);

			internal::CompleteLazyInstance(destrucotr, destructor_arg);
		}
		else {
			instance = state->load(std::memory_order_acquire);
		}
	}
	return reinterpret_cast<Type*>(instance.load());
}


}   // namespace subtle.

}	// namespace base.

#endif // !BASE_LAZY_INSTANCE_HELPERS_H