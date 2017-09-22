#include <iostream>
#include <vector>
#include <thread>

#include "chan.h"

using namespace std;
using namespace ptio;

int main() {
    size_t quota = 10000;
    size_t concurrent = 3;

    Chan<int> jobs(quota);
    for (int i = 0; i < quota; ++i)
        jobs.Push(i);

    Chan<int> loadeds(quota);

    vector<thread> loaders(concurrent);
    for (auto it = loaders.begin(); it != loaders.end(); ++it) {
        *it = thread([&] {
            int job = -1;
            while (jobs.Pop(job)) {
                cout << "loading job: #" << job << endl;
                loadeds.Push(job);
            }
        });
    }

    vector<thread> workers(concurrent);
    for (auto it = workers.begin(); it != workers.end(); ++it) {
        *it = thread([&] {
            int job = -1;
            while (loadeds.Pop(job))
                cout << "doing job: #" << job << endl;
        });
    }

    for (auto it = loaders.begin(); it != loaders.end(); ++it)
        it->join();
    for (auto it = workers.begin(); it != workers.end(); ++it)
        it->join();

    return 0;
}
