#ifndef __THREAD1_H__
#define __THREAD1_H__

#include <chrono>
#include <iostream>
#include <ctime>

class ThreadSleeper {
    int threadID;
    std::chrono::time_point<std::chrono::system_clock> startTime;
    std::chrono::time_point<std::chrono::system_clock> endTime;

public: 
    void ThreadBody(int id);
    int GetThreadID();
    std::chrono::time_point<std::chrono::system_clock> GetStartTime();
    std::chrono::time_point<std::chrono::system_clock> GetEndTime();
};

#endif // end of #ifndef __THREAD1_H__
