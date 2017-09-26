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

  int r_waiting;
  int w_waiting;
  const size_t capacity;
  size_t passed;

public:
  Chan(size_t quota_)
      : capacity(quota_), passed(0), r_waiting(0), w_waiting(0) {}

  // semantic send in golang's channel
  inline void Push(const T &v) {
    unique_lock<mutex> lock(mtx);
    // blocks until sth is removed since queue is full now.
    while (que.size() == capacity) {
      w_waiting++;
      w_cond.wait(lock);
      w_waiting--;
    }

    que.push(v);

    if (r_waiting > 0) {
      // singal waiting reader
      r_cond.notify_all();
    }
  }

  // semantic recv in golang's channel
  inline bool Pop(T &v) {
    unique_lock<mutex> lock(mtx);
    if (passed == capacity)
      return false;
    while (que.empty() && passed < capacity) {
      // wait until there is something to pop
      r_waiting++;
      r_cond.wait(lock);
      r_waiting--;
    }

    // remove first element in the queue
    v = que.front();
    que.pop();
    ++passed;

    if (w_waiting > 0) {
      w_cond.notify_all();
    }
    return true;
  }

  inline int size() {
    unique_lock<mutex> lock(mtx);
    return que.size();
  }

  inline int remaining() {
    unique_lock<mutex> lock(mtx);
    return capacity - passed;
  }
};
} // namespace ptio
