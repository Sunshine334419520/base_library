#ifndef BASE_SIGNALETON_H
#define BASE_SIGNALETON_H

#include <atomic>

#include "base/base_export.h"



namespace base {

namespace internal {

void* kLazyInstanceStateCreating (new int(1));
void* kLazyDefaultInstanceState = nullptr;

BASE_EXPORT bool NeedsLazyInstance(std::atomic<void*>* state);
BASE_EXPORT void CompleteLazyInstance(void (*destructor)(void*),
                                          void* destructor_arg);

    
} // internal


namespace subtle {

template <typename Type> 
Type* GetOrCreateLazyPointer(std::atomic<Type*>* state,
                             Type* (*creator_func)(void*),
                             void* creator_arg,
                             void (*destrucotr)(void*),
                             void* destructor_arg) {
    //std::atomic<void*> instance = state->load();
    //auto instance = state->load();
    std::atomic<void*> instance(state->load(std::memory_order_acquire));

    if (!instance) {
        
        //state->compare_exchange_strong(nullptr, const_cast<void*>(internal::kLazyInstanceStateCreating));
        if (internal::NeedsLazyInstance(state)) {
            instance = (*creator_func)(creator_arg);

            state->store(instance.load(), std::memory_order_release);

            //if (destrucotr)
            internal::CompleteLazyInstance(destrucotr, destructor_arg);
                
        }
        else {
            instance = state->load(std::memory_order_acquire);
        }
    }
    return reinterpret_cast<Type*>(instance);
}


}   // namespace subtle.

template <typename Type>
struct DefaultSingletonTraits {
    // Allocates the object.
    static Type* New() {
        return new Type();
    }

    // Destroys the object.
    static void Delete(Type* x) {
        delete x;
    }

    static const bool kRegisterAtExit = true;

};


// for example.
// Singleton<Type>::get();
template <typename Type,
          typename Traits = DefaultSingletonTraits,
           typename DifferentiatingType = Type>
class Singleton {
 private:
    // 瑕佹兂浣跨敤杩欎釜绫伙紝Type蹇呴』鍖呭惈GetInstance鍑芥暟锛屽苟涓旈伒寰埗瀹氬瑙勫垯.
    friend Type* Type::GetInstance();

    // 鑾峰緱涓€涓猅ype绫诲瀷鎸囬拡锛屽閮ㄤ娇鐢ㄨ繖涓嚱鏁拌幏寰椾竴涓嚎绋嬪畨鍏ㄧ殑鍗曞疄渚嬫寚閽?
    static Type* get() {
        return subtle::GetOrCreateLazyPointer(instance_, CreatorFunc,
                                              nullptr, OnExit, nullptr);
    }

    static Type* CreatorFunc(void* /* creator_arg*/) { return Traits::New(); }

    // 鍒犻櫎鍑芥暟骞朵笉闇€瑕佷繚璇佺嚎绋嬪畨鍏ㄦ€э紝瀹冧粎浠呭湪杩涚▼缁堟鍓嶈皟鐢?
    // 杩欎釜鍑芥暟鍙細鍦ㄤ竴涓嚎绋嬭皟鐢? 鎵€浠ュ鍘熷瓙鐨勮闂互鍙婁慨鏀规垜浠娇鐢?	// std::memory_order_relaxed.
    static void OnExit(void* /* unused*/) {
        Traits::Delete(instance_.load(std::memory_order_relaxed);
        instance_ .store(internal::kLazyDefaultInstanceState, 
						 std::memory_order_relaxed);
    }

    static std::atomic<Type*> instance_;    
};

template <typename Type, typename Traits, typename DifferentiatingType>
std::atomic<Type*> Singleton<Type, Traits, DifferentiatingType>::instance_ =
        base::internal::kLazyDefaultInstanceState;


}



#endif // BASE_SIGNALETON_H