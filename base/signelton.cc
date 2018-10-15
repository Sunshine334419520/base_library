
//#include "singleton.h"
#include "singleton.h"

#include <chrono>
#include <thread>



namespace base {
namespace internal {

bool NeedsLazyInstance(std::atomic<void*>* state) {
   // 灏濊瘯鍘诲垱寤轰竴涓狪nstance,濡傛灉鎴戜滑鏄槸绗竴涓埌鏉ョ殑绾跨▼,杩斿洖true.
   // 鍚﹀垯浠ｈ〃涓嶆槸绗竴涓嚎绋嬪埌鏉ワ紝鍙兘浜х敓绾跨▼瀵规暟鎹浜夋姠.
   if (state->compare_exchange_strong(kLazyDefaultInstanceState,
                                      kLazyInstanceStateCreating,
                                      std::memory_order_acq_rel)) {
      // 鏄涓€涓嚎绋嬪埌鏉ワ紝姣旇緝瀹屼箣鍚庯紝鎶妔tate鍊煎師鍏堝鍊煎鍒朵负kLazyInstanceStateCreateing.
      // 浠ヤ负涓嬩竴涓埌鏉ュ绾跨▼姣旇緝浼氬け璐? 浠庤€屽彲浠ヨ揪鍒板悓姝ュ鏁堟灉锛屽彲浠ョ洿鎺ヨ繑鍥瀟rue.
      return true;
   }

   // 涓嶆槸绗竴涓嚎绋嬶紝鍦ㄨ繖閲岃淇濊瘉鏁版嵁瀵规纭€э紝鍥犳涓€鐩寸瓑寰卻tate涓嶇瓑浜巏LazyInstanceStateCreating.
   if (state->load(std::memory_order_acquire) == kLazyInstanceStateCreating) {
      // 杩欑鎯呭喌闇€瑕佷繚鎶ゅ绾跨▼鐨勫畨鍏紝璁﹕tate != kLazyInstanceStateCreatingd锛?      // 褰撹繖閲岄潰涓嶇瓑浜庢椂锛岃鏄庡彟涓€涓嚎绋嬩慨鏀逛簡瀹冪殑鍊硷紝杩欎釜鍊肩幇鍦ㄦ槸姝ｇ‘鐨勶紝鍙互杩涜杩斿洖
      auto start = std::chrono::system_clock::now();

      do {
         const std::chrono::milliseconds elapsed = std::chrono::duration_cast<
                 std::chrono::milliseconds>(std::chrono::system_clock::now() - start);

         const std::chrono::milliseconds max_sleep_time(1);

         if (elapsed < max_sleep_time)
            std::this_thread::yield();
         else
            std::this_thread::sleep_for(max_sleep_time);
      } while(state->load(std::memory_order_acquire)
              == kLazyInstanceStateCreating);
   }

   return false;
}

void CompleteLazyInstance(void (*destructor)(void*),
                          void* destructor_arg) {

}


}   // namespace internal
}   // namespace bas