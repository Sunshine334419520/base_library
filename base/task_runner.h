/**
* @Author: YangGuang
* @Date:   2018-10-20
* @Email:  guang334419520@126.com
* @Filename: task_runner.h
* @Last modified by:  YangGuang
*/
#ifndef BASE_TASK_RUNNER_H
#define BASE_TASK_RUNNER_H

#include <stddef.h>

#include <chrono>

#include "base/base_export.h"
#include "base/macor.h"
#include "base/callback.h"
#include "base/location.h"

namespace base {

struct TaskRunnerTraits;

// 涓€涓猅askRunner瀵硅薄鏄竴涓敤鏉ヨ繍琛宲osted task鐨勫璞★紝TaskRunner鎻愪緵浜?// 涓€涓彲浠ヨ繍琛屾瘡涓€涓猼ask鐨勬柟娉? TaskRunner 鎻愪緵涓€涓潪甯竪eak鐨勪繚璇侊紝鍦?// 浠€涔堟椂鍊欒繖涓猼ask杩愯锛?// guarantees: 
//
//  - posting鐨則ask涓嶄細鍚屾杩愯锛屾病鏈塒ost*Task鏂规硶鍒扮珛鍗宠繍琛宼ask
//  
//  - 澧炲姞鐨刣elay鍙兘鍦ㄤ换鍔¤繍琛岀殑鏃跺€欏欢杩燂紝 澧炲姞杩欎釜delay涓嶅彲鑳藉奖鍝嶅埌姝ｅ湪
//    杩愯鐨勪换鍔★紝 瀹冨彲浠ヤ娇task鏇存櫄鐨勬墽琛岋紝浣嗘槸瀹冧笉鑳藉璁﹖ask杩囨棭鐨勮繍琛? 
//
// TaskRunner 涓嶄繚璇乸osted task鎸夌収椤哄簭鎵ц锛屾槸鍚﹂噸澶嶏紝鏄惁杩愯鍦ㄤ竴涓?// 鍒跺畾鐨勭嚎绋嬩笂锛岃繖浜涘畠閮戒笉鑳戒繚璇侊紝涔熶笉淇濊瘉浠诲姟涔嬮棿鍦ㄤ竴涓猰emory model涓?// 鍏变韩鏁版嵁. 锛堟崲涓€鍙ヨ瘽璇达紝浣犲簲璇ヤ娇鐢ㄤ綘鑷繁鐨剆ynchronization/locking.
// 濡傛灉浣犻渶瑕佸湪浠诲姟涔嬮棿鍏变韩鏁版嵁)
// 
// 瀹炰緥鍖朤askRunner蹇呴』鏃跺畨鍏ㄧ殑璋冪敤鍦ㄦ瘡涓€涓嚎绋嬩笂闈紝鎵€浠ヤ竴鑸娇鐢?// std::unqiue_ptr or std::shared_ptr
//
// Some theoretical implementations of TaskRunner: 
//  - 涓€涓猅askRunner浣跨敤涓€涓猼hread pool鏉ヨ繍琛宲osted tasks.
//
//  - 涓€涓猅askRunner瀵逛簬姣忎釜task锛岀敤涓€涓猲on-jojinable thread鍘昏繍琛?//    瀹冧滑锛岀劧鍚庣珛鍗抽€€鍑?
//
//  - 涓€涓猅askRunner搴旇鐢ㄤ竴涓猯ist鏉ヤ繚瀛榩osted tasks锛屽苟涓旀彁渚涗竴涓柟娉?//    Run() 鏉ラ殢鏈虹殑椤哄簭杩愯姣忎竴涓彲浠ヨ繍琛岀殑浠诲姟. 
class BASE_EXPORT TaskRunner {
 public: 

     // Posts 杩欎釜缁欎簣鐨勪换鍔″埌杩愯锛?濡傛灉浠诲姟鍙兘鍦ㄥ皢鏉ョ殑鏌愪釜鏃跺埢鎵ц锛岃繑鍥?     // ture锛屽鏋滀换鍔¤偗瀹氫笉浼氳繍琛屽垯杩斿洖false.
     bool PostTask(const Location& from_here,
                   Closure Task);
    
     // 杩欎釜鍍廝ostTask涓€鏍凤紝浣嗘槸閫氳繃杩欎釜鍑芥暟posted鐨則ask鍙細鍦ㄥ欢杩焏elay鏃堕棿锛?     // 鎵嶄細杩愯
     virtual bool PostDelayedTask(const Location& from_here,
                                 Closure Task,
                                 std::chrono::milliseconds delay) = 0;
                                 
     // 濡傛灉杩斿洖true锛屼唬琛ㄥ疄鍦ㄥ綋鍓嶅簭鍒楋紝鎴栬€呰鏄粦瀹氬埌鐨勫綋鍓嶇嚎绋? 
     virtual bool RunsTasksInCurrentSequence() const = 0;

     // Posts |task| 鍒板綋鍓嶇殑TaskRunner锛屽湪浠诲姟杩愯瀹屾垚涔嬪悗锛寍reply| 浼氬彂閫佸埌
     // 璋冪敤杩欎釜PostTaskAndReply()鐨勭嚎绋嬩笂锛?|task| and |reply|閮介渶瑕佷繚璇佸湪
     // 璋冪敤PostTaskAndReplay()绾跨▼涓婂垹闄?     // See the following pseudo-code: 
     //
     // class DataBuffer {
     //      void AddData(void* buf, size_t length);
     //      ...
     //  };
     //
     // class DataLoader {
     //  public:
     //      void GetData() {
     //          std::shared_ptr<DataBuffer> buffer = new DataBuffer();
     //          target_thread_.task_runner()->PostTaskAndReply(
     //              FROM_HERE,
     //              std::bind(&DataBuffer::AddData, buffer),
     //              std::bind(&DdataLoader::OndataReceived, std::weak_ptr<DataBuffer>(buffer)));
     //      }
     //   private: 
     //       void OnDataReceived(std::shared_ptr<DataBuffer> buffer) {
     //    
     //       }
     // };
     //
     // Things to notice:
     //   * 浠诲姟鐨勭粨鏋滄槸閫氳繃task鍜宺eply鐨勫叡浜弬鏁癉ataBuffer缁戝畾鏉ュ疄鐜扮殑
     //   * 杩欎釜DataLoader瀵硅薄娌℃湁鐗规畩鐨勭嚎绋嬪畨鍏? 
     //   * 鐢变簬浣跨敤浜唖td::weak_ptr鎵€浠ュ湪浠诲姟杩愯鐨勬椂鍊欏垹闄ataLoader鏄笉浼氬奖鍝嶅埌
     //     浠诲姟鐨勬帴鍙楃殑锛寃eak_ptr鍙戠幇琚垹闄や簡灏变笉浼氬彂閫佸洖澶嶄换鍔′簡.
    
     bool PostTaskAndReplay(const Location& from_here,
                           Closure task,
                           Closure reply);
                           
 protected: 
     friend struct TaskRunnerTraits;
     TaskRunner();
     virtual ~TaskRunner();

     // 鍦ㄥ璞″簲璇estroyed鐨勬椂鍊欒皟鐢ㄨ繖涓嚱鏁帮紝 杩欎釜鍑芥暟鍙細鍋氱畝鍗曠殑deletes |this|,
     // 濡傛灉浣犳兂瑕佸仛鏇村鐨勬搷浣滐紝姣斿鍒犻櫎鍦ㄦ寚瀹氱殑绾跨▼涓婏紝閭ｄ箞灏辫閲嶈浇瀹?
     virtual void OnDestruct() const;
};

struct BASE_EXPORT TaskRunnerTraits {
    static void Destruct(const TaskRunner* task_runner);
};

}       // namespace base
    
#endif // BASE_TASK_RUNNER_H