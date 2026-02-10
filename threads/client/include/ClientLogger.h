#ifndef CLIENTLOGGER_H
#define CLIENTLOGGER_H
#include <chrono>
#include <vector>

class ClientLogger {
   public:
    ClientLogger(std::vector<long long>& latencies);
    /**
     * Logs performance statistics including average, min, max latencies and
     * throughput.
     * @param start_time The start time of the measurement period.
     * @param end_time The end time of the measurement period.
     */
    void log_performance_statistics(
        std::chrono::high_resolution_clock::time_point start_time,
        std::chrono::high_resolution_clock::time_point end_time);

   private:
    std::vector<long long>& latencies;

    /**
     * Calculates the average latency.
     * @param sum The sum of all latencies.
     * @param size The number of latency measurements.
     */
    double calculate_average_latency(long long sum, int size);

    /**
     * Calculates the total time in milliseconds.
     * @param start_time The start time of the measurement period.
     * @param end_time The end time of the measurement period.
     */
    long long calculate_total_time_in_ms(
        std::chrono::high_resolution_clock::time_point start_time,
        std::chrono::high_resolution_clock::time_point end_time);

    /**
     * Converts milliseconds to seconds.
     * @param total_time_ms The total time in milliseconds.
     */
    double convert_ms_to_seconds(long long total_time_ms);

    /**
     * Calculates throughput in orders per second.
     * @param total_orders The total number of orders processed.
     * @param total_time_seconds The total time in seconds.
     */
    double calculate_throughput(int total_orders, double total_time_seconds);
};

#endif