// Benchmark for CumulativeNormalDistribution optimization
#include <ql/math/distributions/normaldistribution.hpp>
#include <chrono>
#include <iostream>
#include <vector>

using namespace QuantLib;
using namespace std;
using namespace std::chrono;

// Baseline: Create CumulativeNormalDistribution on every call
double blackFormula_baseline(double d1, double d2, int iterations) {
    double sum = 0.0;
    for (int i = 0; i < iterations; i++) {
        CumulativeNormalDistribution phi;  // Created every time
        double nd1 = phi(d1 + i * 0.0001);
        double nd2 = phi(d2 + i * 0.0001);
        sum += nd1 + nd2;
    }
    return sum;
}

// Optimized: Use static CumulativeNormalDistribution
double blackFormula_optimized(double d1, double d2, int iterations) {
    static const CumulativeNormalDistribution phi;  // Created only once
    double sum = 0.0;
    for (int i = 0; i < iterations; i++) {
        double nd1 = phi(d1 + i * 0.0001);
        double nd2 = phi(d2 + i * 0.0001);
        sum += nd1 + nd2;
    }
    return sum;
}

int main() {
    const int iterations = 10000000;  // 10 million iterations
    const double d1 = 0.5;
    const double d2 = 0.3;

    cout << "=== CumulativeNormalDistribution Benchmark ===" << endl;
    cout << "Iterations: " << iterations << endl << endl;

    // Warmup
    blackFormula_baseline(d1, d2, 1000);
    blackFormula_optimized(d1, d2, 1000);

    // Baseline
    auto start = high_resolution_clock::now();
    double result1 = blackFormula_baseline(d1, d2, iterations);
    auto end = high_resolution_clock::now();
    auto baseline_time = duration_cast<milliseconds>(end - start).count();

    // Optimized
    start = high_resolution_clock::now();
    double result2 = blackFormula_optimized(d1, d2, iterations);
    end = high_resolution_clock::now();
    auto optimized_time = duration_cast<milliseconds>(end - start).count();

    cout << "Baseline (create object every time):  " << baseline_time << " ms" << endl;
    cout << "Optimized (static object):             " << optimized_time << " ms" << endl;
    cout << "Speedup:                                " << (double)baseline_time / optimized_time << "x" << endl;
    cout << "Results match: " << (abs(result1 - result2) < 0.0001 ? "YES" : "NO") << endl;

    return 0;
}
