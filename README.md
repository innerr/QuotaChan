# QuotaChan

## Concurrent blocking queue in c++, AKA, a `chan` in Golang
* Easy to use
* Rich features
* Fast
* Clean implement
* Fully tested in large dataset (inside project)

## Easy to use
* Include `chan.h`
* In your code:
    * Create the queue:
        * `Chan<int> jobs(quota)`
    * In one thread:
        * `jobs.Push(666)`
    * In another thread:
        * `int val = 0;`
        * `while (jobs.Pop(job) { /* do anything with val */ }`

## Docs
* Class: `Chan`
    * `Chan(size_t quota = 0, size_t capacity = 0)`
        * Constructor
        * Arg `quota`: `chan.Pop(..)` will return `false`, after it return `true` in `quota` times.
            * This let we easy to known when all jobs are done.
        * Arg `capacity`: `chan.Push(..)` will block (and wait) when there are `capacity` count elements in the chan.
            * This let we easy to control the memory usage.
    * `Push(const T &value)`
        * Job input.
        * Block (and wait untill some one called `Pop(..)`) when `capacity == elements in chan`
        * `T::operator = ()` is used.
        * Thread safe
    * `bool Pop(T &value)`
        * Job output.
        * Return: `true` means succeeded, `false` means closed.
            * Return `false` after return `true` in `quota` times.
        * When it's not closed:
            * Block (and wait untill some one called `Push(..)`) when `capacity == elements in chan`
        * `T::operator = ()` is used.
        * Thread safe

## Benchmark
* Compare to [those implements](http://moodycamel.com/blog/2014/a-fast-general-purpose-lock-free-queue-for-c++#benchmarks), Our implement reach the same speed as boost::lockfree::queue, in from 1 to 128 threads.
