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

public:
    inline Chan(size_t quota_, size_t capacity_) : quota(quota_), capacity(capacity_), passed(0) {}

    inline void Push(const T &v) {
        {
            unique_lock<mutex> lock{mtx};
            if (capacity <= que.size() && passed < quota)
                cond_w.wait(lock);
            que.push(v);
        }
        cond_r.notify_one();
    }

    inline bool Pop(T &v) {
        unique_lock<mutex> lock{mtx};
        if (passed == quota)
            return false;

        while (que.empty() && passed < quota)
            cond_r.wait(lock);

        if (que.empty())
            return false;

        v = que.front();
        que.pop();
        ++passed;

        cond_w.notify_all();

        if (passed == quota)
            cond_r.notify_all();
        return true;
    }
};

}
