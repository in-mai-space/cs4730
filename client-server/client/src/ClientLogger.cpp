#include "../include/ClientLogger.h"

#include <algorithm>
#include <chrono>
#include <iostream>

ClientLogger::ClientLogger(std::vector<long long>& latencies)
    : latencies(latencies) {}

void ClientLogger::log_performance_statistics(
    std::chrono::high_resolution_clock::time_point start_time,
    std::chrono::high_resolution_clock::time_point end_time) {
    if (latencies.empty()) {
        std::cout << "0\t0\t0\t0" << std::endl;
        return;
    }

    long long sum = 0;
    long long min_latency = latencies[0];
    long long max_latency = latencies[0];

    for (long long latency : latencies) {
        sum += latency;
        min_latency = std::min(min_latency, latency);
        max_latency = std::max(max_latency, latency);
    }

    double avg_latency = calculate_average_latency(sum, latencies.size());
    auto total_time_ms = calculate_total_time_in_ms(start_time, end_time);
    double total_time_seconds = convert_ms_to_seconds(total_time_ms);
    double throughput =
        calculate_throughput(latencies.size(), total_time_seconds);

    std::cout << avg_latency << "\t" << min_latency << "\t" << max_latency
              << "\t" << throughput << std::endl;
}

double ClientLogger::calculate_average_latency(long long sum, int size) {
    return static_cast<double>(sum) / size;
}

long long ClientLogger::calculate_total_time_in_ms(
    std::chrono::high_resolution_clock::time_point start_time,
    std::chrono::high_resolution_clock::time_point end_time) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(end_time -
                                                                 start_time)
        .count();
}

double ClientLogger::convert_ms_to_seconds(long long total_time_ms) {
    return static_cast<double>(total_time_ms) / 1000.0;
}

double ClientLogger::calculate_throughput(int total_orders,
                                          double total_time_seconds) {
    if (total_time_seconds <= 0.0) {
        return 0.0;
    }
    return static_cast<double>(total_orders) / total_time_seconds;
}
