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

// Prototypes of what you will write in part_2_your_task.cpp
void part_2_1_work_with_threads(
    int count, std::vector<std::shared_ptr<ThreadSleeper>> &sleeperVector);
void part_2_2_thread_in_pool(int id, std::condition_variable &cv,
                             std::mutex &mtx,
                             std::shared_ptr<std::queue<int>> jobQueue);
void part_2_3_promise_and_future_thread(int multiplier, std::future<int> fut,
                                        std::promise<int> prom);

/*******************************************************************************
    PART 2 - 1
*******************************************************************************/
void part_2_1_multi_thread(int count) {

  // Record when this function started
  auto globalStartTime = std::chrono::system_clock::now();
  // Vector data structure that points to ThreadSleeper instances.
  std::vector<std::shared_ptr<ThreadSleeper>> sleeperVector;

  // TODO: The function you should write in part_2_your_task.cpp
  part_2_1_work_with_threads(count, sleeperVector);

  // Prints output after your function finishes executing.
  std::cout << "\nWhen the implementation is complete, there will be outputs "
               "below.\n";
  std::cout << "Start time and end time of " << count << " threads. \n";
  std::cout << "All threads should execute concurrently and \n";
  std::cout << "start and end roughly at 0 and 3 second marks.\n" << std::endl;

  // Once you properly fill in sleeperVector, this for loop will
  // print execution information for each thread.
  for (auto sleeper : sleeperVector) {
    // Elapsed time between global start and start of a thread.
    // Should be close to 0 ms.
    std::chrono::duration<double, std::milli> start =
        sleeper->GetStartTime() - globalStartTime;

    // Elapsed time between global start and end of a thread.
    // Should be close to 3000 ms.
    std::chrono::duration<double, std::milli> end =
        sleeper->GetEndTime() - globalStartTime;

    // Print time info in a second scale.
    std::cout << "Thread [" << sleeper->GetThreadID() << "]\t";
    std::cout << "Start\t" << start.count() / 1000.0 << " s\t";
    std::cout << "End\t" << end.count() / 1000.0 << " s\t" << std::endl;
  }
}

/*******************************************************************************
    PART 2 - 2
*******************************************************************************/

#define POOL_SIZE 4
#define WORK_AMOUNT 12

// Commands to worker threads in the pool
#define REQ_WORK 1
#define REQ_QUIT 2

// This function should be called in the part_2_2_thread_in_pool
// when REQ_WORK is received.
void part_2_2_process_work(int id) {
  std::cout << "\tid " << id << " working" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(1));
}

void part_2_2_locks_and_thread_pool() {
  std::mutex mtx;
  std::condition_variable cv;
  std::shared_ptr<std::queue<int>> jobQueue(new std::queue<int>());

  // Variables for time keeping
  std::chrono::time_point<std::chrono::system_clock> startTime;
  std::chrono::time_point<std::chrono::system_clock> endTime;

  // Creating a pool of 4 threads
  std::thread t1(&part_2_2_thread_in_pool, 1, std::ref(cv), std::ref(mtx),
                 jobQueue);
  std::thread t2(&part_2_2_thread_in_pool, 2, std::ref(cv), std::ref(mtx),
                 jobQueue);
  std::thread t3(&part_2_2_thread_in_pool, 3, std::ref(cv), std::ref(mtx),
                 jobQueue);
  std::thread t4(&part_2_2_thread_in_pool, 4, std::ref(cv), std::ref(mtx),
                 jobQueue);

  std::cout << "Sleeping for 1 second for threads to run\n"
            << "(to do it properly, one should use a barrier).\n"
            << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // Start measuring time
  startTime = std::chrono::system_clock::now();

  // Enqueue multiple requests for the thread pool to process
  // When a request is enqueued one of the threads in the
  // pool is notified to dequeue and process it.
  for (int i = 0; i < WORK_AMOUNT; i++) {
    std::unique_lock<std::mutex> lk(mtx);
    jobQueue->push(REQ_WORK);
    cv.notify_one();
  }

  // Enqueue quit commands as many as the threads in the pool.
  // Thread receiving this request should terminate.
  for (int i = 0; i < POOL_SIZE; i++) {
    std::unique_lock<std::mutex> lk(mtx);
    jobQueue->push(REQ_QUIT);
    cv.notify_one();
  }

  // Wait for threads to terminate
  t1.join();
  t2.join();
  t3.join();
  t4.join();

  // Measure the end time
  endTime = std::chrono::system_clock::now();

  // Report the duration:
  // The elapsed time should be roughly RoundUp(WORK_AMOUNT / POOL_SIZE).
  std::chrono::duration<double, std::milli> elapsedTime = endTime - startTime;
  std::cout << "\nWhen the implementation is complete, there will be outputs "
               "above.\n";
  std::cout << "The lines printed above could be interleaved\n";
  std::cout << "and it is completely normal which is caused by\n";
  std::cout << "concurrent executions of threads.\n";
  std::cout << "\nTime elapsed to process all work and quit\n";
  std::cout << "should be roughly 3 seconds.\n" << std::endl;
  std::cout << "[Elapsed time: " << elapsedTime.count() / 1000.0;
  std::cout << " seconds]" << std::endl;
}

/*******************************************************************************
    PART 2 - 3
*******************************************************************************/

void part_2_3_promise_and_future() {
  int product;
  int multiplier1 = 4;
  int multiplier2 = 8;

  // Initializes promise and future pairs
  std::promise<int> prom1;
  std::future<int> fut1 = prom1.get_future();

  std::promise<int> prom2;
  std::future<int> fut2 = prom2.get_future();

  std::promise<int> prom3;
  std::future<int> fut3 = prom3.get_future();

  // Create two threads with different multiplier values.
  // The thread should simply multiply the multiplier
  // argument with the received value through future
  // and send the product through promise.
  // Carefully look at how different future and promise
  // variables are given to different threads.
  std::thread t1(&part_2_3_promise_and_future_thread, multiplier1,
                 std::move(fut1), std::move(prom2));
  std::thread t2(&part_2_3_promise_and_future_thread, multiplier2,
                 std::move(fut2), std::move(prom3));

  // Sleep this thread for 1 second
  std::cout << "Sleeping for 1 second." << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // Send value to t1 which should be waiting for
  // value from prom1.
  prom1.set_value(2);

  try {

    // Wait for prom3 to send value and receive the value.
    product = fut3.get();

    // Print the final product which should be 2 * 4 * 8 = 64.
    std::cout << "\n[Final product (2 x 4 x 8 = 64): " << product;
    std::cout << " ]\n" << std::endl;

    // Exception handling
  } catch (std::exception &e) {
    std::cout << "Exception: " << e.what() << std::endl;
    std::cout << "Properly implement part_2_3_promise_and_future_thread\n";
    std::cout << "then this exception will not occur." << std::endl;
  }

  t1.join();
  t2.join();
}

/*******************************************************************************
    Main function
*******************************************************************************/

int main() {

  std::cout << "\n== Part 2-1 multi-threading ==\n" << std::endl;
  part_2_1_multi_thread(3);

  std::cout << "\n== Part 2-2 thread pool ==\n" << std::endl;

  part_2_2_locks_and_thread_pool();

  std::cout << "\n== Part 2-3 promise and future ==\n" << std::endl;

  part_2_3_promise_and_future();

  return 0;
}
