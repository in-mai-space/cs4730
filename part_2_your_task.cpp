#include <chrono>
#include <condition_variable>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "ThreadSleeper.h"

/*******************************************************************************
    PART 2 - 1
*******************************************************************************/
void part_2_1_work_with_threads(
    int count, std::vector<std::shared_ptr<ThreadSleeper>> &sleeperVector) {

  // TODO:
  // - Create threads as many as count.
  // - Each thread should be associated with a ThreadSleeper instance.
  // - ThreadSleeper contains the thread body, or the ThreadBody function.
  //   ThreadBody records the start time, sleeps for 3 seconds, and
  //   records the end time to the ThreadSleeper instance.
  //   Read through ThreadSleeper.h/cpp for details.
  //
  // - 1. You must instantiate the ThreadSleeper instance and add its
  //   pointer to sleeperVector.
  // - 2. Use std::thread to start and run the threads concurrently.
  //   Create threads using the apprach for t5 in the example in
  //   https://en.cppreference.com/w/cpp/thread/thread/thread.html
  //   which passes 1) the pointer to the member function, 2) reference to the
  //   class instance, and 3) parameters to the member function to the
  //   constructor. Pass a unique ID to the thread to store in the
  //   ThreadSleeper instance.
  // - 3. Wait until all created threads terminate using join().
}

/*******************************************************************************
    PART 2 - 2
*******************************************************************************/

#define REQ_WORK 1
#define REQ_QUIT 2

// This function is readily implemented in part_2_main.cpp
// When part_2_2_thread_in_pool receives a REQ_WORK
// it should call this function. This function prints
// the given thread id and works (or sleeps) for 1 second.
void part_2_2_process_work(int id);

// This is the worker thread body in the thread pool.
void part_2_2_thread_in_pool(int id, std::condition_variable &cv,
                             std::mutex &mtx,
                             std::shared_ptr<std::queue<int>> jobQueue) {

  // TODO:
  // You should use the lock to protect the condition
  // variable and shared queue. Once woken up, this
  // thread should check the queue for incoming request,
  // dequeue it, and process it. This should be done
  // in an infinite loop until REQ_QUIT is received.
  // - If the request is REQ_WORK, part_2_2_process_work
  //   function should be called.
  // - If the request is REQ_QUIT, you should exit the
  //   loop and terminate this thread (i.e., let the code
  //   execute up to the end of this function).
}

/*******************************************************************************
    PART 2 - 3
*******************************************************************************/
void part_2_3_promise_and_future_thread(int multiplier, std::future<int> fut,
                                        std::promise<int> prom) {

  // TODO:
  // Wait until you receive a value from the future,
  // Multiply the received value with the multiplier,
  // and send the result over through promise to the
  // future waiting for the value.
}
