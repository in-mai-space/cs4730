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
  std::vector<std::thread> threads;

  // create as many threads as count
  for (int i = 0; i < count; i++) {
    // instantiate ThreadSleeper instance
    std::shared_ptr<ThreadSleeper> sleeper = std::make_shared<ThreadSleeper>();
    // add its pointer to sleepVector
    sleeperVector.push_back(sleeper);
    // create thread and pass member function pointer, class instance, parameter
    // for func
    std::thread t(&ThreadSleeper::ThreadBody, sleeper, i);
    // move threads into thread vector
    threads.push_back(std::move(t));
  }

  // wait until all created threads terminate
  for (auto &th : threads) {
    if (th.joinable()) {
      th.join();
    }
  }
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
  while (true) {
    // lock the mutex protecting the job queue
    std::unique_lock<std::mutex> lock(mtx);

    // wait until the job queue is not empty (wait unlocks and re-locks the
    // mutex)
    while (jobQueue->empty()) {
      cv.wait(lock);
    }
    // dequeue job
    auto job = jobQueue->front();
    jobQueue->pop();

    // if REQ_QUIT, exit loop adn terminate
    if (job == REQ_QUIT) {
      break;
    }

    // if REQ_WORK, release on lock on jobQueue and execute job
    if (job == REQ_WORK) {
      lock.unlock();
      part_2_2_process_work(id);
    }
  }
}

/*******************************************************************************
    PART 2 - 3
*******************************************************************************/
void part_2_3_promise_and_future_thread(int multiplier, std::future<int> fut,
                                        std::promise<int> prom) {
  // wait to receive value from future
  auto value = fut.get();
  // multiply value by multiplier and send back to future
  prom.set_value(value * multiplier);
}
