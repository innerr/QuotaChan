#include "chan_test.h"

using namespace ptio_test;

int main() {
  chan_push_test();
  chan_pop_test();
  chan_push_and_pop_test_3();
  chan_push_and_pop_test_100();
}
