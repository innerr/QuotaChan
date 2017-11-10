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

    condition_variable cond_r;
    condition_variable cond_w;

    const size_t quota;
    const size_t capacity;
    size_t passed;
    bool closed;

public:
    inline Chan(size_t quota_ = 0, size_t capacity_ = 0) : quota(quota_), capacity(capacity_), passed(0), closed(false) {}

    inline bool Empty() {
        unique_lock<mutex> lock{mtx};
        return que.empty();
    }

    inline void Close() {
        {
            unique_lock<mutex> lock{mtx};
            closed = true;
        }
        cond_w.notify_all();
        cond_r.notify_all();
    }

    inline void Push(const T &v) {
        {
            unique_lock<mutex> lock{mtx};
            if (capacity != 0 && capacity <= que.size() && (quota == 0 || passed < quota) && !closed)
                cond_w.wait(lock);
            if (closed)
                return;
            que.push(v);
        }
        cond_r.notify_one();
    }

    inline bool Pop(T &v) {
        unique_lock<mutex> lock{mtx};
        if ((quota != 0 && passed == quota) || closed)
            return false;

        while (que.empty() && (quota == 0 || passed < quota) && !closed)
            cond_r.wait(lock);

        if (que.empty() || closed)
            return false;

        v = que.front();
        que.pop();
        ++passed;

        cond_w.notify_all();

        if (quota != 0 && passed == quota)
            cond_r.notify_all();
        return true;
    }
};

}
