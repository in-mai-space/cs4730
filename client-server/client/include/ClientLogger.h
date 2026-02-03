#ifndef CLIENTLOGGER_H
#define CLIENTLOGGER_H
#include <vector>
#include <chrono>

class ClientLogger {
    public:
        ClientLogger(std::vector<long long>& latencies);
        void log_performance_statistics(std::chrono::high_resolution_clock::time_point start_time, std::chrono::high_resolution_clock::time_point end_time);

    private:
        std::vector<long long>& latencies;

        double calculate_average_latency(long long sum, int size);
        long long calculate_total_time_in_ms(std::chrono::high_resolution_clock::time_point start_time, std::chrono::high_resolution_clock::time_point end_time);
        double convert_ms_to_seconds(long long total_time_ms);
        double calculate_throughput(int total_orders, double total_time_seconds);
};

#endif