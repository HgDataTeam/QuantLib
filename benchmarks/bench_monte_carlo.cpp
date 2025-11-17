// Benchmark for Monte Carlo branch elimination optimization
#include <chrono>
#include <iostream>
#include <vector>
#include <random>

using namespace std;
using namespace std::chrono;

// Baseline: Checks boolean flags on every iteration
double monteCarlo_baseline(size_t samples, bool isControlVariate, bool isAntitheticVariate) {
    mt19937 gen(42);
    normal_distribution<double> dist(0.0, 1.0);

    double sum = 0.0;
    double controlSum = 0.0;

    for (size_t j = 0; j < samples; j++) {
        double sample = dist(gen);
        sum += sample * sample;

        // These branches are checked on EVERY iteration
        if (isControlVariate) {
            double control = sample * 0.5;
            controlSum += control;
        }

        if (isAntitheticVariate) {
            double antiSample = -sample;
            sum += antiSample * antiSample;
        }
    }

    return sum + controlSum;
}

// Optimized: Branch elimination by templating or separate loops
double monteCarlo_optimized_none(size_t samples) {
    mt19937 gen(42);
    normal_distribution<double> dist(0.0, 1.0);

    double sum = 0.0;

    for (size_t j = 0; j < samples; j++) {
        double sample = dist(gen);
        sum += sample * sample;
    }

    return sum;
}

double monteCarlo_optimized_antithetic(size_t samples) {
    mt19937 gen(42);
    normal_distribution<double> dist(0.0, 1.0);

    double sum = 0.0;

    for (size_t j = 0; j < samples; j++) {
        double sample = dist(gen);
        sum += sample * sample;
        // Antithetic path - no branch needed
        double antiSample = -sample;
        sum += antiSample * antiSample;
    }

    return sum;
}

double monteCarlo_optimized_control(size_t samples) {
    mt19937 gen(42);
    normal_distribution<double> dist(0.0, 1.0);

    double sum = 0.0;
    double controlSum = 0.0;

    for (size_t j = 0; j < samples; j++) {
        double sample = dist(gen);
        sum += sample * sample;
        // Control variate - no branch needed
        double control = sample * 0.5;
        controlSum += control;
    }

    return sum + controlSum;
}

double monteCarlo_optimized_both(size_t samples) {
    mt19937 gen(42);
    normal_distribution<double> dist(0.0, 1.0);

    double sum = 0.0;
    double controlSum = 0.0;

    for (size_t j = 0; j < samples; j++) {
        double sample = dist(gen);
        sum += sample * sample;
        // Both optimizations - no branches
        double control = sample * 0.5;
        controlSum += control;
        double antiSample = -sample;
        sum += antiSample * antiSample;
    }

    return sum + controlSum;
}

int main() {
    const size_t samples = 10000000;  // 10 million samples

    cout << "=== Monte Carlo Branch Elimination Benchmark ===" << endl;
    cout << "Samples: " << samples << endl << endl;

    // Test 1: No variance reduction
    cout << "Test 1: No variance reduction" << endl;
    auto start = high_resolution_clock::now();
    double result1 = monteCarlo_baseline(samples, false, false);
    auto end = high_resolution_clock::now();
    auto baseline_time = duration_cast<milliseconds>(end - start).count();

    start = high_resolution_clock::now();
    double result2 = monteCarlo_optimized_none(samples);
    end = high_resolution_clock::now();
    auto optimized_time = duration_cast<milliseconds>(end - start).count();

    cout << "  Baseline:  " << baseline_time << " ms" << endl;
    cout << "  Optimized: " << optimized_time << " ms" << endl;
    cout << "  Speedup:   " << (double)baseline_time / optimized_time << "x" << endl << endl;

    // Test 2: Antithetic variates only
    cout << "Test 2: Antithetic variates" << endl;
    start = high_resolution_clock::now();
    result1 = monteCarlo_baseline(samples, false, true);
    end = high_resolution_clock::now();
    baseline_time = duration_cast<milliseconds>(end - start).count();

    start = high_resolution_clock::now();
    result2 = monteCarlo_optimized_antithetic(samples);
    end = high_resolution_clock::now();
    optimized_time = duration_cast<milliseconds>(end - start).count();

    cout << "  Baseline:  " << baseline_time << " ms" << endl;
    cout << "  Optimized: " << optimized_time << " ms" << endl;
    cout << "  Speedup:   " << (double)baseline_time / optimized_time << "x" << endl << endl;

    // Test 3: Control variates only
    cout << "Test 3: Control variates" << endl;
    start = high_resolution_clock::now();
    result1 = monteCarlo_baseline(samples, true, false);
    end = high_resolution_clock::now();
    baseline_time = duration_cast<milliseconds>(end - start).count();

    start = high_resolution_clock::now();
    result2 = monteCarlo_optimized_control(samples);
    end = high_resolution_clock::now();
    optimized_time = duration_cast<milliseconds>(end - start).count();

    cout << "  Baseline:  " << baseline_time << " ms" << endl;
    cout << "  Optimized: " << optimized_time << " ms" << endl;
    cout << "  Speedup:   " << (double)baseline_time / optimized_time << "x" << endl << endl;

    // Test 4: Both variance reduction techniques
    cout << "Test 4: Both techniques" << endl;
    start = high_resolution_clock::now();
    result1 = monteCarlo_baseline(samples, true, true);
    end = high_resolution_clock::now();
    baseline_time = duration_cast<milliseconds>(end - start).count();

    start = high_resolution_clock::now();
    result2 = monteCarlo_optimized_both(samples);
    end = high_resolution_clock::now();
    optimized_time = duration_cast<milliseconds>(end - start).count();

    cout << "  Baseline:  " << baseline_time << " ms" << endl;
    cout << "  Optimized: " << optimized_time << " ms" << endl;
    cout << "  Speedup:   " << (double)baseline_time / optimized_time << "x" << endl << endl;

    return 0;
}
