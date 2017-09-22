#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

namespace ptio {

using std::condition_variable;
using std::mutex;
using std::queue;
using std::unique_lock;

template <typename T>
class Chan {
    queue<T> que;
    mutable mutex mtx;
    condition_variable cond;

    const size_t quota;
    size_t passed;

public:
    Chan(size_t quota_) : quota(quota_), passed(0) {}

    inline void Push(const T &v) {
        {
            unique_lock<mutex> lock{mtx};
            que.push(v);
        }
        cond.notify_one();
    }

    inline bool Pop(T &v) {
        unique_lock<mutex> lock{mtx};
        if (passed == quota)
            return false;

        while (que.empty() && passed < quota)
            cond.wait(lock);

        if (que.empty())
            return false;

        v = que.front();
        que.pop();
        ++passed;

        if (passed == quota)
            cond.notify_all();
        return true;
    }
};

}
