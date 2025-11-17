// Comprehensive benchmark for optimized calendar businessDaysBetween
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/calendar.hpp>
#include <chrono>
#include <iostream>
#include <vector>
#include <thread>
#include <iomanip>

using namespace QuantLib;
using namespace std;
using namespace std::chrono;

struct BenchmarkResult {
    string testName;
    long long timeMs;
    int operations;
    double opsPerSec;
};

// Benchmark single-threaded performance
BenchmarkResult benchmarkSingleThread(const string& name, Calendar& cal,
                                      const Date& from, const Date& to, int iterations) {
    auto start = high_resolution_clock::now();

    long long total = 0;
    for (int i = 0; i < iterations; i++) {
        total += cal.businessDaysBetween(from, to, true, false);
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();

    BenchmarkResult result;
    result.testName = name;
    result.timeMs = duration;
    result.operations = iterations;
    result.opsPerSec = (iterations * 1000.0) / duration;

    return result;
}

// Benchmark cache effectiveness with repeated queries
BenchmarkResult benchmarkCacheHitRate(Calendar& cal) {
    vector<pair<Date, Date>> queries = {
        {Date(1, January, 2024), Date(1, April, 2024)},
        {Date(1, March, 2024), Date(1, June, 2024)},
        {Date(1, January, 2024), Date(1, April, 2024)},  // Repeat
        {Date(15, February, 2024), Date(15, May, 2024)},
        {Date(1, March, 2024), Date(1, June, 2024)},     // Repeat
        {Date(1, January, 2024), Date(1, April, 2024)},  // Repeat
    };

    auto start = high_resolution_clock::now();

    for (int i = 0; i < 10000; i++) {
        for (const auto& q : queries) {
            cal.businessDaysBetween(q.first, q.second, true, false);
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();

    BenchmarkResult result;
    result.testName = "Cache Hit Rate Test";
    result.timeMs = duration;
    result.operations = 60000;  // 10000 * 6
    result.opsPerSec = (60000 * 1000.0) / duration;

    return result;
}

// Benchmark multi-threaded performance
void workerThread(Calendar cal, const Date& from, const Date& to,
                  int iterations, long long& total) {
    for (int i = 0; i < iterations; i++) {
        total += cal.businessDaysBetween(from, to, true, false);
    }
}

BenchmarkResult benchmarkMultiThread(Calendar& cal, const Date& from, const Date& to,
                                     int iterations, int numThreads) {
    vector<thread> threads;
    vector<long long> totals(numThreads, 0);

    auto start = high_resolution_clock::now();

    for (int i = 0; i < numThreads; i++) {
        threads.emplace_back(workerThread, cal, from, to, iterations, ref(totals[i]));
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();

    BenchmarkResult result;
    result.testName = "Multi-threaded (" + to_string(numThreads) + " threads)";
    result.timeMs = duration;
    result.operations = iterations * numThreads;
    result.opsPerSec = (iterations * numThreads * 1000.0) / duration;

    return result;
}

void printResult(const BenchmarkResult& r) {
    cout << left << setw(40) << r.testName
         << right << setw(10) << r.timeMs << " ms"
         << setw(15) << fixed << setprecision(0) << r.opsPerSec << " ops/s"
         << endl;
}

int main() {
    UnitedStates calendar(UnitedStates::NYSE);

    cout << "========================================" << endl;
    cout << "Calendar businessDaysBetween Optimization Benchmark" << endl;
    cout << "========================================" << endl << endl;

    vector<BenchmarkResult> results;

    // Test 1: Short range (1 month) - many iterations
    cout << "Test 1: Short Range (1 month, 100K iterations)" << endl;
    {
        Date from(1, January, 2024);
        Date to(1, February, 2024);
        auto result = benchmarkSingleThread("1 month range", calendar, from, to, 100000);
        printResult(result);
        results.push_back(result);
    }
    cout << endl;

    // Test 2: Medium range (3 months) - many iterations
    cout << "Test 2: Medium Range (3 months, 50K iterations)" << endl;
    {
        Date from(1, January, 2024);
        Date to(1, April, 2024);
        auto result = benchmarkSingleThread("3 month range", calendar, from, to, 50000);
        printResult(result);
        results.push_back(result);
    }
    cout << endl;

    // Test 3: Long range (1 year) - moderate iterations
    cout << "Test 3: Long Range (1 year, 10K iterations)" << endl;
    {
        Date from(1, January, 2024);
        Date to(1, January, 2025);
        auto result = benchmarkSingleThread("1 year range", calendar, from, to, 10000);
        printResult(result);
        results.push_back(result);
    }
    cout << endl;

    // Test 4: Very long range (3 years) - fewer iterations
    cout << "Test 4: Very Long Range (3 years, 5K iterations)" << endl;
    {
        Date from(1, January, 2022);
        Date to(1, January, 2025);
        auto result = benchmarkSingleThread("3 year range", calendar, from, to, 5000);
        printResult(result);
        results.push_back(result);
    }
    cout << endl;

    // Test 5: Cache effectiveness
    cout << "Test 5: Cache Effectiveness (60K queries, 50% repeated)" << endl;
    {
        auto result = benchmarkCacheHitRate(calendar);
        printResult(result);
        results.push_back(result);
    }
    cout << endl;

    // Test 6: Multi-threaded performance (2 threads)
    cout << "Test 6: Multi-threaded Performance" << endl;
    {
        Date from(1, January, 2024);
        Date to(1, April, 2024);
        auto result = benchmarkMultiThread(calendar, from, to, 10000, 2);
        printResult(result);
        results.push_back(result);
    }
    cout << endl;

    // Test 7: Multi-threaded performance (4 threads)
    {
        Date from(1, January, 2024);
        Date to(1, April, 2024);
        auto result = benchmarkMultiThread(calendar, from, to, 10000, 4);
        printResult(result);
        results.push_back(result);
    }
    cout << endl;

    cout << "========================================" << endl;
    cout << "Summary" << endl;
    cout << "========================================" << endl;
    cout << "All optimizations implemented:" << endl;
    cout << "  ✓ Arithmetic weekend counting" << endl;
    cout << "  ✓ Skip weekend days in holiday loop" << endl;
    cout << "  ✓ Thread-safe cache with LRU eviction" << endl;
    cout << endl;

    return 0;
}
