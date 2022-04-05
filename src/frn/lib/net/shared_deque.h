#ifndef _FRN_LIB_NET_SHAREDDEQUE_H
#define _FRN_LIB_NET_SHAREDDEQUE_H

#include <condition_variable>
#include <mutex>
#include <queue>

namespace frn::lib {
namespace net {

/**
 * @brief A simple thread safe double-ended queue.
 *
 * Based on https://codereview.stackexchange.com/q/238347
 */
template <typename T, typename Allocator = std::allocator<T>>
class SharedDeque {
 public:
  /**
   * @brief Remove the top element from the queue.
   */
  void PopFront();

  /**
   * @brief Read the top element from the queue.
   */
  T &Front();

  /**
   * @brief Insert an item to the back of the queue.
   */
  void PushBack(const T &item);

  /**
   * @brief Move an item to the back of the queue.
   */
  void PushBack(T &&item);

  /**
   * @brief Number of elements currently in the queue.
   */
  std::size_t Size();

 private:
  /**
   * @brief Underlying STL deque.
   */
  std::deque<T, Allocator> mDeck;

  /**
   * @brief mutex.
   */
  std::mutex mMutex;

  /**
   * @brief condition variable.
   */
  std::condition_variable mCond;
};

template <typename T, typename Allocator>
void SharedDeque<T, Allocator>::PopFront() {
  std::unique_lock<std::mutex> lock(mMutex);
  while (mDeck.empty()) {
    mCond.wait(lock);
  }
  mDeck.pop_front();
}

template <typename T, typename Allocator>
T &SharedDeque<T, Allocator>::Front() {
  std::unique_lock<std::mutex> lock(mMutex);
  while (mDeck.empty()) {
    mCond.wait(lock);
  }
  return mDeck.front();
}

template <typename T, typename Allocator>
void SharedDeque<T, Allocator>::PushBack(const T &item) {
  std::unique_lock<std::mutex> lock(mMutex);
  mDeck.push_back(item);
  lock.unlock();
  mCond.notify_one();
}

template <typename T, typename Allocator>
void SharedDeque<T, Allocator>::PushBack(T &&item) {
  std::unique_lock<std::mutex> lock(mMutex);
  mDeck.push_back(std::move(item));
  lock.unlock();
  mCond.notify_one();
}

template <typename T, typename Allocator>
std::size_t SharedDeque<T, Allocator>::Size() {
  std::unique_lock<std::mutex> lock(mMutex);
  auto size = mDeck.size();
  lock.unlock();
  return size;
}

}  // namespace net
}  // namespace frn::lib

#endif  // _FRN_LIB_NET_SHAREDDEQUE_H
