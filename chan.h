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

  inline friend void operator<<(Chan &chan, T &v) {
    unique_lock<mutex> lock(chan.mtx);
    // blocks until sth is removed since queue is full now.
    while (chan.que.size() == chan.quota) {
      chan.w_cond.wait(lock);
    }

    chan.que.push(v);

    // singal waiting reader
    chan.r_cond.notify_all();
    // return chan;
  }

  // semantic recv in golang's channel
  inline friend bool operator>>(Chan &chan, T &v) {
    unique_lock<mutex> lock(chan.mtx);
    if (chan.passed == chan.quota)
      return false;
    while (chan.que.empty() && chan.passed < chan.quota) {
      // wait until there is something to pop
      chan.r_cond.wait(lock);
    }

    // remove first element in the queue
    v = chan.que.front();
    chan.que.pop();
    ++chan.passed;

    chan.w_cond.notify_all();

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
