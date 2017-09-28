#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

namespace ptio {

using std::condition_variable;
using std::mutex;
using std::queue;
using std::unique_lock;

template <typename T> class Chan {
  // buffered queue properties
  queue<T> que;
  mutable mutex mtx;
  condition_variable r_cond;
  condition_variable w_cond;

  const size_t quota;
  size_t passed;

public:
  Chan(size_t quota_) : quota(quota_), passed(0) {}

  // semantic send in golang's channel
  inline void Push(const T &v) {
    unique_lock<mutex> lock(mtx);
    // blocks until sth is removed since queue is full now.
    while (que.size() == quota) {
      w_cond.wait(lock);
    }

    que.push(v);

    // singal waiting reader
    r_cond.notify_all();
  }

  // semantic recv in golang's channel
  inline bool Pop(T &v) {
    unique_lock<mutex> lock(mtx);
    if (passed == quota)
      return false;
    while (que.empty() && passed < quota) {
      // wait until there is something to pop
      r_cond.wait(lock);
    }

    // remove first element in the queue
    v = que.front();
    que.pop();
    ++passed;

    w_cond.notify_all();
    return true;
  }

  inline int size() {
    unique_lock<mutex> lock(mtx);
    return que.size();
  }

  inline int remaining() {
    unique_lock<mutex> lock(mtx);
    return quota - passed;
  }
};
} // namespace ptio
