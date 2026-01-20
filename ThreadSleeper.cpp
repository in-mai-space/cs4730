#include "ThreadSleeper.h"

#include <thread>

int ThreadSleeper::GetThreadID() {
    return threadID;
}

std::chrono::time_point<std::chrono::system_clock> ThreadSleeper::GetStartTime() {
    return startTime;
}

std::chrono::time_point<std::chrono::system_clock> ThreadSleeper::GetEndTime() {
    return endTime;
}

void ThreadSleeper::ThreadBody(int id) {

    threadID = id;

    startTime = std::chrono::system_clock::now();

    std::this_thread::sleep_for(std::chrono::seconds(3));

    endTime = std::chrono::system_clock::now();
}
