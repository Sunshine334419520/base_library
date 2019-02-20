/**
* @Author: YangGuang
* @Date:   2019-01-21
* @Email:  guang334419520@126.com
* @Filename: stl_util.h
* @Last modified by:  YangGuang
*/

#ifndef BASE_SKIPLIST_H
#define BASE_SKIPLIST_H

#include <cassert>
#include <malloc.h>
#include <memory>

#include "base/port/port.h"
#include "base/random.h"

namespace base {

template <typename Key, typename Value>
class SkipList {
 private:
     struct Node;
 public:
     SkipList();
     ~SkipList();

     SkipList(const SkipList&) = delete;
     void operator=(const SkipList&) = delete;

     // 将key插入到跳表中.
     void Insert(const Key& key, const Value& value);

     // 如果在list中寻找到与key相等的条目，那么设置value为对应的值，返回true，
     // 否则返回false.
     bool LookUp(const Key& key, Value* value);

     class Iterator {
      public:
          explicit Iterator(const SkipList* list);

          // 如果iterator在一个有效的node上，返回true.
          bool Vaild() const;

          // REQUIRES: Valid()
          const Key& key() const;

          // REQUIRES: Valid()
          Value& value() const;

          // 跳转到下一个位置
          // REQUIRES: Valid()
          void Next();

          // 跳转到上一个位置
          // REQUIRES: Valid()
          void Prev();

          // Seek 到一个>= target的key值上.
          void Seek(const Key& target);

          // Seek 到第一个node
          void SeekToFirst();

          // Seek 到最后一个node
          void SeekToLast();

      private:
          friend class SkipList<Key, Value>;
          const SkipList<Key, Value>* list_;
          Node* node_;
     };
 private:
     enum { kMaxHeight = 12 };
     Node* const head_;

     base::AtomicPointer max_height_;

     inline int GetMaxHeight() const {
         return static_cast<int>(
             reinterpret_cast<intptr_t>(max_height_.NoBarrier_Load()));
     }
     base::Random rnd_;

     Node* NewNode(const Key& key, const Key& value, int max_height);
     int RandomHeight();
     Node* FindGreaterOrEqual(const Key& key, Node** prev) const;
     Node* FindLessThen(const Key& key) const;
     Node* FindLast() const;

     bool KeyIsAfterNode(const Key& x, const Key& y, Node* n) const {
         return n != nullptr && x > y;
     }
     bool Equal(const Key& x, const Key& y) const { return x == y; }

};

// Node 的实现.
template <typename Key, typename Value>
struct SkipList<Key, Value>::Node {
    explicit Node(const Key& k, const Value& v,int level)
        : key(k), value(v) {
        next_ = new AtomicPointer[level];
    }
    ~Node() { 
        delete []next_;
    }

    const Key key;
    Value value;

    Node* Next(int n) {
        assert(n >= 0);
        return reinterpret_cast<Node*>(next_[n].Acquire_Load());
    }

    void SetNext(int n, Node* x) {
        assert(n >= 0);
        next_[n].Release_Store(x);
    }

    Node* NoBarrier_Next(int n) {
        assert(n >= 0);
        return reinterpret_cast<Node*>(next_[n].NoBarrier_Load());
    }

    void NoBarrier_SetNext(int n, Node* x) {
        assert(n >= 0);
        next_[n].NoBarrier_Store(x);
    }
 private:
     base::AtomicPointer *next_;
     //base::AtomicPointer next_[1];
};

template<typename Key, typename Value>
inline SkipList<Key, Value>::Iterator::Iterator(const SkipList * list)
    : list_(list), node_(nullptr) {
}

template<typename Key, typename Value>
inline bool SkipList<Key, Value>::Iterator::Vaild() const {
    return node_ != nullptr;
}

template<typename Key, typename Value>
inline const Key & SkipList<Key, Value>::Iterator::key() const {
    assert(Vaild());
    return node_->key;
}

template<typename Key, typename Value>
inline Value& SkipList<Key, Value>::Iterator::value() const {
    assert(Vaild());
    return node_->value;
}

template<typename Key, typename Value>
inline void SkipList<Key, Value>::Iterator::Next() {
    assert(Vaild());
    node_ = node_->Next(0);
}

template<typename Key, typename Value>
inline void SkipList<Key, Value>::Iterator::Prev() {
    assert(Vaild());
    node_ = list_->FindLessThen(node_->key);
    if (node_ == list_->head_)
        node_ = nullptr;
}

template<typename Key, typename Value>
inline void SkipList<Key, Value>::Iterator::Seek(const Key & target) {
    node_ = list_->FindGreaterOrEqual(target, nullptr);
}

template<typename Key, typename Value>
inline void SkipList<Key, Value>::Iterator::SeekToFirst() {
    node_ = list_->head_->Next(0);
}

template<typename Key, typename Value>
inline void SkipList<Key, Value>::Iterator::SeekToLast() {
    node_ = list_->FindLast();
    if (node_ == list_->head_)
        node_ = nullptr;
}

template<typename Key, typename Value>
inline SkipList<Key, Value>::SkipList() 
    : head_(NewNode(0, 0, kMaxHeight)),
      max_height_(reinterpret_cast<void*>(1)),
      rnd_(0xdeadbeef) {
    for (int i = 0; i < kMaxHeight; i++) {
        head_->SetNext(i, nullptr);
    }
}

template<typename Key, typename Value>
inline SkipList<Key, Value>::~SkipList() {
    Iterator iter(this);
    iter.SeekToFirst();
    while (iter.Vaild()) {
        Node* tmp = iter.node_;
        iter.Next();
        delete tmp;
    }

    delete head_;
}

template<typename Key, typename Value>
inline void SkipList<Key, Value>::Insert(const Key& key,
                                         const Value& value) {
    Node* prev[kMaxHeight];
    // 寻找插入位置.
    Node* x = FindGreaterOrEqual(key, prev);

    // 不允许key值相同
    assert(x == nullptr || !Equal(key, x->key));

    int height = RandomHeight();
    // 如果随机出来的level大于了他插入之前的高，设置比prev大的高为head
    if (height > GetMaxHeight()) {
        for (int i = GetMaxHeight(); i < height; i++) {
            prev[i] = head_;
        }
        max_height_.NoBarrier_Store(reinterpret_cast<void*>(height));
    }

    // 分配新内存
    Node* new_node = NewNode(key, value, height);
    for (int i = 0; i < height; i++) {
        new_node->NoBarrier_SetNext(i, prev[i]->NoBarrier_Next(i));
        prev[i]->SetNext(i, new_node);
    }
}

template<typename Key, typename Value>
inline bool SkipList<Key, Value>::LookUp(const Key & key,
                                         Value * value) {
    Node* x = FindGreaterOrEqual(key, nullptr);

    if (x != nullptr && Equal(key, x->key)) {
        *value = x->value;
        return true;
    } else {
        return false;
    }
}

template<typename Key, typename Value>
inline typename SkipList<Key, Value>::Node* 
SkipList<Key, Value>::NewNode(const Key& key,
                              const Key& value,
                              int height) {
    /*
    char* mem = (char*)malloc(
        sizeof(Node) + sizeof(base::AtomicPointer) * (height - 1));
    return new (mem) Node(key, value);
    */
    // 由于这个跳表是参考了leveldb的跳表，本来准备用malloc + base::AtomicPointer next_[1]
    // 来实现这个Node结构以及内存分配，但是在最后内存释放的时候真的是把我整的一愣一愣的，除非
    // 实现一个像leveldb中的内存管理，leveldb的内存池是统一释放内存的，如果实现一个像leveldb
    // 一样的内存池，那释放内存倒是非常简单，不用管，只用到最后释放内存池的内存即可，由于我们这个
    // 程序完全不需要内存池，对我们来说没有什么作用，故而采用了new + 指针的模式重新实现，使得
    // 释放内存变得简单许多.
    Node* new_node = new Node(key, value, height);
    return new_node;
}

template<typename Key, typename Value>
inline typename SkipList<Key, Value>::Node* 
SkipList<Key, Value>::FindGreaterOrEqual(
    const Key & key, Node ** prev) const {
    Node* x = head_;
    // 重最高层一层像最底层遍历.
    int level = GetMaxHeight() - 1;
    while (true) {
        // 注释，为什么x->Next(level)代表着x链表的下一个元素
        // next_[3]...                      第3层链表
        // next_[2]...                      第2层链表
        // next_[1].next[1].next[1].next[1] 第1层链表
        Node* next = x->Next(level);
        if (KeyIsAfterNode(key, next->key, next)) {
            // 需要往后面接着查找.
            x = next;
        }
        else {
            if (prev != nullptr) prev[level] = x;
            if (level == 0) {
                return next;
            }
            else {
                // 在前面，减少层级，接着查找.
                --level;
            }
        }
    }
}

template<typename Key, typename Value>
inline typename SkipList<Key, Value>::Node*
SkipList<Key, Value>::FindLessThen(const Key & key) const {
    Node* x = head_;
    int level = GetMaxHeight() - 1;
    while (true) {
        assert(x == head_ || (x->key < key));
        Node* next = x->Next(level);
        if (next == nullptr || next->key >= key) {
            if (level == 0) {
                return x;
            }
            else {
                // Switch to next list
                level--;
            }
        }
        else {
            x = next;
        }
    }
}

template<typename Key, typename Value>
inline typename SkipList<Key, Value>::Node* 
SkipList<Key, Value>::FindLast() const {
    Node* x = head_;
    int level = GetMaxHeight() - 1;
    while (true) {
        Node* next = x->Next(level);
        if (next == nullptr) {
            if (level == 0)
                return x;
            else
                level--;

        }
        else {
            x = next;
        }
    }
}

template<typename Key, typename Value>
inline int SkipList<Key, Value>::RandomHeight() {
    static const unsigned int kBranching = 4;
    int height = 1;
    while (height < kMaxHeight && ((rnd_.Next() % kBranching) == 0)) {
        height++;
    }
    assert(height > 0);
    assert(height <= kMaxHeight);
    return height;
}



}   // namespace base.

#endif // !BASE_SKIPLIST_H
