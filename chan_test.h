#include "chan.h"
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>
namespace ptio_test {

using namespace std;
using namespace ptio;
using std::atomic;
void chan_push_test() {
  size_t quota = 10000;

  Chan<int> jobs(quota);
  for (int i = 0; i < quota; ++i)
    jobs << i;
  assert(jobs.size() == quota);
  std::cout << "push test passed \n";
}

void chan_pop_test() {
  size_t quota = 10000;

  Chan<int> jobs(quota);
  for (int i = 0; i < quota; ++i)
    jobs << i;
  for (int i = 0; i < quota; ++i) {
    int job = -1;
    jobs >> i;
  }

  assert(jobs.remaining() == 0);
  std::cout << "pop test passed \n";
}

void chan_push_and_pop(int quota, int concurrent) {
  Chan<int> jobs(quota);
  for (int i = 0; i < quota; ++i)
    jobs << i;

  Chan<int> loadeds(quota);

  vector<thread> loaders(concurrent);
  for (auto it = loaders.begin(); it != loaders.end(); ++it) {
    *it = thread([&] {
      int job = -1;
      while (jobs >> job) {
        jobs << job;
      }
    });
  }

  vector<thread> workers(concurrent);
  atomic<int> counter;
  counter = 0;
  for (auto it = workers.begin(); it != workers.end(); ++it) {
    *it = thread([&] {
      int job = -1;
      while (loadeds >> job)
        ++counter;
    });
  }

  for (auto it = loaders.begin(); it != loaders.end(); ++it)
    it->join();
  for (auto it = workers.begin(); it != workers.end(); ++it)
    it->join();

  assert(counter == quota);
  std::cout << "push and pop test passed \n";
}
void chan_push_and_pop_test_3() { chan_push_and_pop(10000, 3); };

void chan_push_and_pop_test_100() { chan_push_and_pop(10000, 100); };
} // namespace ptio_test
